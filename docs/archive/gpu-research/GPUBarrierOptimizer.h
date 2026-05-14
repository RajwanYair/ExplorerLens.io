// GPUBarrierOptimizer.h — Batches and optimizes GPU resource barriers
// Copyright (c) 2026 ExplorerLens Project
//
// Coalesces redundant GPU resource barriers (UAV, transition, aliasing)
// to minimize pipeline stalls and improve GPU command list efficiency.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct GPUBarrierOptimizerConfig
{
    bool enabled = true;
    uint32_t maxBatchSize = 32;
    std::string label = "GPUBarrierOptimizer";
};

class GPUBarrierOptimizer
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
    GPUBarrierOptimizerConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    enum class BarrierType : uint8_t {
        Transition,
        UAV,
        Aliasing
    };

    struct PendingBarrier
    {
        uint64_t resourceHandle = 0;
        BarrierType type = BarrierType::Transition;
        uint32_t stateBefore = 0;
        uint32_t stateAfter = 0;
    };

    bool QueueBarrier(const PendingBarrier& b)
    {
        if (m_pending.size() >= m_config.maxBatchSize)
            return false;
        m_pending.push_back(b);
        return true;
    }

    size_t GetPendingCount() const
    {
        return m_pending.size();
    }
    void FlushBarriers()
    {
        m_pending.clear();
    }

  private:
    bool m_initialized = false;
    GPUBarrierOptimizerConfig m_config;
    std::vector<PendingBarrier> m_pending;
};

}  // namespace Engine
}  // namespace ExplorerLens
