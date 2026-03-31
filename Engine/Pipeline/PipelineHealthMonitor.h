// PipelineHealthMonitor.h — Pipeline Health Monitoring and Alerting
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors thumbnail pipeline health by tracking throughput, error rate,
// latency percentiles, and queue depth. Generates alerts when metrics exceed
// configured thresholds. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PipelineAlertLevel : uint8_t {
    Normal,
    Warning,
    Degraded,
    Critical,
    Down
};

enum class PipelineHealthMetric : uint8_t {
    Throughput,
    ErrorRate,
    P50Latency,
    P95Latency,
    P99Latency,
    QueueDepth,
    MemoryPressure
};

struct PipelineHealthThresholds {
    float maxErrorRate = 0.05f;
    float maxP95LatencyMs = 100.0f;
    float maxP99LatencyMs = 500.0f;
    uint32_t maxQueueDepth = 1000;
    float minThroughputPerSec = 10.0f;
};

struct PipelineAlert {
    PipelineAlertLevel level = PipelineAlertLevel::Normal;
    PipelineHealthMetric metric = PipelineHealthMetric::Throughput;
    float currentValue = 0.0f;
    float thresholdValue = 0.0f;
    std::wstring description;
};

struct PipelineHealthSnapshot {
    PipelineAlertLevel overallHealth = PipelineAlertLevel::Normal;
    float currentThroughput = 0.0f;
    float currentErrorRate = 0.0f;
    float p50LatencyMs = 0.0f;
    float p95LatencyMs = 0.0f;
    float p99LatencyMs = 0.0f;
    uint32_t currentQueueDepth = 0;
    uint32_t activeAlertCount = 0;
};

struct PipelineHealthMonitorStats {
    uint64_t totalSamples = 0;
    uint64_t alertsGenerated = 0;
    uint64_t criticalAlerts = 0;
    PipelineAlertLevel peakAlertLevel = PipelineAlertLevel::Normal;
    bool initialized = false;
};

class PipelineHealthMonitor {
public:
    static PipelineHealthMonitor& Instance() {
        static PipelineHealthMonitor instance;
        return instance;
    }

    void Initialize(const PipelineHealthThresholds& thresholds = {}) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_thresholds = thresholds;
        m_latencies.clear();
        m_alerts.clear();
        m_snapshot = {};
        m_stats = {};
        m_stats.initialized = true;
    }

    void RecordSample(float latencyMs, bool success) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalSamples++;
        m_latencies.push_back(latencyMs);
        m_totalCount++;
        if (!success) m_errorCount++;

        if (m_latencies.size() > MAX_LATENCY_WINDOW) {
            m_latencies.erase(m_latencies.begin());
        }

        UpdateSnapshot();
    }

    PipelineHealthSnapshot GetSnapshot() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_snapshot;
    }

    std::vector<PipelineAlert> GetActiveAlerts() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_alerts;
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    PipelineHealthMonitorStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_latencies.clear();
        m_alerts.clear();
    }

private:
    PipelineHealthMonitor() = default;
    ~PipelineHealthMonitor() = default;
    PipelineHealthMonitor(const PipelineHealthMonitor&) = delete;
    PipelineHealthMonitor& operator=(const PipelineHealthMonitor&) = delete;

    static constexpr size_t MAX_LATENCY_WINDOW = 1000;

    void UpdateSnapshot() {
        m_alerts.clear();
        m_snapshot = {};

        if (m_latencies.empty()) return;

        std::vector<float> sorted = m_latencies;
        std::sort(sorted.begin(), sorted.end());

        size_t n = sorted.size();
        m_snapshot.p50LatencyMs = sorted[n * 50 / 100];
        m_snapshot.p95LatencyMs = sorted[n * 95 / 100];
        m_snapshot.p99LatencyMs = sorted[(std::min)(n * 99 / 100, n - 1)];

        if (m_totalCount > 0) {
            m_snapshot.currentErrorRate =
                static_cast<float>(m_errorCount) / static_cast<float>(m_totalCount);
        }

        m_snapshot.overallHealth = PipelineAlertLevel::Normal;

        if (m_snapshot.currentErrorRate > m_thresholds.maxErrorRate) {
            PipelineAlert alert;
            alert.level = PipelineAlertLevel::Degraded;
            alert.metric = PipelineHealthMetric::ErrorRate;
            alert.currentValue = m_snapshot.currentErrorRate;
            alert.thresholdValue = m_thresholds.maxErrorRate;
            m_alerts.push_back(alert);
            m_stats.alertsGenerated++;
            if (alert.level > m_snapshot.overallHealth)
                m_snapshot.overallHealth = alert.level;
        }

        if (m_snapshot.p95LatencyMs > m_thresholds.maxP95LatencyMs) {
            PipelineAlert alert;
            alert.level = PipelineAlertLevel::Warning;
            alert.metric = PipelineHealthMetric::P95Latency;
            alert.currentValue = m_snapshot.p95LatencyMs;
            alert.thresholdValue = m_thresholds.maxP95LatencyMs;
            m_alerts.push_back(alert);
            m_stats.alertsGenerated++;
            if (alert.level > m_snapshot.overallHealth)
                m_snapshot.overallHealth = alert.level;
        }

        if (m_snapshot.p99LatencyMs > m_thresholds.maxP99LatencyMs) {
            PipelineAlert alert;
            alert.level = PipelineAlertLevel::Critical;
            alert.metric = PipelineHealthMetric::P99Latency;
            alert.currentValue = m_snapshot.p99LatencyMs;
            alert.thresholdValue = m_thresholds.maxP99LatencyMs;
            m_alerts.push_back(alert);
            m_stats.alertsGenerated++;
            m_stats.criticalAlerts++;
            if (alert.level > m_snapshot.overallHealth)
                m_snapshot.overallHealth = alert.level;
        }

        m_snapshot.activeAlertCount = static_cast<uint32_t>(m_alerts.size());
        if (m_snapshot.overallHealth > m_stats.peakAlertLevel)
            m_stats.peakAlertLevel = m_snapshot.overallHealth;
    }

    mutable std::mutex m_mutex;
    PipelineHealthThresholds m_thresholds;
    std::vector<float> m_latencies;
    std::vector<PipelineAlert> m_alerts;
    PipelineHealthSnapshot m_snapshot;
    PipelineHealthMonitorStats m_stats;
    uint64_t m_totalCount = 0;
    uint64_t m_errorCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
