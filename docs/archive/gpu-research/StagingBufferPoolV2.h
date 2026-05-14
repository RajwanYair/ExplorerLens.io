// StagingBufferPoolV2.h — Pooled Staging Buffers v2 for DirectStorage Upload
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a pool of reusable staging buffers for DirectStorage GPU upload paths,
// minimizing allocation churn during high-throughput thumbnail decode workloads.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct StagingBuffer
{
    void* data = nullptr;
    uint64_t capacity = 0;  // usable bytes
};

// StagingPoolStats has a distinct name from ProcessPoolManager.h's PoolStats
struct StagingPoolStats
{
    uint32_t totalBuffers = 0;
    uint32_t activeBuffers = 0;
    uint64_t totalAllocatedBytes = 0;
};

class StagingBufferPoolV2
{
  public:
    static constexpr uint32_t DEFAULT_POOL_SIZE = 16;
    static constexpr uint64_t DEFAULT_BUFFER_SIZE = 4ULL * 1024 * 1024;
    static constexpr uint32_t MAX_POOL_SIZE = 256;

    StagingBufferPoolV2() = default;

    ~StagingBufferPoolV2()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& s : m_pool)
            delete[] static_cast<uint8_t*>(s.raw);
        m_pool.clear();
    }

    StagingBufferPoolV2(const StagingBufferPoolV2&) = delete;
    StagingBufferPoolV2& operator=(const StagingBufferPoolV2&) = delete;

    bool Initialize(uint32_t poolSize = DEFAULT_POOL_SIZE, uint64_t bufferSize = DEFAULT_BUFFER_SIZE)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_initialized)
            return true;
        uint32_t count = std::min(poolSize > 0 ? poolSize : DEFAULT_POOL_SIZE, MAX_POOL_SIZE);
        m_bufferSize = (bufferSize > 0) ? bufferSize : DEFAULT_BUFFER_SIZE;
        m_pool.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            Slot s;
            s.raw = new uint8_t[m_bufferSize];
            s.inUse = false;
            m_pool.push_back(s);
        }
        m_stats.totalBuffers = count;
        m_stats.totalAllocatedBytes = static_cast<uint64_t>(count) * m_bufferSize;
        m_initialized = true;
        return true;
    }

    StagingBuffer AcquireBuffer(uint64_t /*sizeHint*/ = 0)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized)
            return {};
        for (auto& s : m_pool) {
            if (!s.inUse) {
                s.inUse = true;
                ++m_stats.activeBuffers;
                StagingBuffer b{};
                b.data = s.raw;
                b.capacity = m_bufferSize;
                return b;
            }
        }
        // Pool exhausted — allocate extra
        Slot s;
        s.raw = new uint8_t[m_bufferSize];
        s.inUse = true;
        m_pool.push_back(s);
        ++m_stats.totalBuffers;
        m_stats.totalAllocatedBytes += m_bufferSize;
        ++m_stats.activeBuffers;
        StagingBuffer b{};
        b.data = s.raw;
        b.capacity = m_bufferSize;
        return b;
    }

    void ReleaseBuffer(StagingBuffer& buf)
    {
        if (!buf.data)
            return;
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& s : m_pool) {
            if (s.raw == buf.data && s.inUse) {
                s.inUse = false;
                if (m_stats.activeBuffers > 0)
                    --m_stats.activeBuffers;
                break;
            }
        }
        buf.data = nullptr;
        buf.capacity = 0;
    }

    void Trim()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pool.begin();
        while (it != m_pool.end() && m_pool.size() > 4) {
            if (!it->inUse) {
                m_stats.totalAllocatedBytes -= m_bufferSize;
                if (m_stats.totalBuffers > 0)
                    --m_stats.totalBuffers;
                delete[] static_cast<uint8_t*>(it->raw);
                it = m_pool.erase(it);
            } else {
                ++it;
            }
        }
    }

    uint32_t GetPoolSize() const
    {
        return m_stats.totalBuffers;
    }
    uint32_t GetActiveCount() const
    {
        return m_stats.activeBuffers;
    }
    uint64_t GetTotalMemoryBytes() const
    {
        return m_stats.totalAllocatedBytes;
    }
    uint64_t GetMaxBufferSize() const
    {
        return m_bufferSize;
    }

    StagingPoolStats GetStats() const
    {
        return m_stats;
    }

  private:
    struct Slot
    {
        void* raw = nullptr;
        bool inUse = false;
    };

    bool m_initialized = false;
    uint64_t m_bufferSize = DEFAULT_BUFFER_SIZE;
    StagingPoolStats m_stats{};
    std::vector<Slot> m_pool;
    mutable std::mutex m_mutex;
};

}  // namespace Engine
}  // namespace ExplorerLens
