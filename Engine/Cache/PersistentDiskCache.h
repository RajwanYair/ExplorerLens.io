#pragma once
//==============================================================================
// PersistentDiskCache — Sprint 192
// Persistent disk cache with warming, smart eviction, and integrity validation
//
// Architecture:
//   1. SQLite-backed metadata index (file path → cache entry)
//   2. Binary blob storage for thumbnail pixel data
//   3. LRU + decode-cost-aware eviction (expensive formats kept longer)
//   4. Cache warming on Explorer folder open
//   5. CRC32 integrity validation on read
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace DarkThumbs { namespace Engine {

/// Cache eviction strategy
enum class EvictionStrategy : uint8_t {
    LRU,              ///< Least Recently Used (default)
    LFU,              ///< Least Frequently Used
    CostAware,        ///< Keep expensive-to-decode entries longer
    SizeAware,        ///< Evict largest entries first
    Hybrid            ///< Combined LRU + cost-weighted
};

/// Cache entry state
enum class CacheEntryState : uint8_t {
    Valid,         ///< Entry is valid and usable
    Stale,         ///< File modified since cache was written
    Corrupted,     ///< CRC32 mismatch
    Expired,       ///< TTL has expired
    Warming        ///< Being pre-generated
};

/// Single cache entry metadata
struct CacheEntry {
    std::wstring filePath;             ///< Original file path
    std::wstring cacheKey;             ///< Hash-based cache key
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t dataSize = 0;             ///< Compressed data size on disk
    uint32_t crc32 = 0;               ///< Integrity checksum
    uint64_t fileModTime = 0;          ///< Last modification time of source
    uint64_t cacheWriteTime = 0;       ///< When cache entry was written
    uint64_t lastAccessTime = 0;       ///< Last read time
    uint32_t accessCount = 0;          ///< Read count for LFU
    double decodeCostMs = 0.0;         ///< How long the original decode took
    CacheEntryState state = CacheEntryState::Valid;
    std::wstring formatName;           ///< Source format for cost analysis
};

/// Cache warming request
struct WarmingRequest {
    std::wstring directoryPath;
    uint32_t maxFiles = 100;           ///< Max files to warm
    uint32_t thumbnailSize = 256;
    bool recursive = false;
    bool prioritizeVisible = true;     ///< Warm visible items first
};

/// Cache statistics
struct DiskCacheStats {
    uint64_t totalEntries = 0;
    uint64_t validEntries = 0;
    uint64_t staleEntries = 0;
    uint64_t corruptedEntries = 0;
    uint64_t totalHits = 0;
    uint64_t totalMisses = 0;
    double hitRatePercent = 0.0;
    uint64_t diskUsageBytes = 0;
    uint64_t maxDiskBytes = 0;
    uint64_t evictedEntries = 0;
    double avgHitTimeMs = 0.0;
    double avgMissTimeMs = 0.0;
    uint32_t warmingProgress = 0;      ///< % of warming complete
};

/// Disk cache configuration
struct DiskCacheConfig {
    std::wstring cacheDirPath;               ///< Cache storage directory
    uint64_t maxDiskSizeMB = 512;            ///< Max disk usage in MB
    uint32_t maxEntries = 50000;             ///< Max cached entries
    uint32_t entryTTLHours = 168;            ///< Time-to-live (1 week)
    EvictionStrategy evictionStrategy = EvictionStrategy::Hybrid;
    bool enableIntegrityCheck = true;        ///< CRC32 on read
    bool enableCompression = true;           ///< Compress stored data
    bool enableWarming = true;               ///< Enable cache warming
    uint32_t warmingBatchSize = 20;          ///< Files per warming batch
    uint32_t warmingIntervalMs = 1000;       ///< Delay between batches
    double costWeightFactor = 1.5;           ///< Weight for decode cost in eviction
};

//==============================================================================
// PersistentDiskCache
//==============================================================================
class PersistentDiskCache {
public:
    PersistentDiskCache();
    explicit PersistentDiskCache(const DiskCacheConfig& config);
    ~PersistentDiskCache();

    PersistentDiskCache(const PersistentDiskCache&) = delete;
    PersistentDiskCache& operator=(const PersistentDiskCache&) = delete;

    /// Open or create the cache database
    bool Open();

    /// Close the cache and flush pending writes
    void Close();

    /// Store a thumbnail in the cache
    bool Put(const std::wstring& filePath,
             uint32_t width, uint32_t height,
             const uint8_t* data, uint32_t dataSize,
             double decodeCostMs, const std::wstring& formatName);

    /// Retrieve a cached thumbnail
    bool Get(const std::wstring& filePath,
             uint32_t& width, uint32_t& height,
             std::vector<uint8_t>& data);

    /// Check if a valid cache entry exists
    bool Contains(const std::wstring& filePath) const;

    /// Remove a specific entry
    bool Remove(const std::wstring& filePath);

    /// Invalidate stale entries (file modified since cache)
    uint32_t InvalidateStale();

    /// Run eviction to stay within disk budget
    uint32_t RunEviction();

    /// Start cache warming for a directory
    bool StartWarming(const WarmingRequest& request);

    /// Stop cache warming
    void StopWarming();

    /// Compact the cache database (SQLite VACUUM)
    bool Compact();

    /// Get cache statistics
    DiskCacheStats GetStats() const;

    /// Check if cache is open
    bool IsOpen() const;

    /// Get configuration
    const DiskCacheConfig& GetConfig() const { return m_config; }

    /// Static helpers
    static const wchar_t* GetEvictionName(EvictionStrategy strategy);
    static const wchar_t* GetEntryStateName(CacheEntryState state);

    /// Generate CRC32 checksum
    static uint32_t ComputeCRC32(const uint8_t* data, uint32_t size);

    /// Generate cache key from file path + size
    static std::wstring GenerateCacheKey(const std::wstring& filePath,
                                          uint32_t thumbnailSize);

private:
    /// Evict entries based on strategy
    uint32_t EvictLRU(uint32_t count);
    uint32_t EvictCostAware(uint32_t count);

    /// Calculate eviction score (higher = more likely to evict)
    double CalculateEvictionScore(const CacheEntry& entry) const;

    DiskCacheConfig m_config;
    bool m_isOpen = false;
    mutable std::mutex m_cacheMutex;
    std::unordered_map<std::wstring, CacheEntry> m_index;

    // Statistics
    uint64_t m_totalHits = 0;
    uint64_t m_totalMisses = 0;
    uint64_t m_evictedEntries = 0;
    double m_totalHitTimeMs = 0.0;
    double m_totalMissTimeMs = 0.0;
};

}} // namespace DarkThumbs::Engine
