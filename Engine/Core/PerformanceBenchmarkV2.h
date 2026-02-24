#pragma once
// Performance Benchmark V2 — benchmark harness with percentile statistics
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Type of benchmark workload
enum class BenchmarkType : uint32_t {
    SingleDecode  = 0,
    BatchDecode   = 1,
    CacheHit      = 2,
    GPURender     = 3,
    FormatConvert = 4,
    EndToEnd      = 5,
    COUNT         = 6
};

/// Result of a single benchmark run
struct BenchmarkResult {
    BenchmarkType type   = BenchmarkType::SingleDecode;
    std::wstring  label;
    uint32_t      iterations = 0;
    double        minMs      = 0.0;
    double        maxMs      = 0.0;
    double        meanMs     = 0.0;
    double        medianMs   = 0.0;
    double        p95Ms      = 0.0;
    double        p99Ms      = 0.0;
    double        stdDevMs   = 0.0;
};

/// Harness for running and reporting benchmarks
class PerformanceBenchmarkV2 {
public:
    PerformanceBenchmarkV2();

    static const wchar_t* GetBenchmarkTypeName(BenchmarkType type);
    static uint32_t GetBenchmarkTypeCount() { return static_cast<uint32_t>(BenchmarkType::COUNT); }

    /// Record a set of timing samples for a benchmark
    BenchmarkResult ComputeStats(const std::wstring& label, BenchmarkType type,
                                  const std::vector<double>& samplesMs);

    /// Store a completed result
    void AddResult(const BenchmarkResult& result);
    /// Get all results
    const std::vector<BenchmarkResult>& GetResults() const { return m_results; }
    /// Clear results
    void Reset();

    /// Check if a result meets the target threshold
    static bool MeetsTarget(const BenchmarkResult& result, double targetP95Ms);

private:
    std::vector<BenchmarkResult> m_results;
};

}} // namespace ExplorerLens::Engine

