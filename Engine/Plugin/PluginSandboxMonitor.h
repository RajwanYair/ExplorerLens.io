// PluginSandboxMonitor.h — Monitors plugin sandbox resource usage
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks CPU time, memory, and I/O consumed by sandboxed plugins to enforce
// resource quotas and detect runaway or malicious plugin behavior.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct PluginSandboxMonitorConfig {
    bool enabled = true;
    uint64_t maxCpuTimeMs = 5000;
    uint64_t maxMemoryMB = 256;
    std::string label = "PluginSandboxMonitor";
};

class PluginSandboxMonitor {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PluginSandboxMonitorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct ResourceUsage {
        uint64_t cpuTimeMs = 0;
        uint64_t memoryMB = 0;
        uint64_t ioReadBytes = 0;
        uint64_t ioWriteBytes = 0;
    };

    bool IsWithinLimits(const ResourceUsage& usage) const {
        return usage.cpuTimeMs <= m_config.maxCpuTimeMs &&
            usage.memoryMB <= m_config.maxMemoryMB;
    }

    bool IsCpuExceeded(const ResourceUsage& usage) const {
        return usage.cpuTimeMs > m_config.maxCpuTimeMs;
    }

private:
    bool m_initialized = false;
    PluginSandboxMonitorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
