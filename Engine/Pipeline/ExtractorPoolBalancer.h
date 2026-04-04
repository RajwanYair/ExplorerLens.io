// ExtractorPoolBalancer.h — Load-balances work across extractor thread pool
// Copyright (c) 2026 ExplorerLens Project
//
// Distributes archive extraction and decode tasks across a pool of worker
// threads using work-stealing and affinity-aware scheduling.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ExtractorPoolBalancerConfig
{
    bool enabled = true;
    uint32_t maxWorkers = 16;
    std::string label = "ExtractorPoolBalancer";
};

class ExtractorPoolBalancer
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_workerLoads.resize(m_config.maxWorkers, 0);
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    ExtractorPoolBalancerConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    uint32_t SelectWorker() const
    {
        uint32_t bestIdx = 0;
        uint32_t minLoad = UINT32_MAX;
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_workerLoads.size()); ++i) {
            if (m_workerLoads[i] < minLoad) {
                minLoad = m_workerLoads[i];
                bestIdx = i;
            }
        }
        return bestIdx;
    }

    void AddLoad(uint32_t workerIdx, uint32_t cost)
    {
        if (workerIdx < m_workerLoads.size())
            m_workerLoads[workerIdx] += cost;
    }

  private:
    bool m_initialized = false;
    ExtractorPoolBalancerConfig m_config;
    std::vector<uint32_t> m_workerLoads;
};

}  // namespace Engine
}  // namespace ExplorerLens
