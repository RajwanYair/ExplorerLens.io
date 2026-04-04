// StartupOptimizer.h — Shell Extension Startup Performance
// Copyright (c) 2026 ExplorerLens Project
//
// Optimizes shell extension initialization by deferring non-critical
// subsystem setup, using lazy-init for decoders, and prefetching
// frequently-used resources during idle time after Explorer starts.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class StartupPhase : uint8_t {
    COMInit,
    CoreDecoders,
    GPUProbe,
    CacheWarm,
    PluginLoad,
    LazyInit,
    Complete,
    COUNT
};

enum class InitPriority : uint8_t {
    Immediate,
    Deferred,
    Lazy,
    OnDemand,
    COUNT
};

struct StartupTask
{
    std::wstring taskName;
    StartupPhase phase = StartupPhase::CoreDecoders;
    InitPriority priority = InitPriority::Immediate;
    double durationMs = 0.0;
    bool completed = false;
    bool skipped = false;
};

struct StartupConfig
{
    uint32_t deferThresholdMs = 50;
    bool enableLazyDecoders = true;
    bool enablePrefetch = true;
    bool parallelInit = true;
    uint32_t maxParallelTasks = 4;
};

struct StartupMetrics
{
    double totalStartupMs = 0.0;
    double criticalPathMs = 0.0;
    uint32_t tasksImmediate = 0;
    uint32_t tasksDeferred = 0;
    uint32_t tasksSkipped = 0;
    bool withinBudget = true;
};

class StartupOptimizer
{
  public:
    void Configure(const StartupConfig& cfg)
    {
        m_config = cfg;
    }
    const StartupConfig& GetConfig() const
    {
        return m_config;
    }

    void RecordTask(const std::wstring& name, StartupPhase phase, double durationMs)
    {
        if (m_taskCount < MAX_TASKS) {
            auto& t = m_tasks[m_taskCount++];
            t.taskName = name;
            t.phase = phase;
            t.durationMs = durationMs;
            t.completed = true;
            t.priority = (durationMs < m_config.deferThresholdMs) ? InitPriority::Immediate : InitPriority::Deferred;
        }
    }

    StartupMetrics ComputeMetrics() const
    {
        StartupMetrics m;
        for (uint32_t i = 0; i < m_taskCount; ++i) {
            m.totalStartupMs += m_tasks[i].durationMs;
            if (m_tasks[i].priority == InitPriority::Immediate)
                m.criticalPathMs += m_tasks[i].durationMs;
            switch (m_tasks[i].priority) {
                case InitPriority::Immediate:
                    m.tasksImmediate++;
                    break;
                case InitPriority::Deferred:
                    m.tasksDeferred++;
                    break;
                default:
                    break;
            }
            if (m_tasks[i].skipped)
                m.tasksSkipped++;
        }
        m.withinBudget = (m.criticalPathMs < 100.0);  // 100ms budget
        return m;
    }

    uint32_t TaskCount() const
    {
        return m_taskCount;
    }

    void Reset()
    {
        m_taskCount = 0;
    }

    static size_t PhaseCount()
    {
        return static_cast<size_t>(StartupPhase::COUNT);
    }
    static size_t PriorityCount()
    {
        return static_cast<size_t>(InitPriority::COUNT);
    }

  private:
    static constexpr uint32_t MAX_TASKS = 64;
    StartupTask m_tasks[MAX_TASKS] = {};
    uint32_t m_taskCount = 0;
    StartupConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
