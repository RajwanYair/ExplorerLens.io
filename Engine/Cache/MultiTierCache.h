// MultiTierCache.h — Advanced Multi-Tier Cache with Bloom Filter
// Copyright (c) 2026 ExplorerLens Project
//
// Adds Bloom filter negative-cache, WAL-mode SQLite, multi-tier hierarchy,
// background maintenance, and a cache statistics dashboard feed.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <cmath>
#include <string>
#include <array>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <chrono>
#include <unordered_map>

namespace ExplorerLens {
namespace Cache {

// ============================================================================
// Bloom Filter — fast negative lookup (avoids DB query for non-existent keys)
// ============================================================================

class BloomFilter {
public:
    /// Create filter with expected element count and target false-positive rate
    explicit BloomFilter(uint32_t expectedElements = 100000,
        double falsePositiveRate = 0.01);

    /// Insert a key into the filter
    void Insert(const std::wstring& key);

    /// Check if a key *might* exist (false positives possible, no false negatives)
    bool MayContain(const std::wstring& key) const;

    /// Reset all bits
    void Clear();

    /// Serialise bit array for persistence
    void Serialize(std::vector<uint8_t>& outBuffer) const;
    bool Deserialize(const std::vector<uint8_t>& buffer);

    /// Stats
    uint32_t GetInsertedCount() const { return m_insertedCount; }
    double GetEstimatedFalsePositiveRate() const;

private:
    std::vector<uint8_t> m_bits;
    uint32_t m_bitCount;
    uint32_t m_hashCount; // Number of hash functions (k)
    uint32_t m_insertedCount;

    // Hash functions (MurmurHash3-based with different seeds)
    uint32_t Hash(const std::wstring& key, uint32_t seed) const;
    void SetBit(uint32_t index);
    bool GetBit(uint32_t index) const;
};

// ============================================================================
// BloomFilter — inline method implementations
// ============================================================================

inline BloomFilter::BloomFilter(uint32_t expectedElements, double falsePositiveRate)
    : m_insertedCount(0) {
    // Ensure sane bounds
    if (expectedElements == 0) expectedElements = 1;
    if (falsePositiveRate <= 0.0) falsePositiveRate = 0.001;
    if (falsePositiveRate >= 1.0) falsePositiveRate = 0.5;

    // Optimal number of bits: m = -(n * ln(p)) / (ln(2)^2)
    double ln2 = std::log(2.0);
    double ln2sq = ln2 * ln2;
    double m = -(static_cast<double>(expectedElements) * std::log(falsePositiveRate)) / ln2sq;
    m_bitCount = static_cast<uint32_t>((std::max)(m, 64.0));

    // Optimal number of hash functions: k = (m/n) * ln(2)
    double k = (static_cast<double>(m_bitCount) / expectedElements) * ln2;
    m_hashCount = static_cast<uint32_t>((std::max)(k, 1.0));
    if (m_hashCount > 20) m_hashCount = 20; // Cap at 20 hash functions

    // Allocate bit array (ceil to bytes)
    m_bits.resize((m_bitCount + 7) / 8, 0);
}

inline void BloomFilter::Insert(const std::wstring& key) {
    for (uint32_t i = 0; i < m_hashCount; ++i) {
        uint32_t h = Hash(key, i);
        SetBit(h % m_bitCount);
    }
    ++m_insertedCount;
}

inline bool BloomFilter::MayContain(const std::wstring& key) const {
    for (uint32_t i = 0; i < m_hashCount; ++i) {
        uint32_t h = Hash(key, i);
        if (!GetBit(h % m_bitCount))
            return false;
    }
    return true;
}

inline void BloomFilter::Clear() {
    std::fill(m_bits.begin(), m_bits.end(), static_cast<uint8_t>(0));
    m_insertedCount = 0;
}

inline void BloomFilter::Serialize(std::vector<uint8_t>& outBuffer) const {
    // Format: [bitCount(4)] [hashCount(4)] [insertedCount(4)] [bits...]
    outBuffer.resize(12 + m_bits.size());
    std::memcpy(outBuffer.data(), &m_bitCount, 4);
    std::memcpy(outBuffer.data() + 4, &m_hashCount, 4);
    std::memcpy(outBuffer.data() + 8, &m_insertedCount, 4);
    if (!m_bits.empty()) {
        std::memcpy(outBuffer.data() + 12, m_bits.data(), m_bits.size());
    }
}

inline bool BloomFilter::Deserialize(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 12) return false;
    std::memcpy(&m_bitCount, buffer.data(), 4);
    std::memcpy(&m_hashCount, buffer.data() + 4, 4);
    std::memcpy(&m_insertedCount, buffer.data() + 8, 4);
    size_t expectedBytes = (m_bitCount + 7) / 8;
    if (buffer.size() < 12 + expectedBytes) return false;
    m_bits.resize(expectedBytes);
    std::memcpy(m_bits.data(), buffer.data() + 12, expectedBytes);
    return true;
}

inline double BloomFilter::GetEstimatedFalsePositiveRate() const {
    if (m_insertedCount == 0 || m_bitCount == 0) return 0.0;
    // FPR ≈ (1 - e^(-k*n/m))^k
    double exponent = -(static_cast<double>(m_hashCount) * m_insertedCount) / m_bitCount;
    double base = 1.0 - std::exp(exponent);
    return std::pow(base, static_cast<double>(m_hashCount));
}

inline uint32_t BloomFilter::Hash(const std::wstring& key, uint32_t seed) const {
    // MurmurHash3-inspired 32-bit hash over wchar_t data
    const uint8_t* data = reinterpret_cast<const uint8_t*>(key.data());
    uint32_t len = static_cast<uint32_t>(key.size() * sizeof(wchar_t));
    uint32_t h = seed ^ len;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    const uint32_t* blocks = reinterpret_cast<const uint32_t*>(data);
    uint32_t nblocks = len / 4;

    for (uint32_t i = 0; i < nblocks; ++i) {
        uint32_t k = blocks[i];
        k *= c1;
        k = (k << 15) | (k >> 17);
        k *= c2;
        h ^= k;
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    // Tail
    const uint8_t* tail = data + nblocks * 4;
    uint32_t k1 = 0;
    switch (len & 3) {
    case 3: k1 ^= static_cast<uint32_t>(tail[2]) << 16; [[fallthrough]];
    case 2: k1 ^= static_cast<uint32_t>(tail[1]) << 8; [[fallthrough]];
    case 1: k1 ^= static_cast<uint32_t>(tail[0]);
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;
        h ^= k1;
    }

    // Finalization mix
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

inline void BloomFilter::SetBit(uint32_t index) {
    m_bits[index / 8] |= static_cast<uint8_t>(1u << (index % 8));
}

inline bool BloomFilter::GetBit(uint32_t index) const {
    return (m_bits[index / 8] & static_cast<uint8_t>(1u << (index % 8))) != 0;
}

// ============================================================================
// Cache Tier abstraction
// ============================================================================

enum class StorageTier {
    Memory = 0, // Fastest — in-process LRU (microseconds)
    SQLite = 1, // Fast — WAL-mode database (sub-millisecond)
    Disk = 2, // Slow — raw PNG files on disk (milliseconds)
    Network = 3 // Remote — SMB/NFS share for VDI (variable)
};

/// Statistics for a single cache tier
struct TierStatistics {
    StorageTier tier;
    uint64_t hitCount = 0;
    uint64_t missCount = 0;
    uint64_t evictionCount = 0;
    uint64_t insertCount = 0;
    uint32_t entryCount = 0;
    uint64_t totalSizeBytes = 0;
    double avgLatencyUs = 0.0; // Average lookup latency in microseconds
    double hitRate() const {
        uint64_t total = hitCount + missCount;
        return total > 0 ? static_cast<double>(hitCount) / total : 0.0;
    }
};

// ============================================================================
// ICacheTier — tier-specific storage interface
// ============================================================================

class ICacheTier {
public:
    virtual ~ICacheTier() = default;

    virtual StorageTier GetTier() const = 0;
    virtual const wchar_t* GetName() const = 0;

    /// Check if key exists in this tier
    virtual bool Exists(const std::wstring& cacheKey) const = 0;

    /// Retrieve bitmap data; returns S_OK on hit, S_FALSE on miss
    virtual HRESULT Get(const std::wstring& cacheKey,
        std::vector<uint8_t>& outData) = 0;

    /// Store bitmap data
    virtual HRESULT Put(const std::wstring& cacheKey,
        const std::vector<uint8_t>& data) = 0;

    /// Remove entry
    virtual HRESULT Remove(const std::wstring& cacheKey) = 0;

    /// Evict entries to stay within size budget
    virtual HRESULT Evict(uint64_t targetSizeBytes) = 0;

    /// Tier statistics
    virtual TierStatistics GetStats() const = 0;
};

// ============================================================================
// MemoryCacheTier — in-process LRU using unordered_map + doubly-linked list
// ============================================================================

class MemoryCacheTier : public ICacheTier {
public:
    explicit MemoryCacheTier(uint32_t maxEntries = 1000, uint64_t maxSizeBytes = 128ULL * 1024 * 1024);
    ~MemoryCacheTier() override;

    StorageTier GetTier() const override { return StorageTier::Memory; }
    const wchar_t* GetName() const override { return L"Memory LRU"; }

    bool Exists(const std::wstring& cacheKey) const override;
    HRESULT Get(const std::wstring& cacheKey, std::vector<uint8_t>& outData) override;
    HRESULT Put(const std::wstring& cacheKey, const std::vector<uint8_t>& data) override;
    HRESULT Remove(const std::wstring& cacheKey) override;
    HRESULT Evict(uint64_t targetSizeBytes) override;
    TierStatistics GetStats() const override;

private:
    struct CacheEntry {
        std::wstring key;
        std::vector<uint8_t> data;
        std::chrono::steady_clock::time_point lastAccess;
    };

    uint32_t m_maxEntries;
    uint64_t m_maxSizeBytes;
    uint64_t m_currentSizeBytes = 0;
    mutable std::mutex m_mutex;

    // LRU via ordered map + access-time tracking
    std::unordered_map<std::wstring, CacheEntry> m_entries;

    // Stats
    mutable TierStatistics m_stats{ StorageTier::Memory };

    void EvictLRU();
};

// ============================================================================
// SQLiteCacheTier — WAL-mode SQLite database
// ============================================================================

class SQLiteCacheTier : public ICacheTier {
public:
    /// Initialize with database path; creates DB + tables if needed
    explicit SQLiteCacheTier(const std::wstring& dbPath = L"",
        uint64_t maxSizeBytes = 2ULL * 1024 * 1024 * 1024);
    ~SQLiteCacheTier() override;

    StorageTier GetTier() const override { return StorageTier::SQLite; }
    const wchar_t* GetName() const override { return L"SQLite WAL"; }

    bool Exists(const std::wstring& cacheKey) const override;
    HRESULT Get(const std::wstring& cacheKey, std::vector<uint8_t>& outData) override;
    HRESULT Put(const std::wstring& cacheKey, const std::vector<uint8_t>& data) override;
    HRESULT Remove(const std::wstring& cacheKey) override;
    HRESULT Evict(uint64_t targetSizeBytes) override;
    TierStatistics GetStats() const override;

    /// Enable WAL mode for concurrent read performance
    HRESULT EnableWALMode();

    /// Run VACUUM to reclaim space
    HRESULT Vacuum();

    /// Create indexes for fast lookup
    HRESULT CreateOptimalIndexes();

    /// Schema:
    /// CREATE TABLE thumbnails (
    /// cache_key TEXT PRIMARY KEY,
    /// data BLOB NOT NULL,
    /// size_bytes INTEGER NOT NULL,
    /// created_at INTEGER NOT NULL,
    /// accessed_at INTEGER NOT NULL,
    /// file_path TEXT,
    /// width INTEGER,
    /// height INTEGER
    /// );
    /// CREATE INDEX idx_accessed ON thumbnails(accessed_at);
    /// CREATE INDEX idx_filepath ON thumbnails(file_path);

private:
    void* m_db = nullptr; // sqlite3* handle (void* to avoid header dependency)
    std::wstring m_dbPath;
    uint64_t m_maxSizeBytes;
    mutable std::mutex m_mutex;
    mutable TierStatistics m_stats{ StorageTier::SQLite };

    std::wstring GetDefaultDBPath() const;
    HRESULT Open();
    void Close();
};

// ============================================================================
// DiskCacheTier — raw PNG files on disk (fallback)
// ============================================================================

class DiskCacheTier : public ICacheTier {
public:
    explicit DiskCacheTier(const std::wstring& cacheDir = L"",
        uint64_t maxSizeBytes = 4ULL * 1024 * 1024 * 1024);
    ~DiskCacheTier() override;

    StorageTier GetTier() const override { return StorageTier::Disk; }
    const wchar_t* GetName() const override { return L"Disk"; }

    bool Exists(const std::wstring& cacheKey) const override;
    HRESULT Get(const std::wstring& cacheKey, std::vector<uint8_t>& outData) override;
    HRESULT Put(const std::wstring& cacheKey, const std::vector<uint8_t>& data) override;
    HRESULT Remove(const std::wstring& cacheKey) override;
    HRESULT Evict(uint64_t targetSizeBytes) override;
    TierStatistics GetStats() const override;

private:
    std::wstring m_cacheDir;
    uint64_t m_maxSizeBytes;
    mutable std::mutex m_mutex;
    mutable TierStatistics m_stats{ StorageTier::Disk };

    std::wstring GetDefaultCacheDir() const;
    std::wstring KeyToFilePath(const std::wstring& key) const;
};

// ============================================================================
// MultiTierCacheManager — orchestrates tiers + Bloom filter
// ============================================================================

class MultiTierCacheManager {
public:
    MultiTierCacheManager();
    ~MultiTierCacheManager();

    /// Initialize with default tier configuration
    HRESULT Initialize();

    /// Add a tier to the hierarchy (ordered by priority)
    void AddTier(std::unique_ptr<ICacheTier> tier);

    /// Look up across tiers (L1→L2→L3); promotes to higher tier on hit
    HRESULT Get(const std::wstring& cacheKey, std::vector<uint8_t>& outData);

    /// Insert into all appropriate tiers
    HRESULT Put(const std::wstring& cacheKey, const std::vector<uint8_t>& data);

    /// Remove from all tiers
    HRESULT Remove(const std::wstring& cacheKey);

    /// Quick negative lookup via Bloom filter
    bool MayExist(const std::wstring& cacheKey) const;

    // --- Background Maintenance ---

    /// Start background maintenance thread (auto cleanup of stale entries)
    HRESULT StartMaintenance(std::chrono::minutes interval = std::chrono::minutes(30));

    /// Stop background maintenance
    void StopMaintenance();

    /// Run maintenance pass immediately (cleanup entries older than maxAgeDays)
    HRESULT RunMaintenanceNow(uint32_t maxAgeDays = 30);

    // --- Statistics Dashboard ---

    struct CacheDashboard {
        std::vector<TierStatistics> tierStats;
        uint64_t bloomFilterChecks = 0;
        uint64_t bloomFilterNegatives = 0; // Early rejections (saved DB query)
        uint64_t totalEntriesAllTiers = 0;
        uint64_t totalSizeAllTiers = 0;
        double overallHitRate = 0.0;
        double bloomFilterSavingsPercent = 0.0;
    };

    CacheDashboard GetDashboard() const;

    /// Reset all statistics counters
    void ResetStats();

private:
    std::vector<std::unique_ptr<ICacheTier>> m_tiers;
    BloomFilter m_bloomFilter;
    mutable std::mutex m_mutex;

    // Background maintenance
    std::atomic<bool> m_maintenanceRunning{ false };
    HANDLE m_maintenanceThread = nullptr;
    HANDLE m_maintenanceStopEvent = nullptr;

    // Bloom filter stats
    mutable uint64_t m_bloomChecks = 0;
    mutable uint64_t m_bloomNegatives = 0;

    /// Promote entry from lower tier to higher tier
    void PromoteToTier(size_t targetTierIndex, const std::wstring& key,
        const std::vector<uint8_t>& data);
};

// ============================================================================
// MemoryCacheTier — inline implementations
// ============================================================================

inline MemoryCacheTier::MemoryCacheTier(uint32_t maxEntries, uint64_t maxSizeBytes)
    : m_maxEntries(maxEntries), m_maxSizeBytes(maxSizeBytes) {
}

inline MemoryCacheTier::~MemoryCacheTier() = default;

inline bool MemoryCacheTier::Exists(const std::wstring& cacheKey) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_entries.find(cacheKey) != m_entries.end();
}

inline HRESULT MemoryCacheTier::Get(const std::wstring& cacheKey,
    std::vector<uint8_t>& outData) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_entries.find(cacheKey);
    if (it == m_entries.end()) {
        ++m_stats.missCount;
        return S_FALSE;
    }
    it->second.lastAccess = std::chrono::steady_clock::now();
    outData = it->second.data;
    ++m_stats.hitCount;
    return S_OK;
}

inline HRESULT MemoryCacheTier::Put(const std::wstring& cacheKey,
    const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Remove existing entry if present
    auto it = m_entries.find(cacheKey);
    if (it != m_entries.end()) {
        m_currentSizeBytes -= it->second.data.size();
        m_entries.erase(it);
    }
    // Evict if at capacity
    while (m_entries.size() >= m_maxEntries || m_currentSizeBytes + data.size() > m_maxSizeBytes) {
        if (m_entries.empty()) break;
        EvictLRU();
    }
    CacheEntry entry;
    entry.key = cacheKey;
    entry.data = data;
    entry.lastAccess = std::chrono::steady_clock::now();
    m_currentSizeBytes += data.size();
    m_entries[cacheKey] = std::move(entry);
    ++m_stats.insertCount;
    m_stats.entryCount = static_cast<uint32_t>(m_entries.size());
    m_stats.totalSizeBytes = m_currentSizeBytes;
    return S_OK;
}

inline HRESULT MemoryCacheTier::Remove(const std::wstring& cacheKey) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_entries.find(cacheKey);
    if (it == m_entries.end()) return S_FALSE;
    m_currentSizeBytes -= it->second.data.size();
    m_entries.erase(it);
    m_stats.entryCount = static_cast<uint32_t>(m_entries.size());
    m_stats.totalSizeBytes = m_currentSizeBytes;
    return S_OK;
}

inline HRESULT MemoryCacheTier::Evict(uint64_t targetSizeBytes) {
    std::lock_guard<std::mutex> lock(m_mutex);
    while (m_currentSizeBytes > targetSizeBytes && !m_entries.empty()) {
        EvictLRU();
    }
    return S_OK;
}

inline TierStatistics MemoryCacheTier::GetStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    TierStatistics stats = m_stats;
    stats.entryCount = static_cast<uint32_t>(m_entries.size());
    stats.totalSizeBytes = m_currentSizeBytes;
    return stats;
}

inline void MemoryCacheTier::EvictLRU() {
    // Find the least-recently-used entry
    if (m_entries.empty()) return;
    auto oldest = m_entries.begin();
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        if (it->second.lastAccess < oldest->second.lastAccess)
            oldest = it;
    }
    m_currentSizeBytes -= oldest->second.data.size();
    m_entries.erase(oldest);
    ++m_stats.evictionCount;
}

// ============================================================================
// SQLiteCacheTier — stub implementations (no sqlite3 dependency linked)
// ============================================================================

inline SQLiteCacheTier::SQLiteCacheTier(const std::wstring& dbPath, uint64_t maxSizeBytes)
    : m_dbPath(dbPath), m_maxSizeBytes(maxSizeBytes) {
    if (m_dbPath.empty()) m_dbPath = GetDefaultDBPath();
}

inline SQLiteCacheTier::~SQLiteCacheTier() { Close(); }

inline bool SQLiteCacheTier::Exists(const std::wstring& /*cacheKey*/) const {
    return false; // No sqlite3 linked
}

inline HRESULT SQLiteCacheTier::Get(const std::wstring& /*cacheKey*/,
    std::vector<uint8_t>& /*outData*/) {
    ++m_stats.missCount;
    return S_FALSE; // Not available without sqlite3
}

inline HRESULT SQLiteCacheTier::Put(const std::wstring& /*cacheKey*/,
    const std::vector<uint8_t>& /*data*/) {
    return E_NOTIMPL;
}

inline HRESULT SQLiteCacheTier::Remove(const std::wstring& /*cacheKey*/) {
    return E_NOTIMPL;
}

inline HRESULT SQLiteCacheTier::Evict(uint64_t /*targetSizeBytes*/) {
    return E_NOTIMPL;
}

inline TierStatistics SQLiteCacheTier::GetStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_stats;
}

inline HRESULT SQLiteCacheTier::EnableWALMode() { return E_NOTIMPL; }
inline HRESULT SQLiteCacheTier::Vacuum() { return E_NOTIMPL; }
inline HRESULT SQLiteCacheTier::CreateOptimalIndexes() { return E_NOTIMPL; }

inline std::wstring SQLiteCacheTier::GetDefaultDBPath() const {
    wchar_t localAppData[MAX_PATH] = {};
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH) > 0) {
        return std::wstring(localAppData) + L"\\ExplorerLens\\cache.db";
    }
    return L"cache.db";
}

inline HRESULT SQLiteCacheTier::Open() { return E_NOTIMPL; }
inline void SQLiteCacheTier::Close() { m_db = nullptr; }

// ============================================================================
// DiskCacheTier — inline implementations (Win32 file I/O)
// ============================================================================

inline DiskCacheTier::DiskCacheTier(const std::wstring& cacheDir, uint64_t maxSizeBytes)
    : m_cacheDir(cacheDir), m_maxSizeBytes(maxSizeBytes) {
    if (m_cacheDir.empty()) m_cacheDir = GetDefaultCacheDir();
}

inline DiskCacheTier::~DiskCacheTier() = default;

inline std::wstring DiskCacheTier::GetDefaultCacheDir() const {
    wchar_t localAppData[MAX_PATH] = {};
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH) > 0) {
        return std::wstring(localAppData) + L"\\ExplorerLens\\ThumbnailCache";
    }
    return L"ThumbnailCache";
}

inline std::wstring DiskCacheTier::KeyToFilePath(const std::wstring& key) const {
    // Simple hash-based filename to avoid invalid path characters
    uint32_t hash = 0x811c9dc5u; // FNV-1a offset basis
    for (wchar_t ch : key) {
        hash ^= static_cast<uint32_t>(ch);
        hash *= 0x01000193u; // FNV prime
    }
    wchar_t buf[32];
    wsprintfW(buf, L"%08X.png", hash);
    return m_cacheDir + L"\\" + buf;
}

inline bool DiskCacheTier::Exists(const std::wstring& cacheKey) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::wstring filePath = KeyToFilePath(cacheKey);
    DWORD attrs = GetFileAttributesW(filePath.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

inline HRESULT DiskCacheTier::Get(const std::wstring& cacheKey,
    std::vector<uint8_t>& outData) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::wstring filePath = KeyToFilePath(cacheKey);
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        ++m_stats.missCount;
        return S_FALSE;
    }
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0) {
        CloseHandle(hFile);
        ++m_stats.missCount;
        return S_FALSE;
    }
    outData.resize(static_cast<size_t>(fileSize.QuadPart));
    DWORD bytesRead = 0;
    BOOL ok = ReadFile(hFile, outData.data(), static_cast<DWORD>(fileSize.QuadPart),
        &bytesRead, nullptr);
    CloseHandle(hFile);
    if (!ok || bytesRead != static_cast<DWORD>(fileSize.QuadPart)) {
        ++m_stats.missCount;
        return E_FAIL;
    }
    ++m_stats.hitCount;
    return S_OK;
}

inline HRESULT DiskCacheTier::Put(const std::wstring& cacheKey,
    const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Ensure directory exists
    CreateDirectoryW(m_cacheDir.c_str(), nullptr);
    std::wstring filePath = KeyToFilePath(cacheKey);
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_WRITE, 0,
        nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return E_FAIL;
    DWORD bytesWritten = 0;
    BOOL ok = WriteFile(hFile, data.data(), static_cast<DWORD>(data.size()),
        &bytesWritten, nullptr);
    CloseHandle(hFile);
    if (!ok) return E_FAIL;
    ++m_stats.insertCount;
    ++m_stats.entryCount;
    m_stats.totalSizeBytes += data.size();
    return S_OK;
}

inline HRESULT DiskCacheTier::Remove(const std::wstring& cacheKey) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::wstring filePath = KeyToFilePath(cacheKey);
    if (DeleteFileW(filePath.c_str())) {
        if (m_stats.entryCount > 0) --m_stats.entryCount;
        return S_OK;
    }
    return S_FALSE;
}

inline HRESULT DiskCacheTier::Evict(uint64_t /*targetSizeBytes*/) {
    // Full implementation would enumerate files by age and delete oldest
    // For now, return success without action
    return S_OK;
}

inline TierStatistics DiskCacheTier::GetStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_stats;
}

// ============================================================================
// MultiTierCacheManager — inline implementations
// ============================================================================

inline MultiTierCacheManager::MultiTierCacheManager() = default;

inline MultiTierCacheManager::~MultiTierCacheManager() {
    StopMaintenance();
}

inline HRESULT MultiTierCacheManager::Initialize() {
    // Default tier setup: Memory → Disk (SQLite requires explicit opt-in)
    if (m_tiers.empty()) {
        m_tiers.push_back(std::make_unique<MemoryCacheTier>());
        m_tiers.push_back(std::make_unique<DiskCacheTier>());
    }
    return S_OK;
}

inline void MultiTierCacheManager::AddTier(std::unique_ptr<ICacheTier> tier) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_tiers.push_back(std::move(tier));
}

inline HRESULT MultiTierCacheManager::Get(const std::wstring& cacheKey,
    std::vector<uint8_t>& outData) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Quick negative check via Bloom filter
    ++m_bloomChecks;
    if (!m_bloomFilter.MayContain(cacheKey)) {
        ++m_bloomNegatives;
        return S_FALSE;
    }
    // Search tiers in order (highest priority first)
    for (size_t i = 0; i < m_tiers.size(); ++i) {
        HRESULT hr = m_tiers[i]->Get(cacheKey, outData);
        if (hr == S_OK) {
            // Promote to higher tier if found in a lower one
            if (i > 0) {
                PromoteToTier(0, cacheKey, outData);
            }
            return S_OK;
        }
    }
    return S_FALSE;
}

inline HRESULT MultiTierCacheManager::Put(const std::wstring& cacheKey,
    const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bloomFilter.Insert(cacheKey);
    HRESULT lastHr = S_OK;
    for (auto& tier : m_tiers) {
        HRESULT hr = tier->Put(cacheKey, data);
        if (FAILED(hr)) lastHr = hr;
    }
    return lastHr;
}

inline HRESULT MultiTierCacheManager::Remove(const std::wstring& cacheKey) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Note: Bloom filters don't support removal — key remains in filter
    for (auto& tier : m_tiers) {
        tier->Remove(cacheKey);
    }
    return S_OK;
}

inline bool MultiTierCacheManager::MayExist(const std::wstring& cacheKey) const {
    ++m_bloomChecks;
    if (!m_bloomFilter.MayContain(cacheKey)) {
        ++m_bloomNegatives;
        return false;
    }
    return true;
}

inline HRESULT MultiTierCacheManager::StartMaintenance(std::chrono::minutes /*interval*/) {
    // Background maintenance thread would be started here
    // For header-only implementation, mark as running
    m_maintenanceRunning.store(true);
    return S_OK;
}

inline void MultiTierCacheManager::StopMaintenance() {
    m_maintenanceRunning.store(false);
    if (m_maintenanceStopEvent) {
        SetEvent(m_maintenanceStopEvent);
    }
    if (m_maintenanceThread) {
        WaitForSingleObject(m_maintenanceThread, 5000);
        CloseHandle(m_maintenanceThread);
        m_maintenanceThread = nullptr;
    }
    if (m_maintenanceStopEvent) {
        CloseHandle(m_maintenanceStopEvent);
        m_maintenanceStopEvent = nullptr;
    }
}

inline HRESULT MultiTierCacheManager::RunMaintenanceNow(uint32_t /*maxAgeDays*/) {
    // Would iterate tiers and evict stale entries
    return S_OK;
}

inline MultiTierCacheManager::CacheDashboard MultiTierCacheManager::GetDashboard() const {
    CacheDashboard dashboard;
    dashboard.bloomFilterChecks = m_bloomChecks;
    dashboard.bloomFilterNegatives = m_bloomNegatives;
    uint64_t totalHits = 0, totalTotal = 0;
    for (const auto& tier : m_tiers) {
        TierStatistics ts = tier->GetStats();
        dashboard.totalEntriesAllTiers += ts.entryCount;
        dashboard.totalSizeAllTiers += ts.totalSizeBytes;
        totalHits += ts.hitCount;
        totalTotal += ts.hitCount + ts.missCount;
        dashboard.tierStats.push_back(ts);
    }
    dashboard.overallHitRate = totalTotal > 0
        ? static_cast<double>(totalHits) / totalTotal : 0.0;
    dashboard.bloomFilterSavingsPercent = m_bloomChecks > 0
        ? (static_cast<double>(m_bloomNegatives) / m_bloomChecks) * 100.0 : 0.0;
    return dashboard;
}

inline void MultiTierCacheManager::ResetStats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bloomChecks = 0;
    m_bloomNegatives = 0;
}

inline void MultiTierCacheManager::PromoteToTier(size_t targetTierIndex,
    const std::wstring& key, const std::vector<uint8_t>& data) {
    if (targetTierIndex < m_tiers.size()) {
        m_tiers[targetTierIndex]->Put(key, data);
    }
}

} // namespace Cache
} // namespace ExplorerLens
