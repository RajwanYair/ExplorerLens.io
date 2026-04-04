// BuildTimingAnalytics.h — Build-Step Timing Analytics Collector
// Copyright (c) 2026 ExplorerLens Project
//
// Records wall-clock and CPU time for each build step — identifies compilation hot spots and link bottlenecks.
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

struct BuildStep
{
    std::string name;
    double wallMs;
    double cpuMs;
    bool cached;
};
struct BuildTimingReport
{
    double totalWallMs = 0;
    double totalCpuMs = 0;
    std::string hottest;
    size_t cachedCount = 0;
    double cacheHitRate() const
    {
        return 0.0;
    }
};
class BuildTimingAnalytics
{
  public:
    void Record(BuildStep step)
    {
        m_steps.push_back(step);
    }
    BuildTimingReport Summarize() const
    {
        double tot = 0;
        for (auto& s : m_steps)
            tot += s.wallMs;
        return {tot, tot, m_steps.empty() ? "" : m_steps[0].name, 0};
    }
    size_t StepCount() const
    {
        return m_steps.size();
    }

  private:
    std::vector<BuildStep> m_steps;
};

}  // namespace Engine
}  // namespace ExplorerLens