#pragma once
// Sprint 435: Usage Analytics Dashboard
// Aggregated usage statistics dashboard — format popularity, decode times,
// cache hit rates, and user interaction patterns (anonymized).
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Analytics metric type
enum class AnalyticsMetric : uint8_t {
  ThumbnailsGenerated = 0,
  CacheHitRate,
  AvgDecodeTimeMs,
  P99DecodeTimeMs,
  FormatDistribution,
  ErrorRate,
  PluginUsage,
  MemoryPeakMB,
  COUNT
};

/// Time aggregation window
enum class AnalyticsTimeWindow : uint8_t {
  LastHour = 0,
  Last24Hours,
  Last7Days,
  Last30Days,
  AllTime,
  COUNT
};

struct AnalyticsSnapshot {
  uint64_t thumbnailsTotal = 0;
  float cacheHitRate = 0.0f;
  double avgDecodeMs = 0.0;
  double p99DecodeMs = 0.0;
  float errorRate = 0.0f;
  uint64_t peakMemoryBytes = 0;
  AnalyticsTimeWindow window = AnalyticsTimeWindow::Last24Hours;
};

class UsageAnalyticsDashboard {
public:
  static constexpr size_t MetricCount() {
    return static_cast<size_t>(AnalyticsMetric::COUNT);
  }
  static constexpr size_t WindowCount() {
    return static_cast<size_t>(AnalyticsTimeWindow::COUNT);
  }

  static const wchar_t *MetricName(AnalyticsMetric m) {
    switch (m) {
    case AnalyticsMetric::ThumbnailsGenerated:
      return L"Thumbnails Generated";
    case AnalyticsMetric::CacheHitRate:
      return L"Cache Hit Rate";
    case AnalyticsMetric::AvgDecodeTimeMs:
      return L"Avg Decode Time (ms)";
    case AnalyticsMetric::P99DecodeTimeMs:
      return L"P99 Decode Time (ms)";
    case AnalyticsMetric::FormatDistribution:
      return L"Format Distribution";
    case AnalyticsMetric::ErrorRate:
      return L"Error Rate";
    case AnalyticsMetric::PluginUsage:
      return L"Plugin Usage";
    case AnalyticsMetric::MemoryPeakMB:
      return L"Peak Memory (MB)";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *WindowName(AnalyticsTimeWindow w) {
    switch (w) {
    case AnalyticsTimeWindow::LastHour:
      return L"Last Hour";
    case AnalyticsTimeWindow::Last24Hours:
      return L"Last 24 Hours";
    case AnalyticsTimeWindow::Last7Days:
      return L"Last 7 Days";
    case AnalyticsTimeWindow::Last30Days:
      return L"Last 30 Days";
    case AnalyticsTimeWindow::AllTime:
      return L"All Time";
    default:
      return L"Unknown";
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
