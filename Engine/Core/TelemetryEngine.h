#pragma once
//==============================================================================
// TelemetryEngine — Sprint 207
// Structured telemetry collection, privacy-preserving analytics, health scoring
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <mutex>

namespace DarkThumbs { namespace Engine {

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
struct TelemetryEvent {
    uint64_t          timestamp = 0;
    TelemetrySeverity severity = TelemetrySeverity::Info;
    TelemetryCategory category = TelemetryCategory::System;
    std::wstring      eventName;
    std::wstring      message;
    double            value = 0.0;
    std::wstring      unit;
    bool              piiSafe = true;  // No personally identifiable info
};

/// Health score result
struct HealthScore {
    double scores[static_cast<int>(HealthDimension::DimensionCount)] = {};
    double overallScore = 0.0;
    std::wstring grade;  // A+, A, B, C, D, F
    std::vector<std::wstring> recommendations;
};

/// Telemetry summary for a session
struct TelemetrySummary {
    uint64_t totalEvents = 0;
    uint64_t errorCount = 0;
    uint64_t warningCount = 0;
    double   avgDecodeMs = 0.0;
    double   avgRenderMs = 0.0;
    double   cacheHitRate = 0.0;
    uint64_t decodesAttempted = 0;
    uint64_t decodesSucceeded = 0;
    double   sessionUptime = 0.0;
};

//------------------------------------------------------------------------------
class TelemetryEngine {
public:
    TelemetryEngine();
    ~TelemetryEngine() = default;

    // Event recording
    void RecordEvent(const TelemetryEvent& event);
    void RecordMetric(TelemetryCategory category, const std::wstring& name,
                      double value, const std::wstring& unit = L"");
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
    void EnablePrivacyMode(bool enable) { m_privacyMode = enable; }
    bool IsPrivacyMode() const { return m_privacyMode; }
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

}} // namespace DarkThumbs::Engine
