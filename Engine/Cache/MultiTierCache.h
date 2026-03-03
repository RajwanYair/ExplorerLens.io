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

} // namespace Cache
} // namespace ExplorerLens
