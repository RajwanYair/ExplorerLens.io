#pragma once
// Diagnostic Dashboard — runtime health metrics and ETW counter aggregation
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Category of health metric
enum class MetricCategory : uint32_t {
 CPU = 0,
 Memory = 1,
 GPU = 2,
 Disk = 3,
 Network = 4,
 Decoder = 5,
 Cache = 6,
 COUNT = 7
};

/// Health status level
enum class DiagHealthLevel : uint32_t {
 Healthy = 0,
 Warning = 1,
 Degraded = 2,
 Critical = 3,
 Unknown = 4,
 COUNT = 5
};

/// A single metric data point
struct MetricPoint {
 std::wstring name;
 MetricCategory category = MetricCategory::CPU;
 double value = 0.0;
 double threshold = 100.0;
 DiagHealthLevel health = DiagHealthLevel::Healthy;
 uint64_t timestamp = 0;
};

/// Snapshot of overall system health
struct HealthSnapshot {
 DiagHealthLevel overall = DiagHealthLevel::Healthy;
 uint32_t metricCount = 0;
 uint32_t warningCount = 0;
 uint32_t criticalCount = 0;
 double uptimeSeconds = 0.0;
};

/// Aggregates runtime diagnostics for the dashboard UI
class DiagnosticDashboard {
public:
 DiagnosticDashboard();

 static const wchar_t* GetCategoryName(MetricCategory cat);
 static const wchar_t* GetHealthName(DiagHealthLevel level);
 static uint32_t GetCategoryCount() { return static_cast<uint32_t>(MetricCategory::COUNT); }

 /// Record a metric data point
 void RecordMetric(const std::wstring& name, MetricCategory cat, double value, double threshold = 100.0);
 /// Get all recorded metrics
 const std::vector<MetricPoint>& GetMetrics() const { return m_metrics; }
 /// Compute health snapshot
 HealthSnapshot GetSnapshot() const;
 /// Clear all metrics
 void Reset();

private:
 std::vector<MetricPoint> m_metrics;
 DiagHealthLevel EvaluateHealth(double value, double threshold) const;
};

}} // namespace ExplorerLens::Engine

