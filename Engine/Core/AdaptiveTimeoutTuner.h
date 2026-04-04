// AdaptiveTimeoutTuner.h — Adaptive Per-Format Timeout Tuner
// Copyright (c) 2026 ExplorerLens Project
//
// Learns per-format decode latency distributions using an online percentile tracker
// and sets adaptive timeouts at the P99 + safety margin, reducing false timeouts
// while still catching runaway decodes.
//
#pragma once
#include <stdint.h>
#include <algorithm>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ATTimeoutPolicy
{
    double baselineMs = 5000.0;
    double learnedP99 = 0.0;  // 0 = not yet learned
    double safetyMulti = 1.5;
    int sampleCount = 0;

    double EffectiveTimeoutMs() const noexcept
    {
        return learnedP99 > 0.0 ? learnedP99 * safetyMulti : baselineMs;
    }
};

class AdaptiveTimeoutTuner
{
  public:
    static constexpr int MIN_SAMPLES = 20;
    static constexpr int MAX_WINDOW = 200;
    static constexpr double DEFAULT_TIMEOUT = 5000.0;  // ms
    static constexpr double MAX_TIMEOUT = 60000.0;
    static constexpr double MIN_TIMEOUT = 200.0;

    explicit AdaptiveTimeoutTuner() = default;

    void RecordLatency(const std::string& format, double latencyMs)
    {
        auto& window = m_windows[format];
        if ((int)window.size() >= MAX_WINDOW)
            window.erase(window.begin());
        window.push_back(latencyMs);
        UpdatePolicy(format);
    }

    double GetTimeoutMs(const std::string& format) const
    {
        auto it = m_policies.find(format);
        if (it == m_policies.end())
            return DEFAULT_TIMEOUT;
        return std::clamp(it->second.EffectiveTimeoutMs(), MIN_TIMEOUT, MAX_TIMEOUT);
    }

    const ATTimeoutPolicy* GetPolicy(const std::string& format) const
    {
        auto it = m_policies.find(format);
        return it != m_policies.end() ? &it->second : nullptr;
    }

    void SetBaseline(const std::string& format, double ms)
    {
        m_policies[format].baselineMs = ms;
    }

    void Reset(const std::string& format)
    {
        m_windows.erase(format);
        m_policies.erase(format);
    }

    int FormatCount() const noexcept
    {
        return (int)m_windows.size();
    }

  private:
    void UpdatePolicy(const std::string& format)
    {
        auto& window = m_windows[format];
        if ((int)window.size() < MIN_SAMPLES)
            return;

        auto& pol = m_policies[format];
        pol.sampleCount = (int)window.size();

        std::vector<double> sorted = window;
        std::sort(sorted.begin(), sorted.end());
        int p99idx = (int)(sorted.size() * 0.99);
        if (p99idx >= (int)sorted.size())
            p99idx = (int)sorted.size() - 1;
        pol.learnedP99 = sorted[p99idx];
    }

    std::unordered_map<std::string, std::vector<double>> m_windows;
    std::unordered_map<std::string, ATTimeoutPolicy> m_policies;
};

}  // namespace Engine
}  // namespace ExplorerLens
