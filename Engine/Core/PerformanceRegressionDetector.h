// PerformanceRegressionDetector.h — Performance Regression Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Detects performance regressions by comparing recent decode latency
// windows against historical baselines. Uses statistical tests (z-score,
// moving average deviation) and raises alerts when degradation exceeds
// configurable thresholds.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct RegressionAlert {
    std::string metric;
    double      baselineValue = 0.0;
    double      currentValue = 0.0;
    double      deviationPercent = 0.0;
    double      zScore = 0.0;
    uint64_t    detectedTick = 0;
    bool        isRegression = false;
};

struct RegressionConfig {
    uint32_t baselineWindowSize = 100;  // Samples for baseline
    uint32_t recentWindowSize = 20;     // Samples for recent comparison
    double   regressionThresholdPercent = 20.0; // Alert if >20% slower
    double   zScoreThreshold = 2.5;     // Statistical significance
};

struct RegressionStats {
    uint32_t totalChecks = 0;
    uint32_t regressionsDetected = 0;
    uint32_t improvementsDetected = 0;
    double   currentBaselineMs = 0.0;
    double   currentRecentMs = 0.0;
};

class PerformanceRegressionDetector {
public:
    PerformanceRegressionDetector() {
        InitializeSRWLock(&m_lock);
    }
    ~PerformanceRegressionDetector() = default;

    static const wchar_t* GetName() { return L"PerformanceRegressionDetector"; }

    void Configure(const RegressionConfig& config) { m_config = config; }

    /// Add a latency sample.
    void AddSample(double latencyMs) {
        AcquireSRWLockExclusive(&m_lock);
        m_allSamples.push_back(latencyMs);
        // Keep only recent enough samples for analysis
        uint32_t maxSamples = m_config.baselineWindowSize + m_config.recentWindowSize;
        if (m_allSamples.size() > maxSamples * 2)
            m_allSamples.erase(m_allSamples.begin(), m_allSamples.begin() + maxSamples);
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Check for regression against baseline.
    RegressionAlert Check() {
        RegressionAlert alert;
        alert.metric = "decode_latency_ms";
        alert.detectedTick = GetTickCount64();

        AcquireSRWLockShared(&m_lock);
        size_t total = m_allSamples.size();
        uint32_t needed = m_config.baselineWindowSize + m_config.recentWindowSize;
        if (total < needed) {
            ReleaseSRWLockShared(&m_lock);
            return alert;
        }

        // Baseline = older samples
        size_t baseStart = total - needed;
        size_t baseEnd = baseStart + m_config.baselineWindowSize;
        // Recent = newer samples
        size_t recentStart = baseEnd;

        double baselineMean = 0.0, baselineStdDev = 0.0;
        ComputeStats(m_allSamples, baseStart, baseEnd, baselineMean, baselineStdDev);

        double recentMean = 0.0, recentStdDev = 0.0;
        ComputeStats(m_allSamples, recentStart, total, recentMean, recentStdDev);
        ReleaseSRWLockShared(&m_lock);

        alert.baselineValue = baselineMean;
        alert.currentValue = recentMean;
        alert.deviationPercent = baselineMean > 0 ?
            100.0 * (recentMean - baselineMean) / baselineMean : 0.0;

        if (baselineStdDev > 0.001)
            alert.zScore = (recentMean - baselineMean) / baselineStdDev;

        alert.isRegression = alert.deviationPercent > m_config.regressionThresholdPercent &&
            std::abs(alert.zScore) > m_config.zScoreThreshold;

        m_stats.totalChecks++;
        m_stats.currentBaselineMs = baselineMean;
        m_stats.currentRecentMs = recentMean;
        if (alert.isRegression) m_stats.regressionsDetected++;
        else if (alert.deviationPercent < -m_config.regressionThresholdPercent)
            m_stats.improvementsDetected++;

        return alert;
    }

    RegressionStats GetStats() const { return m_stats; }

private:
    void ComputeStats(const std::vector<double>& data, size_t start, size_t end,
        double& mean, double& stdDev) const {
        size_t n = end - start;
        if (n == 0) { mean = 0; stdDev = 0; return; }
        double sum = 0;
        for (size_t i = start; i < end; ++i) sum += data[i];
        mean = sum / n;
        double sumSq = 0;
        for (size_t i = start; i < end; ++i) sumSq += (data[i] - mean) * (data[i] - mean);
        stdDev = std::sqrt(sumSq / n);
    }

    SRWLOCK m_lock{};
    RegressionConfig m_config;
    std::vector<double> m_allSamples;
    mutable RegressionStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
