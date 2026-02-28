#pragma once
// UsageTelemetryEngine — anonymous usage telemetry with opt-in/out controls
// Includes TelemetryAnalyticsEngine (merged)
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ─── TelemetryAnalyticsEngine ──────────────────────────────────────────────────

/// Analytics event type
enum class AnalyticsEventType : uint8_t {
    ThumbnailGenerated,
    ThumbnailCacheHit,
    ThumbnailCacheMiss,
    DecoderError,
    GPUFallback,
    FormatDetected,
    PluginLoaded,
    PerformanceMark,
    COUNT
};

/// Telemetry consent level
enum class TelemetryConsent : uint8_t {
    Disabled, // No telemetry
    BasicOnly, // Crash/error only
    UsageStats, // Anonymous usage patterns
    FullDiagnostics, // Detailed diagnostics
    COUNT
};

/// Analytics aggregation period
enum class AggregationPeriod : uint8_t {
    Hourly,
    Daily,
    Weekly,
    Monthly,
    COUNT
};

/// Telemetry data point
struct TelemetryDataPoint {
    AnalyticsEventType event = AnalyticsEventType::ThumbnailGenerated;
    uint64_t timestamp = 0;
    uint32_t count = 1;
    double durationMs = 0;
    std::wstring context;
};

/// Analytics summary
struct AnalyticsSummary {
    uint64_t totalEvents = 0;
    uint64_t thumbnailCount = 0;
    uint64_t cacheHits = 0;
    uint64_t cacheMisses = 0;
    uint32_t errorCount = 0;
    double avgLatencyMs = 0;
    double cacheHitRate = 0;
};

/// Telemetry config
struct TelemetryConfig {
    TelemetryConsent consent = TelemetryConsent::Disabled;
    AggregationPeriod period = AggregationPeriod::Daily;
    uint32_t maxBufferSize = 1000;
    bool localOnly = true; // Never transmit
    bool anonymize = true;
};

/// Telemetry & analytics engine
class TelemetryAnalyticsEngine {
public:
    static const wchar_t* EventName(AnalyticsEventType e) {
        switch (e) {
        case AnalyticsEventType::ThumbnailGenerated:
            return L"Thumbnail Generated";
        case AnalyticsEventType::ThumbnailCacheHit:
            return L"Cache Hit";
        case AnalyticsEventType::ThumbnailCacheMiss:
            return L"Cache Miss";
        case AnalyticsEventType::DecoderError:
            return L"Decoder Error";
        case AnalyticsEventType::GPUFallback:
            return L"GPU Fallback";
        case AnalyticsEventType::FormatDetected:
            return L"Format Detected";
        case AnalyticsEventType::PluginLoaded:
            return L"Plugin Loaded";
        case AnalyticsEventType::PerformanceMark:
            return L"Performance Mark";
        default:
            return L"Unknown";
        }
    }

    static const wchar_t* ConsentName(TelemetryConsent c) {
        switch (c) {
        case TelemetryConsent::Disabled:
            return L"Disabled";
        case TelemetryConsent::BasicOnly:
            return L"Basic Only";
        case TelemetryConsent::UsageStats:
            return L"Usage Stats";
        case TelemetryConsent::FullDiagnostics:
            return L"Full Diagnostics";
        default:
            return L"Unknown";
        }
    }

    static const wchar_t* PeriodName(AggregationPeriod p) {
        switch (p) {
        case AggregationPeriod::Hourly:
            return L"Hourly";
        case AggregationPeriod::Daily:
            return L"Daily";
        case AggregationPeriod::Weekly:
            return L"Weekly";
        case AggregationPeriod::Monthly:
            return L"Monthly";
        default:
            return L"Unknown";
        }
    }

    static constexpr size_t EventCount() {
        return static_cast<size_t>(AnalyticsEventType::COUNT);
    }
    static constexpr size_t ConsentCount() {
        return static_cast<size_t>(TelemetryConsent::COUNT);
    }
    static constexpr size_t PeriodCount() {
        return static_cast<size_t>(AggregationPeriod::COUNT);
    }

    static double CacheHitRate(uint64_t hits, uint64_t misses) {
        uint64_t total = hits + misses;
        return total > 0 ? static_cast<double>(hits) / total : 0.0;
    }
};

// ─── UsageTelemetryEngine ──────────────────────────────────────────────────────

/// Usage telemetry event category
enum class UsageTelemetryCategory : uint32_t {
    Decode = 0,
    Cache = 1,
    GPU = 2,
    Shell = 3,
    Error = 4,
    Performance = 5,
    UserAction = 6,
    COUNT = 7
};

/// Consent level for usage telemetry
enum class UsageConsentLevel : uint32_t {
    None = 0, ///< No telemetry
    Basic = 1, ///< Crash + error only
    Enhanced = 2, ///< + performance data
    Full = 3, ///< All categories
    COUNT = 4
};

/// A single usage telemetry event
struct UsageTelemetryEvent {
    std::wstring name;
    UsageTelemetryCategory category = UsageTelemetryCategory::Decode;
    double value = 0.0;
    uint64_t timestamp = 0;
    bool sent = false;
};

/// Collects and manages anonymous usage telemetry with privacy controls
class UsageTelemetryEngine {
public:
    UsageTelemetryEngine();

    static const wchar_t* GetCategoryName(UsageTelemetryCategory cat);
    static const wchar_t* GetConsentName(UsageConsentLevel level);
    static uint32_t GetCategoryCount() {
        return static_cast<uint32_t>(UsageTelemetryCategory::COUNT);
    }

    void SetConsent(UsageConsentLevel level) { m_consent = level; }
    UsageConsentLevel GetConsent() const { return m_consent; }

    /// Record a telemetry event (respects consent)
    bool RecordEvent(const std::wstring& name, UsageTelemetryCategory cat,
        double value = 0.0);
    /// Get queued events
    const std::vector<UsageTelemetryEvent>& GetEvents() const { return m_events; }
    /// Flush all events
    void Flush();
    /// Total events recorded
    size_t GetEventCount() const { return m_events.size(); }

private:
    UsageConsentLevel m_consent = UsageConsentLevel::None;
    std::vector<UsageTelemetryEvent> m_events;
    bool IsAllowed(UsageTelemetryCategory cat) const;
};

} // namespace Engine
} // namespace ExplorerLens
