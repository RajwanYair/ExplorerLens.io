// ThemeManager.h — Dark/Light Theme and Accent Color Management
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks system theme (light/dark/high-contrast) and accent color, applying
// them to WinUI ElementTheme and XAML resources. Provides callbacks for
// theme change events via WM_WININICHANGE / SPI_GETHIGHCONTRAST monitoring.
//
#pragma once
#include <windows.h>
#include <uxtheme.h>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

#pragma comment(lib, "uxtheme.lib")

namespace ExplorerLens { namespace Engine { namespace WinUI {

enum class ThemeMode {
    Light,
    Dark,
    HighContrastBlack,
    HighContrastWhite,
    HighContrastCustom,
};

struct AccentColor {
    uint8_t  r, g, b, a;
    uint32_t argb() const { return ((uint32_t)a << 24) | ((uint32_t)r << 16) |
                                   ((uint32_t)g << 8) | b; }
};

struct ThemeState {
    ThemeMode   mode         = ThemeMode::Light;
    AccentColor accentColor  = {};
    bool        highContrast = false;
    uint32_t    dpi          = 96;
};

using ThemeChangeCallback = std::function<void(const ThemeState&)>;

class ThemeManager {
public:
    // Initialize and detect current system theme
    static void Initialize(HWND hwndForMessages) {
        s_hwnd = hwndForMessages;
        Refresh();
    }

    // Refresh theme from system — call on WM_WININICHANGE
    static void Refresh() {
        s_state.mode = DetectMode();
        s_state.accentColor = DetectAccentColor();
        s_state.highContrast = IsHighContrastActive();
        s_state.dpi = GetDpiForWindow(s_hwnd ? s_hwnd : GetDesktopWindow());
        NotifyCallbacks();
    }

    static const ThemeState& Current() { return s_state; }

    static void AddChangeCallback(ThemeChangeCallback cb) {
        s_callbacks.push_back(std::move(cb));
    }

    // Returns XAML ElementTheme string: "Light", "Dark", or "Default"
    static const wchar_t* XamlElementTheme() {
        switch (s_state.mode) {
        case ThemeMode::Dark:               return L"Dark";
        case ThemeMode::Light:              return L"Light";
        default:                            return L"Default";
        }
    }

    // Process WM_WININICHANGE or WM_THEMECHANGED in main WNDPROC
    static bool HandleMessage(UINT msg, WPARAM wParam) {
        if (msg == WM_WININICHANGE || msg == WM_THEMECHANGED ||
            (msg == WM_SETTINGCHANGE && wParam == SPI_SETHIGHCONTRAST)) {
            Refresh(); return true;
        }
        return false;
    }

private:
    static ThemeState                    s_state;
    static std::vector<ThemeChangeCallback> s_callbacks;
    static HWND                          s_hwnd;

    static void NotifyCallbacks() {
        for (auto& cb : s_callbacks) cb(s_state);
    }

    static bool IsHighContrastActive() {
        HIGHCONTRASTW hc = {}; hc.cbSize = sizeof(hc);
        SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0);
        return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
    }

    static ThemeMode DetectMode() {
        if (IsHighContrastActive()) {
            HIGHCONTRASTW hc = {}; hc.cbSize = sizeof(hc);
            SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0);
            std::wstring scheme(hc.lpszDefaultScheme ? hc.lpszDefaultScheme : L"");
            if (scheme.find(L"Black") != std::wstring::npos)
                return ThemeMode::HighContrastBlack;
            if (scheme.find(L"White") != std::wstring::npos)
                return ThemeMode::HighContrastWhite;
            return ThemeMode::HighContrastCustom;
        }
        // Read AppsModeValue from registry: 0=Dark, 1=Light
        DWORD val = 1, sz = sizeof(val);
        RegGetValueW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &val, &sz);
        return (val == 0) ? ThemeMode::Dark : ThemeMode::Light;
    }

    static AccentColor DetectAccentColor() {
        DWORD col = 0, sz = sizeof(col);
        RegGetValueW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\DWM",
            L"AccentColor", RRF_RT_REG_DWORD, nullptr, &col, &sz);
        AccentColor ac;
        ac.a = (col >> 24) & 0xFF;
        ac.r = (col >> 16) & 0xFF;
        ac.g = (col >>  8) & 0xFF;
        ac.b = (col)       & 0xFF;
        return ac;
    }
};

ThemeState                    ThemeManager::s_state;
std::vector<ThemeChangeCallback> ThemeManager::s_callbacks;
HWND                          ThemeManager::s_hwnd = nullptr;

}}} // namespace ExplorerLens::Engine::WinUI
