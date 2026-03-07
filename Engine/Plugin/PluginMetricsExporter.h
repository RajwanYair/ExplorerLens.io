// PluginMetricsExporter.h — Plugin Telemetry Collection and Export
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates runtime metrics from loaded plugins (decode times, error rates,
// memory usage) and exports them for monitoring and diagnostics.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

struct PluginMetric {
    std::string pluginId;
    std::string metricName;
    double value = 0.0;
    uint64_t timestampMs = 0;
};

struct PluginMetricsSummary {
    std::string pluginId;
    uint64_t totalDecodes = 0;
    uint64_t totalErrors = 0;
    double avgDecodeMs = 0.0;
    double p95DecodeMs = 0.0;
    uint64_t peakMemoryBytes = 0;
    double errorRate = 0.0;
    uint64_t uptimeMs = 0;
};

struct MetricsExportConfig {
    bool enabled = true;
    uint32_t flushIntervalMs = 5000;
    uint32_t maxMetricsPerPlugin = 10000;
    bool includeTimestamps = true;
};

class PluginMetricsExporter {
public:
    void Configure(const MetricsExportConfig& config) { m_config = config; }

    void Record(const PluginMetric& metric) {
        if (!m_config.enabled) return;
        std::lock_guard lock(m_mutex);
        auto& metrics = m_metrics[metric.pluginId];
        metrics.push_back(metric);
        if (metrics.size() > m_config.maxMetricsPerPlugin)
            metrics.erase(metrics.begin());
    }

    void RecordDecode(const std::string& pluginId, double durationMs, bool success) {
        std::lock_guard lock(m_mutex);
        auto& s = m_summaries[pluginId];
        s.pluginId = pluginId;
        s.totalDecodes++;
        if (!success) s.totalErrors++;
        s.errorRate = s.totalDecodes > 0 ? 100.0 * s.totalErrors / s.totalDecodes : 0.0;
        s.avgDecodeMs = ((s.avgDecodeMs * (s.totalDecodes - 1)) + durationMs) / s.totalDecodes;
    }

    PluginMetricsSummary GetSummary(const std::string& pluginId) const {
        std::lock_guard lock(m_mutex);
        auto it = m_summaries.find(pluginId);
        return it != m_summaries.end() ? it->second : PluginMetricsSummary{};
    }

    std::vector<PluginMetricsSummary> GetAllSummaries() const {
        std::lock_guard lock(m_mutex);
        std::vector<PluginMetricsSummary> result;
        result.reserve(m_summaries.size());
        for (const auto& [k, v] : m_summaries) result.push_back(v);
        return result;
    }

    size_t TrackedPlugins() const {
        std::lock_guard lock(m_mutex);
        return m_summaries.size();
    }

    void Reset() {
        std::lock_guard lock(m_mutex);
        m_metrics.clear();
        m_summaries.clear();
    }

private:
    mutable std::mutex m_mutex;
    MetricsExportConfig m_config;
    std::unordered_map<std::string, std::vector<PluginMetric>> m_metrics;
    std::unordered_map<std::string, PluginMetricsSummary> m_summaries;
};

} // namespace Engine
} // namespace ExplorerLens
