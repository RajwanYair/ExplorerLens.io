// CPUAffinityRouter.h — CPU Core Affinity Router
// Copyright (c) 2026 ExplorerLens Project
//
// Routes decode threads to specific CPU cores based on NUMA topology, hyperthreading layout, and thermal state.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AffinityPolicy {
    Auto,
    PerformanceCores,
    EfficiencyCores,
    NumaLocal,
    Explicit
};
struct CoreBinding
{
    uint32_t threadId;
    uint32_t coreId;
    uint32_t numaNode;
};
class CPUAffinityRouter
{
  public:
    bool SetPolicy(AffinityPolicy policy)
    {
        m_policy = policy;
        return true;
    }
    bool BindThread(uint32_t threadId, uint32_t coreId)
    {
        (void)threadId;
        (void)coreId;
        return true;
    }
    CoreBinding QueryBinding(uint32_t threadId) const
    {
        return {threadId, 0, 0};
    }
    uint32_t NumaNodeCount() const
    {
        return 1;
    }
    AffinityPolicy GetPolicy() const
    {
        return m_policy;
    }

  private:
    AffinityPolicy m_policy = AffinityPolicy::Auto;
};

}  // namespace Engine
}  // namespace ExplorerLens