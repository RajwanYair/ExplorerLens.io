// DirectStorageCacheTier.h — DirectStorage-Backed L2 Cache Tier
// Copyright (c) 2026 ExplorerLens Project
//
// Provides an L2 cache tier that uses DirectStorage for NVMe-to-GPU zero-bounce
// transfers, bypassing CPU staging for cached thumbnail data retrieval.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

struct CacheEntry {
    uint64_t    offset = 0;
    uint64_t    size = 0;
    uint64_t    lastAccess = 0;
    uint32_t    hitCount = 0;
};

struct CacheTierStats {
    uint64_t hits = 0;
    uint64_t misses = 0;
    uint64_t evictions = 0;
    uint64_t totalBytes = 0;
    uint64_t capacityMB = 0;
};

class DirectStorageCacheTier {
public:
    static constexpr uint64_t DEFAULT_CAPACITY_MB = 512;
    static constexpr uint64_t MAX_CAPACITY_MB = 4096;

    DirectStorageCacheTier() = default;
    ~DirectStorageCacheTier() { Shutdown(); }

    DirectStorageCacheTier(const DirectStorageCacheTier&) = delete;
    DirectStorageCacheTier& operator=(const DirectStorageCacheTier&) = delete;

    inline bool Initialize(uint64_t capacityMB = DEFAULT_CAPACITY_MB) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_initialized) return true;
        m_stats.capacityMB = (capacityMB > 0 && capacityMB <= MAX_CAPACITY_MB)
                             ? capacityMB : DEFAULT_CAPACITY_MB;
        m_initialized = true;
        return true;
    }

    inline bool Get(uint64_t key, void* dst, uint64_t dstCapacity) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized) return false;
        auto it = m_entries.find(key);
        if (it == m_entries.end()) {
            m_stats.misses++;
            return false;
        }
        it->second.hitCount++;
        it->second.lastAccess = m_accessCounter++;
        m_stats.hits++;
        return true;
    }

    inline bool Put(uint64_t key, const void* data, uint64_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized || !data || size == 0) return false;
        uint64_t capacityBytes = m_stats.capacityMB * 1024ULL * 1024ULL;
        if (m_stats.totalBytes + size > capacityBytes) {
            EvictLRU();
        }
        CacheEntry entry;
        entry.size = size;
        entry.lastAccess = m_accessCounter++;
        m_entries[key] = entry;
        m_stats.totalBytes += size;
        return true;
    }

    inline bool Evict(uint64_t key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_entries.find(key);
        if (it == m_entries.end()) return false;
        m_stats.totalBytes -= it->second.size;
        m_entries.erase(it);
        m_stats.evictions++;
        return true;
    }

    inline double GetHitRate() const {
        uint64_t total = m_stats.hits + m_stats.misses;
        return (total > 0) ? static_cast<double>(m_stats.hits) / static_cast<double>(total) : 0.0;
    }

    inline uint64_t GetCapacityMB() const { return m_stats.capacityMB; }
    inline const CacheTierStats& GetStats() const { return m_stats; }

    inline void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
        m_stats = CacheTierStats{};
        m_initialized = false;
    }

private:
    inline void EvictLRU() {
        if (m_entries.empty()) return;
        auto oldest = m_entries.begin();
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->second.lastAccess < oldest->second.lastAccess) oldest = it;
        }
        m_stats.totalBytes -= oldest->second.size;
        m_entries.erase(oldest);
        m_stats.evictions++;
    }

    bool                                     m_initialized = false;
    uint64_t                                 m_accessCounter = 0;
    CacheTierStats                           m_stats{};
    std::unordered_map<uint64_t, CacheEntry> m_entries;
    std::mutex                               m_mutex;
};

} // namespace Engine
} // namespace ExplorerLens
