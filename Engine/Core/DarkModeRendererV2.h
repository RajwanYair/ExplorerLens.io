// DarkModeRendererV2.h — Owner-Draw Dark Mode Rendering Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Provides owner-draw rendering routines for dark mode controls in the
// LENSManager GUI. Uses the undocumented SetPreferredAppMode/
// AllowDarkModeForWindow APIs from uxtheme.dll ordinals, with GDI+
// fallback for custom drawing.

#pragma once

#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Dark mode color scheme
struct DarkColorScheme
{
    COLORREF background = RGB(32, 32, 32);
    COLORREF surface = RGB(44, 44, 44);
    COLORREF surfaceAlt = RGB(56, 56, 56);
    COLORREF text = RGB(230, 230, 230);
    COLORREF textSecondary = RGB(160, 160, 160);
    COLORREF textDisabled = RGB(100, 100, 100);
    COLORREF accent = RGB(0, 120, 215);
    COLORREF accentHover = RGB(30, 150, 245);
    COLORREF accentPressed = RGB(0, 90, 170);
    COLORREF border = RGB(80, 80, 80);
    COLORREF borderFocused = RGB(0, 120, 215);
    COLORREF error = RGB(240, 80, 80);
    COLORREF warning = RGB(240, 200, 40);
    COLORREF success = RGB(50, 200, 80);
    COLORREF scrollbar = RGB(75, 75, 75);
    COLORREF scrollbarThumb = RGB(110, 110, 110);
    COLORREF tooltipBg = RGB(50, 50, 50);
    COLORREF tooltipText = RGB(220, 220, 220);
};

/// Light mode color scheme (for comparison/fallback)
struct LightColorScheme
{
    COLORREF background = RGB(255, 255, 255);
    COLORREF surface = RGB(243, 243, 243);
    COLORREF surfaceAlt = RGB(230, 230, 230);
    COLORREF text = RGB(30, 30, 30);
    COLORREF textSecondary = RGB(100, 100, 100);
    COLORREF accent = RGB(0, 120, 215);
    COLORREF border = RGB(200, 200, 200);
};

/// Preferred dark mode API (Windows 10 1809+)
enum class PreferredAppMode : int {
    Default = 0,
    AllowDark = 1,
    ForceDark = 2,
    ForceLight = 3,
    Max = 4
};

/// Dark mode rendering engine
class DarkModeRendererV2
{
  public:
    static DarkModeRendererV2& Instance()
    {
        static DarkModeRendererV2 inst;
        return inst;
    }

    /// Check if system dark mode is active
    bool IsSystemDarkMode() const
    {
        HKEY hKey = nullptr;
        DWORD value = 1;  // default to light
        DWORD size = sizeof(value);
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0,
                          KEY_READ, &hKey)
            == ERROR_SUCCESS) {
            RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
            RegCloseKey(hKey);
        }
        return value == 0;
    }

    /// Enable dark mode for the application (Windows 10 1809+)
    bool EnableDarkMode(bool enable)
    {
        m_isDarkMode = enable;
        if (!LoadUxThemeAPIs())
            return false;

        if (m_pfnSetPreferredAppMode) {
            m_pfnSetPreferredAppMode(enable ? PreferredAppMode::AllowDark : PreferredAppMode::Default);
        }
        return true;
    }

    /// Apply dark mode to a specific window
    bool ApplyToWindow(HWND hwnd)
    {
        if (!hwnd)
            return false;
        if (!LoadUxThemeAPIs())
            return false;

        if (m_pfnAllowDarkModeForWindow) {
            m_pfnAllowDarkModeForWindow(hwnd, m_isDarkMode ? TRUE : FALSE);
        }

        // Set title bar dark mode (Windows 10 2004+)
        BOOL darkMode = m_isDarkMode ? TRUE : FALSE;
        // DWMWA_USE_IMMERSIVE_DARK_MODE = 20
        DwmSetWindowAttribute(hwnd, 20, &darkMode, sizeof(darkMode));

        // Force redraw
        InvalidateRect(hwnd, nullptr, TRUE);
        return true;
    }

    /// Get current color scheme
    const DarkColorScheme& GetDarkScheme() const
    {
        return m_darkScheme;
    }
    const LightColorScheme& GetLightScheme() const
    {
        return m_lightScheme;
    }

    bool IsDarkMode() const
    {
        return m_isDarkMode;
    }

    /// Get appropriate background color based on current mode
    COLORREF GetBackground() const
    {
        return m_isDarkMode ? m_darkScheme.background : m_lightScheme.background;
    }
    COLORREF GetSurface() const
    {
        return m_isDarkMode ? m_darkScheme.surface : m_lightScheme.surface;
    }
    COLORREF GetText() const
    {
        return m_isDarkMode ? m_darkScheme.text : m_lightScheme.text;
    }
    COLORREF GetAccent() const
    {
        return m_darkScheme.accent;
    }
    COLORREF GetBorder() const
    {
        return m_isDarkMode ? m_darkScheme.border : m_lightScheme.border;
    }

    // ── Owner-draw helpers ──────────────────────────────────

    /// Fill a rectangle with the background color
    void FillBackground(HDC hdc, const RECT& rc) const
    {
        HBRUSH brush = CreateSolidBrush(GetBackground());
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);
    }

    /// Draw text with appropriate color
    void DrawText(HDC hdc, const wchar_t* text, RECT& rc, UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE) const
    {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, GetText());
        ::DrawTextW(hdc, text, -1, &rc, format);
    }

    /// Draw a rounded rectangle border
    void DrawRoundedBorder(HDC hdc, const RECT& rc, int radius = 4, bool focused = false) const
    {
        COLORREF color = focused ? m_darkScheme.borderFocused : GetBorder();
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    /// Create a solid color brush for the current scheme
    HBRUSH CreateBackgroundBrush() const
    {
        return CreateSolidBrush(GetBackground());
    }

    HBRUSH CreateSurfaceBrush() const
    {
        return CreateSolidBrush(GetSurface());
    }

  private:
    DarkModeRendererV2()
    {
        m_isDarkMode = IsSystemDarkMode();
    }

    ~DarkModeRendererV2()
    {
        if (m_hUxTheme)
            FreeLibrary(m_hUxTheme);
    }

    // UxTheme undocumented APIs (loaded by ordinal)
    using FnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode);
    using FnAllowDarkModeForWindow = BOOL(WINAPI*)(HWND, BOOL);
    using FnRefreshImmersiveColorPolicyState = void(WINAPI*)();

    // DwmApi
    using FnDwmSetWindowAttribute = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
    FnDwmSetWindowAttribute DwmSetWindowAttribute = nullptr;

    FnSetPreferredAppMode m_pfnSetPreferredAppMode = nullptr;
    FnAllowDarkModeForWindow m_pfnAllowDarkModeForWindow = nullptr;
    FnRefreshImmersiveColorPolicyState m_pfnRefreshPolicy = nullptr;
    HMODULE m_hUxTheme = nullptr;
    HMODULE m_hDwmApi = nullptr;

    bool m_apisLoaded = false;
    bool m_isDarkMode = false;

    DarkColorScheme m_darkScheme;
    LightColorScheme m_lightScheme;

    bool LoadUxThemeAPIs()
    {
        if (m_apisLoaded)
            return true;

        m_hUxTheme = LoadLibraryW(L"uxtheme.dll");
        if (m_hUxTheme) {
            // Ordinal 135: SetPreferredAppMode (Win10 1903+)
            m_pfnSetPreferredAppMode =
                reinterpret_cast<FnSetPreferredAppMode>(GetProcAddress(m_hUxTheme, MAKEINTRESOURCEA(135)));
            // Ordinal 133: AllowDarkModeForWindow
            m_pfnAllowDarkModeForWindow =
                reinterpret_cast<FnAllowDarkModeForWindow>(GetProcAddress(m_hUxTheme, MAKEINTRESOURCEA(133)));
            // Ordinal 104: RefreshImmersiveColorPolicyState
            m_pfnRefreshPolicy =
                reinterpret_cast<FnRefreshImmersiveColorPolicyState>(GetProcAddress(m_hUxTheme, MAKEINTRESOURCEA(104)));
        }

        m_hDwmApi = LoadLibraryW(L"dwmapi.dll");
        if (m_hDwmApi) {
            DwmSetWindowAttribute =
                reinterpret_cast<FnDwmSetWindowAttribute>(GetProcAddress(m_hDwmApi, "DwmSetWindowAttribute"));
        }

        m_apisLoaded = true;
        return m_pfnSetPreferredAppMode != nullptr;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
