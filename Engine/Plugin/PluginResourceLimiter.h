#pragma once
// ============================================================================
// PluginResourceLimiter.h — Per-plugin resource quota enforcement
// ExplorerLens Engine v15.0.0 "Zenith"
// ============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

// Resource limit definition
struct PluginResourceQuota {
    uint64_t maxMemoryBytes = 256ULL * 1024 * 1024;  // 256 MB
    uint64_t maxCpuTimeMs = 30000;                  // 30s CPU time
    uint32_t maxFileHandles = 64;
    uint32_t maxThreads = 8;
    uint64_t maxDiskIOBytes = 1ULL * 1024 * 1024 * 1024; // 1 GB I/O
    bool     enableMemoryLimit = true;
    bool     enableCpuLimit = true;
    bool     enableHandleLimit = true;
};

// Per-plugin resource usage
struct PluginResourceUsage {
    std::wstring pluginId;
    uint64_t     memoryBytes = 0;
    uint64_t     peakMemoryBytes = 0;
    uint64_t     cpuTimeMs = 0;
    uint32_t     fileHandles = 0;
    uint32_t     threadCount = 0;
    uint64_t     diskIOBytes = 0;
    bool         overQuota = false;
    bool         terminated = false;
};

// Enforcement action
enum class ResourceLimitAction : uint32_t {
    None = 0,
    Warning = 1,
    Throttle = 2,
    Terminate = 3
};

// Limiter stats
struct PluginResourceLimiterStats {
    uint64_t totalWarnings = 0;
    uint64_t totalThrottles = 0;
    uint64_t totalTerminations = 0;
    uint32_t pluginsOverQuota = 0;
};

// ========================================================================
// PluginResourceLimiter — Job Object based resource enforcement
// ========================================================================
class PluginResourceLimiter {
public:
    static PluginResourceLimiter& Instance() {
        static PluginResourceLimiter instance;
        return instance;
    }

    void Initialize(const PluginResourceQuota& defaultQuota = {}) {
        m_defaultQuota = defaultQuota;
        m_pluginUsage.clear();
        m_pluginQuotas.clear();
        m_stats = {};
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Register plugin with default quota
    void RegisterPlugin(const std::wstring& pluginId) {
        m_pluginQuotas[pluginId] = m_defaultQuota;
        PluginResourceUsage usage;
        usage.pluginId = pluginId;
        m_pluginUsage[pluginId] = usage;
    }

    // Set custom quota for a plugin
    void SetQuota(const std::wstring& pluginId, const PluginResourceQuota& quota) {
        m_pluginQuotas[pluginId] = quota;
    }

    // Record resource usage
    ResourceLimitAction RecordMemoryUsage(const std::wstring& pluginId, uint64_t bytes) {
        auto it = m_pluginUsage.find(pluginId);
        if (it == m_pluginUsage.end()) return ResourceLimitAction::None;

        it->second.memoryBytes = bytes;
        if (bytes > it->second.peakMemoryBytes) it->second.peakMemoryBytes = bytes;

        return CheckQuota(pluginId, it->second);
    }

    ResourceLimitAction RecordCpuTime(const std::wstring& pluginId, uint64_t cpuMs) {
        auto it = m_pluginUsage.find(pluginId);
        if (it == m_pluginUsage.end()) return ResourceLimitAction::None;

        it->second.cpuTimeMs = cpuMs;
        return CheckQuota(pluginId, it->second);
    }

    // Get usage for a plugin
    PluginResourceUsage GetUsage(const std::wstring& pluginId) const {
        auto it = m_pluginUsage.find(pluginId);
        return (it != m_pluginUsage.end()) ? it->second : PluginResourceUsage{};
    }

    // Get quota for a plugin
    PluginResourceQuota GetQuota(const std::wstring& pluginId) const {
        auto it = m_pluginQuotas.find(pluginId);
        return (it != m_pluginQuotas.end()) ? it->second : m_defaultQuota;
    }

    // Check if plugin is over quota
    bool IsOverQuota(const std::wstring& pluginId) const {
        auto it = m_pluginUsage.find(pluginId);
        return (it != m_pluginUsage.end()) ? it->second.overQuota : false;
    }

    // Get registered plugin count
    uint32_t GetPluginCount() const { return static_cast<uint32_t>(m_pluginUsage.size()); }

    // Get stats
    PluginResourceLimiterStats GetStats() const {
        PluginResourceLimiterStats stats = m_stats;
        stats.pluginsOverQuota = 0;
        for (auto& [id, usage] : m_pluginUsage) {
            if (usage.overQuota) stats.pluginsOverQuota++;
        }
        return stats;
    }

private:
    PluginResourceLimiter() = default;

    ResourceLimitAction CheckQuota(const std::wstring& pluginId, PluginResourceUsage& usage) {
        auto qIt = m_pluginQuotas.find(pluginId);
        if (qIt == m_pluginQuotas.end()) return ResourceLimitAction::None;

        auto& quota = qIt->second;
        ResourceLimitAction action = ResourceLimitAction::None;

        // Memory check
        if (quota.enableMemoryLimit && usage.memoryBytes > quota.maxMemoryBytes) {
            double ratio = static_cast<double>(usage.memoryBytes) / static_cast<double>(quota.maxMemoryBytes);
            if (ratio > 1.5) {
                action = ResourceLimitAction::Terminate;
                m_stats.totalTerminations++;
            }
            else if (ratio > 1.2) {
                action = ResourceLimitAction::Throttle;
                m_stats.totalThrottles++;
            }
            else {
                action = ResourceLimitAction::Warning;
                m_stats.totalWarnings++;
            }
            usage.overQuota = true;
        }

        // CPU time check
        if (quota.enableCpuLimit && usage.cpuTimeMs > quota.maxCpuTimeMs) {
            ResourceLimitAction cpuAction = ResourceLimitAction::Terminate;
            m_stats.totalTerminations++;
            if (static_cast<uint32_t>(cpuAction) > static_cast<uint32_t>(action)) {
                action = cpuAction;
            }
            usage.overQuota = true;
        }

        if (action == ResourceLimitAction::None) {
            usage.overQuota = false;
        }

        return action;
    }

    PluginResourceQuota m_defaultQuota;
    PluginResourceLimiterStats m_stats;
    std::unordered_map<std::wstring, PluginResourceQuota> m_pluginQuotas;
    std::unordered_map<std::wstring, PluginResourceUsage> m_pluginUsage;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
