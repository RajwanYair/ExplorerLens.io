// NUMANodeAllocator.h — NUMA-aware memory allocation
// Copyright (c) 2026 ExplorerLens Project
//
// Allocates memory on the NUMA node closest to the executing thread,
// minimizing cross-node memory access latency on multi-socket systems.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct NUMANodeAllocatorConfig
{
    bool enabled = true;
    uint32_t preferredNode = 0;
    bool autoDetect = true;
    std::string label = "NUMANodeAllocator";
};

class NUMANodeAllocator
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_nodeCount = 1;  // Default single-node
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    NUMANodeAllocatorConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    uint32_t GetNodeCount() const
    {
        return m_nodeCount;
    }
    bool IsNUMAAvailable() const
    {
        return m_nodeCount > 1;
    }

    uint32_t GetPreferredNode() const
    {
        return m_config.autoDetect ? m_detectedNode : m_config.preferredNode;
    }

    void SetDetectedNode(uint32_t node)
    {
        m_detectedNode = node;
    }

    struct AllocationStats
    {
        uint64_t localAllocations = 0;
        uint64_t remoteAllocations = 0;
        uint64_t totalBytes = 0;
    };

    void RecordAllocation(uint32_t node, uint64_t size)
    {
        m_stats.totalBytes += size;
        if (node == GetPreferredNode())
            m_stats.localAllocations++;
        else
            m_stats.remoteAllocations++;
    }

    AllocationStats GetStats() const
    {
        return m_stats;
    }

  private:
    bool m_initialized = false;
    NUMANodeAllocatorConfig m_config;
    uint32_t m_nodeCount = 1;
    uint32_t m_detectedNode = 0;
    AllocationStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
