// MemoryPageFaultMonitor.h — Monitors page fault rates for memory pressure detection
// Copyright (c) 2026 ExplorerLens Project
//
// Samples process page fault counters at intervals to detect memory pressure
// conditions. Hard page faults indicate disk paging and trigger cache trimming.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct MemoryPageFaultMonitorConfig {
    bool enabled = true;
    uint32_t sampleIntervalMs = 500;
    uint32_t hardFaultThreshold = 100;
    std::string label = "MemoryPageFaultMonitor";
};

class MemoryPageFaultMonitor {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    MemoryPageFaultMonitorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    void RecordSample(uint32_t hardFaults, uint32_t softFaults) {
        m_lastHardFaults = hardFaults;
        m_lastSoftFaults = softFaults;
        m_totalSamples++;
    }

    bool IsUnderPressure() const {
        return m_lastHardFaults >= m_config.hardFaultThreshold;
    }

    uint32_t GetLastHardFaults() const { return m_lastHardFaults; }
    uint32_t GetTotalSamples() const { return m_totalSamples; }

private:
    bool m_initialized = false;
    MemoryPageFaultMonitorConfig m_config;
    uint32_t m_lastHardFaults = 0;
    uint32_t m_lastSoftFaults = 0;
    uint32_t m_totalSamples = 0;
};

}
} // namespace ExplorerLens::Engine
