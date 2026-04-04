// DecoderPoolManager.h — Thread-Safe Decoder Instance Pool
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a pool of reusable decoder instances to reduce allocation
// overhead and improve throughput for concurrent thumbnail generation.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DecoderPoolState : uint8_t {
    Idle,
    Active,
    Draining,
    Shutdown
};

struct PooledDecoderInfo
{
    uint32_t decoderId = 0;
    std::string formatType;
    bool inUse = false;
    uint64_t useCount = 0;
    uint64_t lastUsedTimestamp = 0;
};

struct PoolMetrics
{
    uint32_t totalDecoders = 0;
    uint32_t activeDecoders = 0;
    uint32_t idleDecoders = 0;
    uint64_t totalAcquisitions = 0;
    uint64_t totalReleases = 0;
    uint64_t poolMisses = 0;
};

class DecoderPoolManager
{
  public:
    explicit DecoderPoolManager(uint32_t maxPoolSize = 16) : m_maxPoolSize(maxPoolSize) {}

    uint32_t AcquireDecoder(const std::string& formatType)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_metrics.totalAcquisitions++;
        for (auto& decoder : m_pool) {
            if (!decoder.inUse && decoder.formatType == formatType) {
                decoder.inUse = true;
                decoder.useCount++;
                m_metrics.activeDecoders++;
                m_metrics.idleDecoders--;
                return decoder.decoderId;
            }
        }
        m_metrics.poolMisses++;
        if (m_pool.size() < m_maxPoolSize) {
            uint32_t id = static_cast<uint32_t>(m_pool.size()) + 1;
            m_pool.push_back({id, formatType, true, 1, 0});
            m_metrics.totalDecoders++;
            m_metrics.activeDecoders++;
            return id;
        }
        return 0;
    }

    void ReleaseDecoder(uint32_t decoderId)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& decoder : m_pool) {
            if (decoder.decoderId == decoderId && decoder.inUse) {
                decoder.inUse = false;
                m_metrics.totalReleases++;
                m_metrics.activeDecoders--;
                m_metrics.idleDecoders++;
                break;
            }
        }
    }

    PoolMetrics GetMetrics() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_metrics;
    }

    DecoderPoolState GetState() const
    {
        return m_state;
    }
    uint32_t GetMaxPoolSize() const
    {
        return m_maxPoolSize;
    }

    void Drain()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state = DecoderPoolState::Draining;
        m_pool.erase(std::remove_if(m_pool.begin(), m_pool.end(), [](const PooledDecoderInfo& d) { return !d.inUse; }),
                     m_pool.end());
        m_state = DecoderPoolState::Idle;
    }

  private:
    mutable std::mutex m_mutex;
    std::vector<PooledDecoderInfo> m_pool;
    uint32_t m_maxPoolSize;
    DecoderPoolState m_state = DecoderPoolState::Idle;
    PoolMetrics m_metrics;
};

}  // namespace Engine
}  // namespace ExplorerLens
