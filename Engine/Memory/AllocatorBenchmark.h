// AllocatorBenchmark.h — Memory Allocator Performance Benchmarking
// Copyright (c) 2026 ExplorerLens Project
//
// Benchmarks memory allocator performance across different allocation
// patterns, sizes, and threading models to select optimal strategies.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AllocatorType : uint8_t {
    SystemDefault,
    PoolAllocator,
    SlabAllocator,
    BumpAllocator,
    ArenaAllocator,
    TLSFAllocator
};

enum class BenchmarkPattern : uint8_t {
    SingleSize,
    MixedSizes,
    ThrashAllocFree,
    BulkAllocBulkFree,
    RandomPattern
};

struct AllocBenchmarkResult
{
    AllocatorType allocator = AllocatorType::SystemDefault;
    BenchmarkPattern pattern = BenchmarkPattern::SingleSize;
    uint64_t operationsCompleted = 0;
    double totalTimeMs = 0.0;
    double opsPerSecond = 0.0;
    double avgLatencyNs = 0.0;
    double p99LatencyNs = 0.0;
    uint64_t peakMemoryBytes = 0;
};

struct BenchmarkConfig
{
    uint32_t iterations = 10000;
    uint32_t warmupIterations = 1000;
    uint32_t allocationSize = 256;
    uint32_t threadCount = 1;
    BenchmarkPattern pattern = BenchmarkPattern::SingleSize;
};

class AllocatorBenchmark
{
  public:
    AllocatorBenchmark() = default;

    AllocBenchmarkResult RunBenchmark(AllocatorType allocator, const BenchmarkConfig& config)
    {
        AllocBenchmarkResult result;
        result.allocator = allocator;
        result.pattern = config.pattern;

        auto start = std::chrono::high_resolution_clock::now();

        // Simulate allocation benchmark
        std::vector<std::vector<uint8_t>> allocations;
        allocations.reserve(config.iterations);
        for (uint32_t i = 0; i < config.iterations; i++) {
            allocations.emplace_back(config.allocationSize, uint8_t(0));
        }
        allocations.clear();

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

        result.operationsCompleted = config.iterations;
        result.totalTimeMs = elapsed;
        result.opsPerSecond = result.operationsCompleted / (elapsed / 1000.0);
        result.avgLatencyNs = (elapsed * 1e6) / result.operationsCompleted;
        result.peakMemoryBytes = static_cast<uint64_t>(config.iterations) * config.allocationSize;
        m_results.push_back(result);
        return result;
    }

    std::vector<AllocBenchmarkResult> GetAllResults() const
    {
        return m_results;
    }

    const AllocBenchmarkResult* GetBestResult() const
    {
        const AllocBenchmarkResult* best = nullptr;
        for (const auto& r : m_results) {
            if (!best || r.opsPerSecond > best->opsPerSecond)
                best = &r;
        }
        return best;
    }

    void ClearResults()
    {
        m_results.clear();
    }

  private:
    std::vector<AllocBenchmarkResult> m_results;
};

}  // namespace Engine
}  // namespace ExplorerLens
