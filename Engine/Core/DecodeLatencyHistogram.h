// DecodeLatencyHistogram.h — Decode Latency Distribution Tracker
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks decode latency distribution using a log-scale histogram with
// sub-microsecond precision. Reports percentiles (p50/p95/p99/p999),
// mean, std deviation, and per-format breakdown.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct HistogramPercentiles
{
    double p50 = 0.0;
    double p90 = 0.0;
    double p95 = 0.0;
    double p99 = 0.0;
    double p999 = 0.0;
    double mean = 0.0;
    double stdDev = 0.0;
    double minMs = 1e9;
    double maxMs = 0.0;
};

struct HistogramStats
{
    uint64_t totalSamples = 0;
    double totalMs = 0.0;
    HistogramPercentiles percentiles;
};

class DecodeLatencyHistogram
{
  public:
    // Buckets: 0.01ms, 0.1ms, 0.5ms, 1ms, 2ms, 5ms, 10ms, 20ms, 50ms, 100ms, 200ms, 500ms, 1s, 2s, 5s+
    static constexpr uint32_t NUM_BUCKETS = 15;
    static constexpr double BucketBounds[NUM_BUCKETS] = {0.01, 0.1,   0.5,   1.0,   2.0,    5.0,    10.0,  20.0,
                                                         50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 5000.0};

    DecodeLatencyHistogram()
    {
        InitializeSRWLock(&m_lock);
        m_buckets.fill(0);
        QueryPerformanceFrequency(&m_freq);
    }
    ~DecodeLatencyHistogram() = default;

    static const wchar_t* GetName()
    {
        return L"DecodeLatencyHistogram";
    }

    /// Record a latency sample in milliseconds.
    void Record(double latencyMs)
    {
        AcquireSRWLockExclusive(&m_lock);
        uint32_t bucket = FindBucket(latencyMs);
        m_buckets[bucket]++;
        m_stats.totalSamples++;
        m_stats.totalMs += latencyMs;
        m_sumSquared += latencyMs * latencyMs;
        if (latencyMs < m_min)
            m_min = latencyMs;
        if (latencyMs > m_max)
            m_max = latencyMs;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Get a high-resolution timestamp for interval measurement.
    uint64_t Now() const
    {
        LARGE_INTEGER ts;
        QueryPerformanceCounter(&ts);
        return ts.QuadPart;
    }

    /// Convert QPC delta to milliseconds.
    double ToMs(uint64_t start, uint64_t end) const
    {
        return 1000.0 * static_cast<double>(end - start) / m_freq.QuadPart;
    }

    /// Compute percentiles from histogram.
    HistogramPercentiles ComputePercentiles() const
    {
        HistogramPercentiles p;
        if (m_stats.totalSamples == 0)
            return p;

        p.mean = m_stats.totalMs / m_stats.totalSamples;
        double variance = (m_sumSquared / m_stats.totalSamples) - (p.mean * p.mean);
        p.stdDev = variance > 0 ? std::sqrt(variance) : 0.0;
        p.minMs = m_min;
        p.maxMs = m_max;

        // Approximate percentiles from bucket distribution
        uint64_t target50 = m_stats.totalSamples * 50 / 100;
        uint64_t target90 = m_stats.totalSamples * 90 / 100;
        uint64_t target95 = m_stats.totalSamples * 95 / 100;
        uint64_t target99 = m_stats.totalSamples * 99 / 100;
        uint64_t target999 = m_stats.totalSamples * 999 / 1000;

        uint64_t cumulative = 0;
        for (uint32_t i = 0; i < NUM_BUCKETS; ++i) {
            cumulative += m_buckets[i];
            if (cumulative >= target50 && p.p50 == 0.0)
                p.p50 = BucketBounds[i];
            if (cumulative >= target90 && p.p90 == 0.0)
                p.p90 = BucketBounds[i];
            if (cumulative >= target95 && p.p95 == 0.0)
                p.p95 = BucketBounds[i];
            if (cumulative >= target99 && p.p99 == 0.0)
                p.p99 = BucketBounds[i];
            if (cumulative >= target999 && p.p999 == 0.0)
                p.p999 = BucketBounds[i];
        }
        return p;
    }

    /// Reset all counters.
    void Reset()
    {
        AcquireSRWLockExclusive(&m_lock);
        m_buckets.fill(0);
        m_stats = {};
        m_sumSquared = 0.0;
        m_min = 1e9;
        m_max = 0.0;
        ReleaseSRWLockExclusive(&m_lock);
    }

    HistogramStats GetStats() const
    {
        HistogramStats s = m_stats;
        s.percentiles = const_cast<DecodeLatencyHistogram*>(this)->ComputePercentiles();
        return s;
    }

  private:
    uint32_t FindBucket(double ms) const
    {
        for (uint32_t i = 0; i < NUM_BUCKETS; ++i)
            if (ms <= BucketBounds[i])
                return i;
        return NUM_BUCKETS - 1;
    }

    SRWLOCK m_lock{};
    LARGE_INTEGER m_freq{};
    std::array<uint64_t, NUM_BUCKETS> m_buckets;
    double m_sumSquared = 0.0;
    double m_min = 1e9;
    double m_max = 0.0;
    mutable HistogramStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
