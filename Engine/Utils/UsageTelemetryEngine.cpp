#include "UsageTelemetryEngine.h"
#include <chrono>

namespace ExplorerLens {
namespace Engine {

UsageTelemetryEngine::UsageTelemetryEngine()
    : m_consent(UsageConsentLevel::None) {}

const wchar_t *
UsageTelemetryEngine::GetCategoryName(UsageTelemetryCategory cat) {
  switch (cat) {
  case UsageTelemetryCategory::Decode:
    return L"Decode";
  case UsageTelemetryCategory::Cache:
    return L"Cache";
  case UsageTelemetryCategory::GPU:
    return L"GPU";
  case UsageTelemetryCategory::Shell:
    return L"Shell";
  case UsageTelemetryCategory::Error:
    return L"Error";
  case UsageTelemetryCategory::Performance:
    return L"Performance";
  case UsageTelemetryCategory::UserAction:
    return L"User Action";
  default:
    return L"Unknown";
  }
}

const wchar_t *UsageTelemetryEngine::GetConsentName(UsageConsentLevel level) {
  switch (level) {
  case UsageConsentLevel::None:
    return L"None";
  case UsageConsentLevel::Basic:
    return L"Basic";
  case UsageConsentLevel::Enhanced:
    return L"Enhanced";
  case UsageConsentLevel::Full:
    return L"Full";
  default:
    return L"Unknown";
  }
}

bool UsageTelemetryEngine::IsAllowed(UsageTelemetryCategory cat) const {
  if (m_consent == UsageConsentLevel::None)
    return false;
  if (m_consent == UsageConsentLevel::Full)
    return true;
  if (m_consent == UsageConsentLevel::Basic) {
    return cat == UsageTelemetryCategory::Error;
  }
  return cat == UsageTelemetryCategory::Error ||
         cat == UsageTelemetryCategory::Performance;
}

bool UsageTelemetryEngine::RecordEvent(const std::wstring &name,
                                       UsageTelemetryCategory cat,
                                       double value) {
  if (!IsAllowed(cat))
    return false;
  UsageTelemetryEvent evt;
  evt.name = name;
  evt.category = cat;
  evt.value = value;
  auto now = std::chrono::system_clock::now();
  evt.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch())
                      .count();
  m_events.push_back(std::move(evt));
  return true;
}

void UsageTelemetryEngine::Flush() { m_events.clear(); }

} // namespace Engine
} // namespace ExplorerLens
