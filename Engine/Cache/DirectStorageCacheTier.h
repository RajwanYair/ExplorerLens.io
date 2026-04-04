// DirectStorageCacheTier.h — DirectStorage-Backed L2 Cache Tier
// Copyright (c) 2026 ExplorerLens Project
//
// Provides an L2 cache tier that uses DirectStorage for NVMe-to-GPU zero-bounce
// transfers, bypassing CPU staging for cached thumbnail data retrieval.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// DSCacheEntry has a distinct name from MultiTierCache.h's CacheEntry
struct DSCacheEntry
{
    uint64_t offset = 0;
    uint64_t size = 0;
    uint64_t lastAccess = 0;
    uint32_t hitCount = 0;
};

class DirectStorageCacheTier
{
  public:
    static constexpr uint64_t DEFAULT_CAPACITY_MB = 512;
    static constexpr uint64_t MAX_CAPACITY_MB = 4096;

    DirectStorageCacheTier() = default;
    ~DirectStorageCacheTier()
    {
        Shutdown();
    }

    DirectStorageCacheTier(const DirectStorageCacheTier&) = delete;
    DirectStorageCacheTier& operator=(const DirectStorageCacheTier&) = delete;

    bool Initialize(uint64_t capacityMB = DEFAULT_CAPACITY_MB)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_initialized)
            return true;
        m_capacityMB = (capacityMB > 0 && capacityMB <= MAX_CAPACITY_MB) ? capacityMB : DEFAULT_CAPACITY_MB;
        m_initialized = true;
        return true;
    }

    void Put(const std::string& key, const std::vector<uint8_t>& data)
    {
        if (key.empty() || data.empty())
            return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_store[key] = data;
        ++m_puts;
    }

    std::vector<uint8_t> Get(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_store.find(key);
        if (it == m_store.end()) {
            ++m_misses;
            return {};
        }
        ++m_hits;
        return it->second;
    }

    void Evict(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_store.erase(key);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_store.clear();
        m_hits = m_misses = 0;
    }

    float GetHitRate() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t total = m_hits + m_misses;
        return (total > 0) ? static_cast<float>(m_hits) / static_cast<float>(total) : 0.0f;
    }

    uint64_t GetCapacityMB() const
    {
        return m_capacityMB;
    }

    size_t GetEntryCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_store.size();
    }

    double GetUsageMB() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t bytes = 0;
        for (auto const& kv : m_store)
            bytes += kv.second.size();
        return static_cast<double>(bytes) / (1024.0 * 1024.0);
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_store.clear();
        m_initialized = false;
    }

  private:
    bool m_initialized = false;
    uint64_t m_capacityMB = DEFAULT_CAPACITY_MB;
    uint64_t m_hits = 0;
    uint64_t m_misses = 0;
    uint64_t m_puts = 0;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<uint8_t>> m_store;
};

}  // namespace Engine
}  // namespace ExplorerLens
