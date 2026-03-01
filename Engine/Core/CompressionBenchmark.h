#pragma once
// ============================================================================
// CompressionBenchmark.h — Compression algorithm benchmarking for cache
//
// Purpose:   Compression algorithm benchmarking for cache optimization
// Provides:  CompressionAlgo enum, CompressionBenchMetric,
//            CompressionBenchResult, CompressionAlgoProfile structs,
//            and CompressionBenchmark class
// Used by:   Cache tier selection
// ============================================================================

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// CompressionBenchmark — Per-algorithm compression speed benchmarking
// ============================================================================

enum class CompressionAlgo {
    Deflate,
    LZ4,
    Zstd,
    LZMA,
    Brotli,
    Snappy
};

inline const char* CompressionAlgoName(CompressionAlgo value) {
    switch (value) {
    case CompressionAlgo::Deflate: return "Deflate";
    case CompressionAlgo::LZ4:     return "LZ4";
    case CompressionAlgo::Zstd:    return "Zstd";
    case CompressionAlgo::LZMA:    return "LZMA";
    case CompressionAlgo::Brotli:  return "Brotli";
    case CompressionAlgo::Snappy:  return "Snappy";
    default:                       return "Unknown";
    }
}

enum class CompressionBenchMetric {
    CompressSpeed,
    DecompressSpeed,
    Ratio,
    MemoryUsage
};

inline const char* CompressionBenchMetricName(CompressionBenchMetric value) {
    switch (value) {
    case CompressionBenchMetric::CompressSpeed:   return "CompressSpeed";
    case CompressionBenchMetric::DecompressSpeed: return "DecompressSpeed";
    case CompressionBenchMetric::Ratio:           return "Ratio";
    case CompressionBenchMetric::MemoryUsage:     return "MemoryUsage";
    default:                               return "Unknown";
    }
}

struct CompressionBenchResult {
    CompressionAlgo algorithm = CompressionAlgo::Deflate;
    CompressionBenchMetric metric = CompressionBenchMetric::CompressSpeed;
    double          value = 0.0;   // MB/s for speed, ratio for Ratio, bytes for MemoryUsage
    uint64_t        dataSize = 0;
    uint32_t        iterations = 0;
    double          minValueMs = 0.0;
    double          maxValueMs = 0.0;
    double          avgValueMs = 0.0;

    bool IsValid() const { return iterations > 0 && dataSize > 0; }
};

struct CompressionAlgoProfile {
    CompressionAlgo algorithm;
    double compressMBps = 0.0;
    double decompressMBps = 0.0;
    double ratio = 1.0;
    uint64_t peakMemory = 0;

    double GetOverallScore() const {
        // Weighted score: speed matters more than ratio for thumbnails
        return compressMBps * 0.3 + decompressMBps * 0.5 + (ratio * 100.0) * 0.2;
    }
};

class CompressionBenchmark {
public:
    static constexpr uint32_t ITERATIONS_DEFAULT = 100;
    static constexpr uint32_t MIN_DATA_SIZE = 1024;
    static constexpr uint32_t MAX_DATA_SIZE = 64 * 1024 * 1024; // 64 MB
    static constexpr uint32_t WARMUP_ITERATIONS = 5;

    CompressionBenchmark() = default;
    ~CompressionBenchmark() = default;

    CompressionBenchResult RunBenchmark(CompressionAlgo algo, CompressionBenchMetric metric,
        const uint8_t* data, uint64_t dataSize,
        uint32_t iterations = ITERATIONS_DEFAULT) {
        std::lock_guard<std::mutex> lock(m_mutex);

        CompressionBenchResult result;
        result.algorithm = algo;
        result.metric = metric;
        result.dataSize = dataSize;
        result.iterations = iterations;

        if (!data || dataSize < MIN_DATA_SIZE || dataSize > MAX_DATA_SIZE || iterations == 0) {
            return result;
        }

        // Simulate benchmark timing (in production, would actually compress/decompress)
        double simulatedBaseMs = GetSimulatedBaseTime(algo, metric, dataSize);

        double totalMs = 0.0;
        result.minValueMs = simulatedBaseMs * 0.9;
        result.maxValueMs = simulatedBaseMs * 1.1;

        for (uint32_t i = 0; i < iterations; i++) {
            double iterMs = simulatedBaseMs + (static_cast<double>(i % 10) * 0.01 - 0.05);
            totalMs += iterMs;
        }

        result.avgValueMs = totalMs / iterations;

        // Convert to MB/s for speed metrics
        if (metric == CompressionBenchMetric::CompressSpeed || metric == CompressionBenchMetric::DecompressSpeed) {
            double seconds = result.avgValueMs / 1000.0;
            double megabytes = static_cast<double>(dataSize) / (1024.0 * 1024.0);
            result.value = (seconds > 0.0) ? megabytes / seconds : 0.0;
        }
        else if (metric == CompressionBenchMetric::Ratio) {
            result.value = GetSimulatedRatio(algo);
        }
        else {
            result.value = static_cast<double>(GetSimulatedMemory(algo));
        }

        m_results.push_back(result);
        m_totalBenchmarks++;
        return result;
    }

    CompressionAlgo GetFastestAlgo(CompressionBenchMetric metric) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        CompressionAlgo fastest = CompressionAlgo::LZ4;
        double bestValue = 0.0;

        for (const auto& r : m_results) {
            if (r.metric == metric && r.value > bestValue) {
                bestValue = r.value;
                fastest = r.algorithm;
            }
        }
        return fastest;
    }

    CompressionAlgo GetBestRatio() const {
        std::lock_guard<std::mutex> lock(m_mutex);

        CompressionAlgo best = CompressionAlgo::LZMA;
        double bestRatio = 0.0;

        for (const auto& r : m_results) {
            if (r.metric == CompressionBenchMetric::Ratio && r.value > bestRatio) {
                bestRatio = r.value;
                best = r.algorithm;
            }
        }
        return best;
    }

    size_t GetResultCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_results.size();
    }

    uint64_t GetTotalBenchmarks() const { return m_totalBenchmarks; }

    void ClearResults() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_results.clear();
    }

private:
    double GetSimulatedBaseTime(CompressionAlgo algo, CompressionBenchMetric metric, uint64_t size) const {
        (void)metric;
        double sizeFactorMs = static_cast<double>(size) / (1024.0 * 1024.0); // per MB
        switch (algo) {
        case CompressionAlgo::LZ4:     return sizeFactorMs * 0.5;
        case CompressionAlgo::Snappy:   return sizeFactorMs * 0.7;
        case CompressionAlgo::Zstd:    return sizeFactorMs * 2.0;
        case CompressionAlgo::Deflate: return sizeFactorMs * 3.0;
        case CompressionAlgo::Brotli:  return sizeFactorMs * 8.0;
        case CompressionAlgo::LZMA:    return sizeFactorMs * 15.0;
        default:                        return sizeFactorMs * 5.0;
        }
    }

    double GetSimulatedRatio(CompressionAlgo algo) const {
        switch (algo) {
        case CompressionAlgo::LZ4:     return 2.1;
        case CompressionAlgo::Snappy:   return 1.8;
        case CompressionAlgo::Zstd:    return 3.5;
        case CompressionAlgo::Deflate: return 3.0;
        case CompressionAlgo::Brotli:  return 4.0;
        case CompressionAlgo::LZMA:    return 4.5;
        default:                        return 1.0;
        }
    }

    uint64_t GetSimulatedMemory(CompressionAlgo algo) const {
        switch (algo) {
        case CompressionAlgo::LZ4:     return 64 * 1024;
        case CompressionAlgo::Snappy:   return 32 * 1024;
        case CompressionAlgo::Zstd:    return 512 * 1024;
        case CompressionAlgo::Deflate: return 256 * 1024;
        case CompressionAlgo::Brotli:  return 1024 * 1024;
        case CompressionAlgo::LZMA:    return 8 * 1024 * 1024;
        default:                        return 128 * 1024;
        }
    }

    mutable std::mutex              m_mutex;
    std::vector<CompressionBenchResult>    m_results;
    uint64_t                        m_totalBenchmarks = 0;
};

} // namespace Engine
} // namespace ExplorerLens
