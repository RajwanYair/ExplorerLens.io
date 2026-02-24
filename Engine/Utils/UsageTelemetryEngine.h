#pragma once
// UsageTelemetryEngine — anonymous usage telemetry with opt-in/out controls
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

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
  None = 0,     ///< No telemetry
  Basic = 1,    ///< Crash + error only
  Enhanced = 2, ///< + performance data
  Full = 3,     ///< All categories
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

  static const wchar_t *GetCategoryName(UsageTelemetryCategory cat);
  static const wchar_t *GetConsentName(UsageConsentLevel level);
  static uint32_t GetCategoryCount() {
    return static_cast<uint32_t>(UsageTelemetryCategory::COUNT);
  }

  void SetConsent(UsageConsentLevel level) { m_consent = level; }
  UsageConsentLevel GetConsent() const { return m_consent; }

  /// Record a telemetry event (respects consent)
  bool RecordEvent(const std::wstring &name, UsageTelemetryCategory cat,
                   double value = 0.0);
  /// Get queued events
  const std::vector<UsageTelemetryEvent> &GetEvents() const { return m_events; }
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
