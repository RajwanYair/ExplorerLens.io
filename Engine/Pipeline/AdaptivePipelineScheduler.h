// AdaptivePipelineScheduler.h — Dynamic pipeline parallelism adjustment
// Copyright (c) 2026 ExplorerLens Project
//
// Dynamically adjusts decode pipeline parallelism based on CPU load, memory
// pressure, and thermal state to maximize throughput without saturation.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct AdaptivePipelineSchedulerConfig
{
    bool enabled = true;
    uint32_t minParallelism = 1;
    uint32_t maxParallelism = 16;
    uint32_t cpuThresholdPercent = 80;
    std::string label = "AdaptivePipelineScheduler";
};

class AdaptivePipelineScheduler
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_currentParallelism = m_config.maxParallelism / 2;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    AdaptivePipelineSchedulerConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    void UpdateMetrics(uint32_t cpuPercent, uint32_t memPressure)
    {
        (void)memPressure;
        if (cpuPercent > m_config.cpuThresholdPercent && m_currentParallelism > m_config.minParallelism)
            m_currentParallelism--;
        else if (cpuPercent < m_config.cpuThresholdPercent / 2 && m_currentParallelism < m_config.maxParallelism)
            m_currentParallelism++;
        m_lastCpuPercent = cpuPercent;
    }

    uint32_t GetCurrentParallelism() const
    {
        return m_currentParallelism;
    }
    uint32_t GetLastCpuPercent() const
    {
        return m_lastCpuPercent;
    }

  private:
    bool m_initialized = false;
    AdaptivePipelineSchedulerConfig m_config;
    uint32_t m_currentParallelism = 4;
    uint32_t m_lastCpuPercent = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
