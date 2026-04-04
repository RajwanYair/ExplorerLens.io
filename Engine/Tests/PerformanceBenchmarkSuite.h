// PerformanceBenchmarkSuite.h — Automated Latency/Throughput Benchmark
// Copyright (c) 2026 ExplorerLens Project
//
// Measures decode throughput, cache hit latency, and GPU upload performance
// across all registered formats. Outputs JSON results for CI regression gating.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct BenchmarkSample
{
    double minMs;
    double maxMs;
    double avgMs;
    double p50Ms;
    double p95Ms;
    double p99Ms;
    double throughputPerSec;
    uint32_t iterations;
};

struct FormatBenchmarkResult
{
    std::string extension;
    std::string decoderName;
    BenchmarkSample cold;  // first-decode, cache cold
    BenchmarkSample warm;  // cache warmed, repeat decodes
    bool gpuAccelerated;
    bool passed;  // true if p95 <= SLO budget
    double sloBudgetMs;
};

struct BenchmarkReport
{
    std::string timestamp;
    std::string engineVersion;
    std::string gpuName;
    std::vector<FormatBenchmarkResult> results;
    uint32_t passed;
    uint32_t failed;
    double overallThroughput;  // img/sec aggregate
};

class PerformanceBenchmarkSuite
{
  public:
    struct Config
    {
        std::string corpusDir;  // directory of test files per format
        uint32_t warmIterations{10};
        uint32_t coldIterations{3};
        bool includeGPUBench{true};
        bool includeCacheBench{true};
        bool emitJson{true};
        std::string jsonOutputPath;
    };

    explicit PerformanceBenchmarkSuite(Config cfg);

    // Run all benchmarks. Returns the complete report.
    [[nodiscard]] BenchmarkReport RunAll();

    // Run benchmark for a single format extension.
    [[nodiscard]] FormatBenchmarkResult RunFormat(const std::string& ext);

    // Compare current report against a baseline JSON — returns regression count.
    [[nodiscard]] int CompareWithBaseline(const BenchmarkReport& current, const std::string& baselineJsonPath,
                                          double regressionThresholdPct = 10.0);

    // Serialise a BenchmarkReport to JSON string.
    static std::string ToJson(const BenchmarkReport& report);

    // Load a previously saved report from JSON.
    static BenchmarkReport FromJson(const std::string& json);

  private:
    static BenchmarkSample MeasureDecodeStats(const std::string& filePath, uint32_t iterations, bool clearCache);

    Config m_cfg;
};

// Convenience macro for one-shot timing in test binaries
#define BENCH_MS(label, iterations, expr)                                                          \
    do {                                                                                           \
        auto _t0 = std::chrono::high_resolution_clock::now();                                      \
        for (uint32_t _i = 0; _i < (iterations); ++_i) {                                           \
            expr;                                                                                  \
        }                                                                                          \
        auto _t1 = std::chrono::high_resolution_clock::now();                                      \
        double _avg = std::chrono::duration<double, std::milli>(_t1 - _t0).count() / (iterations); \
        printf("[BENCH] %-32s avg=%.3f ms over %u iterations\n", (label), _avg, (iterations));     \
    } while (0)

}  // namespace Engine
}  // namespace ExplorerLens
