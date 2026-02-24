//==============================================================================
// ExplorerLens Engine — Parallel Batch Decode
// Activate ParallelBatchDecoder with thread pool and per-format concurrency.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace ExplorerLens { namespace Engine {

/// Batch decode scheduling policy
enum class BatchPolicy : uint8_t {
    RoundRobin,       // Distribute evenly across threads
    FormatGrouped,    // Group same-format files together
    SizeOrdered,      // Smallest files first (quickest wins)
    PriorityBased,    // Honor per-file priority
    Adaptive          // Runtime selection based on workload
};

/// Batch decode statistics
struct BatchDecodeStats {
    uint32_t totalFiles       = 0;
    uint32_t completed        = 0;
    uint32_t failed           = 0;
    uint32_t skipped          = 0;
    uint32_t threadCount      = 0;
    double   throughput       = 0;    // images/sec
    double   avgDecodeMs      = 0;    // avg per-file decode time
    double   totalTimeMs      = 0;    // wall clock time
    double   cpuUtilization   = 0;    // 0.0-1.0
};

/// Batch decode configuration
struct BatchDecodeConfig {
    uint32_t maxThreads         = 4;
    uint32_t maxQueueDepth      = 1024;
    uint32_t batchSize          = 64;     // files per batch submission
    uint32_t timeoutPerFileMs   = 5000;
    uint32_t maxFileSizeMB      = 256;
    BatchPolicy policy          = BatchPolicy::Adaptive;
    bool     enableGPU          = true;
    bool     skipOnError        = true;
};

/// Parallel batch decoder
class ParallelBatchProcessor {
public:
    /// Policy display name
    static const wchar_t* PolicyName(BatchPolicy p) {
        switch (p) {
            case BatchPolicy::RoundRobin:    return L"Round Robin";
            case BatchPolicy::FormatGrouped: return L"Format Grouped";
            case BatchPolicy::SizeOrdered:   return L"Size Ordered";
            case BatchPolicy::PriorityBased: return L"Priority Based";
            case BatchPolicy::Adaptive:      return L"Adaptive";
            default: return L"Unknown";
        }
    }

    /// Optimal thread count based on CPU cores
    static uint32_t OptimalThreadCount(uint32_t cpuCores) {
        if (cpuCores <= 2) return 2;
        if (cpuCores <= 4) return 3;
        if (cpuCores <= 8) return 6;
        return cpuCores - 2; // Leave headroom
    }

    /// Validate config
    static bool ValidateConfig(const BatchDecodeConfig& cfg) {
        if (cfg.maxThreads == 0 || cfg.maxThreads > 64) return false;
        if (cfg.maxQueueDepth == 0) return false;
        if (cfg.batchSize == 0 || cfg.batchSize > cfg.maxQueueDepth) return false;
        if (cfg.timeoutPerFileMs < 500) return false;
        return true;
    }

    /// Policy count
    static constexpr size_t PolicyCount() { return 5; }

    /// Calculate throughput
    static double CalculateThroughput(uint32_t filesDecoded, double totalTimeMs) {
        if (totalTimeMs <= 0) return 0;
        return (filesDecoded * 1000.0) / totalTimeMs;
    }
};

}} // namespace ExplorerLens::Engine

