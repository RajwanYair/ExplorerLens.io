// TelemetryAnalyticsEngine.h — Privacy-first telemetry and usage analytics
// Copyright (c) 2026 ExplorerLens Project
//
// Provides opt-in telemetry collection with configurable consent levels,
// aggregation periods, and local-only processing for policy compliance.
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <cwchar>

namespace ExplorerLens {
namespace Engine {

enum class AnalyticsEventType : uint8_t {
    ThumbnailGenerated = 0,
    CacheHit = 1,
    CacheMiss = 2,
    DecoderError = 3,
    GPUFallback = 4,
    PluginLoaded = 5,
    SessionStart = 6,
    SessionEnd = 7,
    COUNT
};

enum class TelemetryConsent : uint8_t {
    Disabled = 0,
    LocalOnly = 1,
    Anonymous = 2,
    Enhanced = 3,
    COUNT
};

enum class AggregationPeriod : uint8_t {
    Hourly = 0,
    Daily = 1,
    Weekly = 2,
    Monthly = 3,
    COUNT
};

struct TelemetryConfig
{
    TelemetryConsent consent = TelemetryConsent::Disabled;
    bool localOnly = true;
    bool anonymize = true;
    uint32_t maxEvents = 10000;
};

class TelemetryAnalyticsEngine
{
  public:
    static const wchar_t* EventName(AnalyticsEventType e) noexcept
    {
        switch (e) {
            case AnalyticsEventType::ThumbnailGenerated:
                return L"ThumbnailGenerated";
            case AnalyticsEventType::CacheHit:
                return L"CacheHit";
            case AnalyticsEventType::CacheMiss:
                return L"CacheMiss";
            case AnalyticsEventType::DecoderError:
                return L"DecoderError";
            case AnalyticsEventType::GPUFallback:
                return L"GPUFallback";
            case AnalyticsEventType::PluginLoaded:
                return L"PluginLoaded";
            case AnalyticsEventType::SessionStart:
                return L"SessionStart";
            case AnalyticsEventType::SessionEnd:
                return L"SessionEnd";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* ConsentName(TelemetryConsent c) noexcept
    {
        switch (c) {
            case TelemetryConsent::Disabled:
                return L"Disabled";
            case TelemetryConsent::LocalOnly:
                return L"Local Only";
            case TelemetryConsent::Anonymous:
                return L"Anonymous";
            case TelemetryConsent::Enhanced:
                return L"Enhanced";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* PeriodName(AggregationPeriod p) noexcept
    {
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

    static double CacheHitRate(uint64_t hits, uint64_t misses) noexcept
    {
        uint64_t total = hits + misses;
        return total == 0 ? 0.0 : static_cast<double>(hits) / static_cast<double>(total);
    }

    static constexpr size_t EventCount() noexcept
    {
        return static_cast<size_t>(AnalyticsEventType::COUNT);
    }

    static constexpr size_t ConsentCount() noexcept
    {
        return static_cast<size_t>(TelemetryConsent::COUNT);
    }

    static constexpr size_t PeriodCount() noexcept
    {
        return static_cast<size_t>(AggregationPeriod::COUNT);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
