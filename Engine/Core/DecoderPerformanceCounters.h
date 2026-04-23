// DecoderPerformanceCounters.h — Per-format decode performance metrics
// Copyright (c) 2026 ExplorerLens Project
//
// Collects rolling P50/P95/P99 latency histograms and throughput counts for
// each decoder. Thread-safe via atomic operations. Designed for zero overhead
// when disabled via the ENABLE_PERF_COUNTERS compile flag.
//
#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// LatencyHistogram — compact rolling histogram (lock-free, 64 buckets)
// ---------------------------------------------------------------------------

class LatencyHistogram {
public:
    // Bucket boundaries (milliseconds): 0-1, 1-2, 2-4, 4-8 … up to >2048 ms
    static constexpr size_t BUCKET_COUNT = 64;

    void Record(double ms) noexcept;

    double Percentile(double p) const noexcept;  // p in [0,1]
    double P50() const noexcept { return Percentile(0.50); }
    double P95() const noexcept { return Percentile(0.95); }
    double P99() const noexcept { return Percentile(0.99); }

    uint64_t SampleCount() const noexcept;
    double   Mean() const noexcept;
    void     Reset() noexcept;

private:
    static int BucketFor(double ms) noexcept;
    mutable std::array<std::atomic<uint64_t>, BUCKET_COUNT> m_buckets{};
    mutable std::atomic<uint64_t> m_sumUs{0};
};

// ---------------------------------------------------------------------------
// FormatCounters — per-format decode counters
// ---------------------------------------------------------------------------

struct FormatCounters {
    std::atomic<uint64_t> decodeAttempts{0};
    std::atomic<uint64_t> decodeSuccesses{0};
    std::atomic<uint64_t> decodeFailures{0};
    std::atomic<uint64_t> cacheHits{0};
    std::atomic<uint64_t> cacheMisses{0};
    std::atomic<uint64_t> gpuDecodes{0};
    std::atomic<uint64_t> cpuDecodes{0};
    std::atomic<uint64_t> bytesDecoded{0};

    LatencyHistogram coldLatency;      // First-decode latency (ms)
    LatencyHistogram warmLatency;      // Cache-hit latency (ms)

    double HitRate() const noexcept {
        uint64_t h = cacheHits.load(std::memory_order_relaxed);
        uint64_t m = cacheMisses.load(std::memory_order_relaxed);
        uint64_t t = h + m;
        return t == 0 ? 0.0 : static_cast<double>(h) / static_cast<double>(t);
    }

    double ErrorRate() const noexcept {
        uint64_t a = decodeAttempts.load(std::memory_order_relaxed);
        uint64_t f = decodeFailures.load(std::memory_order_relaxed);
        return a == 0 ? 0.0 : static_cast<double>(f) / static_cast<double>(a);
    }

    void Reset() noexcept {
        decodeAttempts  .store(0, std::memory_order_relaxed);
        decodeSuccesses .store(0, std::memory_order_relaxed);
        decodeFailures  .store(0, std::memory_order_relaxed);
        cacheHits       .store(0, std::memory_order_relaxed);
        cacheMisses     .store(0, std::memory_order_relaxed);
        gpuDecodes      .store(0, std::memory_order_relaxed);
        cpuDecodes      .store(0, std::memory_order_relaxed);
        bytesDecoded    .store(0, std::memory_order_relaxed);
        coldLatency.Reset();
        warmLatency.Reset();
    }
};

// ---------------------------------------------------------------------------
// PerFormatReport — snapshot for JSON/ETW export
// ---------------------------------------------------------------------------

struct PerFormatReport {
    std::string format;
    uint64_t    totalAttempts  = 0;
    uint64_t    totalSuccesses = 0;
    uint64_t    totalFailures  = 0;
    double      hitRate        = 0.0;
    double      errorRate      = 0.0;
    double      coldP50Ms      = 0.0;
    double      coldP95Ms      = 0.0;
    double      coldP99Ms      = 0.0;
    double      warmP50Ms      = 0.0;
    double      coldMeanMs     = 0.0;
    uint64_t    gpuDecodes     = 0;
    uint64_t    cpuDecodes     = 0;
    std::string Serialize() const;  // Returns JSON object string
};

// ---------------------------------------------------------------------------
// DecoderPerformanceCounters — the central metrics service (singleton)
// ---------------------------------------------------------------------------

class DecoderPerformanceCounters {
public:
    static DecoderPerformanceCounters& Instance() noexcept;

    // Record a completed cold decode (no cache hit).
    void RecordDecode(
        std::string_view format,
        double           latencyMs,
        bool             success,
        bool             usedGpu = false,
        uint64_t         bytesDecoded = 0) noexcept;

    // Record a cache hit with retrieval latency.
    void RecordCacheHit(std::string_view format, double latencyMs) noexcept;

    // Record a cache miss (decode will follow).
    void RecordCacheMiss(std::string_view format) noexcept;

    // Get the counters for a specific format (creates if absent).
    const FormatCounters* GetCounters(std::string_view format) const noexcept;

    // Generate a snapshot report for all tracked formats.
    std::vector<PerFormatReport> GenerateReport() const;

    // Serialize all counters to a JSON string.
    std::string SerializeJson() const;

    // Reset all counters (e.g., at start of benchmark run).
    void ResetAll() noexcept;

    // Returns total decode attempts across all formats.
    uint64_t TotalAttempts() const noexcept;
    uint64_t TotalSuccesses() const noexcept;
    uint64_t TotalFailures() const noexcept;

private:
    DecoderPerformanceCounters() = default;
    FormatCounters& GetOrCreate(std::string_view format) noexcept;

    // Per-format counters. In practice the number of formats is small (<50),
    // so a simple map with a shared mutex is fine here.
    mutable std::atomic_flag                            m_lock = ATOMIC_FLAG_INIT;
    std::unordered_map<std::string, std::unique_ptr<FormatCounters>> m_counters;
};

// ---------------------------------------------------------------------------
// RAII decode timer — auto-records on destruction
// ---------------------------------------------------------------------------

class ScopedDecodeTimer {
public:
    ScopedDecodeTimer(std::string_view format, bool gpuPath = false) noexcept
        : m_format(format)
        , m_gpuPath(gpuPath)
        , m_start(std::chrono::steady_clock::now())
    {}

    // Call on decode success.
    void Succeed(uint64_t bytesDecoded = 0) noexcept {
        m_succeeded  = true;
        m_bytes      = bytesDecoded;
    }

    ~ScopedDecodeTimer() noexcept {
        auto end  = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - m_start).count();
        DecoderPerformanceCounters::Instance().RecordDecode(
            m_format, ms, m_succeeded, m_gpuPath, m_bytes);
    }

private:
    std::string_view                         m_format;
    bool                                     m_gpuPath   = false;
    bool                                     m_succeeded = false;
    uint64_t                                 m_bytes     = 0;
    std::chrono::steady_clock::time_point    m_start;
};

} // namespace Engine
} // namespace ExplorerLens
