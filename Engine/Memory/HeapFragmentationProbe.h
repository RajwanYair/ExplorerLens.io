// HeapFragmentationProbe.h — Probes CRT heap fragmentation level
// Copyright (c) 2026 ExplorerLens Project
//
// Queries the process heap state to measure fragmentation ratio, helping
// decide when to compact or switch to pool allocators for decode buffers.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct HeapFragmentationProbeConfig
{
    bool enabled = true;
    uint32_t fragmentationWarningPercent = 30;
    std::string label = "HeapFragmentationProbe";
};

class HeapFragmentationProbe
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
    HeapFragmentationProbeConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct HeapStats
    {
        uint64_t committedBytes = 0;
        uint64_t usedBytes = 0;
        uint32_t fragmentationPercent = 0;
    };

    HeapStats Probe(uint64_t committed, uint64_t used) const
    {
        HeapStats stats;
        stats.committedBytes = committed;
        stats.usedBytes = used;
        if (committed > 0)
            stats.fragmentationPercent = static_cast<uint32_t>((committed - used) * 100 / committed);
        return stats;
    }

    bool NeedsCompaction(const HeapStats& stats) const
    {
        return stats.fragmentationPercent >= m_config.fragmentationWarningPercent;
    }

  private:
    bool m_initialized = false;
    HeapFragmentationProbeConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
