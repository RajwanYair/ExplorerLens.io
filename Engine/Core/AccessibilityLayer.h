// AccessibilityLayer.h — High Contrast Mode and Screen Reader Support
// Copyright (c) 2026 ExplorerLens Project
//
// Adapts thumbnail overlays and Manager UI elements for Windows accessibility
// features: high contrast themes, narrator announcements, and focus indicators.
// Integrates with the Windows Accessibility APIs (UI Automation / MSAA).
//
#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine { namespace Core {

enum class HighContrastTheme : uint8_t {
    None          = 0,   // Standard colour scheme
    BlackOnWhite  = 1,   // HC Black
    WhiteOnBlack  = 2,   // HC White
    Custom        = 3    // User-defined high-contrast theme
};

enum class AccessibilityFeature : uint32_t {
    None            = 0x00,
    HighContrast    = 0x01,
    ReduceMotion    = 0x02,   // WM_SETTINGCHANGE + SPI_GETCLIENTAREAANIMATION
    NarratorActive  = 0x04,   // Screen reader detected
    MagnifierActive = 0x08,
    CaptionEnabled  = 0x10,
    FocusIndicators = 0x20    // Always-visible keyboard focus rings
};

inline AccessibilityFeature operator|(AccessibilityFeature a, AccessibilityFeature b) {
    return static_cast<AccessibilityFeature>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

struct AccessibilityState {
    HighContrastTheme  theme           = HighContrastTheme::None;
    AccessibilityFeature features      = AccessibilityFeature::None;
    COLORREF           foregroundColor = RGB(0, 0, 0);
    COLORREF           backgroundColor = RGB(255, 255, 255);
    COLORREF           accentColor     = RGB(0, 120, 215);
    uint32_t           dpiScale        = 100;  // percentage
};

class AccessibilityLayer {
public:
    static AccessibilityLayer& Instance() {
        static AccessibilityLayer inst;
        return inst;
    }

    // Read current Windows accessibility state
    void Refresh() {
        HIGHCONTRAST hc{ sizeof(HIGHCONTRAST) };
        SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0);
        if (hc.dwFlags & HCF_HIGHCONTRASTON) {
            m_state.features = m_state.features | AccessibilityFeature::HighContrast;
            // Determine specific theme from color scheme name
            std::wstring name = hc.lpszDefaultScheme ? hc.lpszDefaultScheme : L"";
            if (name.find(L"Black") != std::wstring::npos)
                m_state.theme = HighContrastTheme::BlackOnWhite;
            else if (name.find(L"White") != std::wstring::npos)
                m_state.theme = HighContrastTheme::WhiteOnBlack;
            else
                m_state.theme = HighContrastTheme::Custom;
            m_state.foregroundColor = GetSysColor(COLOR_WINDOWTEXT);
            m_state.backgroundColor = GetSysColor(COLOR_WINDOW);
        } else {
            m_state.theme    = HighContrastTheme::None;
            m_state.features = AccessibilityFeature::None;
        }

        // Check reduce-motion preference
        BOOL animate = TRUE;
        SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &animate, 0);
        if (!animate)
            m_state.features = m_state.features | AccessibilityFeature::ReduceMotion;

        // Check if a screen reader is running
        BOOL srActive = FALSE;
        SystemParametersInfoW(SPI_GETSCREENREADER, 0, &srActive, 0);
        if (srActive)
            m_state.features = m_state.features | AccessibilityFeature::NarratorActive;

        FireChangeCallbacks();
    }

    const AccessibilityState& State() const { return m_state; }

    bool IsHighContrast() const {
        return (static_cast<uint32_t>(m_state.features) &
                static_cast<uint32_t>(AccessibilityFeature::HighContrast)) != 0;
    }

    bool ReduceMotion() const {
        return (static_cast<uint32_t>(m_state.features) &
                static_cast<uint32_t>(AccessibilityFeature::ReduceMotion)) != 0;
    }

    bool NarratorActive() const {
        return (static_cast<uint32_t>(m_state.features) &
                static_cast<uint32_t>(AccessibilityFeature::NarratorActive)) != 0;
    }

    // Adapt a colour for accessibility (returns original or HC override)
    COLORREF AdaptColor(COLORREF original, bool isForeground) const {
        if (!IsHighContrast()) return original;
        return isForeground ? m_state.foregroundColor : m_state.backgroundColor;
    }

    // Post UIA Notification for screen-reader announcements
    void AnnounceToScreenReader(const std::wstring& text) const {
        if (!NarratorActive()) return;
        // Production: use IUIAutomationEventHandler / NotifyWinEvent
        NotifyWinEvent(EVENT_SYSTEM_ALERT, GetDesktopWindow(), OBJID_CLIENT, CHILDID_SELF);
        (void)text;
    }

    using ChangeFn = std::function<void(const AccessibilityState&)>;
    void OnChange(ChangeFn fn) { m_cbs.push_back(std::move(fn)); }

private:
    AccessibilityLayer() { Refresh(); }

    void FireChangeCallbacks() {
        for (auto& fn : m_cbs) fn(m_state);
    }

    AccessibilityState       m_state;
    std::vector<ChangeFn>    m_cbs;
};

}}} // namespace ExplorerLens::Engine::Core
