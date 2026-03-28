// TelemetryAggregator.h — Telemetry Collection and Aggregation
// Copyright (c) 2026 ExplorerLens Project
//
// Collects operational telemetry from all engine subsystems and aggregates it
// into structured reports for diagnostics export, performance dashboards, and
// health monitoring. Privacy-respecting: no file content or PII is collected.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <chrono>
#include <sstream>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

/// Telemetry event severity
enum class AggregatorSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
    Critical = 3,
};

/// Telemetry metric types
enum class AggregatorMetricType : uint8_t {
    Counter,        // Monotonically increasing count
    Gauge,          // Point-in-time value
    Histogram,      // Distribution of values
    Timer,          // Duration measurement
};

/// Single telemetry event
struct AggregatorEvent {
    int64_t             timestamp;      // QPC ticks
    AggregatorSeverity  severity;
    std::string         category;       // e.g., "Decoder", "Cache", "GPU"
    std::string         name;           // e.g., "decode_failure", "cache_eviction"
    std::string         message;        // Human-readable details
    double              value;          // Numeric value (optional)
    uint32_t            threadId;
};

/// Aggregated metric over a time window
struct AggregatedMetric {
    std::string     name;
    AggregatorMetricType      type = AggregatorMetricType::Counter;
    double          sum = 0.0;
    double          min = 1e18;
    double          max = -1e18;
    double          avg = 0.0;
    uint64_t        count = 0;
    double          lastValue = 0.0;
};

/// Engine health snapshot
struct EngineHealthReport {
    // Uptime
    double          uptimeSeconds = 0.0;

    // Thumbnail pipeline
    uint64_t        totalThumbnails = 0;
    uint64_t        successfulThumbnails = 0;
    uint64_t        failedThumbnails = 0;
    double          avgDecodeTimeMs = 0.0;
    double          throughputImgSec = 0.0;

    // Cache
    double          cacheHitRatio = 0.0;
    uint64_t        cacheEntries = 0;
    size_t          cacheMemoryBytes = 0;

    // GPU
    bool            gpuAvailable = false;
    std::string     gpuName;
    size_t          gpuMemoryUsed = 0;

    // Memory
    size_t          processMemoryBytes = 0;
    size_t          peakMemoryBytes = 0;

    // Errors
    uint64_t        totalErrors = 0;
    uint64_t        totalWarnings = 0;
    std::string     lastError;

    // System
    uint32_t        cpuCores = 0;
    std::string     cpuBrand;
    std::string     osVersion;
    std::string     engineVersion;
};

/// Central telemetry collection and aggregation engine.
/// Thread-safe, minimal overhead by design (< 1us per event).
///
/// Usage:
///   auto& telem = TelemetryAggregator::Instance();
///   telem.RecordEvent("Cache", "hit", "", 1.0);
///   telem.IncrementCounter("thumbnails_generated");
///   telem.RecordTimer("decode_time_ms", 14.5);
///   auto report = telem.GenerateHealthReport();
///
class TelemetryAggregator {
public:
    static TelemetryAggregator& Instance() {
        static TelemetryAggregator instance;
        return instance;
    }

    /// Record a telemetry event
    void RecordEvent(const char* category, const char* name,
        const char* message = "", double value = 0.0,
        AggregatorSeverity severity = AggregatorSeverity::Info) {
        LARGE_INTEGER tick;
        QueryPerformanceCounter(&tick);

        AggregatorEvent evt;
        evt.timestamp = tick.QuadPart;
        evt.severity = severity;
        evt.category = category ? category : "";
        evt.name = name ? name : "";
        evt.message = message ? message : "";
        evt.value = value;
        evt.threadId = GetCurrentThreadId();

        {
            std::lock_guard<std::mutex> lock(m_eventMutex);
            m_events.push_back(std::move(evt));

            // Keep bounded event history (last 10000)
            if (m_events.size() > 10000) {
                m_events.erase(m_events.begin(), m_events.begin() + 5000);
            }
        }

        // Update counters
        if (severity == AggregatorSeverity::Error || severity == AggregatorSeverity::Critical) {
            m_totalErrors.fetch_add(1, std::memory_order_relaxed);
        }
        if (severity == AggregatorSeverity::Warning) {
            m_totalWarnings.fetch_add(1, std::memory_order_relaxed);
        }
    }

    /// Increment a named counter
    void IncrementCounter(const char* name, uint64_t delta = 1) {
        std::lock_guard<std::mutex> lock(m_metricMutex);
        auto& metric = m_metrics[name];
        metric.name = name;
        metric.type = AggregatorMetricType::Counter;
        metric.count += delta;
        metric.sum += static_cast<double>(delta);
        metric.lastValue = static_cast<double>(metric.count);
    }

    /// Set a gauge value
    void SetGauge(const char* name, double value) {
        std::lock_guard<std::mutex> lock(m_metricMutex);
        auto& metric = m_metrics[name];
        metric.name = name;
        metric.type = AggregatorMetricType::Gauge;
        metric.lastValue = value;
        metric.count++;
        metric.sum += value;
        metric.min = (std::min)(metric.min, value);
        metric.max = (std::max)(metric.max, value);
        metric.avg = metric.sum / metric.count;
    }

    /// Record a timer value (milliseconds)
    void RecordTimer(const char* name, double ms) {
        std::lock_guard<std::mutex> lock(m_metricMutex);
        auto& metric = m_metrics[name];
        metric.name = name;
        metric.type = AggregatorMetricType::Timer;
        metric.count++;
        metric.sum += ms;
        metric.lastValue = ms;
        metric.min = (std::min)(metric.min, ms);
        metric.max = (std::max)(metric.max, ms);
        metric.avg = metric.sum / metric.count;
    }

    /// Get a specific metric
    AggregatedMetric GetMetric(const char* name) const {
        std::lock_guard<std::mutex> lock(m_metricMutex);
        auto it = m_metrics.find(name);
        return (it != m_metrics.end()) ? it->second : AggregatedMetric{};
    }

    /// Get all metrics
    std::vector<AggregatedMetric> GetAllMetrics() const {
        std::lock_guard<std::mutex> lock(m_metricMutex);
        std::vector<AggregatedMetric> result;
        result.reserve(m_metrics.size());
        for (const auto& [key, val] : m_metrics) {
            result.push_back(val);
        }
        return result;
    }

    /// Get recent events (last N)
    std::vector<AggregatorEvent> GetRecentEvents(uint32_t count = 100) const {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        uint32_t n = (std::min)(count, static_cast<uint32_t>(m_events.size()));
        return std::vector<AggregatorEvent>(m_events.end() - n, m_events.end());
    }

    /// Generate comprehensive health report
    EngineHealthReport GenerateHealthReport() const {
        EngineHealthReport report;

        // Uptime
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        report.uptimeSeconds = static_cast<double>(now.QuadPart - m_startTime.QuadPart) / m_qpcFreq.QuadPart;

        // Version
        report.engineVersion = "15.0.0";

        // Counters
        report.totalErrors = m_totalErrors.load(std::memory_order_relaxed);
        report.totalWarnings = m_totalWarnings.load(std::memory_order_relaxed);

        // Metrics
        {
            std::lock_guard<std::mutex> lock(m_metricMutex);
            auto it = m_metrics.find("thumbnails_generated");
            if (it != m_metrics.end()) {
                report.totalThumbnails = static_cast<uint64_t>(it->second.count);
            }
            it = m_metrics.find("decode_time_ms");
            if (it != m_metrics.end()) {
                report.avgDecodeTimeMs = it->second.avg;
            }
        }

        if (report.uptimeSeconds > 0) {
            report.throughputImgSec = report.totalThumbnails / report.uptimeSeconds;
        }

        // Process memory
        PROCESS_MEMORY_COUNTERS_EX pmc = {};
        pmc.cb = sizeof(pmc);
        if (GetProcessMemoryInfo(GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
            report.processMemoryBytes = pmc.WorkingSetSize;
            report.peakMemoryBytes = pmc.PeakWorkingSetSize;
        }

        // CPU
        SYSTEM_INFO si = {};
        GetSystemInfo(&si);
        report.cpuCores = si.dwNumberOfProcessors;

        return report;
    }

    /// Generate a text diagnostic report
    std::string GenerateDiagnosticText() const {
        auto report = GenerateHealthReport();
        std::ostringstream ss;

        ss << "╔══════════════════════════════════════════╗\n"
            << "║    ExplorerLens Telemetry Report v15     ║\n"
            << "╚══════════════════════════════════════════╝\n\n";

        ss << "Engine: " << report.engineVersion << "\n"
            << "Uptime: " << report.uptimeSeconds << " seconds\n"
            << "CPU Cores: " << report.cpuCores << "\n"
            << "Memory: " << (report.processMemoryBytes / 1024 / 1024) << " MB"
            << " (Peak: " << (report.peakMemoryBytes / 1024 / 1024) << " MB)\n\n";

        ss << "--- Thumbnails ---\n"
            << "Generated: " << report.totalThumbnails << "\n"
            << "Avg Decode: " << report.avgDecodeTimeMs << " ms\n"
            << "Throughput: " << report.throughputImgSec << " img/sec\n\n";

        ss << "--- Errors ---\n"
            << "Errors: " << report.totalErrors << "\n"
            << "Warnings: " << report.totalWarnings << "\n\n";

        ss << "--- Metrics ---\n";
        auto metrics = GetAllMetrics();
        for (const auto& m : metrics) {
            ss << "  " << m.name << ": count=" << m.count
                << " avg=" << m.avg << " min=" << m.min
                << " max=" << m.max << "\n";
        }

        return ss.str();
    }

    /// Reset all telemetry
    void Reset() {
        {
            std::lock_guard<std::mutex> lock(m_eventMutex);
            m_events.clear();
        }
        {
            std::lock_guard<std::mutex> lock(m_metricMutex);
            m_metrics.clear();
        }
        m_totalErrors.store(0);
        m_totalWarnings.store(0);
        QueryPerformanceCounter(&m_startTime);
    }

private:
    TelemetryAggregator() {
        QueryPerformanceFrequency(&m_qpcFreq);
        QueryPerformanceCounter(&m_startTime);
    }

    LARGE_INTEGER           m_qpcFreq = {};
    LARGE_INTEGER           m_startTime = {};
    std::atomic<uint64_t>   m_totalErrors{ 0 };
    std::atomic<uint64_t>   m_totalWarnings{ 0 };

    mutable std::mutex      m_eventMutex;
    std::vector<AggregatorEvent> m_events;

    mutable std::mutex      m_metricMutex;
    std::unordered_map<std::string, AggregatedMetric> m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
