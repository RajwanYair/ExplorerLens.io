#include "TelemetryEngine.h"
#include <chrono>

namespace DarkThumbs { namespace Engine {

TelemetryEngine::TelemetryEngine() : m_consent(ConsentLevel::None) {}

const wchar_t* TelemetryEngine::GetCategoryName(TelemetryCategory cat) {
    switch (cat) {
        case TelemetryCategory::Decode:      return L"Decode";
        case TelemetryCategory::Cache:       return L"Cache";
        case TelemetryCategory::GPU:         return L"GPU";
        case TelemetryCategory::Shell:       return L"Shell";
        case TelemetryCategory::Error:       return L"Error";
        case TelemetryCategory::Performance: return L"Performance";
        case TelemetryCategory::UserAction:  return L"User Action";
        default:                             return L"Unknown";
    }
}

const wchar_t* TelemetryEngine::GetConsentName(ConsentLevel level) {
    switch (level) {
        case ConsentLevel::None:     return L"None";
        case ConsentLevel::Basic:    return L"Basic";
        case ConsentLevel::Enhanced: return L"Enhanced";
        case ConsentLevel::Full:     return L"Full";
        default:                     return L"Unknown";
    }
}

bool TelemetryEngine::IsAllowed(TelemetryCategory cat) const {
    if (m_consent == ConsentLevel::None) return false;
    if (m_consent == ConsentLevel::Full) return true;
    if (m_consent == ConsentLevel::Basic) {
        return cat == TelemetryCategory::Error;
    }
    // Enhanced: error + performance
    return cat == TelemetryCategory::Error || cat == TelemetryCategory::Performance;
}

bool TelemetryEngine::RecordEvent(const std::wstring& name, TelemetryCategory cat, double value) {
    if (!IsAllowed(cat)) return false;
    TelemetryEvent evt;
    evt.name = name;
    evt.category = cat;
    evt.value = value;
    auto now = std::chrono::system_clock::now();
    evt.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    m_events.push_back(std::move(evt));
    return true;
}

void TelemetryEngine::Flush() {
    m_events.clear();
}

}} // namespace DarkThumbs::Engine
