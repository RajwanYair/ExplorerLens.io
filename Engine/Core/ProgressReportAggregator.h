// ProgressReportAggregator.h — Aggregates decode progress from multiple workers
// Copyright (c) 2026 ExplorerLens Project
//
// Collects per-file decode progress from parallel worker threads and
// aggregates into a unified progress percentage for UI reporting.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct ProgressReportAggregatorConfig
{
    bool enabled = true;
    uint32_t maxConcurrentTasks = 64;
    std::string label = "ProgressReportAggregator";
};

class ProgressReportAggregator
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
    ProgressReportAggregatorConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    void ReportProgress(uint32_t taskId, float percent)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_progress[taskId] = percent;
    }

    float GetAggregateProgress() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_progress.empty())
            return 0.0f;
        float total = 0.0f;
        for (const auto& [id, pct] : m_progress)
            total += pct;
        return total / static_cast<float>(m_progress.size());
    }

    size_t GetActiveTaskCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_progress.size();
    }

  private:
    bool m_initialized = false;
    ProgressReportAggregatorConfig m_config;
    mutable std::mutex m_mutex;
    std::unordered_map<uint32_t, float> m_progress;
};

}  // namespace Engine
}  // namespace ExplorerLens
