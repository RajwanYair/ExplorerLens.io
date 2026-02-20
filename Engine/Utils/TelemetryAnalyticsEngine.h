//==============================================================================
// DarkThumbs Engine — Sprint 296: Telemetry & Analytics Engine
// Privacy-respecting usage analytics with opt-in telemetry pipeline.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Telemetry event type
enum class TelemetryEvent : uint8_t {
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
    Disabled,           // No telemetry
    BasicOnly,          // Crash/error only
    UsageStats,         // Anonymous usage patterns
    FullDiagnostics,    // Detailed diagnostics
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
    TelemetryEvent event    = TelemetryEvent::ThumbnailGenerated;
    uint64_t       timestamp = 0;
    uint32_t       count    = 1;
    double         durationMs = 0;
    std::wstring   context;
};

/// Analytics summary
struct AnalyticsSummary {
    uint64_t    totalEvents     = 0;
    uint64_t    thumbnailCount  = 0;
    uint64_t    cacheHits       = 0;
    uint64_t    cacheMisses     = 0;
    uint32_t    errorCount      = 0;
    double      avgLatencyMs    = 0;
    double      cacheHitRate    = 0;
};

/// Telemetry config
struct TelemetryConfig {
    TelemetryConsent consent    = TelemetryConsent::Disabled;
    AggregationPeriod period   = AggregationPeriod::Daily;
    uint32_t    maxBufferSize   = 1000;
    bool        localOnly       = true;     // Never transmit
    bool        anonymize       = true;
};

/// Telemetry & analytics engine
class TelemetryAnalyticsEngine {
public:
    static const wchar_t* EventName(TelemetryEvent e) {
        switch (e) {
            case TelemetryEvent::ThumbnailGenerated: return L"Thumbnail Generated";
            case TelemetryEvent::ThumbnailCacheHit:  return L"Cache Hit";
            case TelemetryEvent::ThumbnailCacheMiss: return L"Cache Miss";
            case TelemetryEvent::DecoderError:       return L"Decoder Error";
            case TelemetryEvent::GPUFallback:        return L"GPU Fallback";
            case TelemetryEvent::FormatDetected:     return L"Format Detected";
            case TelemetryEvent::PluginLoaded:       return L"Plugin Loaded";
            case TelemetryEvent::PerformanceMark:    return L"Performance Mark";
            default: return L"Unknown";
        }
    }

    static const wchar_t* ConsentName(TelemetryConsent c) {
        switch (c) {
            case TelemetryConsent::Disabled:         return L"Disabled";
            case TelemetryConsent::BasicOnly:        return L"Basic Only";
            case TelemetryConsent::UsageStats:       return L"Usage Stats";
            case TelemetryConsent::FullDiagnostics:  return L"Full Diagnostics";
            default: return L"Unknown";
        }
    }

    static const wchar_t* PeriodName(AggregationPeriod p) {
        switch (p) {
            case AggregationPeriod::Hourly:  return L"Hourly";
            case AggregationPeriod::Daily:   return L"Daily";
            case AggregationPeriod::Weekly:  return L"Weekly";
            case AggregationPeriod::Monthly: return L"Monthly";
            default: return L"Unknown";
        }
    }

    static constexpr size_t EventCount() { return static_cast<size_t>(TelemetryEvent::COUNT); }
    static constexpr size_t ConsentCount() { return static_cast<size_t>(TelemetryConsent::COUNT); }
    static constexpr size_t PeriodCount() { return static_cast<size_t>(AggregationPeriod::COUNT); }

    static double CacheHitRate(uint64_t hits, uint64_t misses) {
        uint64_t total = hits + misses;
        return total > 0 ? static_cast<double>(hits) / total : 0.0;
    }
};

}} // namespace DarkThumbs::Engine
