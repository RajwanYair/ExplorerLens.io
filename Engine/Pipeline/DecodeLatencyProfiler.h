// DecodeLatencyProfiler.h — Per-Format Decode Latency Profiler
// Copyright (c) 2026 ExplorerLens Project
//
// Captures wall-clock P50/P95/P99 histograms per format tag at decode time.
// Uses a fixed-capacity ring buffer (256 samples per format) to bound memory.
// JSON report export is provided for CI ingestion.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

// Per-format statistics computed on demand from the ring buffer.
struct LatencyPercentiles
{
    std::string formatTag;
    uint32_t    sampleCount;
    double      p50Ms;
    double      p95Ms;
    double      p99Ms;
    double      minMs;
    double      maxMs;
    double      meanMs;
};

class DecodeLatencyProfiler
{
public:
    // Maximum ring-buffer capacity per format.
    static constexpr uint32_t RING_CAPACITY = 256;

    // Record a single decode latency for the given format tag.
    void RecordSample(const std::string& formatTag, double latencyMs);

    // Compute percentile statistics for one format.
    // Returns all-zero struct if no samples are recorded for the tag.
    LatencyPercentiles GetPercentiles(const std::string& formatTag) const;

    // Convenience helpers.
    double GetP50(const std::string& formatTag) const;
    double GetP95(const std::string& formatTag) const;
    double GetP99(const std::string& formatTag) const;

    // Clear all recorded samples.
    void Reset() noexcept { m_rings.clear(); }

    // Number of distinct format tags recorded.
    uint32_t FormatCount() const noexcept
    {
        return static_cast<uint32_t>(m_rings.size());
    }

    // Number of samples recorded for a specific format tag.
    uint32_t SampleCount(const std::string& formatTag) const noexcept;

    // Export all recorded percentile data as JSON.
    std::string ToJSON() const;

private:
    struct Ring
    {
        double   buf[RING_CAPACITY] = {};
        uint32_t head  = 0;  // next write index (wraps)
        uint32_t count = 0;  // number of valid entries

        void Push(double v) noexcept
        {
            buf[head % RING_CAPACITY] = v;
            ++head;
            if (count < RING_CAPACITY)
                ++count;
        }

        std::vector<double> Sorted() const
        {
            const uint32_t n = count;
            std::vector<double> v;
            v.reserve(n);
            uint32_t start = (count == RING_CAPACITY) ? head % RING_CAPACITY : 0;
            for (uint32_t i = 0; i < n; ++i)
                v.push_back(buf[(start + i) % RING_CAPACITY]);
            std::sort(v.begin(), v.end());
            return v;
        }

        static double Percentile(const std::vector<double>& sorted, double pct) noexcept
        {
            if (sorted.empty())
                return 0.0;
            const double idx = (pct / 100.0) * (sorted.size() - 1);
            const size_t lo  = static_cast<size_t>(idx);
            const size_t hi  = lo + 1 < sorted.size() ? lo + 1 : lo;
            const double frac = idx - lo;
            return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
        }
    };

    std::unordered_map<std::string, Ring> m_rings;
};

} // namespace Engine
} // namespace ExplorerLens
