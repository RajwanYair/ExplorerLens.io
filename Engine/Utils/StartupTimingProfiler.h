// StartupTimingProfiler.h — Profiles engine startup phase timing
// Copyright (c) 2026 ExplorerLens Project
//
// Measures time spent in each startup phase (COM init, decoder discovery,
// cache warm, GPU probe) for cold-start optimization analysis.
//
#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct StartupTimingProfilerConfig
{
    bool enabled = true;
    uint32_t maxPhases = 32;
    std::string label = "StartupTimingProfiler";
};

class StartupTimingProfiler
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_startTime = std::chrono::high_resolution_clock::now();
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    StartupTimingProfilerConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct PhaseRecord
    {
        std::string name;
        double startMs = 0.0;
        double endMs = 0.0;
        double DurationMs() const
        {
            return endMs - startMs;
        }
    };

    void BeginPhase(const std::string& name)
    {
        auto now = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(now - m_startTime).count();
        m_phases.push_back({name, ms, 0.0});
    }

    void EndPhase()
    {
        if (m_phases.empty())
            return;
        auto now = std::chrono::high_resolution_clock::now();
        m_phases.back().endMs = std::chrono::duration<double, std::milli>(now - m_startTime).count();
    }

    double GetTotalStartupMs() const
    {
        if (m_phases.empty())
            return 0.0;
        return m_phases.back().endMs;
    }

    size_t GetPhaseCount() const
    {
        return m_phases.size();
    }

  private:
    bool m_initialized = false;
    StartupTimingProfilerConfig m_config;
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::vector<PhaseRecord> m_phases;
};

}  // namespace Engine
}  // namespace ExplorerLens
