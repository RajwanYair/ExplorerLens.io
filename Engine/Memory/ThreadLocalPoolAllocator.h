// ThreadLocalPoolAllocator.h — Per-Thread Pool Allocator
// Copyright (c) 2026 ExplorerLens Project
//
// Provides lock-free allocation for decode threads by maintaining separate
// memory pools per thread, eliminating contention on hot allocation paths.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct TLPoolStats
{
    uint32_t threadId = 0;
    uint64_t allocations = 0;
    uint64_t deallocations = 0;
    uint64_t bytesAllocated = 0;
    uint64_t bytesFreed = 0;
    uint64_t peakUsedBytes = 0;
    uint32_t poolExpansions = 0;
};

struct TLPoolConfig
{
    uint64_t initialPoolSize = 256 * 1024;      // 256KB per thread
    uint64_t maxPoolSize = 4ULL * 1024 * 1024;  // 4MB max per thread
    uint64_t growthIncrement = 256 * 1024;
    uint32_t maxThreads = 16;
};

class ThreadLocalPoolAllocator
{
  public:
    void Configure(const TLPoolConfig& config)
    {
        m_config = config;
    }

    struct Pool
    {
        std::vector<uint8_t> memory;
        size_t offset = 0;
        TLPoolStats stats;

        void* Allocate(size_t bytes, size_t alignment = 16)
        {
            size_t aligned = (offset + alignment - 1) & ~(alignment - 1);
            if (aligned + bytes > memory.size())
                return nullptr;
            void* ptr = memory.data() + aligned;
            offset = aligned + bytes;
            stats.allocations++;
            stats.bytesAllocated += bytes;
            if (offset > stats.peakUsedBytes)
                stats.peakUsedBytes = offset;
            return ptr;
        }

        void Reset()
        {
            offset = 0;
        }

        bool Grow(size_t increment)
        {
            size_t newSize = memory.size() + increment;
            memory.resize(newSize);
            stats.poolExpansions++;
            return true;
        }
    };

    Pool CreatePool(uint32_t threadId) const
    {
        Pool p;
        p.memory.resize(m_config.initialPoolSize);
        p.stats.threadId = threadId;
        return p;
    }

    bool CanGrow(const Pool& pool) const
    {
        return pool.memory.size() + m_config.growthIncrement <= m_config.maxPoolSize;
    }

    size_t GrowthIncrement() const
    {
        return m_config.growthIncrement;
    }

    TLPoolConfig GetConfig() const
    {
        return m_config;
    }

  private:
    TLPoolConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
