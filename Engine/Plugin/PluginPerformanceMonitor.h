// PluginPerformanceMonitor.h — Real-Time Plugin Performance Tracking
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors plugin execution performance including decode latency,
// memory consumption, CPU time, and throughput. Flags slow plugins
// and provides data for adaptive timeout and priority decisions.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class PluginPerfMetric : uint8_t {
    DecodeLatency, MemoryUsage, CPUTime, Throughput, ErrorRate, COUNT
};

enum class PluginPerfGrade : uint8_t {
    Excellent, Good, Acceptable, Poor, Failing, COUNT
};

struct PluginPerfSnapshot {
    std::wstring pluginId;
    double avgLatencyMs = 0.0;
    double p99LatencyMs = 0.0;
    size_t peakMemoryKB = 0;
    double cpuTimeMs = 0.0;
    double throughputPerSec = 0.0;
    float errorRate = 0.0f;
    uint32_t invocationCount = 0;
    PluginPerfGrade grade = PluginPerfGrade::Good;
};

struct PluginPerfThresholds {
    double maxAvgLatencyMs = 100.0;
    double maxP99LatencyMs = 500.0;
    size_t maxMemoryKB = 512 * 1024;
    float maxErrorRate = 0.05f;
    double minThroughput = 10.0;
};

class PluginPerformanceMonitor {
public:
    void SetThresholds(const PluginPerfThresholds& t) { m_thresholds = t; }
    const PluginPerfThresholds& GetThresholds() const { return m_thresholds; }

    PluginPerfGrade Grade(const PluginPerfSnapshot& snap) const {
        if (snap.avgLatencyMs > m_thresholds.maxAvgLatencyMs * 3 ||
            snap.errorRate > m_thresholds.maxErrorRate * 3)
            return PluginPerfGrade::Failing;
        if (snap.avgLatencyMs > m_thresholds.maxAvgLatencyMs * 2 ||
            snap.errorRate > m_thresholds.maxErrorRate * 2)
            return PluginPerfGrade::Poor;
        if (snap.avgLatencyMs > m_thresholds.maxAvgLatencyMs ||
            snap.errorRate > m_thresholds.maxErrorRate)
            return PluginPerfGrade::Acceptable;
        if (snap.avgLatencyMs < m_thresholds.maxAvgLatencyMs * 0.5 &&
            snap.errorRate < m_thresholds.maxErrorRate * 0.5)
            return PluginPerfGrade::Excellent;
        return PluginPerfGrade::Good;
    }

    void RecordInvocation(const std::wstring& pluginId, double latencyMs, size_t memKB) {
        m_lastPluginId = pluginId;
        m_totalInvocations++;
        m_totalLatencyMs += latencyMs;
        if (memKB > m_peakMemKB) m_peakMemKB = memKB;
    }

    uint32_t TotalInvocations() const { return m_totalInvocations; }
    double AvgLatency() const {
        return m_totalInvocations > 0 ? m_totalLatencyMs / m_totalInvocations : 0.0;
    }
    size_t PeakMemoryKB() const { return m_peakMemKB; }

    void Reset() {
        m_totalInvocations = 0;
        m_totalLatencyMs = 0.0;
        m_peakMemKB = 0;
    }

    static size_t MetricCount() { return static_cast<size_t>(PluginPerfMetric::COUNT); }
    static size_t GradeCount() { return static_cast<size_t>(PluginPerfGrade::COUNT); }

private:
    PluginPerfThresholds m_thresholds;
    std::wstring m_lastPluginId;
    uint32_t m_totalInvocations = 0;
    double m_totalLatencyMs = 0.0;
    size_t m_peakMemKB = 0;
};

} // namespace Engine
} // namespace ExplorerLens
