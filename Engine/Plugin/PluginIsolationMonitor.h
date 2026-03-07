// PluginIsolationMonitor.h — Plugin Sandbox Health Monitor
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors plugin sandbox health including memory consumption, CPU usage,
// handle leaks, and violation attempts with automatic quarantine support.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SandboxViolationType : uint8_t {
    None,
    MemoryExceeded,
    CPUExceeded,
    HandleLeak,
    FileAccessViolation,
    NetworkAccessViolation,
    RegistryAccessViolation,
    CrashDetected
};

enum class QuarantineState : uint8_t {
    Active,
    Warning,
    Quarantined,
    Terminated
};

struct PluginSandboxStatus {
    std::string pluginId;
    QuarantineState state = QuarantineState::Active;
    uint64_t memoryUsageBytes = 0;
    float cpuUsagePercent = 0.0f;
    uint32_t handleCount = 0;
    uint32_t violationCount = 0;
    uint64_t uptimeMs = 0;
};

struct SandboxLimits {
    uint64_t maxMemoryBytes = 256 * 1024 * 1024;
    float maxCPUPercent = 25.0f;
    uint32_t maxHandles = 256;
    uint32_t maxViolationsBeforeQuarantine = 3;
};

struct IsolationMetrics {
    uint64_t totalViolations = 0;
    uint64_t quarantineEvents = 0;
    uint64_t terminationEvents = 0;
    uint32_t activePlugins = 0;
    uint32_t quarantinedPlugins = 0;
};

class PluginIsolationMonitor {
public:
    PluginIsolationMonitor() = default;

    void RegisterPlugin(const std::string& pluginId) {
        PluginSandboxStatus status;
        status.pluginId = pluginId;
        m_plugins[pluginId] = status;
        m_metrics.activePlugins++;
    }

    void UpdateStatus(const std::string& pluginId, uint64_t memoryBytes,
        float cpuPercent, uint32_t handles) {
        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end()) return;
        auto& status = it->second;
        status.memoryUsageBytes = memoryBytes;
        status.cpuUsagePercent = cpuPercent;
        status.handleCount = handles;

        if (memoryBytes > m_limits.maxMemoryBytes ||
            cpuPercent > m_limits.maxCPUPercent ||
            handles > m_limits.maxHandles) {
            ReportViolation(pluginId, SandboxViolationType::MemoryExceeded);
        }
    }

    void ReportViolation(const std::string& pluginId, SandboxViolationType type) {
        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end()) return;
        it->second.violationCount++;
        m_metrics.totalViolations++;
        (void)type;

        if (it->second.violationCount >= m_limits.maxViolationsBeforeQuarantine) {
            it->second.state = QuarantineState::Quarantined;
            m_metrics.quarantineEvents++;
            m_metrics.quarantinedPlugins++;
        }
        else if (it->second.violationCount > 0) {
            it->second.state = QuarantineState::Warning;
        }
    }

    PluginSandboxStatus GetStatus(const std::string& pluginId) const {
        auto it = m_plugins.find(pluginId);
        return it != m_plugins.end() ? it->second : PluginSandboxStatus{};
    }

    IsolationMetrics GetMetrics() const { return m_metrics; }
    void SetLimits(const SandboxLimits& limits) { m_limits = limits; }
    SandboxLimits GetLimits() const { return m_limits; }

private:
    std::unordered_map<std::string, PluginSandboxStatus> m_plugins;
    SandboxLimits m_limits;
    IsolationMetrics m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
