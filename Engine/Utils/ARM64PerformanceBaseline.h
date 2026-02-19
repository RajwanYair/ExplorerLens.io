#pragma once
// Sprint 158 — ARM64 Performance Baseline
// Captures ARM64-specific throughput and latency baselines for CI trend tracking.
// Includes NEON vs scalar fallback comparison.

#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs::Platform {

// ─── KPI constants ────────────────────────────────────────────────────────────

struct ARM64KPIs {
    // Single thumbnail decode p95 (ms)
    static constexpr double kSingleThumbP95Ms       = 150.0;  // ARM64 initial target
    // Batch throughput (images/sec)
    static constexpr double kBatchThroughputImgSec  = 100.0;  // ARM64 initial target
    // Cache-hit average (ms)
    static constexpr double kCacheHitAvgMs          = 5.0;
    // Cold start (ms)
    static constexpr double kColdStartMs            = 750.0;  // ARM64 first load
};

// ─── Benchmark sample ─────────────────────────────────────────────────────────

struct BenchmarkSample {
    std::string label;
    double      minMs       { 0.0 };
    double      maxMs       { 0.0 };
    double      meanMs      { 0.0 };
    double      p95Ms       { 0.0 };
    double      p99Ms       { 0.0 };
    uint32_t    samples     { 0 };
    double      throughput  { 0.0 };  // images/sec for batch tests

    bool MeetsP95Budget(double budgetMs) const { return p95Ms <= budgetMs; }
};

// ─── NEON vs scalar comparison ────────────────────────────────────────────────

struct SIMDComparisonResult {
    std::string     operation;      // e.g., "RGB rescale 256x256"
    BenchmarkSample neonPath;
    BenchmarkSample scalarPath;
    double          speedupRatio    { 1.0 };  // neon / scalar (>1 = NEON faster)
    bool            neonWins        { false };

    static constexpr double kMinExpectedSpeedup = 1.0;  // NEON must not be slower

    std::string Summary() const {
        return operation + ": NEON=" + std::to_string(neonPath.meanMs) + "ms" +
               " scalar=" + std::to_string(scalarPath.meanMs) + "ms" +
               " ratio=" + std::to_string(speedupRatio) +
               (neonWins ? " [NEON wins]" : " [scalar faster!]");
    }
};

// ─── ARM64 baseline record ────────────────────────────────────────────────────

struct ARM64PerformanceBaseline {
    std::string                         capturedAt;     // ISO-8601 timestamp
    std::string                         deviceModel;    // e.g., "Surface Pro X"
    std::string                         cpuModel;       // e.g., "Qualcomm Snapdragon 8cx Gen 3"

    BenchmarkSample                     singleThumb;
    BenchmarkSample                     batchThroughput;
    BenchmarkSample                     cacheHit;
    BenchmarkSample                     coldStart;

    std::vector<SIMDComparisonResult>   simdComparisons;

    bool MeetsAllKPIs() const {
        return singleThumb.MeetsP95Budget(ARM64KPIs::kSingleThumbP95Ms) &&
               batchThroughput.throughput >= ARM64KPIs::kBatchThroughputImgSec &&
               cacheHit.MeetsP95Budget(ARM64KPIs::kCacheHitAvgMs) &&
               coldStart.MeetsP95Budget(ARM64KPIs::kColdStartMs);
    }

    std::string Summary() const {
        return "ARM64 Baseline: thumb.p95=" + std::to_string(singleThumb.p95Ms) + "ms" +
               " batch=" + std::to_string(batchThroughput.throughput) + "img/s" +
               " cache=" + std::to_string(cacheHit.meanMs) + "ms" +
               " cold=" + std::to_string(coldStart.meanMs) + "ms — " +
               (MeetsAllKPIs() ? "PASS" : "FAIL");
    }

    static ARM64PerformanceBaseline CreateMock() {
        ARM64PerformanceBaseline b;
        b.capturedAt   = "2026-02-19T00:00:00Z";
        b.deviceModel  = "Surface Pro X (emulated)";
        b.cpuModel     = "Qualcomm Snapdragon 8cx Gen 3";
        b.singleThumb  = { "SingleThumb", 15.0, 95.0, 42.0, 88.0, 93.0, 200 };
        b.batchThroughput = { "Batch20", 0.0, 0.0, 0.0, 0.0, 0.0, 20, 115.0 };
        b.cacheHit     = { "CacheHit", 0.5, 4.8, 2.1, 4.2, 4.6, 100 };
        b.coldStart    = { "ColdStart", 600.0, 720.0, 650.0, 710.0, 718.0, 5 };
        b.simdComparisons = {
            { "RGB rescale 256x256",
              { "NEON",   0.8, 2.1, 1.1, 1.8, 2.0, 500 },
              { "Scalar", 1.5, 4.2, 2.3, 3.8, 4.1, 500 },
              2.09, true },
        };
        return b;
    }
};

// ─── Trend comparator ────────────────────────────────────────────────────────

struct BaselineTrendResult {
    double  deltaP95Pct         { 0.0 };   // +ve = regression, -ve = improvement
    double  deltaThroughputPct  { 0.0 };
    bool    hasRegression       { false };

    static constexpr double kRegressionThreshold = 10.0;  // >10% = regression

    static BaselineTrendResult Compare(const ARM64PerformanceBaseline& prev,
                                       const ARM64PerformanceBaseline& curr) {
        BaselineTrendResult r;
        if (prev.singleThumb.p95Ms > 0) {
            r.deltaP95Pct = 100.0 * (curr.singleThumb.p95Ms - prev.singleThumb.p95Ms)
                            / prev.singleThumb.p95Ms;
        }
        if (prev.batchThroughput.throughput > 0) {
            r.deltaThroughputPct = 100.0 * (curr.batchThroughput.throughput - prev.batchThroughput.throughput)
                                   / prev.batchThroughput.throughput;
        }
        r.hasRegression = r.deltaP95Pct > kRegressionThreshold ||
                          r.deltaThroughputPct < -kRegressionThreshold;
        return r;
    }
};

} // namespace DarkThumbs::Platform
