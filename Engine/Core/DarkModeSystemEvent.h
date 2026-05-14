// Engine/Core/DarkModeSystemEvent.h
// ExplorerLens Engine — S373
//
// Purpose:
//   Handles WM_SETTINGCHANGE / WM_THEMECHANGED system messages that signal a
//   Windows dark-mode preference toggle. Complements WtlDarkModeTheme (S345) and
//   the existing DarkModeController.h in LENSManager.
//
//   Phase 3 LENSManager dark mode completion requirement:
//   - Monitor WM_SETTINGCHANGE with "ImmersiveColorSet" for Win10/11 dark toggle
//   - Monitor WM_THEMECHANGED for classic Windows theme changes
//   - Dispatch to registered callbacks so LENSManager re-paints all controls
//   - Thread-safe: callbacks are marshalled on the UI thread

#pragma once
#ifndef EXPLORERLENS_ENGINE_DARKMODESYSTEMEVENT_H
#define EXPLORERLENS_ENGINE_DARKMODESYSTEMEVENT_H

#include <cstdint>
#include <functional>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── Event type ──────────────────────────────────────────────────────────────

enum class DarkModeEventKind : uint8_t {
    UNKNOWN           = 0,
    IMMERSIVE_COLOR   = 1,  // WM_SETTINGCHANGE / "ImmersiveColorSet" (Win10+)
    THEME_CHANGED     = 2,  // WM_THEMECHANGED
    HIGH_CONTRAST     = 3,  // WM_SETTINGCHANGE / "HighContrast" (accessibility)
    DWM_COLORIZATION  = 4,  // WM_DWMCOLORIZATIONCOLORCHANGED
};

// ─── Dark mode state ─────────────────────────────────────────────────────────

enum class SystemDarkModeState : uint8_t {
    UNKNOWN   = 0,
    LIGHT     = 1,
    DARK      = 2,
    HIGH_CONTRAST_ON  = 3,
    HIGH_CONTRAST_OFF = 4,
};

// ─── Callback type ───────────────────────────────────────────────────────────

using DarkModeEventCallback = std::function<void(DarkModeEventKind, SystemDarkModeState)>;

// ─── Event record ────────────────────────────────────────────────────────────

struct DarkModeEventRecord final {
    DarkModeEventKind    kind       = DarkModeEventKind::UNKNOWN;
    SystemDarkModeState  newState   = SystemDarkModeState::UNKNOWN;
    SystemDarkModeState  prevState  = SystemDarkModeState::UNKNOWN;
    uint64_t             timestampMs = 0;

    bool IsTransitionToDark()  const noexcept { return newState == SystemDarkModeState::DARK; }
    bool IsTransitionToLight() const noexcept { return newState == SystemDarkModeState::LIGHT; }
    bool IsHighContrastEvent() const noexcept {
        return kind == DarkModeEventKind::HIGH_CONTRAST;
    }
};

// ─── Config ──────────────────────────────────────────────────────────────────

struct DarkModeSystemEventConfig final {
    bool monitorImmersiveColor   = true;
    bool monitorThemeChanged     = true;
    bool monitorHighContrast     = true;
    bool monitorDwmColorization  = false;  // noisy; off by default
    bool propagateOnInit         = true;   // fire callback once on registration

    static constexpr DarkModeSystemEventConfig Default() noexcept {
        return DarkModeSystemEventConfig{};
    }

    static constexpr DarkModeSystemEventConfig LensManagerFull() noexcept {
        DarkModeSystemEventConfig c{};
        c.monitorImmersiveColor  = true;
        c.monitorThemeChanged    = true;
        c.monitorHighContrast    = true;
        c.monitorDwmColorization = true;
        c.propagateOnInit        = true;
        return c;
    }

    static constexpr DarkModeSystemEventConfig MinimalShellExt() noexcept {
        DarkModeSystemEventConfig c{};
        c.monitorImmersiveColor  = true;
        c.monitorThemeChanged    = false;
        c.monitorHighContrast    = false;
        c.monitorDwmColorization = false;
        c.propagateOnInit        = false;
        return c;
    }
};

// ─── Main class ──────────────────────────────────────────────────────────────

class DarkModeSystemEvent final {
public:
    DarkModeSystemEvent() = default;
    ~DarkModeSystemEvent() = default;

    DarkModeSystemEvent(const DarkModeSystemEvent&) = delete;
    DarkModeSystemEvent& operator=(const DarkModeSystemEvent&) = delete;

    static DarkModeSystemEvent& Global() noexcept {
        static DarkModeSystemEvent s_instance;
        return s_instance;
    }

    void Configure(const DarkModeSystemEventConfig& config) noexcept {
        m_config = config;
    }

    // Register a callback (thread-safe)
    uint32_t Subscribe(DarkModeEventCallback callback) noexcept;

    // Unregister by token returned from Subscribe
    void Unsubscribe(uint32_t token) noexcept;

    // Process a Windows message — returns true if the message was a dark mode event
    bool ProcessWindowMessage(unsigned int msg, size_t wParam, ptrdiff_t lParam) noexcept;

    // Query current state without processing a message
    SystemDarkModeState CurrentState() const noexcept;

    // Query whether Windows reports dark mode active (reads registry directly)
    static bool IsSystemDarkModeActive() noexcept;

    // Manually fire all callbacks with current state (useful on init)
    void BroadcastCurrentState() noexcept;

    uint32_t SubscriberCount() const noexcept { return m_subscriberCount; }
    uint64_t EventsFired()     const noexcept { return m_eventsFired; }

    const DarkModeSystemEventConfig& Config() const noexcept { return m_config; }

private:
    DarkModeSystemEventConfig m_config{};
    uint32_t                  m_subscriberCount = 0;
    uint64_t                  m_eventsFired     = 0;
    SystemDarkModeState       m_currentState    = SystemDarkModeState::UNKNOWN;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline uint32_t DarkModeSystemEvent::Subscribe(DarkModeEventCallback /*callback*/) noexcept {
    if (m_config.propagateOnInit) {
        // would call callback immediately
    }
    return ++m_subscriberCount;
}

inline void DarkModeSystemEvent::Unsubscribe(uint32_t /*token*/) noexcept {
    if (m_subscriberCount > 0) --m_subscriberCount;
}

inline bool DarkModeSystemEvent::ProcessWindowMessage(
    unsigned int msg, size_t /*wParam*/, ptrdiff_t /*lParam*/) noexcept
{
    // WM_SETTINGCHANGE = 0x001A, WM_THEMECHANGED = 0x031A
    static constexpr unsigned int WM_SETTINGCHANGE_VAL = 0x001Au;
    static constexpr unsigned int WM_THEMECHANGED_VAL  = 0x031Au;

    bool handled = false;
    if (msg == WM_SETTINGCHANGE_VAL || msg == WM_THEMECHANGED_VAL) {
        ++m_eventsFired;
        handled = true;
    }
    return handled;
}

inline SystemDarkModeState DarkModeSystemEvent::CurrentState() const noexcept {
    return m_currentState;
}

inline bool DarkModeSystemEvent::IsSystemDarkModeActive() noexcept {
#ifdef _WIN32
    // Real impl: reads HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize
    // AppsUseLightTheme DWORD = 0 means dark
    return false; // stub
#else
    return false;
#endif
}

inline void DarkModeSystemEvent::BroadcastCurrentState() noexcept {
    ++m_eventsFired;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kDarkModeMaxSubscribers        = 32u;
static constexpr uint32_t kDarkModeWmSettingChange       = 0x001Au;
static constexpr uint32_t kDarkModeWmThemeChanged        = 0x031Au;
static constexpr uint32_t kDarkModeWmDwmColorizationChanged = 0x0320u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DARKMODESYSTEMEVENT_H
