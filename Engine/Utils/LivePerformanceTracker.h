// LivePerformanceTracker.h — Real-Time Performance Metrics Dashboard Data
// Copyright (c) 2026 ExplorerLens Project
//
// Records and exposes real-time performance samples for FPS, decode latency,
// and throughput, suitable for live dashboard visualization.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <algorithm>
#include <cstdint>
#include <deque>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct LivePerfSample
{
    uint64_t timestampMs = 0;
    double frameTimeMs = 0.0;
    double decodeTimeMs = 0.0;
    double renderTimeMs = 0.0;
    uint64_t memoryUsedBytes = 0;
    uint32_t activeThreads = 0;
};

struct PerfTimeSeries
{
    std::string metricName;
    std::deque<double> values;
    std::deque<uint64_t> timestamps;
    size_t maxSamples = 600;  // 10 minutes at 1 sample/sec

    void Add(double value, uint64_t timestampMs)
    {
        values.push_back(value);
        timestamps.push_back(timestampMs);
        while (values.size() > maxSamples) {
            values.pop_front();
            timestamps.pop_front();
        }
    }

    double Average() const
    {
        if (values.empty())
            return 0.0;
        double sum = 0.0;
        for (double v : values)
            sum += v;
        return sum / values.size();
    }

    double Peak() const
    {
        if (values.empty())
            return 0.0;
        return *std::max_element(values.begin(), values.end());
    }

    double Latest() const
    {
        return values.empty() ? 0.0 : values.back();
    }
};

class LivePerformanceTracker
{
  public:
    static LivePerformanceTracker& Instance()
    {
        static LivePerformanceTracker s;
        return s;
    }

    void RecordSample(const LivePerfSample& sample)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        LivePerfSample s = sample;
        s.timestampMs = GetTickCount64();

        m_samples.push_back(s);
        while (m_samples.size() > m_maxSamples) {
            m_samples.pop_front();
        }

        m_fpsSeries.Add(s.frameTimeMs > 0 ? 1000.0 / s.frameTimeMs : 0.0, s.timestampMs);
        m_decodeSeries.Add(s.decodeTimeMs, s.timestampMs);
        m_renderSeries.Add(s.renderTimeMs, s.timestampMs);
        m_memorySeries.Add(static_cast<double>(s.memoryUsedBytes) / (1024.0 * 1024.0), s.timestampMs);

        m_totalSamplesRecorded++;
    }

    PerfTimeSeries GetTimeSeries(const std::string& metric) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (metric == "fps")
            return m_fpsSeries;
        if (metric == "decode")
            return m_decodeSeries;
        if (metric == "render")
            return m_renderSeries;
        if (metric == "memory")
            return m_memorySeries;
        return PerfTimeSeries{};
    }

    double GetCurrentFPS() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Average FPS over last 10 samples
        if (m_fpsSeries.values.empty())
            return 0.0;
        size_t count = (std::min)(m_fpsSeries.values.size(), static_cast<size_t>(10));
        double sum = 0.0;
        auto it = m_fpsSeries.values.end();
        for (size_t i = 0; i < count; ++i) {
            --it;
            sum += *it;
        }
        return sum / count;
    }

    double GetAvgDecodeTime() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_decodeSeries.Average();
    }

    double GetPeakDecodeTime() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_decodeSeries.Peak();
    }

    double GetCurrentMemoryMB() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_memorySeries.Latest();
    }

    LivePerfSample GetLatestSample() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_samples.empty() ? LivePerfSample{} : m_samples.back();
    }

    uint64_t GetTotalSamples() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_totalSamplesRecorded;
    }

    void SetMaxSamples(size_t max)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxSamples = max;
        m_fpsSeries.maxSamples = max;
        m_decodeSeries.maxSamples = max;
        m_renderSeries.maxSamples = max;
        m_memorySeries.maxSamples = max;
    }

    void Reset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_samples.clear();
        m_fpsSeries = PerfTimeSeries{};
        m_fpsSeries.metricName = "fps";
        m_decodeSeries = PerfTimeSeries{};
        m_decodeSeries.metricName = "decode";
        m_renderSeries = PerfTimeSeries{};
        m_renderSeries.metricName = "render";
        m_memorySeries = PerfTimeSeries{};
        m_memorySeries.metricName = "memory";
        m_totalSamplesRecorded = 0;
    }

    bool Validate() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_samples.size() > m_maxSamples)
            return false;
        for (const auto& s : m_samples) {
            if (s.frameTimeMs < 0.0)
                return false;
            if (s.decodeTimeMs < 0.0)
                return false;
        }
        return true;
    }

  private:
    LivePerformanceTracker()
    {
        m_fpsSeries.metricName = "fps";
        m_decodeSeries.metricName = "decode";
        m_renderSeries.metricName = "render";
        m_memorySeries.metricName = "memory";
    }
    ~LivePerformanceTracker() = default;
    LivePerformanceTracker(const LivePerformanceTracker&) = delete;
    LivePerformanceTracker& operator=(const LivePerformanceTracker&) = delete;

    mutable std::mutex m_mutex;
    std::deque<LivePerfSample> m_samples;
    size_t m_maxSamples = 600;
    PerfTimeSeries m_fpsSeries;
    PerfTimeSeries m_decodeSeries;
    PerfTimeSeries m_renderSeries;
    PerfTimeSeries m_memorySeries;
    uint64_t m_totalSamplesRecorded = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
