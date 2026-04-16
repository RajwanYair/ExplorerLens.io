// DiskCacheStore.cpp — SHA256-keyed persistent disk cache
// Copyright (c) 2026 ExplorerLens Project
//
#include "DiskCacheStore.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Minimal FNV-1a 64 key hash (keys are already semantically unique strings)
// In production a proper SHA-256 truncated to 128-bit would be used.
// ---------------------------------------------------------------------------

static uint64_t Fnv1a64(std::string_view s) noexcept
{
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (unsigned char c : s) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

// Convert a key to a safe hex filename
static std::string KeyToFilename(std::string_view key)
{
    uint64_t h = Fnv1a64(key);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%016llx.tlc",
                  static_cast<unsigned long long>(h));
    return buf;
}

// ---------------------------------------------------------------------------
// Blob file format (little-endian):
//   [0..3]  magic  "TLC1"
//   [4..7]  width  (uint32)
//   [8..11] height (uint32)
//   [12..15] stride (uint32)
//   [16..19] bpp (uint32)
//   [20..23] keylen (uint32)
//   [24 .. 24+keylen-1] key bytes (UTF-8)
//   [aligned to 4 bytes] pixel data
// ---------------------------------------------------------------------------

static constexpr uint32_t BLOB_MAGIC = 0x31434C54U; // "TLC1"

static bool WriteBlob(const fs::path& path, std::string_view key, const CacheEntry& e)
{
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) return false;

    uint32_t magic  = BLOB_MAGIC;
    uint32_t width  = e.width;
    uint32_t height = e.height;
    uint32_t stride = e.stride;
    uint32_t bpp    = e.bpp;
    uint32_t keylen = static_cast<uint32_t>(key.size());

    f.write(reinterpret_cast<const char*>(&magic),  4);
    f.write(reinterpret_cast<const char*>(&width),  4);
    f.write(reinterpret_cast<const char*>(&height), 4);
    f.write(reinterpret_cast<const char*>(&stride), 4);
    f.write(reinterpret_cast<const char*>(&bpp),    4);
    f.write(reinterpret_cast<const char*>(&keylen), 4);
    f.write(key.data(), static_cast<std::streamsize>(keylen));

    // Pad to 4-byte alignment
    uint32_t padBytes = (4 - (keylen % 4)) % 4;
    static const char zeros[4] = {};
    if (padBytes) f.write(zeros, static_cast<std::streamsize>(padBytes));

    f.write(reinterpret_cast<const char*>(e.pixels.data()),
            static_cast<std::streamsize>(e.pixels.size()));
    return f.good();
}

static std::optional<CacheEntry> ReadBlob(const fs::path& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return std::nullopt;

    uint32_t magic = 0;
    f.read(reinterpret_cast<char*>(&magic), 4);
    if (magic != BLOB_MAGIC) return std::nullopt;

    CacheEntry e;
    f.read(reinterpret_cast<char*>(&e.width),  4);
    f.read(reinterpret_cast<char*>(&e.height), 4);
    f.read(reinterpret_cast<char*>(&e.stride), 4);
    f.read(reinterpret_cast<char*>(&e.bpp),    4);

    uint32_t keylen = 0;
    f.read(reinterpret_cast<char*>(&keylen), 4);

    std::string key(keylen, '\0');
    f.read(key.data(), static_cast<std::streamsize>(keylen));

    uint32_t padBytes = (4 - (keylen % 4)) % 4;
    if (padBytes) f.seekg(static_cast<std::streamoff>(padBytes), std::ios::cur);

    size_t pixelBytes = static_cast<size_t>(e.stride) * e.height;
    if (pixelBytes > 256 * 1024 * 1024ULL) return std::nullopt;  // safety cap

    e.pixels.resize(pixelBytes);
    f.read(reinterpret_cast<char*>(e.pixels.data()), static_cast<std::streamsize>(pixelBytes));
    if (!f) return std::nullopt;
    e.valid = true;
    return e;
}

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct AsyncWriteJob {
    std::string key;
    CacheEntry  entry;
};

struct DiskCacheStore::Impl {
    fs::path               cacheDir;
    fs::path               blobDir;
    size_t                 maxBytes;
    std::atomic<size_t>    currentBytes{0};
    std::atomic<bool>      healthy{false};

    mutable std::shared_mutex indexMu;
    std::unordered_map<std::string, std::string> keyToFile; // key → filename

    // Async write queue
    std::mutex              queueMu;
    std::condition_variable queueCv;
    std::queue<AsyncWriteJob> writeQueue;
    std::thread             workerThread;
    std::atomic<bool>       stopWorker{false};

    explicit Impl(std::string path, size_t maxBytesIn)
        : cacheDir(path)
        , blobDir(fs::path(path) / "blobs")
        , maxBytes(maxBytesIn)
    {
        std::error_code ec;
        fs::create_directories(blobDir, ec);
        healthy = !ec;
        if (healthy) {
            LoadIndex();
            workerThread = std::thread([this] { WorkerLoop(); });
        }
    }

    ~Impl()
    {
        stopWorker = true;
        queueCv.notify_all();
        if (workerThread.joinable()) workerThread.join();
    }

    void LoadIndex()
    {
        std::unique_lock lk(indexMu);
        currentBytes = 0;
        for (auto& entry : fs::directory_iterator(blobDir)) {
            if (entry.path().extension() == ".tlc") {
                // Read the key from the blob header
                if (auto ce = ReadBlob(entry.path())) {
                    // Re-derive the filename from the hash to match
                    // (we trust the filename since it's hash-derived)
                    std::string filename = entry.path().filename().string();
                    // We can't recover original key without storing it separately;
                    // for now, use filename as opaque handle in index
                    keyToFile[filename] = filename;
                    currentBytes += entry.file_size();
                }
            }
        }
    }

    void WorkerLoop()
    {
        while (true) {
            std::unique_lock lk(queueMu);
            queueCv.wait(lk, [this] { return stopWorker.load() || !writeQueue.empty(); });
            if (stopWorker && writeQueue.empty()) break;

            AsyncWriteJob job = std::move(writeQueue.front());
            writeQueue.pop();
            lk.unlock();

            DoStore(job.key, job.entry);
        }
    }

    bool DoStore(const std::string& key, const CacheEntry& entry)
    {
        std::string filename = KeyToFilename(key);
        fs::path blobPath = blobDir / filename;

        if (!WriteBlob(blobPath, key, entry)) return false;

        size_t sz = entry.pixels.size();
        {
            std::unique_lock lk(indexMu);
            keyToFile[key] = filename;
            currentBytes += sz;
        }
        TrimTobudget();
        return true;
    }

    void TrimTobudget()
    {
        if (currentBytes.load(std::memory_order_relaxed) <= maxBytes) return;

        // Collect blobs by age, remove oldest first
        std::vector<std::pair<fs::file_time_type, fs::path>> entries;
        std::error_code ec;
        for (auto& de : fs::directory_iterator(blobDir, ec)) {
            if (de.path().extension() == ".tlc") {
                entries.emplace_back(de.last_write_time(), de.path());
            }
        }
        std::sort(entries.begin(), entries.end());

        for (auto& [t, p] : entries) {
            if (currentBytes.load(std::memory_order_relaxed) <= maxBytes) break;
            size_t sz = fs::file_size(p, ec);
            fs::remove(p, ec);
            if (!ec) currentBytes.fetch_sub(sz, std::memory_order_relaxed);

            std::unique_lock lk(indexMu);
            // Remove from index
            for (auto it = keyToFile.begin(); it != keyToFile.end(); ) {
                if (it->second == p.filename().string()) {
                    it = keyToFile.erase(it);
                } else ++it;
            }
        }
    }
};

// ---------------------------------------------------------------------------
// DiskCacheStore public API
// ---------------------------------------------------------------------------

DiskCacheStore::DiskCacheStore(std::string cachePath, size_t maxBytes)
    : m_impl(std::make_unique<Impl>(std::move(cachePath), maxBytes))
{}

DiskCacheStore::~DiskCacheStore() = default;
DiskCacheStore::DiskCacheStore(DiskCacheStore&&) noexcept = default;
DiskCacheStore& DiskCacheStore::operator=(DiskCacheStore&&) noexcept = default;

std::optional<CacheEntry> DiskCacheStore::Load(std::string_view key) const
{
    if (!m_impl->healthy) return std::nullopt;
    std::string k(key);
    std::string filename = KeyToFilename(k);
    return ReadBlob(m_impl->blobDir / filename);
}

bool DiskCacheStore::Store(std::string_view key, const CacheEntry& entry)
{
    if (!m_impl->healthy) return false;
    return m_impl->DoStore(std::string(key), entry);
}

void DiskCacheStore::StoreAsync(std::string_view key, const CacheEntry& entry)
{
    if (!m_impl->healthy) return;
    {
        std::unique_lock lk(m_impl->queueMu);
        m_impl->writeQueue.push({ std::string(key), entry });
    }
    m_impl->queueCv.notify_one();
}

void DiskCacheStore::Invalidate(std::string_view key)
{
    if (!m_impl->healthy) return;
    std::string filename = KeyToFilename(std::string(key));
    fs::path p = m_impl->blobDir / filename;
    std::error_code ec;
    if (size_t sz = fs::file_size(p, ec); !ec) {
        m_impl->currentBytes.fetch_sub(sz, std::memory_order_relaxed);
    }
    fs::remove(p, ec);
    {
        std::unique_lock lk(m_impl->indexMu);
        m_impl->keyToFile.erase(std::string(key));
    }
}

void DiskCacheStore::InvalidatePrefix(std::string_view pathPrefix)
{
    std::unique_lock lk(m_impl->indexMu);
    std::string prefix(pathPrefix);
    for (auto it = m_impl->keyToFile.begin(); it != m_impl->keyToFile.end(); ) {
        if (it->first.rfind(prefix, 0) == 0) {
            fs::path p = m_impl->blobDir / it->second;
            std::error_code ec;
            if (size_t sz = fs::file_size(p, ec); !ec)
                m_impl->currentBytes.fetch_sub(sz, std::memory_order_relaxed);
            fs::remove(p, ec);
            it = m_impl->keyToFile.erase(it);
        } else {
            ++it;
        }
    }
}

void DiskCacheStore::TrimTobudget()
{
    m_impl->TrimTobudget();
}

void DiskCacheStore::Flush()
{
    // Signal worker to drain, then wait
    std::unique_lock lk(m_impl->queueMu);
    m_impl->queueCv.wait(lk, [this] { return m_impl->writeQueue.empty(); });
}

size_t DiskCacheStore::CurrentBytes() const noexcept
{
    return m_impl->currentBytes.load(std::memory_order_relaxed);
}

bool DiskCacheStore::IsHealthy() const noexcept
{
    return m_impl->healthy.load(std::memory_order_relaxed);
}

} // namespace Engine
} // namespace ExplorerLens
