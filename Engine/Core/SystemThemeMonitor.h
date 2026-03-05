// SystemThemeMonitor.h — Real-Time System Theme Change Monitor
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors WM_SETTINGCHANGE and registry changes for AppsUseLightTheme
// to detect dark/light mode transitions in real-time. Notifies all
// registered thumbnail renderers to invalidate cached theme colors.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

/// Theme transition event
enum class ThemeTransition : uint8_t {
    LightToDark,
    DarkToLight,
    ToHighContrast,
    FromHighContrast,
    AccentColorChanged,
    COUNT
};

/// Callback signature for theme change notifications
using ThemeChangeCallback = std::function<void(ThemeTransition)>;

/// System theme monitor — watches for OS theme changes
class SystemThemeMonitor {
public:
    /// Register a callback for theme changes
    uint32_t RegisterCallback(ThemeChangeCallback cb) {
        uint32_t id = m_nextId++;
        m_callbacks.push_back({ id, std::move(cb) });
        return id;
    }

    /// Unregister a callback by ID
    bool UnregisterCallback(uint32_t id) {
        for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it) {
            if (it->id == id) {
                m_callbacks.erase(it);
                return true;
            }
        }
        return false;
    }

    /// Simulate a theme change (for testing)
    void SimulateTransition(ThemeTransition t) {
        m_lastTransition = t;
        m_transitionCount++;
        for (auto& entry : m_callbacks) {
            if (entry.callback) entry.callback(t);
        }
    }

    /// Get current detected theme state
    bool IsDarkMode() const { return m_isDark; }
    void SetDarkMode(bool dark) { m_isDark = dark; }

    /// Stats
    uint32_t TransitionCount() const { return m_transitionCount; }
    uint32_t CallbackCount()   const { return static_cast<uint32_t>(m_callbacks.size()); }
    ThemeTransition LastTransition() const { return m_lastTransition; }

    static const wchar_t* TransitionName(ThemeTransition t) {
        switch (t) {
        case ThemeTransition::LightToDark:        return L"LightToDark";
        case ThemeTransition::DarkToLight:        return L"DarkToLight";
        case ThemeTransition::ToHighContrast:     return L"ToHighContrast";
        case ThemeTransition::FromHighContrast:   return L"FromHighContrast";
        case ThemeTransition::AccentColorChanged: return L"AccentColorChanged";
        default: return L"Unknown";
        }
    }

    static size_t TransitionTypeCount() { return static_cast<size_t>(ThemeTransition::COUNT); }

private:
    struct CallbackEntry {
        uint32_t id;
        ThemeChangeCallback callback;
    };
    std::vector<CallbackEntry> m_callbacks;
    uint32_t m_nextId = 1;
    uint32_t m_transitionCount = 0;
    bool m_isDark = false;
    ThemeTransition m_lastTransition = ThemeTransition::LightToDark;
};

} // namespace Engine
} // namespace ExplorerLens
