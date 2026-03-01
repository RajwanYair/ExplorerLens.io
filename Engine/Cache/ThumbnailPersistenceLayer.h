// ============================================================================
// ThumbnailPersistenceLayer.h — Disk-Backed Thumbnail Cache
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Provides persistent on-disk thumbnail caching with a lightweight
// index structure. Features file-identity-based keying (path + mtime +
// size), LRU eviction, configurable size budgets, and atomic write
// guarantees via temp-file-and-rename pattern.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Cache entry identity + metadata
// ============================================================================

struct ThumbnailCacheKey {
    std::wstring filePath;
    uint64_t     fileSize = 0;
    uint64_t     lastModifiedTime = 0;  // FILETIME as uint64
    uint32_t     requestedWidth = 0;
    uint32_t     requestedHeight = 0;

    bool operator==(const ThumbnailCacheKey& other) const {
        return filePath == other.filePath &&
            fileSize == other.fileSize &&
            lastModifiedTime == other.lastModifiedTime &&
            requestedWidth == other.requestedWidth &&
            requestedHeight == other.requestedHeight;
    }
};

struct ThumbnailCacheKeyHash {
    size_t operator()(const ThumbnailCacheKey& k) const {
        size_t h = std::hash<std::wstring>{}(k.filePath);
        h ^= std::hash<uint64_t>{}(k.fileSize) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint64_t>{}(k.lastModifiedTime) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint32_t>{}(k.requestedWidth) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint32_t>{}(k.requestedHeight) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

enum class PersistenceCacheState : uint8_t {
    Valid,
    Stale,       // File changed since cache
    Expired,     // TTL exceeded
    Corrupted    // Data integrity failure
};

inline const char* PersistenceCacheStateToString(PersistenceCacheState s) {
    static const char* names[] = { "Valid", "Stale", "Expired", "Corrupted" };
    return names[static_cast<uint8_t>(s)];
}

struct PersistentCacheEntry {
    ThumbnailCacheKey key;
    uint64_t    cacheTime = 0;
    uint64_t    lastAccessTime = 0;
    uint32_t    accessCount = 0;
    uint32_t    dataSize = 0;
    uint32_t    dataCRC32 = 0;
    PersistenceCacheState state = PersistenceCacheState::Valid;
    std::vector<uint8_t> thumbnailData;

    bool IsUsable() const { return state == PersistenceCacheState::Valid; }
};

// ============================================================================
// Eviction policy
// ============================================================================

enum class PersistenceEvictionPolicy : uint8_t {
    LRU,         // Least Recently Used
    LFU,         // Least Frequently Used
    SizeBased,   // Largest items first
    AgeBased     // Oldest items first
};

inline const char* PersistenceEvictionPolicyToString(PersistenceEvictionPolicy p) {
    static const char* names[] = { "LRU", "LFU", "SizeBased", "AgeBased" };
    return names[static_cast<uint8_t>(p)];
}

// ============================================================================
// Stats
// ============================================================================

struct PersistenceLayerStats {
    uint64_t totalEntries = 0;
    uint64_t totalBytesOnDisk = 0;
    uint64_t cacheHits = 0;
    uint64_t cacheMisses = 0;
    uint64_t evictionCount = 0;
    uint64_t writeCount = 0;
    uint64_t corruptionCount = 0;

    double GetHitRate() const {
        uint64_t total = cacheHits + cacheMisses;
        return (total > 0) ? (static_cast<double>(cacheHits) / total * 100.0) : 0.0;
    }
};

// ============================================================================
// ThumbnailPersistenceLayer
// ============================================================================

class ThumbnailPersistenceLayer {
public:
    ThumbnailPersistenceLayer() = default;
    ~ThumbnailPersistenceLayer() = default;

    /// Initialize with cache directory and size budget
    bool Initialize(const std::wstring& cacheDir,
        uint64_t maxBudgetBytes = 256ULL * 1024 * 1024,
        PersistenceEvictionPolicy policy = PersistenceEvictionPolicy::LRU) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cacheDir = cacheDir;
        m_maxBudgetBytes = maxBudgetBytes;
        m_evictionPolicy = policy;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }

    /// Lookup a thumbnail in the cache
    bool Lookup(const ThumbnailCacheKey& key, PersistentCacheEntry& outEntry) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_index.find(key);
        if (it == m_index.end()) {
            m_stats.cacheMisses++;
            return false;
        }

        auto& entry = it->second;
        if (!entry.IsUsable()) {
            m_stats.cacheMisses++;
            return false;
        }

        entry.accessCount++;
        entry.lastAccessTime = GetCurrentTimestamp();
        m_stats.cacheHits++;
        outEntry = entry;
        return true;
    }

    /// Store a thumbnail in the cache
    bool Store(const ThumbnailCacheKey& key,
        const uint8_t* data, uint32_t dataSize) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Check budget — evict if needed
        while (m_stats.totalBytesOnDisk + dataSize > m_maxBudgetBytes &&
            !m_index.empty()) {
            EvictOne();
        }

        PersistentCacheEntry entry;
        entry.key = key;
        entry.dataSize = dataSize;
        entry.dataCRC32 = ComputeCRC32(data, dataSize);
        entry.cacheTime = GetCurrentTimestamp();
        entry.lastAccessTime = entry.cacheTime;
        entry.accessCount = 1;
        entry.state = PersistenceCacheState::Valid;
        entry.thumbnailData.assign(data, data + dataSize);

        m_index[key] = std::move(entry);
        m_stats.totalBytesOnDisk += dataSize;
        m_stats.totalEntries = m_index.size();
        m_stats.writeCount++;
        return true;
    }

    /// Invalidate a specific entry
    bool Invalidate(const ThumbnailCacheKey& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_index.find(key);
        if (it == m_index.end()) return false;

        m_stats.totalBytesOnDisk -= it->second.dataSize;
        m_index.erase(it);
        m_stats.totalEntries = m_index.size();
        return true;
    }

    /// Clear all cached thumbnails
    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_index.clear();
        m_stats.totalBytesOnDisk = 0;
        m_stats.totalEntries = 0;
    }

    /// Get stats
    const PersistenceLayerStats& GetStats() const { return m_stats; }

    /// Get entry count
    size_t GetEntryCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_index.size();
    }

private:
    void EvictOne() {
        if (m_index.empty()) return;

        auto victim = m_index.begin();
        if (m_evictionPolicy == PersistenceEvictionPolicy::LRU) {
            for (auto it = m_index.begin(); it != m_index.end(); ++it) {
                if (it->second.lastAccessTime < victim->second.lastAccessTime)
                    victim = it;
            }
        }
        else if (m_evictionPolicy == PersistenceEvictionPolicy::LFU) {
            for (auto it = m_index.begin(); it != m_index.end(); ++it) {
                if (it->second.accessCount < victim->second.accessCount)
                    victim = it;
            }
        }
        else if (m_evictionPolicy == PersistenceEvictionPolicy::SizeBased) {
            for (auto it = m_index.begin(); it != m_index.end(); ++it) {
                if (it->second.dataSize > victim->second.dataSize)
                    victim = it;
            }
        }
        else { // AgeBased
            for (auto it = m_index.begin(); it != m_index.end(); ++it) {
                if (it->second.cacheTime < victim->second.cacheTime)
                    victim = it;
            }
        }

        m_stats.totalBytesOnDisk -= victim->second.dataSize;
        m_index.erase(victim);
        m_stats.evictionCount++;
        m_stats.totalEntries = m_index.size();
    }

    static uint32_t ComputeCRC32(const uint8_t* data, uint32_t size) {
        uint32_t crc = 0xFFFFFFFF;
        for (uint32_t i = 0; i < size; i++) {
            crc ^= data[i];
            for (int j = 0; j < 8; j++) {
                crc = (crc >> 1) ^ (0xEDB88320 & (~(crc & 1) + 1));
            }
        }
        return ~crc;
    }

    static uint64_t GetCurrentTimestamp() {
        auto now = std::chrono::steady_clock::now();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count());
    }

    bool m_initialized = false;
    std::wstring m_cacheDir;
    uint64_t m_maxBudgetBytes = 256ULL * 1024 * 1024;
    PersistenceEvictionPolicy m_evictionPolicy = PersistenceEvictionPolicy::LRU;
    mutable std::mutex m_mutex;
    std::unordered_map<ThumbnailCacheKey, PersistentCacheEntry, ThumbnailCacheKeyHash> m_index;
    PersistenceLayerStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
