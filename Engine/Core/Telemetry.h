#pragma once
//==============================================================================
// ExplorerLens Engine — Unified Telemetry Module
//
// Consolidates: TelemetryEngine, TelemetryPipeline, TelemetryPipelineV2,
//               TelemetryDashboard, TelemetryHooks
//
// Provides structured telemetry collection, privacy-preserving analytics,
// health scoring, time-series aggregation, diagnostic dashboards, and
// lightweight hooks for production instrumentation.
//==============================================================================

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <map>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

//==============================================================================
// Section 1: Core Telemetry Engine
// Structured telemetry collection, privacy-preserving analytics, health scoring
//==============================================================================

namespace ExplorerLens {
namespace Engine {

/// Telemetry event severity
enum class TelemetrySeverity : uint8_t {
    Debug = 0,
    Info,
    Warning,
    Error,
    Critical
};

/// Telemetry event category
enum class TelemetryCategory : uint8_t {
    Decode = 0,
    Render,
    Cache,
    Plugin,
    Shell,
    GPU,
    Memory,
    IO,
    Network,
    System
};

/// Health score dimensions
enum class HealthDimension : uint8_t {
    Performance = 0,
    Reliability,
    Compatibility,
    Coverage,
    Memory,
    GPU,
    DimensionCount
};

/// Single telemetry event
struct TelemetryEvent
{
    uint64_t timestamp = 0;
    TelemetrySeverity severity = TelemetrySeverity::Info;
    TelemetryCategory category = TelemetryCategory::System;
    std::wstring eventName;
    std::wstring message;
    double value = 0.0;
    std::wstring unit;
    bool piiSafe = true;  // No personally identifiable info
};

/// Health score result
struct HealthScore
{
    double scores[static_cast<int>(HealthDimension::DimensionCount)] = {};
    double overallScore = 0.0;
    std::wstring grade;  // A+, A, B, C, D, F
    std::vector<std::wstring> recommendations;
};

/// Telemetry summary for a session
struct TelemetrySummary
{
    uint64_t totalEvents = 0;
    uint64_t errorCount = 0;
    uint64_t warningCount = 0;
    double avgDecodeMs = 0.0;
    double avgRenderMs = 0.0;
    double cacheHitRate = 0.0;
    uint64_t decodesAttempted = 0;
    uint64_t decodesSucceeded = 0;
    double sessionUptime = 0.0;
};

//------------------------------------------------------------------------------
class TelemetryEngine
{
  public:
    TelemetryEngine();
    ~TelemetryEngine() = default;

    // Event recording
    void RecordEvent(const TelemetryEvent& event);
    void RecordMetric(TelemetryCategory category, const std::wstring& name, double value,
                      const std::wstring& unit = L"");
    void RecordError(TelemetryCategory category, const std::wstring& message);

    // Query
    uint64_t GetEventCount() const;
    uint64_t GetEventCount(TelemetrySeverity severity) const;
    uint64_t GetEventCount(TelemetryCategory category) const;
    std::vector<TelemetryEvent> GetRecentEvents(uint32_t count) const;

    // Health scoring
    HealthScore ComputeHealthScore() const;
    static const wchar_t* GetGrade(double score);

    // Summary
    TelemetrySummary GetSummary() const;
    std::wstring ExportJSON() const;

    // Privacy
    void EnablePrivacyMode(bool enable)
    {
        m_privacyMode = enable;
    }
    bool IsPrivacyMode() const
    {
        return m_privacyMode;
    }
    void PurgeEvents();

    // Helpers
    static const wchar_t* GetSeverityName(TelemetrySeverity severity);
    static const wchar_t* GetCategoryName(TelemetryCategory category);
    static const wchar_t* GetDimensionName(HealthDimension dimension);

  private:
    mutable std::mutex m_mutex;
    std::vector<TelemetryEvent> m_events;
    bool m_privacyMode = false;
    uint64_t m_sessionStartMs = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens

//==============================================================================
// Section 2: Telemetry Pipeline
// Time-series aggregation with decode metrics, GPU utilization, cache
// performance, memory pressure, and error rates
//==============================================================================

namespace ExplorerLens {
namespace Engine {

/// Telemetry event categories (pipeline)
enum class PipelineTelemetryCategory : uint8_t {
    DecodePerformance = 0,
    CacheHitRate = 1,
    GPUUtilization = 2,
    MemoryPressure = 3,
    ErrorRate = 4,
    FormatDistribution = 5,
    PluginHealth = 6,
    ShellIntegration = 7,
    UserInteraction = 8,
    SystemHealth = 9,
    COUNT
};

/// Time window for aggregation
enum class TimeWindow : uint8_t {
    Last1Min = 0,
    Last5Min = 1,
    Last15Min = 2,
    Last1Hour = 3,
    Last24Hour = 4,
    Lifetime = 5,
    COUNT
};

/// A single telemetry sample
struct TelemetrySample
{
    PipelineTelemetryCategory category = PipelineTelemetryCategory::DecodePerformance;
    double value = 0.0;
    uint64_t timestampMs = 0;
    std::wstring label;
    std::wstring unit;
};

/// Aggregated telemetry statistics for a time window
struct TelemetryAggregate
{
    PipelineTelemetryCategory category = PipelineTelemetryCategory::DecodePerformance;
    TimeWindow window = TimeWindow::Last1Min;
    double min = 0.0;
    double max = 0.0;
    double mean = 0.0;
    double median = 0.0;
    double p95 = 0.0;
    double p99 = 0.0;
    double stdDev = 0.0;
    uint64_t sampleCount = 0;
};

/// System health snapshot
struct SystemHealthSnapshot
{
    double cpuUsagePercent = 0.0;
    uint64_t availableMemoryMB = 0;
    uint64_t peakWorkingSetMB = 0;
    double gpuUsagePercent = 0.0;
    uint64_t gpuMemoryUsedMB = 0;
    uint32_t activeDecoderThreads = 0;
    uint32_t pendingDecodeRequests = 0;
    uint64_t cacheEntryCount = 0;
    double cacheHitRate = 0.0;
    double avgDecodeTimeMs = 0.0;
    uint64_t totalThumbnailsGenerated = 0;
    uint64_t uptimeSeconds = 0;
};

/// Telemetry Pipeline
class TelemetryPipeline
{
  public:
    static const wchar_t* CategoryName(PipelineTelemetryCategory c)
    {
        switch (c) {
            case PipelineTelemetryCategory::DecodePerformance:
                return L"Decode Performance";
            case PipelineTelemetryCategory::CacheHitRate:
                return L"Cache Hit Rate";
            case PipelineTelemetryCategory::GPUUtilization:
                return L"GPU Utilization";
            case PipelineTelemetryCategory::MemoryPressure:
                return L"Memory Pressure";
            case PipelineTelemetryCategory::ErrorRate:
                return L"Error Rate";
            case PipelineTelemetryCategory::FormatDistribution:
                return L"Format Distribution";
            case PipelineTelemetryCategory::PluginHealth:
                return L"Plugin Health";
            case PipelineTelemetryCategory::ShellIntegration:
                return L"Shell Integration";
            case PipelineTelemetryCategory::UserInteraction:
                return L"User Interaction";
            case PipelineTelemetryCategory::SystemHealth:
                return L"System Health";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* WindowName(TimeWindow w)
    {
        switch (w) {
            case TimeWindow::Last1Min:
                return L"1 Minute";
            case TimeWindow::Last5Min:
                return L"5 Minutes";
            case TimeWindow::Last15Min:
                return L"15 Minutes";
            case TimeWindow::Last1Hour:
                return L"1 Hour";
            case TimeWindow::Last24Hour:
                return L"24 Hours";
            case TimeWindow::Lifetime:
                return L"Lifetime";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t CategoryCount()
    {
        return static_cast<size_t>(PipelineTelemetryCategory::COUNT);
    }

    static constexpr size_t WindowCount()
    {
        return static_cast<size_t>(TimeWindow::COUNT);
    }

    /// Generate a health snapshot from current system state
    static SystemHealthSnapshot CaptureHealth()
    {
        SystemHealthSnapshot snap;
        snap.uptimeSeconds = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count());
        return snap;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens

//==============================================================================
// Section 3: Telemetry Pipeline V2
// Privacy-first structured telemetry with differential privacy,
// local aggregation, and configurable data retention policies
//==============================================================================

namespace ExplorerLens {
namespace Engine {

/// Telemetry data classification
enum class TelemetryLevel : uint8_t {
    Off = 0,          // No telemetry
    DiagnosticsOnly,  // Crash/error data only
    BasicUsage,       // Feature usage counts (anonymized)
    Enhanced,         // Performance metrics + usage
    Full,             // All data (opt-in only)
    COUNT
};

/// Privacy mechanism applied
enum class PrivacyMechanism : uint8_t {
    None = 0,             // Raw data (Full mode only)
    LocalAggregation,     // Aggregate before send
    KAnonymity,           // k-anonymity grouping
    DifferentialPrivacy,  // Laplace noise injection
    SecureAggregation,    // Encrypted aggregation
    COUNT
};

struct TelemetryV2Event
{
    uint64_t eventId = 0;
    const wchar_t* category = nullptr;
    const wchar_t* action = nullptr;
    double value = 0.0;
    TelemetryLevel level = TelemetryLevel::BasicUsage;
    PrivacyMechanism privacy = PrivacyMechanism::LocalAggregation;
    uint64_t timestampMs = 0;
    bool isSampled = false;
};

struct TelemetryPipelineConfig
{
    TelemetryLevel level = TelemetryLevel::BasicUsage;
    PrivacyMechanism defaultPrivacy = PrivacyMechanism::LocalAggregation;
    double samplingRate = 0.1;  // 10% sampling
    uint32_t batchSize = 100;
    uint32_t flushIntervalSec = 300;  // 5 minutes
    uint32_t retentionDays = 30;
    double epsilonDP = 1.0;  // Differential privacy epsilon
    bool consentRequired = true;
    bool offlineBuffer = true;
};

struct TelemetryStats
{
    uint64_t eventsCollected = 0;
    uint64_t eventsSent = 0;
    uint64_t eventsDropped = 0;
    uint64_t batchesFlushed = 0;
    double avgBatchSizeKB = 0.0;
    size_t bufferUsedBytes = 0;
};

class TelemetryPipelineV2
{
  public:
    static constexpr size_t LevelCount()
    {
        return static_cast<size_t>(TelemetryLevel::COUNT);
    }
    static constexpr size_t PrivacyCount()
    {
        return static_cast<size_t>(PrivacyMechanism::COUNT);
    }

    static const wchar_t* LevelName(TelemetryLevel l)
    {
        switch (l) {
            case TelemetryLevel::Off:
                return L"Off";
            case TelemetryLevel::DiagnosticsOnly:
                return L"Diagnostics Only";
            case TelemetryLevel::BasicUsage:
                return L"Basic Usage";
            case TelemetryLevel::Enhanced:
                return L"Enhanced";
            case TelemetryLevel::Full:
                return L"Full";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* PrivacyName(PrivacyMechanism p)
    {
        switch (p) {
            case PrivacyMechanism::None:
                return L"None";
            case PrivacyMechanism::LocalAggregation:
                return L"Local Aggregation";
            case PrivacyMechanism::KAnonymity:
                return L"k-Anonymity";
            case PrivacyMechanism::DifferentialPrivacy:
                return L"Differential Privacy";
            case PrivacyMechanism::SecureAggregation:
                return L"Secure Aggregation";
            default:
                return L"Unknown";
        }
    }

    /// Check if level requires explicit consent
    static bool RequiresConsent(TelemetryLevel level)
    {
        return level >= TelemetryLevel::Enhanced;
    }

    /// Minimum privacy for given level
    static PrivacyMechanism MinPrivacy(TelemetryLevel level)
    {
        switch (level) {
            case TelemetryLevel::Off:
                return PrivacyMechanism::None;
            case TelemetryLevel::DiagnosticsOnly:
                return PrivacyMechanism::LocalAggregation;
            case TelemetryLevel::BasicUsage:
                return PrivacyMechanism::KAnonymity;
            case TelemetryLevel::Enhanced:
                return PrivacyMechanism::DifferentialPrivacy;
            case TelemetryLevel::Full:
                return PrivacyMechanism::LocalAggregation;
            default:
                return PrivacyMechanism::DifferentialPrivacy;
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens

//==============================================================================
// Section 4: Telemetry Hooks
// Lightweight telemetry hooks, RAII scope, singleton collector
//==============================================================================

namespace ExplorerLens {
namespace Engine {

/// Telemetry hook event types (renamed from TelemetryEvent to avoid
/// collision with the TelemetryEvent struct in Section 1)
enum class TelemetryEventType {
    DecoderSuccess,
    DecoderFailure,
    CacheHit,
    CacheMiss,
    GPUUsed,
    GPUFallback,
    SlowPath,
    FastPath
};

/// Telemetry data point (hooks)
struct TelemetryData
{
    TelemetryEventType event;
    std::wstring category;
    std::wstring detail;
    uint64_t value;
    std::chrono::steady_clock::time_point timestamp;
};

/// Lightweight telemetry collection system (singleton)
class TelemetryCollector
{
  public:
    static TelemetryCollector& GetInstance()
    {
        static TelemetryCollector instance;
        return instance;
    }

    using TelemetryCallback = std::function<void(const TelemetryData&)>;

    void SetCallback(TelemetryCallback callback)
    {
        m_callback = callback;
    }

    void Record(TelemetryEventType event, const wchar_t* category = nullptr, const wchar_t* detail = nullptr,
                uint64_t value = 0)
    {
        TelemetryData data;
        data.event = event;
        data.category = category ? category : L"";
        data.detail = detail ? detail : L"";
        data.value = value;
        data.timestamp = std::chrono::steady_clock::now();

        if (m_callback) {
            m_callback(data);
        }

        // Update counters
        m_totalEvents++;
        UpdateEventCounter(event);
    }

    uint64_t GetTotalEvents() const
    {
        return m_totalEvents.load();
    }
    uint64_t GetEventCount(TelemetryEventType event) const
    {
        size_t idx = static_cast<size_t>(event);
        return idx < m_eventCounts.size() ? m_eventCounts[idx].load() : 0;
    }

  private:
    TelemetryCollector() : m_eventCounts(8) {}  // 8 event types
    ~TelemetryCollector() = default;
    TelemetryCollector(const TelemetryCollector&) = delete;
    TelemetryCollector& operator=(const TelemetryCollector&) = delete;

    void UpdateEventCounter(TelemetryEventType event)
    {
        size_t idx = static_cast<size_t>(event);
        if (idx < m_eventCounts.size()) {
            m_eventCounts[idx]++;
        }
    }

    TelemetryCallback m_callback;
    std::atomic<uint64_t> m_totalEvents{0};
    std::vector<std::atomic<uint64_t>> m_eventCounts;
};

/// RAII telemetry scope for measuring duration
class TelemetryScope
{
  public:
    TelemetryScope(TelemetryEventType event, const wchar_t* category)
        : m_event(event)
        , m_category(category)
        , m_start(std::chrono::high_resolution_clock::now())
    {}

    ~TelemetryScope()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();

        TelemetryCollector::GetInstance().Record(m_event, m_category.c_str(), L"duration",
                                                 static_cast<uint64_t>(durationMs));
    }

  private:
    TelemetryEventType m_event;
    std::wstring m_category;
    std::chrono::high_resolution_clock::time_point m_start;
};

#define DT_TELEMETRY(event, category) ExplorerLens::Engine::TelemetryScope __dtTelem(event, category)

}  // namespace Engine
}  // namespace ExplorerLens

//==============================================================================
// Section 5: Telemetry Dashboard
// Diagnostic collection, health scoring, performance trend analysis,
// real-time dashboard data model, and system resource monitoring
//==============================================================================

namespace ExplorerLens::Engine::Core {

//------------------------------------------------------------------------------
// Health Level — Component health status
//------------------------------------------------------------------------------

enum class HealthLevel : uint8_t {
    Healthy,    // Green — all metrics within normal range
    Degraded,   // Yellow — some metrics borderline
    Unhealthy,  // Red — failures detected, needs attention
    Critical,   // Black — non-functional, immediate action needed
    Unknown     // Grey — insufficient data
};

inline const char* HealthLevelName(HealthLevel h)
{
    switch (h) {
        case HealthLevel::Healthy:
            return "Healthy";
        case HealthLevel::Degraded:
            return "Degraded";
        case HealthLevel::Unhealthy:
            return "Unhealthy";
        case HealthLevel::Critical:
            return "Critical";
        case HealthLevel::Unknown:
            return "Unknown";
        default:
            return "Unknown";
    }
}

inline int HealthScore(HealthLevel h)
{
    switch (h) {
        case HealthLevel::Healthy:
            return 100;
        case HealthLevel::Degraded:
            return 70;
        case HealthLevel::Unhealthy:
            return 30;
        case HealthLevel::Critical:
            return 0;
        case HealthLevel::Unknown:
            return -1;
        default:
            return -1;
    }
}

//------------------------------------------------------------------------------
// Metric Type — Categories of telemetry data
//------------------------------------------------------------------------------

enum class MetricType : uint8_t {
    Counter,    // Monotonically increasing (requests, errors)
    Gauge,      // Current value (memory, queue depth)
    Histogram,  // Distribution (latency percentiles)
    Timer       // Duration measurement
};

inline const char* MetricTypeName(MetricType t)
{
    switch (t) {
        case MetricType::Counter:
            return "Counter";
        case MetricType::Gauge:
            return "Gauge";
        case MetricType::Histogram:
            return "Histogram";
        case MetricType::Timer:
            return "Timer";
        default:
            return "Unknown";
    }
}

//------------------------------------------------------------------------------
// Metric Sample — Single data point
//------------------------------------------------------------------------------

struct TelemetryMetricSample
{
    std::string name;
    double value = 0.0;
    int64_t timestamp = 0;  // Unix ms
    MetricType type = MetricType::Gauge;
    std::string unit;  // "ms", "bytes", "count", "%"

    std::string FormattedValue() const
    {
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
            if (!unit.empty())
                ss << " " << unit;
        }
        return ss.str();
    }
};

//------------------------------------------------------------------------------
// Statistics — Computed distribution stats
//------------------------------------------------------------------------------

struct Statistics
{
    double min = 0.0;
    double max = 0.0;
    double mean = 0.0;
    double median = 0.0;
    double p95 = 0.0;
    double p99 = 0.0;
    double stddev = 0.0;
    size_t count = 0;

    bool IsEmpty() const
    {
        return count == 0;
    }

    static Statistics Compute(std::vector<double> values)
    {
        Statistics s;
        if (values.empty())
            return s;

        s.count = values.size();
        std::sort(values.begin(), values.end());

        s.min = values.front();
        s.max = values.back();
        s.median = values[values.size() / 2];
        s.p95 = values[static_cast<size_t>(values.size() * 0.95)];
        s.p99 = values[(std::min)(static_cast<size_t>(values.size() * 0.99), values.size() - 1)];

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

    std::string Summary() const
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << "n=" << count << " min=" << min << " max=" << max
           << " mean=" << mean << " p95=" << p95 << " p99=" << p99;
        return ss.str();
    }
};

//------------------------------------------------------------------------------
// Decoder Telemetry — Per-decoder performance data
//------------------------------------------------------------------------------

struct DecoderTelemetry
{
    std::string decoderName;
    size_t totalDecodes = 0;
    size_t successCount = 0;
    size_t failureCount = 0;
    size_t timeoutCount = 0;
    double avgDecodeMs = 0.0;
    double p95DecodeMs = 0.0;
    double maxDecodeMs = 0.0;
    uint64_t totalBytesIn = 0;
    uint64_t totalBytesOut = 0;

    double SuccessRate() const
    {
        if (totalDecodes == 0)
            return 0.0;
        return (static_cast<double>(successCount) / static_cast<double>(totalDecodes)) * 100.0;
    }

    double FailureRate() const
    {
        if (totalDecodes == 0)
            return 0.0;
        return (static_cast<double>(failureCount) / static_cast<double>(totalDecodes)) * 100.0;
    }

    HealthLevel Health() const
    {
        if (totalDecodes == 0)
            return HealthLevel::Unknown;
        double failRate = FailureRate();
        if (failRate > 50.0)
            return HealthLevel::Critical;
        if (failRate > 20.0)
            return HealthLevel::Unhealthy;
        if (failRate > 5.0)
            return HealthLevel::Degraded;
        return HealthLevel::Healthy;
    }
};

//------------------------------------------------------------------------------
// Cache Telemetry — Cache performance metrics
//------------------------------------------------------------------------------

struct CacheTelemetry
{
    size_t totalRequests = 0;
    size_t hits = 0;
    size_t misses = 0;
    size_t evictions = 0;
    uint64_t currentSizeBytes = 0;
    uint64_t maxSizeBytes = 0;
    size_t entryCount = 0;
    double avgLookupMs = 0.0;

    double HitRate() const
    {
        if (totalRequests == 0)
            return 0.0;
        return (static_cast<double>(hits) / static_cast<double>(totalRequests)) * 100.0;
    }

    double MissRate() const
    {
        return 100.0 - HitRate();
    }

    double Utilization() const
    {
        if (maxSizeBytes == 0)
            return 0.0;
        return (static_cast<double>(currentSizeBytes) / static_cast<double>(maxSizeBytes)) * 100.0;
    }

    HealthLevel Health() const
    {
        if (totalRequests == 0)
            return HealthLevel::Unknown;
        double hitRate = HitRate();
        if (hitRate >= 80.0)
            return HealthLevel::Healthy;
        if (hitRate >= 50.0)
            return HealthLevel::Degraded;
        return HealthLevel::Unhealthy;
    }
};

//------------------------------------------------------------------------------
// System Metrics — Resource usage
//------------------------------------------------------------------------------

struct SystemMetrics
{
    double cpuUsagePercent = 0.0;
    uint64_t memoryUsedBytes = 0;
    uint64_t memoryTotalBytes = 0;
    uint64_t diskUsedBytes = 0;
    uint64_t diskTotalBytes = 0;
    uint32_t threadCount = 0;
    uint32_t handleCount = 0;
    double gpuUsagePercent = 0.0;
    uint64_t gpuMemoryUsed = 0;

    double MemoryUsagePercent() const
    {
        if (memoryTotalBytes == 0)
            return 0.0;
        return (static_cast<double>(memoryUsedBytes) / static_cast<double>(memoryTotalBytes)) * 100.0;
    }

    double DiskUsagePercent() const
    {
        if (diskTotalBytes == 0)
            return 0.0;
        return (static_cast<double>(diskUsedBytes) / static_cast<double>(diskTotalBytes)) * 100.0;
    }

    HealthLevel OverallHealth() const
    {
        if (cpuUsagePercent > 90.0 || MemoryUsagePercent() > 95.0)
            return HealthLevel::Critical;
        if (cpuUsagePercent > 75.0 || MemoryUsagePercent() > 85.0)
            return HealthLevel::Degraded;
        return HealthLevel::Healthy;
    }
};

//------------------------------------------------------------------------------
// Dashboard Data — Complete diagnostic snapshot
//------------------------------------------------------------------------------

struct DashboardData
{
    // Timestamps
    int64_t snapshotTime = 0;
    int64_t uptimeMs = 0;
    std::string version;

    // Component health
    std::vector<DecoderTelemetry> decoders;
    CacheTelemetry cache;
    SystemMetrics system;

    // Aggregate stats
    size_t totalThumbnailsGenerated = 0;
    size_t totalErrors = 0;
    double overallSuccessRate = 0.0;

    HealthLevel OverallHealth() const
    {
        if (totalErrors > 100 || system.OverallHealth() == HealthLevel::Critical)
            return HealthLevel::Critical;
        if (cache.Health() == HealthLevel::Unhealthy)
            return HealthLevel::Degraded;
        return HealthLevel::Healthy;
    }

    std::string UptimeHuman() const
    {
        int64_t seconds = uptimeMs / 1000;
        int64_t hours = seconds / 3600;
        int64_t minutes = (seconds % 3600) / 60;
        int64_t secs = seconds % 60;
        std::ostringstream ss;
        if (hours > 0)
            ss << hours << "h ";
        if (minutes > 0 || hours > 0)
            ss << minutes << "m ";
        ss << secs << "s";
        return ss.str();
    }

    size_t HealthyDecoderCount() const
    {
        return std::count_if(decoders.begin(), decoders.end(),
                             [](const DecoderTelemetry& d) { return d.Health() == HealthLevel::Healthy; });
    }
};

//------------------------------------------------------------------------------
// Diagnostic Export — Serialize dashboard data for reporting
//------------------------------------------------------------------------------

struct DiagnosticExport
{
    static std::string ToText(const DashboardData& data)
    {
        std::ostringstream ss;
        ss << "=== ExplorerLens Diagnostics ===" << "\n"
           << "Version: " << data.version << "\n"
           << "Uptime: " << data.UptimeHuman() << "\n"
           << "Overall Health: " << HealthLevelName(data.OverallHealth()) << "\n"
           << "Thumbnails Generated: " << data.totalThumbnailsGenerated << "\n"
           << "Total Errors: " << data.totalErrors << "\n"
           << "\n--- Cache ---\n"
           << "Hit Rate: " << std::fixed << std::setprecision(1) << data.cache.HitRate() << "%" << "\n"
           << "Entries: " << data.cache.entryCount << "\n"
           << "Utilization: " << data.cache.Utilization() << "%" << "\n"
           << "\n--- Decoders (" << data.decoders.size() << ") ---\n";

        for (const auto& dec : data.decoders) {
            ss << "  " << dec.decoderName << ": " << dec.successCount << "/" << dec.totalDecodes << " ("
               << std::setprecision(1) << dec.SuccessRate() << "%), " << std::setprecision(2) << dec.avgDecodeMs
               << " ms avg"
               << " [" << HealthLevelName(dec.Health()) << "]" << "\n";
        }

        return ss.str();
    }

    static std::string ToJSON(const DashboardData& data)
    {
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

//------------------------------------------------------------------------------
// Diagnostics Config — What telemetry to collect
//------------------------------------------------------------------------------

struct DiagnosticsConfig
{
    bool enabled = true;
    bool collectTimings = true;
    bool collectMemory = true;
    bool collectGPU = false;
    bool collectDecoderStats = true;
    uint32_t sampleIntervalMs = 5000;  // 5 seconds
    uint32_t retentionMinutes = 60;    // 1 hour of history

    static DiagnosticsConfig Default()
    {
        return {};
    }

    static DiagnosticsConfig Detailed()
    {
        DiagnosticsConfig c;
        c.collectGPU = true;
        c.sampleIntervalMs = 1000;
        c.retentionMinutes = 1440;  // 24 hours
        return c;
    }

    static DiagnosticsConfig Minimal()
    {
        DiagnosticsConfig c;
        c.collectTimings = false;
        c.collectMemory = false;
        c.sampleIntervalMs = 30000;
        c.retentionMinutes = 15;
        return c;
    }

    static DiagnosticsConfig Disabled()
    {
        DiagnosticsConfig c;
        c.enabled = false;
        return c;
    }
};

}  // namespace ExplorerLens::Engine::Core
