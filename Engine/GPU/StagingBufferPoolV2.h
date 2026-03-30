// StagingBufferPoolV2.h — Pooled Staging Buffers v2 for DirectStorage Upload
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a pool of reusable staging buffers for DirectStorage GPU upload paths,
// minimizing allocation churn during high-throughput thumbnail decode workloads.
//
#pragma once

#include <cstdint>
#include <vector>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

struct StagingBuffer {
    uint64_t id = 0;
    void*    data = nullptr;
    uint64_t size = 0;
    bool     inUse = false;
};

struct PoolStats {
    uint32_t totalBuffers = 0;
    uint32_t activeBuffers = 0;
    uint32_t peakActive = 0;
    uint64_t totalAllocatedBytes = 0;
    uint64_t totalAcquires = 0;
    uint64_t totalReleases = 0;
};

class StagingBufferPoolV2 {
public:
    static constexpr uint32_t DEFAULT_POOL_SIZE = 16;
    static constexpr uint64_t DEFAULT_BUFFER_SIZE = 4ULL * 1024 * 1024;
    static constexpr uint32_t MAX_POOL_SIZE = 256;

    StagingBufferPoolV2() = default;

    ~StagingBufferPoolV2() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& buf : m_pool) {
            delete[] static_cast<uint8_t*>(buf.data);
            buf.data = nullptr;
        }
        m_pool.clear();
    }

    StagingBufferPoolV2(const StagingBufferPoolV2&) = delete;
    StagingBufferPoolV2& operator=(const StagingBufferPoolV2&) = delete;

    inline bool Initialize(uint32_t poolSize = DEFAULT_POOL_SIZE,
                           uint64_t bufferSize = DEFAULT_BUFFER_SIZE) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_initialized) return true;
        uint32_t count = (poolSize > 0 && poolSize <= MAX_POOL_SIZE) ? poolSize : DEFAULT_POOL_SIZE;
        m_bufferSize = (bufferSize > 0) ? bufferSize : DEFAULT_BUFFER_SIZE;
        m_pool.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            StagingBuffer buf;
            buf.id = m_nextId++;
            buf.data = new uint8_t[m_bufferSize];
            buf.size = m_bufferSize;
            buf.inUse = false;
            m_pool.push_back(buf);
            m_stats.totalAllocatedBytes += m_bufferSize;
        }
        m_stats.totalBuffers = count;
        m_initialized = true;
        return true;
    }

    inline StagingBuffer* AcquireBuffer() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized) return nullptr;
        for (auto& buf : m_pool) {
            if (!buf.inUse) {
                buf.inUse = true;
                m_stats.activeBuffers++;
                m_stats.totalAcquires++;
                if (m_stats.activeBuffers > m_stats.peakActive)
                    m_stats.peakActive = m_stats.activeBuffers;
                return &buf;
            }
        }
        return nullptr;
    }

    inline bool ReleaseBuffer(uint64_t bufferId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& buf : m_pool) {
            if (buf.id == bufferId && buf.inUse) {
                buf.inUse = false;
                m_stats.activeBuffers--;
                m_stats.totalReleases++;
                return true;
            }
        }
        return false;
    }

    inline uint32_t GetPoolSize() const { return m_stats.totalBuffers; }
    inline uint32_t GetActiveCount() const { return m_stats.activeBuffers; }
    inline const PoolStats& GetStats() const { return m_stats; }

    inline uint32_t Trim() {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint32_t trimmed = 0;
        auto it = m_pool.begin();
        while (it != m_pool.end()) {
            if (!it->inUse && m_stats.totalBuffers > 4) {
                delete[] static_cast<uint8_t*>(it->data);
                m_stats.totalAllocatedBytes -= it->size;
                m_stats.totalBuffers--;
                it = m_pool.erase(it);
                trimmed++;
            } else {
                ++it;
            }
        }
        return trimmed;
    }

private:
    bool                        m_initialized = false;
    uint64_t                    m_bufferSize = DEFAULT_BUFFER_SIZE;
    uint64_t                    m_nextId = 1;
    PoolStats                   m_stats{};
    std::vector<StagingBuffer>  m_pool;
    std::mutex                  m_mutex;
};

} // namespace Engine
} // namespace ExplorerLens
