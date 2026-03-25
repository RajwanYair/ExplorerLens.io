// BenchmarkCommand.h — lens benchmark: Throughput & Latency Benchmarking
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 22 (v15.4.0 "Zenith-U"): Reports decode throughput (img/sec) and
// latency percentiles (p50, p95, p99) for each supported format category.
// Reads from a configurable file corpus or uses built-in synthetic workload.
//
#pragma once
#include "CommandRouter.h"
#include <chrono>
#include <vector>

namespace ExplorerLens {
namespace CLI {

struct BenchmarkResult {
    std::wstring category;
    uint32_t     sampleCount = 0;
    double       p50Ms       = 0.0;
    double       p95Ms       = 0.0;
    double       p99Ms       = 0.0;
    double       meanMs      = 0.0;
    double       throughput  = 0.0;   // img/sec
};

class BenchmarkCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"benchmark"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Measure thumbnail decode throughput and latency";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens benchmark [--corpus <dir>] [--iterations <n>] [--json]";
    }

    // Runs the full synthetic benchmark over all format categories.
    // iterations controls how many samples per category (min 1).
    // Used by unit tests to verify result structure without real file I/O.
    std::vector<BenchmarkResult> RunSyntheticBenchmark(uint32_t iterations = 10);

private:
    BenchmarkResult RunCategoryBenchmark(const std::wstring& category,
                                         uint32_t iterations);
    void PrintTextResults(const std::vector<BenchmarkResult>& results) const;
    void PrintJsonResults(const std::vector<BenchmarkResult>& results) const;
};

} // namespace CLI
} // namespace ExplorerLens
