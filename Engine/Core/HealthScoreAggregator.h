// HealthScoreAggregator.h — System Health Score Aggregation
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates multiple health signals (memory pressure, error rate,
// CPU utilization, queue depth, cache hit rate) into a single 0-100
// health score. Triggers alerts at configurable thresholds.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AggregateHealthLevel : uint8_t {
    Healthy = 0,
    Degraded = 1,
    Warning = 2,
    Critical = 3
};

struct HealthSignal
{
    std::string name;
    double value = 0.0;    // Raw value
    double weight = 1.0;   // 0.0-1.0
    double score = 100.0;  // Normalized 0-100
    AggregateHealthLevel level = AggregateHealthLevel::Healthy;
};

struct HealthThresholds
{
    double healthyMin = 80.0;
    double degradedMin = 60.0;
    double warningMin = 30.0;
    // Below warningMin = Critical
};

struct HealthAggregateStats
{
    double overallScore = 100.0;
    AggregateHealthLevel overallLevel = AggregateHealthLevel::Healthy;
    uint32_t signalCount = 0;
    uint32_t degradedCount = 0;
    uint32_t criticalCount = 0;
    uint64_t assessmentCount = 0;
    double trendDelta = 0.0;  // Score change over last window
};

class HealthScoreAggregator
{
  public:
    HealthScoreAggregator()
    {
        InitializeSRWLock(&m_lock);
    }
    ~HealthScoreAggregator() = default;

    static const wchar_t* GetName()
    {
        return L"HealthScoreAggregator";
    }

    void SetThresholds(const HealthThresholds& t)
    {
        m_thresholds = t;
    }

    /// Add or update a health signal.
    void UpdateSignal(const std::string& name, double rawValue, double score, double weight = 1.0)
    {
        AcquireSRWLockExclusive(&m_lock);
        score = std::clamp(score, 0.0, 100.0);
        weight = std::clamp(weight, 0.0, 1.0);

        HealthSignal sig;
        sig.name = name;
        sig.value = rawValue;
        sig.score = score;
        sig.weight = weight;
        sig.level = ClassifyScore(score);

        bool found = false;
        for (auto& s : m_signals) {
            if (s.name == name) {
                s = sig;
                found = true;
                break;
            }
        }
        if (!found)
            m_signals.push_back(sig);
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Compute overall health score.
    double Assess()
    {
        AcquireSRWLockExclusive(&m_lock);
        if (m_signals.empty()) {
            ReleaseSRWLockExclusive(&m_lock);
            return 100.0;
        }

        double weightedSum = 0.0;
        double totalWeight = 0.0;
        for (const auto& s : m_signals) {
            weightedSum += s.score * s.weight;
            totalWeight += s.weight;
        }

        double previousScore = m_lastScore;
        m_lastScore = totalWeight > 0 ? (weightedSum / totalWeight) : 100.0;
        m_assessmentCount++;

        // Track trend
        m_trendDelta = m_lastScore - previousScore;

        ReleaseSRWLockExclusive(&m_lock);
        return m_lastScore;
    }

    AggregateHealthLevel ClassifyScore(double score) const
    {
        if (score >= m_thresholds.healthyMin)
            return AggregateHealthLevel::Healthy;
        if (score >= m_thresholds.degradedMin)
            return AggregateHealthLevel::Degraded;
        if (score >= m_thresholds.warningMin)
            return AggregateHealthLevel::Warning;
        return AggregateHealthLevel::Critical;
    }

    static const char* LevelName(AggregateHealthLevel level)
    {
        switch (level) {
            case AggregateHealthLevel::Healthy:
                return "Healthy";
            case AggregateHealthLevel::Degraded:
                return "Degraded";
            case AggregateHealthLevel::Warning:
                return "Warning";
            case AggregateHealthLevel::Critical:
                return "Critical";
            default:
                return "Unknown";
        }
    }

    HealthAggregateStats GetStats() const
    {
        HealthAggregateStats stats;
        stats.overallScore = m_lastScore;
        stats.overallLevel = ClassifyScore(m_lastScore);
        stats.signalCount = static_cast<uint32_t>(m_signals.size());
        stats.assessmentCount = m_assessmentCount;
        stats.trendDelta = m_trendDelta;
        for (const auto& s : m_signals) {
            if (s.level == AggregateHealthLevel::Degraded || s.level == AggregateHealthLevel::Warning)
                stats.degradedCount++;
            if (s.level == AggregateHealthLevel::Critical)
                stats.criticalCount++;
        }
        return stats;
    }

  private:
    SRWLOCK m_lock{};
    std::vector<HealthSignal> m_signals;
    HealthThresholds m_thresholds;
    double m_lastScore = 100.0;
    double m_trendDelta = 0.0;
    uint64_t m_assessmentCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
