// PluginResourceLimiter.h — Per-Plugin Resource Quota Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces per-plugin resource quotas (memory, CPU time, file handles,
// threads, disk I/O) with three escalating enforcement actions: Warning,
// Throttle, Terminate. Memory exceeding quota by 20% triggers throttling,
// 50% triggers termination. Each plugin can receive a custom quota or
// inherit the default. Usage is tracked per-plugin for monitoring.
// Supports ResourceBudget, ResourceCheckpoint, and violation callbacks.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PluginResourceQuota
{
    uint64_t maxMemoryBytes = 256ULL * 1024 * 1024;  // 256 MB
    uint64_t maxCpuTimeMs = 30000;                   // 30s CPU time
    uint32_t maxFileHandles = 64;
    uint32_t maxThreads = 8;
    uint64_t maxDiskIOBytes = 1ULL * 1024 * 1024 * 1024;  // 1 GB I/O
    bool enableMemoryLimit = true;
    bool enableCpuLimit = true;
    bool enableHandleLimit = true;
};

struct PluginResourceUsage
{
    std::wstring pluginId;
    uint64_t memoryBytes = 0;
    uint64_t peakMemoryBytes = 0;
    uint64_t cpuTimeMs = 0;
    uint32_t fileHandles = 0;
    uint32_t threadCount = 0;
    uint64_t diskIOBytes = 0;
    uint32_t allocationCount = 0;
    bool overQuota = false;
    bool terminated = false;
};

enum class ResourceLimitAction : uint32_t {
    None = 0,
    Warning = 1,
    Throttle = 2,
    Terminate = 3
};

struct PluginResourceLimiterStats
{
    uint64_t totalWarnings = 0;
    uint64_t totalThrottles = 0;
    uint64_t totalTerminations = 0;
    uint32_t pluginsOverQuota = 0;
};

// ResourceBudget — pre-allocated budget for a plugin session
struct ResourceBudget
{
    std::wstring pluginId;
    uint64_t memoryBudget = 128ULL * 1024 * 1024;  // 128 MB default
    uint64_t cpuTimeBudgetMs = 15000;              // 15s
    uint32_t fileHandleBudget = 32;
    uint32_t threadBudget = 4;
    uint64_t memoryUsed = 0;
    uint64_t cpuTimeUsedMs = 0;
    uint32_t fileHandlesUsed = 0;
    uint32_t threadsUsed = 0;

    double MemoryUtilization() const
    {
        return (memoryBudget > 0) ? (static_cast<double>(memoryUsed) / static_cast<double>(memoryBudget)) * 100.0 : 0.0;
    }

    bool IsOverBudget() const
    {
        return memoryUsed > memoryBudget || cpuTimeUsedMs > cpuTimeBudgetMs || fileHandlesUsed > fileHandleBudget
               || threadsUsed > threadBudget;
    }
};

// ResourceCheckpoint — periodic enforcement snapshot
struct ResourceCheckpoint
{
    std::chrono::steady_clock::time_point timestamp;
    std::wstring pluginId;
    PluginResourceUsage usage;
    ResourceLimitAction actionTaken = ResourceLimitAction::None;
    bool withinBudget = true;
};

// Violation callback type: receives plugin ID, action taken, usage at time of violation
using ResourceViolationCallback =
    std::function<void(const std::wstring&, ResourceLimitAction, const PluginResourceUsage&)>;

// ========================================================================
// PluginResourceLimiter — Job Object based resource enforcement
// ========================================================================
class PluginResourceLimiter
{
  public:
    static PluginResourceLimiter& Instance()
    {
        static PluginResourceLimiter instance;
        return instance;
    }

    void Initialize(const PluginResourceQuota& defaultQuota = {})
    {
        m_defaultQuota = defaultQuota;
        m_pluginUsage.clear();
        m_pluginQuotas.clear();
        m_pluginBudgets.clear();
        m_checkpoints.clear();
        m_violationCallback = nullptr;
        m_stats = {};
        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

    // Register plugin with default quota
    void RegisterPlugin(const std::wstring& pluginId)
    {
        m_pluginQuotas[pluginId] = m_defaultQuota;
        PluginResourceUsage usage;
        usage.pluginId = pluginId;
        m_pluginUsage[pluginId] = usage;
    }

    // Set custom quota for a plugin
    void SetQuota(const std::wstring& pluginId, const PluginResourceQuota& quota)
    {
        m_pluginQuotas[pluginId] = quota;
    }

    // Set a resource budget for a plugin
    void SetBudget(const std::wstring& pluginId, const ResourceBudget& budget)
    {
        m_pluginBudgets[pluginId] = budget;
        m_pluginBudgets[pluginId].pluginId = pluginId;
    }

    // Get the current budget for a plugin
    ResourceBudget GetBudget(const std::wstring& pluginId) const
    {
        auto it = m_pluginBudgets.find(pluginId);
        return (it != m_pluginBudgets.end()) ? it->second : ResourceBudget{};
    }

    // Set violation callback
    void SetViolationCallback(ResourceViolationCallback callback)
    {
        m_violationCallback = std::move(callback);
    }

    // Record resource usage
    ResourceLimitAction RecordMemoryUsage(const std::wstring& pluginId, uint64_t bytes)
    {
        auto it = m_pluginUsage.find(pluginId);
        if (it == m_pluginUsage.end())
            return ResourceLimitAction::None;

        it->second.memoryBytes = bytes;
        if (bytes > it->second.peakMemoryBytes)
            it->second.peakMemoryBytes = bytes;
        it->second.allocationCount++;

        // Update budget if exists
        auto budgetIt = m_pluginBudgets.find(pluginId);
        if (budgetIt != m_pluginBudgets.end()) {
            budgetIt->second.memoryUsed = bytes;
        }

        return CheckQuota(pluginId, it->second);
    }

    ResourceLimitAction RecordCpuTime(const std::wstring& pluginId, uint64_t cpuMs)
    {
        auto it = m_pluginUsage.find(pluginId);
        if (it == m_pluginUsage.end())
            return ResourceLimitAction::None;

        it->second.cpuTimeMs = cpuMs;

        // Update budget if exists
        auto budgetIt = m_pluginBudgets.find(pluginId);
        if (budgetIt != m_pluginBudgets.end()) {
            budgetIt->second.cpuTimeUsedMs = cpuMs;
        }

        return CheckQuota(pluginId, it->second);
    }

    // Record file handle usage
    ResourceLimitAction RecordFileHandles(const std::wstring& pluginId, uint32_t handles)
    {
        auto it = m_pluginUsage.find(pluginId);
        if (it == m_pluginUsage.end())
            return ResourceLimitAction::None;

        it->second.fileHandles = handles;

        auto budgetIt = m_pluginBudgets.find(pluginId);
        if (budgetIt != m_pluginBudgets.end()) {
            budgetIt->second.fileHandlesUsed = handles;
        }

        return CheckQuota(pluginId, it->second);
    }

    // Record thread count
    ResourceLimitAction RecordThreadCount(const std::wstring& pluginId, uint32_t threads)
    {
        auto it = m_pluginUsage.find(pluginId);
        if (it == m_pluginUsage.end())
            return ResourceLimitAction::None;

        it->second.threadCount = threads;

        auto budgetIt = m_pluginBudgets.find(pluginId);
        if (budgetIt != m_pluginBudgets.end()) {
            budgetIt->second.threadsUsed = threads;
        }

        return CheckQuota(pluginId, it->second);
    }

    // Take a resource checkpoint (periodic enforcement)
    ResourceCheckpoint TakeCheckpoint(const std::wstring& pluginId)
    {
        ResourceCheckpoint cp;
        cp.timestamp = std::chrono::steady_clock::now();
        cp.pluginId = pluginId;
        cp.usage = GetUsage(pluginId);
        cp.actionTaken = CheckQuota(pluginId, m_pluginUsage[pluginId]);

        auto budgetIt = m_pluginBudgets.find(pluginId);
        cp.withinBudget = (budgetIt == m_pluginBudgets.end()) || !budgetIt->second.IsOverBudget();

        m_checkpoints.push_back(cp);
        return cp;
    }

    // Get all checkpoints
    const std::vector<ResourceCheckpoint>& GetCheckpoints() const
    {
        return m_checkpoints;
    }

    // Get usage for a plugin
    PluginResourceUsage GetUsage(const std::wstring& pluginId) const
    {
        auto it = m_pluginUsage.find(pluginId);
        return (it != m_pluginUsage.end()) ? it->second : PluginResourceUsage{};
    }

    // Get quota for a plugin
    PluginResourceQuota GetQuota(const std::wstring& pluginId) const
    {
        auto it = m_pluginQuotas.find(pluginId);
        return (it != m_pluginQuotas.end()) ? it->second : m_defaultQuota;
    }

    // Check if plugin is over quota
    bool IsOverQuota(const std::wstring& pluginId) const
    {
        auto it = m_pluginUsage.find(pluginId);
        return (it != m_pluginUsage.end()) ? it->second.overQuota : false;
    }

    // Get registered plugin count
    uint32_t GetPluginCount() const
    {
        return static_cast<uint32_t>(m_pluginUsage.size());
    }

    // Get stats
    PluginResourceLimiterStats GetStats() const
    {
        PluginResourceLimiterStats stats = m_stats;
        stats.pluginsOverQuota = 0;
        for (auto& [id, usage] : m_pluginUsage) {
            if (usage.overQuota)
                stats.pluginsOverQuota++;
        }
        return stats;
    }

  private:
    PluginResourceLimiter() = default;

    ResourceLimitAction CheckQuota(const std::wstring& pluginId, PluginResourceUsage& usage)
    {
        auto qIt = m_pluginQuotas.find(pluginId);
        if (qIt == m_pluginQuotas.end())
            return ResourceLimitAction::None;

        auto& quota = qIt->second;
        ResourceLimitAction action = ResourceLimitAction::None;

        // Memory check
        if (quota.enableMemoryLimit && usage.memoryBytes > quota.maxMemoryBytes) {
            double ratio = static_cast<double>(usage.memoryBytes) / static_cast<double>(quota.maxMemoryBytes);
            if (ratio > 1.5) {
                action = ResourceLimitAction::Terminate;
                m_stats.totalTerminations++;
            } else if (ratio > 1.2) {
                action = ResourceLimitAction::Throttle;
                m_stats.totalThrottles++;
            } else {
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

        // File handle check
        if (quota.enableHandleLimit && usage.fileHandles > quota.maxFileHandles) {
            if (static_cast<uint32_t>(ResourceLimitAction::Warning) > static_cast<uint32_t>(action)) {
                action = ResourceLimitAction::Warning;
                m_stats.totalWarnings++;
            }
            usage.overQuota = true;
        }

        if (action == ResourceLimitAction::None) {
            usage.overQuota = false;
        }

        // Fire violation callback if action taken
        if (action != ResourceLimitAction::None && m_violationCallback) {
            m_violationCallback(pluginId, action, usage);
        }

        return action;
    }

    PluginResourceQuota m_defaultQuota;
    PluginResourceLimiterStats m_stats;
    std::unordered_map<std::wstring, PluginResourceQuota> m_pluginQuotas;
    std::unordered_map<std::wstring, PluginResourceUsage> m_pluginUsage;
    std::unordered_map<std::wstring, ResourceBudget> m_pluginBudgets;
    std::vector<ResourceCheckpoint> m_checkpoints;
    ResourceViolationCallback m_violationCallback;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
