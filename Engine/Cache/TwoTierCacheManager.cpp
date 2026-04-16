// TwoTierCacheManager.cpp — L1 + L2 cache coordination
// Copyright (c) 2026 ExplorerLens Project
//
#include "TwoTierCacheManager.h"
#include "DiskCacheStore.h"

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <list>
#include <unordered_map>
#include <cstring>
#include <cassert>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <shlobj.h>   // SHGetKnownFolderPath, FOLDERID_LocalAppData
#endif

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Minimal in-process LRU for L1 (no external dependency on SubMsCache here)
// ---------------------------------------------------------------------------

namespace {

struct LruNode {
    std::string  key;
    CacheEntry   entry;
};

class L1Lru {
public:
    explicit L1Lru(size_t maxBytes) : m_maxBytes(maxBytes), m_currentBytes(0) {}

    std::optional<CacheEntry> Get(const std::string& key)
    {
        std::unique_lock lk(m_mu);
        auto it = m_map.find(key);
        if (it == m_map.end()) return std::nullopt;
        // Move to front (MRU)
        m_list.splice(m_list.begin(), m_list, it->second);
        return it->second->entry;
    }

    void Put(const std::string& key, CacheEntry entry)
    {
        std::unique_lock lk(m_mu);
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            m_currentBytes -= it->second->entry.SizeBytes();
            m_list.erase(it->second);
            m_map.erase(it);
        }
        size_t sz = entry.SizeBytes();
        m_list.push_front({ key, std::move(entry) });
        m_map[key] = m_list.begin();
        m_currentBytes += sz;
        Evict();
    }

    void Invalidate(const std::string& key)
    {
        std::unique_lock lk(m_mu);
        auto it = m_map.find(key);
        if (it == m_map.end()) return;
        m_currentBytes -= it->second->entry.SizeBytes();
        m_list.erase(it->second);
        m_map.erase(it);
    }

    void InvalidatePrefix(const std::string& prefix)
    {
        std::unique_lock lk(m_mu);
        for (auto it = m_list.begin(); it != m_list.end(); ) {
            if (it->key.rfind(prefix, 0) == 0) {
                m_currentBytes -= it->entry.SizeBytes();
                m_map.erase(it->key);
                it = m_list.erase(it);
            } else {
                ++it;
            }
        }
    }

    size_t CurrentBytes() const noexcept { return m_currentBytes.load(std::memory_order_relaxed); }

    size_t EntryCount() const noexcept {
        std::shared_lock lk(m_mu);
        return m_map.size();
    }

    void TrimTobudget()
    {
        std::unique_lock lk(m_mu);
        Evict();
    }

private:
    void Evict()  // must hold unique lock
    {
        while (m_currentBytes.load(std::memory_order_relaxed) > m_maxBytes && !m_list.empty()) {
            auto& back = m_list.back();
            m_currentBytes -= back.entry.SizeBytes();
            m_map.erase(back.key);
            m_list.pop_back();
        }
    }

    mutable std::shared_mutex                            m_mu;
    std::list<LruNode>                                   m_list;
    std::unordered_map<std::string, std::list<LruNode>::iterator> m_map;
    size_t                                               m_maxBytes;
    std::atomic<size_t>                                  m_currentBytes;
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct TwoTierCacheManager::Impl {
    TwoTierCacheConfig         config;
    L1Lru                      l1;
    std::unique_ptr<DiskCacheStore> l2;

    std::atomic<uint64_t>      l1Hits{0};
    std::atomic<uint64_t>      l2Hits{0};
    std::atomic<uint64_t>      misses{0};
    std::atomic<uint64_t>      inserts{0};

    explicit Impl(TwoTierCacheConfig cfg)
        : config(std::move(cfg))
        , l1(config.l1MaxBytes)
    {
        if (config.enableL2) {
            std::string path = config.l2CachePath;
            if (path.empty()) {
#ifdef _WIN32
                PWSTR wpath = nullptr;
                if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &wpath))) {
                    int n = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, nullptr, 0, nullptr, nullptr);
                    std::string u8(static_cast<size_t>(n), '\0');
                    WideCharToMultiByte(CP_UTF8, 0, wpath, -1, u8.data(), n, nullptr, nullptr);
                    CoTaskMemFree(wpath);
                    path = u8 + "\\ExplorerLens\\Cache";
                } else {
                    path = "C:\\ProgramData\\ExplorerLens\\Cache";
                }
#else
                path = "/tmp/explorerlens-cache";
#endif
            }
            l2 = std::make_unique<DiskCacheStore>(path, config.l2MaxBytes);
        }
    }
};

// ---------------------------------------------------------------------------
// TwoTierCacheManager public API
// ---------------------------------------------------------------------------

TwoTierCacheManager::TwoTierCacheManager(TwoTierCacheConfig config)
    : m_impl(std::make_unique<Impl>(std::move(config)))
{}

TwoTierCacheManager::~TwoTierCacheManager() = default;

TwoTierCacheManager::TwoTierCacheManager(TwoTierCacheManager&&) noexcept = default;
TwoTierCacheManager& TwoTierCacheManager::operator=(TwoTierCacheManager&&) noexcept = default;

std::pair<CacheEntry, CacheTier> TwoTierCacheManager::Lookup(std::string_view key) const
{
    std::string k(key);

    // L1 lookup
    if (auto hit = m_impl->l1.Get(k)) {
        ++m_impl->l1Hits;
        return { std::move(*hit), CacheTier::L1 };
    }

    // L2 lookup
    if (m_impl->l2) {
        if (auto hit = m_impl->l2->Load(k)) {
            ++m_impl->l2Hits;
            // Promote to L1
            m_impl->l1.Put(k, *hit);
            return { std::move(*hit), CacheTier::L2 };
        }
    }

    ++m_impl->misses;
    return { {}, CacheTier::Miss };
}

void TwoTierCacheManager::Insert(std::string_view key, CacheEntry entry)
{
    std::string k(key);
    ++m_impl->inserts;
    m_impl->l1.Put(k, entry);
    if (m_impl->l2) {
        if (m_impl->config.writeThrough) {
            m_impl->l2->Store(k, entry);
        } else {
            m_impl->l2->StoreAsync(k, entry);
        }
    }
}

void TwoTierCacheManager::Invalidate(std::string_view key)
{
    std::string k(key);
    m_impl->l1.Invalidate(k);
    if (m_impl->l2) m_impl->l2->Invalidate(k);
}

void TwoTierCacheManager::InvalidatePrefix(std::string_view pathPrefix)
{
    std::string p(pathPrefix);
    m_impl->l1.InvalidatePrefix(p);
    if (m_impl->l2) m_impl->l2->InvalidatePrefix(p);
}

void TwoTierCacheManager::TrimTobudget()
{
    m_impl->l1.TrimTobudget();
    if (m_impl->l2) m_impl->l2->TrimTobudget();
}

void TwoTierCacheManager::FlushL2()
{
    if (m_impl->l2) m_impl->l2->Flush();
}

size_t TwoTierCacheManager::L1CurrentBytes() const noexcept
{
    return m_impl->l1.CurrentBytes();
}

size_t TwoTierCacheManager::L2CurrentBytes() const noexcept
{
    return m_impl->l2 ? m_impl->l2->CurrentBytes() : 0;
}

bool TwoTierCacheManager::IsL2Available() const noexcept
{
    return m_impl->l2 && m_impl->l2->IsHealthy();
}

std::string TwoTierCacheManager::StatsJson() const
{
    uint64_t l1h = m_impl->l1Hits.load();
    uint64_t l2h = m_impl->l2Hits.load();
    uint64_t ms  = m_impl->misses.load();
    uint64_t ins = m_impl->inserts.load();
    uint64_t total = l1h + l2h + ms;
    double hr = (total > 0) ? (static_cast<double>(l1h + l2h) / static_cast<double>(total) * 100.0) : 0.0;

    std::ostringstream ss;
    ss << "{"
       << "\"l1_hits\":" << l1h << ","
       << "\"l2_hits\":" << l2h << ","
       << "\"misses\":"  << ms  << ","
       << "\"inserts\":" << ins << ","
       << "\"hit_rate_pct\":" << hr << ","
       << "\"l1_bytes\":" << L1CurrentBytes() << ","
       << "\"l2_bytes\":" << L2CurrentBytes()
       << "}";
    return ss.str();
}

} // namespace Engine
} // namespace ExplorerLens
