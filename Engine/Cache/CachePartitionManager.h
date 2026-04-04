// CachePartitionManager.h — Multi-Tenant Cache Segmentation
// Copyright (c) 2026 ExplorerLens Project
//
// Partitions the cache into isolated segments by file type, source folder,
// or decoder, enabling independent eviction policies and size limits.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct CachePartition
{
    std::string name;
    uint64_t maxBytes = 64ULL * 1024 * 1024;
    uint64_t usedBytes = 0;
    uint32_t entryCount = 0;
    uint32_t hitCount = 0;
    uint32_t missCount = 0;
    double hitRate() const
    {
        uint32_t total = hitCount + missCount;
        return total > 0 ? 100.0 * hitCount / total : 0.0;
    }
};

struct PartitionConfig
{
    uint64_t totalBudgetBytes = 512ULL * 1024 * 1024;
    uint32_t maxPartitions = 16;
    bool autoBalance = true;
    double rebalanceThreshold = 0.2;  // Rebalance when utilization differs by 20%
};

class CachePartitionManager
{
  public:
    void Configure(const PartitionConfig& config)
    {
        m_config = config;
    }

    bool CreatePartition(const std::string& name, uint64_t maxBytes)
    {
        std::lock_guard lock(m_mutex);
        if (m_partitions.size() >= m_config.maxPartitions)
            return false;
        if (m_partitions.count(name))
            return false;
        CachePartition p;
        p.name = name;
        p.maxBytes = maxBytes;
        m_partitions[name] = p;
        return true;
    }

    bool CanAllocate(const std::string& partition, uint64_t bytes) const
    {
        std::lock_guard lock(m_mutex);
        auto it = m_partitions.find(partition);
        if (it == m_partitions.end())
            return false;
        return it->second.usedBytes + bytes <= it->second.maxBytes;
    }

    void RecordAllocation(const std::string& partition, uint64_t bytes)
    {
        std::lock_guard lock(m_mutex);
        auto it = m_partitions.find(partition);
        if (it != m_partitions.end()) {
            it->second.usedBytes += bytes;
            it->second.entryCount++;
        }
    }

    void RecordHit(const std::string& partition)
    {
        std::lock_guard lock(m_mutex);
        auto it = m_partitions.find(partition);
        if (it != m_partitions.end())
            it->second.hitCount++;
    }

    void RecordMiss(const std::string& partition)
    {
        std::lock_guard lock(m_mutex);
        auto it = m_partitions.find(partition);
        if (it != m_partitions.end())
            it->second.missCount++;
    }

    uint64_t TotalUsed() const
    {
        std::lock_guard lock(m_mutex);
        uint64_t total = 0;
        for (const auto& [k, v] : m_partitions)
            total += v.usedBytes;
        return total;
    }

    size_t PartitionCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_partitions.size();
    }

  private:
    mutable std::mutex m_mutex;
    PartitionConfig m_config;
    std::unordered_map<std::string, CachePartition> m_partitions;
};

}  // namespace Engine
}  // namespace ExplorerLens
