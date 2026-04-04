// BitmapMemoryRecycler.h — Bitmap buffer reuse pool
// Copyright (c) 2026 ExplorerLens Project
//
// Recycles bitmap buffers to avoid repeated allocation/deallocation,
// maintaining a pool of pre-sized buffers for common thumbnail dimensions.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct BitmapMemoryRecyclerConfig
{
    bool enabled = true;
    uint32_t maxPoolSize = 32;
    uint32_t maxBufferSizeMB = 16;
    std::string label = "BitmapMemoryRecycler";
};

class BitmapMemoryRecycler
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    BitmapMemoryRecyclerConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct PoolEntry
    {
        uint64_t sizeBytes = 0;
        bool inUse = false;
    };

    bool HasAvailable(uint64_t minSize) const
    {
        for (const auto& e : m_pool)
            if (!e.inUse && e.sizeBytes >= minSize)
                return true;
        return false;
    }

    bool Acquire(uint64_t size)
    {
        for (auto& e : m_pool) {
            if (!e.inUse && e.sizeBytes >= size) {
                e.inUse = true;
                m_acquireCount++;
                return true;
            }
        }
        if (m_pool.size() < m_config.maxPoolSize) {
            m_pool.push_back({size, true});
            m_allocCount++;
            return true;
        }
        return false;
    }

    void Release(uint64_t size)
    {
        for (auto& e : m_pool) {
            if (e.inUse && e.sizeBytes == size) {
                e.inUse = false;
                m_releaseCount++;
                return;
            }
        }
    }

    uint32_t GetPoolSize() const
    {
        return static_cast<uint32_t>(m_pool.size());
    }
    uint64_t GetRecycleRate() const
    {
        return m_acquireCount > 0 ? m_acquireCount - m_allocCount : 0;
    }

  private:
    bool m_initialized = false;
    BitmapMemoryRecyclerConfig m_config;
    std::vector<PoolEntry> m_pool;
    uint64_t m_acquireCount = 0;
    uint64_t m_allocCount = 0;
    uint64_t m_releaseCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
