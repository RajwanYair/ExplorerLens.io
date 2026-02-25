// PerformanceDashboard.h — Real-Time Performance Metrics for LENSManager
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a data model and UI helper for a performance dashboard tab in
// Settings. Shows cache hit rate, thumbnail generation times, GPU utilization,
// per-format stats.
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <windows.h>

namespace ExplorerLens {

// ============================================================================
// Performance metric types
// ============================================================================

struct PerformanceMetric {
  std::wstring name;
  double value = 0.0;
  double target = 0.0; // Performance target
  std::wstring unit;   // "ms", "%", "img/s", "MB"
  bool meetsTarget = false;
};

struct FormatPerfInfo {
  std::wstring formatName;
  int decodeCount = 0;
  double avgDecodeMs = 0.0;
  double maxDecodeMs = 0.0;
  double cacheHitRate = 0.0;
  bool gpuAccelerated = false;
};

// ============================================================================
// PerformanceDashboard — Aggregated system performance view
// ============================================================================

class PerformanceDashboard {
public:
  // Singleton accessor
  static PerformanceDashboard &Instance() {
    static PerformanceDashboard s_instance;
    return s_instance;
  }

  // ====================================================================
  // Collect current metrics from engine subsystems
  // ====================================================================
  void RefreshMetrics() {
    m_metrics.clear();

    // Core thumbnail metrics
    AddMetric(L"Avg Thumbnail Time", m_avgThumbnailMs, 17.0, L"ms");
    AddMetric(L"P99 Thumbnail Time", m_p99ThumbnailMs, 50.0, L"ms");
    AddMetric(L"Batch Throughput", m_batchThroughput, 235.0, L"img/s");
    AddMetric(L"Cache Hit Rate", m_cacheHitRate, 85.0, L"%");
    AddMetric(L"Cache Latency", m_cacheLatencyMs, 1.0, L"ms");

    // Memory metrics
    AddMetric(L"Memory Usage", m_memoryUsageMB, 150.0, L"MB");
    AddMetric(L"Bitmap Pool Utilization", m_bitmapPoolUtil, 80.0, L"%");

    // GPU metrics
    AddMetric(L"GPU Decode Active", m_gpuDecodeActive ? 100.0 : 0.0, 100.0,
              L"bool");
    AddMetric(L"GPU Memory", m_gpuMemoryMB, 256.0, L"MB");
  }

  // ====================================================================
  // Getters for UI binding
  // ====================================================================
  const std::vector<PerformanceMetric> &GetMetrics() const { return m_metrics; }
  const std::vector<FormatPerfInfo> &GetFormatStats() const {
    return m_formatStats;
  }

  // ====================================================================
  // Record a decode event (called by engine)
  // ====================================================================
  void RecordDecode(const std::wstring &format, double decodeMs, bool fromCache,
                    bool gpuAccel) {
    m_totalDecodes++;
    if (fromCache)
      m_cacheHits++;
    m_totalDecodeMs += decodeMs;
    m_avgThumbnailMs = m_totalDecodeMs / m_totalDecodes;
    if (decodeMs > m_p99ThumbnailMs)
      m_p99ThumbnailMs = decodeMs;
    m_cacheHitRate =
        (m_totalDecodes > 0) ? (100.0 * m_cacheHits / m_totalDecodes) : 0.0;

    // Update per-format stats
    bool found = false;
    for (auto &fs : m_formatStats) {
      if (fs.formatName == format) {
        fs.decodeCount++;
        fs.avgDecodeMs = ((fs.avgDecodeMs * (fs.decodeCount - 1)) + decodeMs) /
                         fs.decodeCount;
        if (decodeMs > fs.maxDecodeMs)
          fs.maxDecodeMs = decodeMs;
        fs.gpuAccelerated = gpuAccel;
        found = true;
        break;
      }
    }
    if (!found) {
      FormatPerfInfo info;
      info.formatName = format;
      info.decodeCount = 1;
      info.avgDecodeMs = decodeMs;
      info.maxDecodeMs = decodeMs;
      info.gpuAccelerated = gpuAccel;
      m_formatStats.push_back(info);
    }
  }

  // ====================================================================
  // Reset counters
  // ====================================================================
  void Reset() {
    m_totalDecodes = 0;
    m_cacheHits = 0;
    m_totalDecodeMs = 0.0;
    m_avgThumbnailMs = 0.0;
    m_p99ThumbnailMs = 0.0;
    m_batchThroughput = 0.0;
    m_cacheHitRate = 0.0;
    m_cacheLatencyMs = 0.0;
    m_memoryUsageMB = 0.0;
    m_bitmapPoolUtil = 0.0;
    m_gpuDecodeActive = false;
    m_gpuMemoryMB = 0.0;
    m_metrics.clear();
    m_formatStats.clear();
  }

  // ====================================================================
  // Format a summary string for status bar / tooltip
  // ====================================================================
  std::wstring GetSummary() const {
    wchar_t buf[256];
    swprintf_s(buf, L"Avg: %.1fms | Cache: %.0f%% | Decodes: %llu",
               m_avgThumbnailMs, m_cacheHitRate, m_totalDecodes);
    return buf;
  }

private:
  PerformanceDashboard() = default;

  void AddMetric(const std::wstring &name, double value, double target,
                 const std::wstring &unit) {
    PerformanceMetric m;
    m.name = name;
    m.value = value;
    m.target = target;
    m.unit = unit;
    m.meetsTarget = (unit == L"img/s" || unit == L"%") ? (value >= target)
                                                       : (value <= target);
    m_metrics.push_back(m);
  }

  // Running counters
  uint64_t m_totalDecodes = 0;
  uint64_t m_cacheHits = 0;
  double m_totalDecodeMs = 0.0;

  // Derived metrics
  double m_avgThumbnailMs = 0.0;
  double m_p99ThumbnailMs = 0.0;
  double m_batchThroughput = 0.0;
  double m_cacheHitRate = 0.0;
  double m_cacheLatencyMs = 0.0;
  double m_memoryUsageMB = 0.0;
  double m_bitmapPoolUtil = 0.0;
  bool m_gpuDecodeActive = false;
  double m_gpuMemoryMB = 0.0;

  std::vector<PerformanceMetric> m_metrics;
  std::vector<FormatPerfInfo> m_formatStats;
};

} // namespace ExplorerLens
