// BenchmarkBaseline.cpp — Benchmark Baseline Comparison Utility
// Copyright (c) 2026 ExplorerLens Project
//
#include "BenchmarkBaseline.h"
#include <cmath>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

void BenchmarkBaseline::SetBaselineValue(const std::string& metricName,
                                         double value,
                                         bool higherIsBetter)
{
    m_baseline[metricName] = {value, higherIsBetter};
}

bool BenchmarkBaseline::LoadBaseline(const std::string& /*jsonPath*/) noexcept
{
    // A full JSON parser is project-scope infrastructure; in this iteration
    // we accept injected baselines via SetBaselineValue (used in tests and CI
    // bootstrap).  Returning false indicates "not loaded from disk".
    return false;
}

BaselineCompareResult BenchmarkBaseline::Compare(
    const std::unordered_map<std::string, double>& current) const
{
    BaselineCompareResult result;
    result.regressionDetected = false;
    result.worstDeltaPct      = 0.0;
    result.baselineVersion    = m_baselineVersion;

    for (const auto& kv : m_baseline) {
        const std::string& name = kv.first;
        const double       baseVal = kv.second.value;
        const bool         hib     = kv.second.higherIsBetter;

        auto it = current.find(name);
        if (it == current.end())
            continue;

        const double curVal = it->second;
        double deltaPct = 0.0;
        if (baseVal != 0.0) {
            // Raw signed delta as percentage of baseline.
            deltaPct = ((curVal - baseVal) / std::abs(baseVal)) * 100.0;
            // For latency (lower is better) a positive delta is a regression.
            // For throughput (higher is better) a negative delta is a regression.
            if (!hib) deltaPct = -deltaPct;  // flip so positive = regression
        }

        const bool isRegression = deltaPct > m_regressionThresholdPct;
        if (isRegression)
            result.regressionDetected = true;
        if (std::abs(deltaPct) > std::abs(result.worstDeltaPct))
            result.worstDeltaPct = deltaPct;

        BaselineMetric m{};
        m.name          = name;
        m.baselineValue = baseVal;
        m.currentValue  = curVal;
        m.deltaPct      = deltaPct;
        m.verdict       = isRegression ? BaselineVerdict::Regression : BaselineVerdict::Pass;
        result.metrics.push_back(m);
    }

    return result;
}

std::string BenchmarkBaseline::ResultToJSON(const BaselineCompareResult& result)
{
    std::ostringstream oss;
    oss << "{\"regressionDetected\":" << (result.regressionDetected ? "true" : "false")
        << ",\"worstDeltaPct\":" << result.worstDeltaPct
        << ",\"baselineVersion\":\"" << result.baselineVersion << "\""
        << ",\"metrics\":[\n";
    bool first = true;
    for (const auto& m : result.metrics) {
        if (!first) oss << ",\n";
        first = false;
        oss << "  {\"name\":\"" << m.name << "\""
            << ",\"baseline\":" << m.baselineValue
            << ",\"current\":" << m.currentValue
            << ",\"deltaPct\":" << m.deltaPct
            << ",\"verdict\":\"" << (m.IsRegression() ? "regression" : "pass") << "\"}";
    }
    oss << "\n]}";
    return oss.str();
}

} // namespace Engine
} // namespace ExplorerLens
