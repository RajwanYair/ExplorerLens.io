#pragma once
// Sprint 230: Telemetry Engine — anonymous usage telemetry with opt-in/out controls
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Telemetry event category
enum class TelemetryCategory : uint32_t {
    Decode     = 0,
    Cache      = 1,
    GPU        = 2,
    Shell      = 3,
    Error      = 4,
    Performance = 5,
    UserAction = 6,
    COUNT      = 7
};

/// Consent level for telemetry
enum class ConsentLevel : uint32_t {
    None       = 0,   ///< No telemetry
    Basic      = 1,   ///< Crash + error only
    Enhanced   = 2,   ///< + performance data
    Full       = 3,   ///< All categories
    COUNT      = 4
};

/// A single telemetry event
struct TelemetryEvent {
    std::wstring      name;
    TelemetryCategory category = TelemetryCategory::Decode;
    double            value    = 0.0;
    uint64_t          timestamp = 0;
    bool              sent     = false;
};

/// Collects and manages telemetry with privacy controls
class TelemetryEngine {
public:
    TelemetryEngine();

    static const wchar_t* GetCategoryName(TelemetryCategory cat);
    static const wchar_t* GetConsentName(ConsentLevel level);
    static uint32_t GetCategoryCount() { return static_cast<uint32_t>(TelemetryCategory::COUNT); }

    void SetConsent(ConsentLevel level) { m_consent = level; }
    ConsentLevel GetConsent() const { return m_consent; }

    /// Record a telemetry event (respects consent)
    bool RecordEvent(const std::wstring& name, TelemetryCategory cat, double value = 0.0);
    /// Get queued events
    const std::vector<TelemetryEvent>& GetEvents() const { return m_events; }
    /// Flush all events
    void Flush();
    /// Total events recorded
    size_t GetEventCount() const { return m_events.size(); }

private:
    ConsentLevel m_consent = ConsentLevel::None;
    std::vector<TelemetryEvent> m_events;
    bool IsAllowed(TelemetryCategory cat) const;
};

}} // namespace DarkThumbs::Engine
