// EnterpriseMetricsDashboard.h — Real-Time Enterprise KPI Dashboard
// Copyright (c) 2026 ExplorerLens Project
//
// Provides EnterpriseMetricsDashboard for collecting, aggregating, and streaming
// real-time KPI metrics including latency percentiles, error rates, and cache hit ratios.
//
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cstdint>
#include <optional>
#include <utility>

namespace ExplorerLens::Engine {

enum class MetricType : uint8_t {
    Latency        = 0,
    ErrorRate      = 1,
    Throughput     = 2,
    CacheHit       = 3,
    PluginAdoption = 4
};

struct PercentileSet {
    double p50{0.0};
    double p95{0.0};
    double p99{0.0};
    double p999{0.0};
};

struct MetricSnapshot {
    std::string   name;
    MetricType    type{MetricType::Latency};
    double        value{0.0};
    PercentileSet percentiles;
    uint64_t      sampleCount{0};
    std::chrono::system_clock::time_point timestamp;
};

struct DashboardConfig {
    uint32_t    aggregationWindowMs{5000};
    uint32_t    maxSamplesPerMetric{10000};
    bool        streamingEnabled{false};
    std::string outputEndpoint;
};

class EnterpriseMetricsDashboard {
public:
    explicit EnterpriseMetricsDashboard(DashboardConfig config = {});
    ~EnterpriseMetricsDashboard();

    EnterpriseMetricsDashboard(const EnterpriseMetricsDashboard&)            = delete;
    EnterpriseMetricsDashboard& operator=(const EnterpriseMetricsDashboard&) = delete;

    // Metric ingestion
    void RecordMetric(const std::string& name, MetricType type, double value);
    void RecordMetricBatch(
        const std::vector<std::pair<std::string, double>>& samples, MetricType type);

    // Snapshot queries
    std::optional<MetricSnapshot> GetSnapshot(const std::string& name) const;
    std::vector<MetricSnapshot>   GetAllSnapshots() const;

    // Percentile computation
    PercentileSet GetPercentiles(const std::string& name) const;

    // Streaming
    void StartStreaming();
    void StopStreaming();
    bool IsStreaming() const noexcept;

    using StreamCallback = std::function<void(const MetricSnapshot&)>;
    void StreamMetrics(StreamCallback cb);

    // Serialization
    std::string GetDashboardJson() const;

    // Lifecycle
    bool ResetMetric(const std::string& name);
    void ResetAll();

private:
    DashboardConfig m_config;
    bool            m_streaming{false};
    StreamCallback  m_streamCallback;

    PercentileSet ComputePercentiles(const std::vector<double>& samples) const;
    void          FlushToStream(const MetricSnapshot& snapshot);
    bool          ValidateMetricName(const std::string& name) const;
};

} // namespace ExplorerLens::Engine
