// UserPreferenceEngine.h — User Preference Management
// Copyright (c) 2026 ExplorerLens Project
//
// Manages user preferences for thumbnail quality, size, overlay display,
// theme selection, and per-format settings. Persists to registry and
// supports enterprise policy override for managed environments.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class PreferenceKey : uint8_t {
    ThumbnailQuality, ThumbnailSize, OverlayEnabled, ThemeMode,
    CacheEnabled, GPUEnabled, AnimationEnabled, AccessibilityMode, COUNT
};

enum class PreferenceSource : uint8_t {
    Default, UserSetting, EnterprisePolicy, CommandLine, COUNT
};

struct PreferenceValue {
    PreferenceKey key = PreferenceKey::ThumbnailQuality;
    PreferenceSource source = PreferenceSource::Default;
    int32_t intValue = 0;
    float floatValue = 0.0f;
    std::wstring stringValue;
    bool boolValue = false;
};

struct PreferenceStats {
    uint32_t totalPrefs = 0;
    uint32_t userOverrides = 0;
    uint32_t policyOverrides = 0;
    uint32_t defaultsUsed = 0;
};

class UserPreferenceEngine {
public:
    void SetPreference(const PreferenceValue& pref) {
        uint8_t idx = static_cast<uint8_t>(pref.key);
        if (idx < static_cast<uint8_t>(PreferenceKey::COUNT)) {
            m_prefs[idx] = pref;
            m_set[idx] = true;
        }
    }

    PreferenceValue GetPreference(PreferenceKey key) const {
        uint8_t idx = static_cast<uint8_t>(key);
        if (idx < static_cast<uint8_t>(PreferenceKey::COUNT) && m_set[idx]) {
            return m_prefs[idx];
        }
        PreferenceValue def;
        def.key = key;
        def.source = PreferenceSource::Default;
        return def;
    }

    bool IsSet(PreferenceKey key) const {
        uint8_t idx = static_cast<uint8_t>(key);
        return idx < static_cast<uint8_t>(PreferenceKey::COUNT) && m_set[idx];
    }

    int32_t GetInt(PreferenceKey key, int32_t def = 0) const {
        auto p = GetPreference(key);
        return (p.source != PreferenceSource::Default) ? p.intValue : def;
    }

    bool GetBool(PreferenceKey key, bool def = false) const {
        auto p = GetPreference(key);
        return (p.source != PreferenceSource::Default) ? p.boolValue : def;
    }

    PreferenceStats ComputeStats() const {
        PreferenceStats stats;
        for (uint8_t i = 0; i < static_cast<uint8_t>(PreferenceKey::COUNT); ++i) {
            stats.totalPrefs++;
            if (!m_set[i]) { stats.defaultsUsed++; continue; }
            switch (m_prefs[i].source) {
            case PreferenceSource::UserSetting: stats.userOverrides++; break;
            case PreferenceSource::EnterprisePolicy: stats.policyOverrides++; break;
            default: stats.defaultsUsed++; break;
            }
        }
        return stats;
    }

    void Reset() {
        for (auto& s : m_set) s = false;
    }

    static size_t KeyCount() { return static_cast<size_t>(PreferenceKey::COUNT); }
    static size_t SourceCount() { return static_cast<size_t>(PreferenceSource::COUNT); }

private:
    static constexpr size_t MAX_PREFS = static_cast<size_t>(PreferenceKey::COUNT);
    PreferenceValue m_prefs[MAX_PREFS] = {};
    bool m_set[MAX_PREFS] = {};
};

} // namespace Engine
} // namespace ExplorerLens
