// PerformanceDashboard.h — Real-Time Performance Monitoring Dashboard
// Copyright (c) 2026 ExplorerLens Project
//
// Collects metrics from all subsystems (Decode, Cache, GPU, Memory, IO, Plugin)
// using a rolling window of 10000 samples per metric. Provides percentile
// computation (P95/P99), CSV export, and atomic snapshot capture. Thread-safe
// via SRWLOCK for high-concurrency shell-extension workloads.

#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// A single timestamped metric sample.
struct MetricSample {
  double value = 0.0;
  std::chrono::steady_clock::time_point timestamp{};
};

/// Summary statistics for a named metric.
struct MetricSummary {
  double current = 0.0;
  double min = 0.0;
  double max = 0.0;
  double avg = 0.0;
  double p95 = 0.0;
  double p99 = 0.0;
  uint64_t sampleCount = 0;
};

/// Snapshot of all metric current values at a single instant.
struct DashboardSnapshot {
  struct Entry {
    std::string category;
    std::string name;
    double      currentValue = 0.0;
    uint64_t    sampleCount = 0;
  };
  std::chrono::steady_clock::time_point capturedAt{};
  std::vector<Entry> entries;
};

/// Real-time performance monitoring dashboard collecting metrics from all
/// engine subsystems. Singleton, thread-safe, header-only.
class PerformanceDashboard {
public:
  // ── Singleton ──────────────────────────────────────────────────────────
  static PerformanceDashboard& Instance() {
    static PerformanceDashboard s_instance;
    return s_instance;
  }

  // ── Metric recording ──────────────────────────────────────────────────
  /// Record a raw metric sample.  category must be one of the six supported
  /// strings: "Decode", "Cache", "GPU", "Memory", "IO", "Plugin".
  inline void RecordMetric(const std::string& category,
    const std::string& name,
    double value) {
    MetricSample sample;
    sample.value = value;
    sample.timestamp = std::chrono::steady_clock::now();

    AcquireSRWLockExclusive(&m_lock);
    auto& ring = m_metrics[category][name];
    if (ring.samples.size() >= kMaxSamples) {
      ring.samples[ring.writePos % kMaxSamples] = sample;
    }
    else {
      ring.samples.push_back(sample);
    }
    ring.writePos++;
    ReleaseSRWLockExclusive(&m_lock);
  }

  /// Convenience: record a latency measurement in microseconds.
  inline void RecordLatency(const std::string& category,
    const std::string& name,
    uint64_t microseconds) {
    RecordMetric(category, name, static_cast<double>(microseconds));
  }

  /// Convenience: record a throughput measurement (items/second).
  inline void RecordThroughput(const std::string& category,
    const std::string& name,
    double itemsPerSecond) {
    RecordMetric(category, name, itemsPerSecond);
  }

  /// Convenience: increment a counter metric by 1.
  inline void RecordCounter(const std::string& category,
    const std::string& name) {
    AcquireSRWLockExclusive(&m_lock);
    auto& ring = m_metrics[category][name];
    double prev = 0.0;
    if (!ring.samples.empty()) {
      size_t lastIdx = (ring.writePos - 1) % ring.samples.size();
      prev = ring.samples[lastIdx].value;
    }
    ReleaseSRWLockExclusive(&m_lock);
    RecordMetric(category, name, prev + 1.0);
  }

  // ── Queries ───────────────────────────────────────────────────────────
  /// Compute summary (min/max/avg/p95/p99) over the rolling window.
  inline MetricSummary GetMetricSummary(const std::string& category,
    const std::string& name) const {
    MetricSummary summary{};
    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

    auto catIt = m_metrics.find(category);
    if (catIt == m_metrics.end()) {
      ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
      return summary;
    }
    auto metIt = catIt->second.find(name);
    if (metIt == catIt->second.end()) {
      ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
      return summary;
    }

    const auto& ring = metIt->second;
    if (ring.samples.empty()) {
      ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
      return summary;
    }

    // Copy values for percentile computation
    std::vector<double> vals;
    vals.reserve(ring.samples.size());
    for (const auto& s : ring.samples) {
      vals.push_back(s.value);
    }
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

    summary.sampleCount = static_cast<uint64_t>(vals.size());
    summary.current = vals.back();

    double total = 0.0;
    summary.min = vals[0];
    summary.max = vals[0];
    for (double v : vals) {
      total = total + v;
      summary.min = (std::min)(summary.min, v);
      summary.max = (std::max)(summary.max, v);
    }
    summary.avg = total / static_cast<double>(vals.size());

    // P95 / P99 via sorted-array percentile selection
    std::sort(vals.begin(), vals.end());
    summary.p95 = PercentileFromSorted(vals, 0.95);
    summary.p99 = PercentileFromSorted(vals, 0.99);

    return summary;
  }

  /// List all category names that have been recorded.
  inline std::vector<std::string> GetCategories() const {
    std::vector<std::string> result;
    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    result.reserve(m_metrics.size());
    for (const auto& kv : m_metrics) {
      result.push_back(kv.first);
    }
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    return result;
  }

  /// List all metric names within a category.
  inline std::vector<std::string> GetMetricNames(const std::string& category) const {
    std::vector<std::string> result;
    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    auto catIt = m_metrics.find(category);
    if (catIt != m_metrics.end()) {
      result.reserve(catIt->second.size());
      for (const auto& kv : catIt->second) {
        result.push_back(kv.first);
      }
    }
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    return result;
  }

  /// Capture a snapshot of all metrics' current values at this instant.
  inline DashboardSnapshot GetSnapshot() const {
    DashboardSnapshot snap;
    snap.capturedAt = std::chrono::steady_clock::now();

    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    for (const auto& catPair : m_metrics) {
      for (const auto& metPair : catPair.second) {
        DashboardSnapshot::Entry entry;
        entry.category = catPair.first;
        entry.name = metPair.first;
        entry.sampleCount = static_cast<uint64_t>(metPair.second.samples.size());
        if (!metPair.second.samples.empty()) {
          size_t lastIdx = (metPair.second.writePos - 1) % metPair.second.samples.size();
          entry.currentValue = metPair.second.samples[lastIdx].value;
        }
        snap.entries.push_back(std::move(entry));
      }
    }
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    return snap;
  }

  // ── Export / Reset ─────────────────────────────────────────────────────
  /// Export all metric history to a CSV file.
  inline void ExportCSV(const std::wstring& path) const {
    std::ofstream ofs(path, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) return;

    ofs << "Category,Metric,Timestamp_us,Value\n";

    auto epoch = std::chrono::steady_clock::time_point{};

    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    for (const auto& catPair : m_metrics) {
      for (const auto& metPair : catPair.second) {
        const auto& ring = metPair.second;
        // Iterate in chronological order
        size_t count = ring.samples.size();
        size_t start = (count >= kMaxSamples) ? (ring.writePos % kMaxSamples) : 0;
        for (size_t i = 0; i < count; ++i) {
          size_t idx = (start + i) % count;
          const auto& s = ring.samples[idx];
          auto us = std::chrono::duration_cast<std::chrono::microseconds>(
            s.timestamp - epoch).count();
          ofs << catPair.first << ","
            << metPair.first << ","
            << us << ","
            << s.value << "\n";
        }
      }
    }
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    ofs.flush();
  }

  /// Clear all collected metrics.
  inline void Reset() {
    AcquireSRWLockExclusive(&m_lock);
    m_metrics.clear();
    ReleaseSRWLockExclusive(&m_lock);
  }

private:
  PerformanceDashboard() {
    InitializeSRWLock(&m_lock);
  }
  ~PerformanceDashboard() = default;
  PerformanceDashboard(const PerformanceDashboard&) = delete;
  PerformanceDashboard& operator=(const PerformanceDashboard&) = delete;

  /// Compute percentile from a pre-sorted vector (nearest-rank method).
  static double PercentileFromSorted(const std::vector<double>& sorted, double p) {
    if (sorted.empty()) return 0.0;
    if (sorted.size() == 1) return sorted[0];
    double rank = p * static_cast<double>(sorted.size() - 1);
    size_t lo = static_cast<size_t>(std::floor(rank));
    size_t hi = static_cast<size_t>(std::ceil(rank));
    if (lo == hi) return sorted[lo];
    double frac = rank - static_cast<double>(lo);
    return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
  }

  static constexpr size_t kMaxSamples = 10000;

  /// Ring buffer for metric samples.
  struct MetricRing {
    std::vector<MetricSample> samples;
    size_t writePos = 0;  ///< Total writes (monotonic), mod kMaxSamples for index
  };

  /// category -> (metric name -> ring buffer)
  std::unordered_map<std::string,
    std::unordered_map<std::string, MetricRing>> m_metrics;

  mutable SRWLOCK m_lock = SRWLOCK_INIT;
};

} // namespace Engine
} // namespace ExplorerLens
