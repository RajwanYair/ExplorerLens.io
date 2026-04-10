// GPUDecodePerformanceGate.h — Per-PR P95 Latency Regression Gate
// Copyright (c) 2026 ExplorerLens Project
//
// Automated performance regression gate that blocks any build where P95
// decode latency degrades by more than 5% or batch throughput drops by
// more than 10% from the stored baseline. Used by CI and local dev.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct PerformanceSample {
    std::string formatName;    // e.g. "JPEG", "PNG", "AVIF"
    float       p50Ms  = 0.0f; // 50th percentile decode time
    float       p95Ms  = 0.0f; // 95th percentile decode time
    float       p99Ms  = 0.0f; // 99th percentile decode time
    float       batchImgPerSec = 0.0f; // Batch throughput
};

struct GateThresholds {
    float maxP95RegressionPct    = 5.0f;   // Block if P95 increases > 5%
    float maxThroughputDropPct   = 10.0f;  // Block if batch drops > 10%
    float warnP50RegressionPct   = 3.0f;   // Warn if P50 increases > 3%
};

enum class GPUGateVerdict : uint8_t {
    Pass    = 0,
    Warn    = 1,
    Block   = 2,
};

struct GPUGateResult {
    GPUGateVerdict verdict       = GPUGateVerdict::Pass;
    std::string formatName;
    std::string metric;
    float       baselineValue = 0.0f;
    float       currentValue  = 0.0f;
    float       deltaPct      = 0.0f;
    std::string message;
};

class GPUDecodePerformanceGate {
public:
    explicit GPUDecodePerformanceGate(GateThresholds thresholds = {});

    // Load baseline from a JSON file (Engine/Tests/benchmarks/baseline.json).
    bool LoadBaseline(const wchar_t* baselinePath) noexcept;

    // Evaluate current samples against baseline.
    // Returns one GPUGateResult per metric that was checked.
    std::vector<GPUGateResult> Evaluate(
        const std::vector<PerformanceSample>& current) const noexcept;

    // Overall verdict: worst across all results.
    GPUGateVerdict OverallVerdict(const std::vector<GPUGateResult>& results) const noexcept;

    // Save current samples as the new baseline.
    bool SaveBaseline(
        const std::vector<PerformanceSample>& samples,
        const wchar_t*                        outputPath) const noexcept;

    // Convenience: returns true if all metrics pass (no blocking regressions).
    bool AllPass(const std::vector<PerformanceSample>& current) const noexcept;

    const GateThresholds& GetThresholds() const noexcept { return m_thresholds; }

private:
    GateThresholds                                   m_thresholds;
    std::unordered_map<std::string, PerformanceSample> m_baseline;
    bool                                             m_baselineLoaded = false;
};

}} // namespace ExplorerLens::Engine
