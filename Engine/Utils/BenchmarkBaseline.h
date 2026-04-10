// BenchmarkBaseline.h — Benchmark Baseline Comparison Utility
// Copyright (c) 2026 ExplorerLens Project
//
// Loads the on-disk baseline.json produced by prior CI runs and compares
// current benchmark results against stored P50/P95/throughput baselines.
// Emits per-metric delta percentages and an aggregate pass/fail verdict.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class BaselineVerdict : uint8_t
{
    Pass       = 0,
    Regression = 1,   // delta exceeds tolerance
};

struct BaselineMetric
{
    std::string      name;
    double           baselineValue;   // stored reference value
    double           currentValue;    // measured this run
    double           deltaPct;        // signed percentage change
    BaselineVerdict  verdict;

    bool IsRegression() const noexcept { return verdict == BaselineVerdict::Regression; }
};

struct BaselineCompareResult
{
    std::vector<BaselineMetric> metrics;
    bool                        regressionDetected;  // any metric regressed
    double                      worstDeltaPct;       // absolute worst-case delta
    std::string                 baselineVersion;     // e.g. "34.6.0"
    std::string                 currentVersion;

    bool IsClean() const noexcept { return !regressionDetected; }
};

class BenchmarkBaseline
{
public:
    // Tolerance: a delta above this percentage is a regression.
    static constexpr double DEFAULT_REGRESSION_THRESHOLD_PCT = 10.0;

    // Direction: for throughput metrics (img/sec), higher is better.
    // For latency metrics (ms), lower is better.
    void SetRegressionThreshold(double pct) noexcept
    {
        m_regressionThresholdPct = pct;
    }

    // Load baseline JSON from disk (produced by Bump-Version.ps1).
    // Returns false if the file does not exist or is malformed.
    bool LoadBaseline(const std::string& jsonPath) noexcept;

    // Inject baseline values directly (for unit tests — no file I/O).
    void SetBaselineValue(const std::string& metricName, double value,
                          bool higherIsBetter = false);

    // Compare current measurements against the loaded/injected baseline.
    // `current` maps metric name → measured value (same units as baseline).
    BaselineCompareResult Compare(
        const std::unordered_map<std::string, double>& current) const;

    // Number of baseline metrics currently loaded.
    uint32_t MetricCount() const noexcept
    {
        return static_cast<uint32_t>(m_baseline.size());
    }

    // Serialize the comparison result to JSON for CI consumption.
    static std::string ResultToJSON(const BaselineCompareResult& result);

private:
    struct StoredMetric
    {
        double value;
        bool   higherIsBetter;
    };

    std::unordered_map<std::string, StoredMetric> m_baseline;
    double m_regressionThresholdPct = DEFAULT_REGRESSION_THRESHOLD_PCT;
    std::string m_baselineVersion;
};

} // namespace Engine
} // namespace ExplorerLens
