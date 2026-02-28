// PerformanceDashboard.h — Real-Time Performance Statistics Dashboard
// Copyright (c) 2026 ExplorerLens Project
//
// Collects and displays engine performance metrics: cache hit rates,
// GPU utilization, decode throughput, memory pressure, and latency
// percentiles. Feeds the LENSManager "Performance" tab.

#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Performance metric category (dashboard-specific — distinct from
/// DiagnosticDashboard::DashboardMetricCategory)
enum class DashboardMetricCategory : uint8_t {
 Cache = 0,
 GPU = 1,
 Decode = 2,
 Memory = 3,
 Pipeline = 4,
 IO = 5,
 Count = 6
};

/// A single tracked metric
struct PerformanceMetric {
 const char *name = nullptr; ///< e.g., "CacheHitRate"
 const char *unit = nullptr; ///< e.g., "%", "ms", "MB"
 DashboardMetricCategory category = DashboardMetricCategory::Cache;
 double current = 0;
 double min = 0;
 double max = 0;
 double average = 0;
 uint64_t sampleCount = 0;
};

/// Latency histogram with percentile calculation
struct LatencyHistogram {
 static constexpr uint32_t BUCKET_COUNT = 64;
 static constexpr uint32_t BUCKET_WIDTH_US =
 500; // 500µs per bucket, up to 32ms

 uint32_t buckets[BUCKET_COUNT] = {};
 uint64_t totalSamples = 0;
 uint64_t totalTimeUs = 0;

 void Record(uint32_t timeUs) {
 uint32_t idx = timeUs / BUCKET_WIDTH_US;
 if (idx >= BUCKET_COUNT)
 idx = BUCKET_COUNT - 1;
 buckets[idx]++;
 totalSamples++;
 totalTimeUs += timeUs;
 }

 /// Get percentile value in microseconds
 uint32_t Percentile(double p) const {
 if (totalSamples == 0)
 return 0;
 uint64_t target =
 static_cast<uint64_t>(static_cast<double>(totalSamples) * p / 100.0);
 uint64_t cumulative = 0;
 for (uint32_t i = 0; i < BUCKET_COUNT; ++i) {
 cumulative += buckets[i];
 if (cumulative >= target)
 return (i + 1) * BUCKET_WIDTH_US;
 }
 return BUCKET_COUNT * BUCKET_WIDTH_US;
 }

 double AverageUs() const {
 return totalSamples > 0 ? static_cast<double>(totalTimeUs) /
 static_cast<double>(totalSamples)
 : 0.0;
 }

 void Reset() {
 memset(buckets, 0, sizeof(buckets));
 totalSamples = 0;
 totalTimeUs = 0;
 }
};

/// Named dashboard metric for GUI display
enum class DashboardMetric : uint8_t {
 AvgDecodeTime = 0,
 CacheHitRate = 1,
 GPUUsage = 2,
 MemoryUsage = 3,
 Throughput = 4,
 DecodeP95 = 5,
 DecodeP99 = 6,
 VRAMUsage = 7,
 BitmapPoolMB = 8,
 Uptime = 9,
 MetricCount = 10
};

/// Performance dashboard — singleton data collector
class PerformanceDashboard {
public:
 static PerformanceDashboard &Instance() {
 static PerformanceDashboard inst;
 return inst;
 }

 /// Named metric count (for GUI)
 static constexpr uint32_t MetricCount() {
 return static_cast<uint32_t>(DashboardMetric::MetricCount);
 }

 /// Named metric display name
 static const wchar_t *MetricName(DashboardMetric m) {
 switch (m) {
 case DashboardMetric::AvgDecodeTime:
 return L"Avg Decode Time";
 case DashboardMetric::CacheHitRate:
 return L"Cache Hit Rate";
 case DashboardMetric::GPUUsage:
 return L"GPU Usage";
 case DashboardMetric::MemoryUsage:
 return L"Memory Usage";
 case DashboardMetric::Throughput:
 return L"Throughput";
 case DashboardMetric::DecodeP95:
 return L"Decode P95";
 case DashboardMetric::DecodeP99:
 return L"Decode P99";
 case DashboardMetric::VRAMUsage:
 return L"VRAM Usage";
 case DashboardMetric::BitmapPoolMB:
 return L"Bitmap Pool";
 case DashboardMetric::Uptime:
 return L"Uptime";
 default:
 return L"Unknown";
 }
 }

 // ── Cache Metrics ──────────────────────────────────────
 void RecordCacheHit() { m_cacheHits++; }
 void RecordCacheMiss() { m_cacheMisses++; }

 double GetCacheHitRate() const {
 uint64_t total = m_cacheHits + m_cacheMisses;
 return total > 0 ? (static_cast<double>(m_cacheHits.load()) /
 static_cast<double>(total)) *
 100.0
 : 0.0;
 }

 uint64_t GetCacheHits() const { return m_cacheHits; }
 uint64_t GetCacheMisses() const { return m_cacheMisses; }

 // ── Decode Metrics ─────────────────────────────────────
 void RecordDecode(uint32_t durationUs, bool gpuAccel) {
 m_decodeLatency.Record(durationUs);
 m_totalDecodes++;
 if (gpuAccel)
 m_gpuDecodes++;
 }

 uint64_t GetTotalDecodes() const { return m_totalDecodes; }
 uint64_t GetGPUDecodes() const { return m_gpuDecodes; }

 double GetDecodeP50Us() const {
 return static_cast<double>(m_decodeLatency.Percentile(50));
 }
 double GetDecodeP95Us() const {
 return static_cast<double>(m_decodeLatency.Percentile(95));
 }
 double GetDecodeP99Us() const {
 return static_cast<double>(m_decodeLatency.Percentile(99));
 }
 double GetDecodeAvgUs() const { return m_decodeLatency.AverageUs(); }

 const LatencyHistogram &GetDecodeHistogram() const { return m_decodeLatency; }

 // ── Memory Metrics ─────────────────────────────────────
 void SetWorkingSetMB(double mb) { m_workingSetMB = mb; }
 void SetCommitChargeMB(double mb) { m_commitMB = mb; }
 void SetBitmapPoolSizeMB(double mb) { m_bitmapPoolMB = mb; }

 double GetWorkingSetMB() const { return m_workingSetMB; }
 double GetCommitChargeMB() const { return m_commitMB; }
 double GetBitmapPoolSizeMB() const { return m_bitmapPoolMB; }

 // ── GPU Metrics ────────────────────────────────────────
 void SetGPUUsagePercent(float pct) { m_gpuUsagePct = pct; }
 void SetVRAMUsedMB(float mb) { m_vramUsedMB = mb; }
 void SetVRAMBudgetMB(float mb) { m_vramBudgetMB = mb; }

 float GetGPUUsagePercent() const { return m_gpuUsagePct; }
 float GetVRAMUsedMB() const { return m_vramUsedMB; }
 float GetVRAMBudgetMB() const { return m_vramBudgetMB; }

 // ── Throughput ─────────────────────────────────────────
 void RecordBatchComplete(uint32_t imageCount, uint32_t durationMs) {
 m_batchCount++;
 m_batchImages += imageCount;
 if (durationMs > 0) {
 m_lastThroughput = static_cast<float>(imageCount) * 1000.0f /
 static_cast<float>(durationMs);
 }
 }

 float GetLastThroughput() const { return m_lastThroughput; }
 uint64_t GetTotalBatchImages() const { return m_batchImages; }

 // ── Uptime ─────────────────────────────────────────────
 double GetUptimeSeconds() const {
 ULONGLONG now = GetTickCount64();
 return static_cast<double>(now - m_startTick) / 1000.0;
 }

 // ── Reset ──────────────────────────────────────────────
 void ResetAll() {
 m_cacheHits = 0;
 m_cacheMisses = 0;
 m_totalDecodes = 0;
 m_gpuDecodes = 0;
 m_decodeLatency.Reset();
 m_batchCount = 0;
 m_batchImages = 0;
 m_lastThroughput = 0;
 m_startTick = GetTickCount64();
 }

 // ── Summary string for status bar ──────────────────────
 std::string GetSummaryString() const {
 char buf[256];
 snprintf(buf, sizeof(buf),
 "Cache: %.1f%% | Decodes: %llu | P50: %.1fms | GPU: %.0f%% | Mem: "
 "%.0fMB",
 GetCacheHitRate(),
 static_cast<unsigned long long>(m_totalDecodes.load()),
 GetDecodeP50Us() / 1000.0, static_cast<double>(m_gpuUsagePct),
 m_workingSetMB);
 return buf;
 }

private:
 PerformanceDashboard() : m_startTick(GetTickCount64()) {}

 // Cache
 std::atomic<uint64_t> m_cacheHits{0};
 std::atomic<uint64_t> m_cacheMisses{0};

 // Decode
 LatencyHistogram m_decodeLatency{};
 std::atomic<uint64_t> m_totalDecodes{0};
 std::atomic<uint64_t> m_gpuDecodes{0};

 // Memory
 double m_workingSetMB = 0;
 double m_commitMB = 0;
 double m_bitmapPoolMB = 0;

 // GPU
 float m_gpuUsagePct = 0;
 float m_vramUsedMB = 0;
 float m_vramBudgetMB = 0;

 // Throughput
 std::atomic<uint64_t> m_batchCount{0};
 std::atomic<uint64_t> m_batchImages{0};
 float m_lastThroughput = 0;

 // Uptime
 ULONGLONG m_startTick = 0;
};

} // namespace Engine
} // namespace ExplorerLens
