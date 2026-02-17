//==============================================================================
// DarkThumbs Engine — Sprint 48: Telemetry & Diagnostics Dashboard
//
// Provides structured diagnostic collection (decode times, cache hit rates,
// error counts), health scoring for decoders, performance trend analysis,
// diagnostic export (JSON/Text), real-time dashboard data model, and
// system resource monitoring.
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <numeric>
#include <cmath>

namespace DarkThumbs::Engine::Core {

//==============================================================================
// Health Level — Component health status
//==============================================================================

enum class HealthLevel : uint8_t {
    Healthy,       // Green — all metrics within normal range
    Degraded,      // Yellow — some metrics borderline
    Unhealthy,     // Red — failures detected, needs attention
    Critical,      // Black — non-functional, immediate action needed
    Unknown        // Grey — insufficient data
};

inline const char* HealthLevelName(HealthLevel h) {
    switch (h) {
        case HealthLevel::Healthy:   return "Healthy";
        case HealthLevel::Degraded:  return "Degraded";
        case HealthLevel::Unhealthy: return "Unhealthy";
        case HealthLevel::Critical:  return "Critical";
        case HealthLevel::Unknown:   return "Unknown";
        default:                     return "Unknown";
    }
}

inline int HealthScore(HealthLevel h) {
    switch (h) {
        case HealthLevel::Healthy:   return 100;
        case HealthLevel::Degraded:  return 70;
        case HealthLevel::Unhealthy: return 30;
        case HealthLevel::Critical:  return 0;
        case HealthLevel::Unknown:   return -1;
        default:                     return -1;
    }
}

//==============================================================================
// Metric Type — Categories of telemetry data
//==============================================================================

enum class MetricType : uint8_t {
    Counter,       // Monotonically increasing (requests, errors)
    Gauge,         // Current value (memory, queue depth)
    Histogram,     // Distribution (latency percentiles)
    Timer          // Duration measurement
};

inline const char* MetricTypeName(MetricType t) {
    switch (t) {
        case MetricType::Counter:   return "Counter";
        case MetricType::Gauge:     return "Gauge";
        case MetricType::Histogram: return "Histogram";
        case MetricType::Timer:     return "Timer";
        default:                    return "Unknown";
    }
}

//==============================================================================
// Metric Sample — Single data point
//==============================================================================

struct MetricSample {
    std::string name;
    double      value    = 0.0;
    int64_t     timestamp = 0; // Unix ms
    MetricType  type     = MetricType::Gauge;
    std::string unit;          // "ms", "bytes", "count", "%"

    std::string FormattedValue() const {
        std::ostringstream ss;
        if (type == MetricType::Timer) {
            ss << std::fixed << std::setprecision(2) << value << " ms";
        } else if (unit == "%") {
            ss << std::fixed << std::setprecision(1) << value << "%";
        } else if (unit == "bytes") {
            if (value >= 1024.0 * 1024.0) {
                ss << std::fixed << std::setprecision(1) << (value / (1024.0 * 1024.0)) << " MB";
            } else if (value >= 1024.0) {
                ss << std::fixed << std::setprecision(1) << (value / 1024.0) << " KB";
            } else {
                ss << std::fixed << std::setprecision(0) << value << " B";
            }
        } else {
            ss << std::fixed << std::setprecision(0) << value;
            if (!unit.empty()) ss << " " << unit;
        }
        return ss.str();
    }
};

//==============================================================================
// Statistics — Computed distribution stats
//==============================================================================

struct Statistics {
    double min    = 0.0;
    double max    = 0.0;
    double mean   = 0.0;
    double median = 0.0;
    double p95    = 0.0;
    double p99    = 0.0;
    double stddev = 0.0;
    size_t count  = 0;

    bool IsEmpty() const { return count == 0; }

    static Statistics Compute(std::vector<double> values) {
        Statistics s;
        if (values.empty()) return s;

        s.count = values.size();
        std::sort(values.begin(), values.end());

        s.min = values.front();
        s.max = values.back();
        s.median = values[values.size() / 2];
        s.p95 = values[static_cast<size_t>(values.size() * 0.95)];
        s.p99 = values[std::min(static_cast<size_t>(values.size() * 0.99), values.size() - 1)];

        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        s.mean = sum / static_cast<double>(s.count);

        if (s.count > 1) {
            double sqSum = 0.0;
            for (auto v : values) {
                sqSum += (v - s.mean) * (v - s.mean);
            }
            s.stddev = std::sqrt(sqSum / static_cast<double>(s.count - 1));
        }

        return s;
    }

    std::string Summary() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2)
           << "n=" << count
           << " min=" << min
           << " max=" << max
           << " mean=" << mean
           << " p95=" << p95
           << " p99=" << p99;
        return ss.str();
    }
};

//==============================================================================
// Decoder Telemetry — Per-decoder performance data
//==============================================================================

struct DecoderTelemetry {
    std::string decoderName;
    size_t      totalDecodes   = 0;
    size_t      successCount   = 0;
    size_t      failureCount   = 0;
    size_t      timeoutCount   = 0;
    double      avgDecodeMs    = 0.0;
    double      p95DecodeMs    = 0.0;
    double      maxDecodeMs    = 0.0;
    uint64_t    totalBytesIn   = 0;
    uint64_t    totalBytesOut  = 0;

    double SuccessRate() const {
        if (totalDecodes == 0) return 0.0;
        return (static_cast<double>(successCount) / static_cast<double>(totalDecodes)) * 100.0;
    }

    double FailureRate() const {
        if (totalDecodes == 0) return 0.0;
        return (static_cast<double>(failureCount) / static_cast<double>(totalDecodes)) * 100.0;
    }

    HealthLevel Health() const {
        if (totalDecodes == 0) return HealthLevel::Unknown;
        double failRate = FailureRate();
        if (failRate > 50.0) return HealthLevel::Critical;
        if (failRate > 20.0) return HealthLevel::Unhealthy;
        if (failRate > 5.0)  return HealthLevel::Degraded;
        return HealthLevel::Healthy;
    }
};

//==============================================================================
// Cache Telemetry — Cache performance metrics
//==============================================================================

struct CacheTelemetry {
    size_t   totalRequests = 0;
    size_t   hits          = 0;
    size_t   misses        = 0;
    size_t   evictions     = 0;
    uint64_t currentSizeBytes = 0;
    uint64_t maxSizeBytes     = 0;
    size_t   entryCount       = 0;
    double   avgLookupMs      = 0.0;

    double HitRate() const {
        if (totalRequests == 0) return 0.0;
        return (static_cast<double>(hits) / static_cast<double>(totalRequests)) * 100.0;
    }

    double MissRate() const {
        return 100.0 - HitRate();
    }

    double Utilization() const {
        if (maxSizeBytes == 0) return 0.0;
        return (static_cast<double>(currentSizeBytes) / static_cast<double>(maxSizeBytes)) * 100.0;
    }

    HealthLevel Health() const {
        if (totalRequests == 0) return HealthLevel::Unknown;
        double hitRate = HitRate();
        if (hitRate >= 80.0) return HealthLevel::Healthy;
        if (hitRate >= 50.0) return HealthLevel::Degraded;
        return HealthLevel::Unhealthy;
    }
};

//==============================================================================
// System Metrics — Resource usage
//==============================================================================

struct SystemMetrics {
    double   cpuUsagePercent  = 0.0;
    uint64_t memoryUsedBytes  = 0;
    uint64_t memoryTotalBytes = 0;
    uint64_t diskUsedBytes    = 0;
    uint64_t diskTotalBytes   = 0;
    uint32_t threadCount      = 0;
    uint32_t handleCount      = 0;
    double   gpuUsagePercent  = 0.0;
    uint64_t gpuMemoryUsed    = 0;

    double MemoryUsagePercent() const {
        if (memoryTotalBytes == 0) return 0.0;
        return (static_cast<double>(memoryUsedBytes) / static_cast<double>(memoryTotalBytes)) * 100.0;
    }

    double DiskUsagePercent() const {
        if (diskTotalBytes == 0) return 0.0;
        return (static_cast<double>(diskUsedBytes) / static_cast<double>(diskTotalBytes)) * 100.0;
    }

    HealthLevel OverallHealth() const {
        if (cpuUsagePercent > 90.0 || MemoryUsagePercent() > 95.0)
            return HealthLevel::Critical;
        if (cpuUsagePercent > 75.0 || MemoryUsagePercent() > 85.0)
            return HealthLevel::Degraded;
        return HealthLevel::Healthy;
    }
};

//==============================================================================
// Dashboard Data — Complete diagnostic snapshot
//==============================================================================

struct DashboardData {
    // Timestamps
    int64_t   snapshotTime = 0;
    int64_t   uptimeMs     = 0;
    std::string version;

    // Component health
    std::vector<DecoderTelemetry> decoders;
    CacheTelemetry               cache;
    SystemMetrics                system;

    // Aggregate stats
    size_t totalThumbnailsGenerated = 0;
    size_t totalErrors              = 0;
    double overallSuccessRate       = 0.0;

    HealthLevel OverallHealth() const {
        if (totalErrors > 100 || system.OverallHealth() == HealthLevel::Critical)
            return HealthLevel::Critical;
        if (cache.Health() == HealthLevel::Unhealthy)
            return HealthLevel::Degraded;
        return HealthLevel::Healthy;
    }

    std::string UptimeHuman() const {
        int64_t seconds = uptimeMs / 1000;
        int64_t hours   = seconds / 3600;
        int64_t minutes = (seconds % 3600) / 60;
        int64_t secs    = seconds % 60;
        std::ostringstream ss;
        if (hours > 0) ss << hours << "h ";
        if (minutes > 0 || hours > 0) ss << minutes << "m ";
        ss << secs << "s";
        return ss.str();
    }

    size_t HealthyDecoderCount() const {
        return std::count_if(decoders.begin(), decoders.end(),
            [](const DecoderTelemetry& d) { return d.Health() == HealthLevel::Healthy; });
    }
};

//==============================================================================
// Diagnostic Export — Serialize dashboard data for reporting
//==============================================================================

struct DiagnosticExport {
    static std::string ToText(const DashboardData& data) {
        std::ostringstream ss;
        ss << "=== DarkThumbs Diagnostics ===" << "\n"
           << "Version: " << data.version << "\n"
           << "Uptime: " << data.UptimeHuman() << "\n"
           << "Overall Health: " << HealthLevelName(data.OverallHealth()) << "\n"
           << "Thumbnails Generated: " << data.totalThumbnailsGenerated << "\n"
           << "Total Errors: " << data.totalErrors << "\n"
           << "\n--- Cache ---\n"
           << "Hit Rate: " << std::fixed << std::setprecision(1)
           << data.cache.HitRate() << "%" << "\n"
           << "Entries: " << data.cache.entryCount << "\n"
           << "Utilization: " << data.cache.Utilization() << "%" << "\n"
           << "\n--- Decoders (" << data.decoders.size() << ") ---\n";

        for (const auto& dec : data.decoders) {
            ss << "  " << dec.decoderName << ": "
               << dec.successCount << "/" << dec.totalDecodes
               << " (" << std::setprecision(1) << dec.SuccessRate() << "%), "
               << std::setprecision(2) << dec.avgDecodeMs << " ms avg"
               << " [" << HealthLevelName(dec.Health()) << "]" << "\n";
        }

        return ss.str();
    }

    static std::string ToJSON(const DashboardData& data) {
        std::ostringstream ss;
        ss << "{\n"
           << "  \"version\": \"" << data.version << "\",\n"
           << "  \"uptime_ms\": " << data.uptimeMs << ",\n"
           << "  \"health\": \"" << HealthLevelName(data.OverallHealth()) << "\",\n"
           << "  \"thumbnails_generated\": " << data.totalThumbnailsGenerated << ",\n"
           << "  \"total_errors\": " << data.totalErrors << ",\n"
           << "  \"cache\": {\n"
           << "    \"hit_rate\": " << std::fixed << std::setprecision(1) << data.cache.HitRate() << ",\n"
           << "    \"entries\": " << data.cache.entryCount << ",\n"
           << "    \"utilization\": " << data.cache.Utilization() << "\n"
           << "  },\n"
           << "  \"decoders\": " << data.decoders.size() << "\n"
           << "}\n";
        return ss.str();
    }
};

//==============================================================================
// Diagnostics Config — What telemetry to collect
//==============================================================================

struct DiagnosticsConfig {
    bool   enabled            = true;
    bool   collectTimings     = true;
    bool   collectMemory      = true;
    bool   collectGPU         = false;
    bool   collectDecoderStats = true;
    uint32_t sampleIntervalMs = 5000;  // 5 seconds
    uint32_t retentionMinutes = 60;    // 1 hour of history

    static DiagnosticsConfig Default() {
        return {};
    }

    static DiagnosticsConfig Detailed() {
        DiagnosticsConfig c;
        c.collectGPU = true;
        c.sampleIntervalMs = 1000;
        c.retentionMinutes = 1440; // 24 hours
        return c;
    }

    static DiagnosticsConfig Minimal() {
        DiagnosticsConfig c;
        c.collectTimings = false;
        c.collectMemory = false;
        c.sampleIntervalMs = 30000;
        c.retentionMinutes = 15;
        return c;
    }

    static DiagnosticsConfig Disabled() {
        DiagnosticsConfig c;
        c.enabled = false;
        return c;
    }
};

} // namespace DarkThumbs::Engine::Core
