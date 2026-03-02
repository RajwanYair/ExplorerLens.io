#ifndef _DARKMODEHELPER_H_
#define _DARKMODEHELPER_H_

#include <Windows.h>
#include <uxtheme.h>
#include <dwmapi.h>

#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

namespace DarkMode
{
// Windows 10 1809+ dark mode support
enum PreferredAppMode
{
    Default = 0,
    AllowDark = 1,
    ForceDark = 2,
    ForceLight = 3,
    Max = 4
};

// Undocumented ordinal functions in uxtheme.dll
typedef BOOL(WINAPI* fnAllowDarkModeForWindow)(HWND hWnd, BOOL allow);
typedef PreferredAppMode(WINAPI* fnSetPreferredAppMode)(PreferredAppMode appMode);
typedef void (WINAPI* fnRefreshImmersiveColorPolicyState)();
typedef BOOL(WINAPI* fnShouldAppsUseDarkMode)();

// Theme colors
struct ThemeColors
{
    COLORREF background;
    COLORREF groupBox;
    COLORREF text;
    COLORREF disabledText;
    COLORREF buttonFace;
    COLORREF buttonHighlight;
    COLORREF border;
};

// Get system theme colors
inline ThemeColors GetLightTheme() {
    return {
        RGB(240, 240, 240),  // background
        RGB(255, 255, 255),  // groupBox
        RGB(0, 0, 0),        // text
        RGB(109, 109, 109),  // disabledText
        RGB(225, 225, 225),  // buttonFace
        RGB(255, 255, 255),  // buttonHighlight
        RGB(172, 172, 172)   // border
    };
}

inline ThemeColors GetDarkTheme() {
    return {
        RGB(32, 32, 32),     // background
        RGB(45, 45, 45),     // groupBox
        RGB(255, 255, 255),  // text
        RGB(153, 153, 153),  // disabledText
        RGB(55, 55, 55),     // buttonFace
        RGB(70, 70, 70),     // buttonHighlight
        RGB(85, 85, 85)      // border
    };
}

// Check if Windows is using dark mode
inline bool IsSystemDarkMode() {
    // Check registry: HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme
    DWORD value = 1; // Default to light mode
    DWORD size = sizeof(DWORD);
    HKEY key;

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, nullptr,
            reinterpret_cast<BYTE*>(&value), &size);
        RegCloseKey(key);
    }

    return (value == 0); // 0 = dark mode, 1 = light mode
}

// Enable dark mode for a window (Windows 10 1809+)
inline bool EnableDarkModeForWindow(HWND hWnd, bool enable) {
    static fnAllowDarkModeForWindow AllowDarkModeForWindow = nullptr;

    if (!AllowDarkModeForWindow) {
        HMODULE hUxtheme = LoadLibraryW(L"uxtheme.dll");
        if (hUxtheme) {
            // Use ordinal 133 for AllowDarkModeForWindow
            AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(
                GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));
        }
    }

    if (AllowDarkModeForWindow) {
        return AllowDarkModeForWindow(hWnd, enable ? TRUE : FALSE) != 0;
    }

    return false;
}

// Set preferred app mode (Windows 10 1903+)
inline bool SetAppDarkMode(bool enable) {
    static fnSetPreferredAppMode SetPreferredAppMode = nullptr;
    static fnRefreshImmersiveColorPolicyState RefreshImmersiveColorPolicyState = nullptr;

    if (!SetPreferredAppMode) {
        HMODULE hUxtheme = LoadLibraryW(L"uxtheme.dll");
        if (hUxtheme) {
            // Use ordinal 135 for SetPreferredAppMode
            SetPreferredAppMode = reinterpret_cast<fnSetPreferredAppMode>(
                GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135)));

            // Use ordinal 104 for RefreshImmersiveColorPolicyState
            RefreshImmersiveColorPolicyState = reinterpret_cast<fnRefreshImmersiveColorPolicyState>(
                GetProcAddress(hUxtheme, MAKEINTRESOURCEA(104)));
        }
    }

    if (SetPreferredAppMode && RefreshImmersiveColorPolicyState) {
        SetPreferredAppMode(enable ? AllowDark : Default);
        RefreshImmersiveColorPolicyState();
        return true;
    }

    return false;
}

// Set dark mode for window title bar (Windows 11 and Windows 10 20H1+)
inline bool SetDarkModeForTitleBar(HWND hWnd, bool enable) {
    // DWMWA_USE_IMMERSIVE_DARK_MODE (20) - Windows 10 20H1+
    // DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 (19) - Windows 10 1903-2004
    BOOL value = enable ? TRUE : FALSE;
    HRESULT hr = DwmSetWindowAttribute(hWnd, 20, &value, sizeof(value));

    if (FAILED(hr)) {
        // Try the older attribute for Windows 10 1903-2004
        hr = DwmSetWindowAttribute(hWnd, 19, &value, sizeof(value));
    }

    return SUCCEEDED(hr);
}

// Apply theme colors to dialog
inline void ApplyThemeToDialog(HWND hDlg, const ThemeColors& theme) {
    // Set dialog background brush
    SetClassLongPtr(hDlg, GCLP_HBRBACKGROUND,
        reinterpret_cast<LONG_PTR>(CreateSolidBrush(theme.background)));

    // Apply theme to all child windows
    EnumChildWindows(hDlg, [](HWND hChild, LPARAM lParam) -> BOOL
        {
            TCHAR className[256];
            GetClassName(hChild, className, 256);

            // Update control colors
            InvalidateRect(hChild, nullptr, TRUE);
            return TRUE;
        }, 0);

    // Force redraw
    InvalidateRect(hDlg, nullptr, TRUE);
    UpdateWindow(hDlg);
}

/*
 * Return a themed background brush and set text/background colors on the
 * given device context. Called from WM_CTLCOLOR* message handlers to paint
 * controls with the correct dark or light mode colors.
 */
inline HBRUSH GetControlBrush(HDC hdc, HWND hControl, bool isDarkMode) {
    static HBRUSH hBrushLight = nullptr;
    static HBRUSH hBrushDark = nullptr;
    static HBRUSH hBrushLightGroupBox = nullptr;
    static HBRUSH hBrushDarkGroupBox = nullptr;

    if (!hBrushLight) {
        hBrushLight = CreateSolidBrush(GetLightTheme().background);
        hBrushDark = CreateSolidBrush(GetDarkTheme().background);
        hBrushLightGroupBox = CreateSolidBrush(GetLightTheme().groupBox);
        hBrushDarkGroupBox = CreateSolidBrush(GetDarkTheme().groupBox);
    }

    ThemeColors theme = isDarkMode ? GetDarkTheme() : GetLightTheme();

    SetTextColor(hdc, theme.text);
    SetBkColor(hdc, theme.background);
    SetBkMode(hdc, TRANSPARENT);

    TCHAR className[64] = {};
    if (hControl) {
        GetClassName(hControl, className, 64);
    }

    if (_tcsicmp(className, _T("Edit")) == 0) {
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, theme.groupBox);
        return isDarkMode ? hBrushDarkGroupBox : hBrushLightGroupBox;
    }

    return isDarkMode ? hBrushDark : hBrushLight;
}

// Set dark mode scrollbar theme for a window (Windows 10 1903+)
inline void SetDarkScrollbar(HWND hWnd, bool darkMode) {
    // Explorer theme enables dark scrollbars on Windows 10 1903+
    if (darkMode) {
        SetWindowTheme(hWnd, L"DarkMode_Explorer", nullptr);
    }
    else {
        SetWindowTheme(hWnd, L"Explorer", nullptr);
    }
}

// Apply dark scrollbars to all child scrollable controls
inline void ApplyDarkScrollbars(HWND hDlg, bool darkMode) {
    EnumChildWindows(hDlg, [](HWND hChild, LPARAM lParam) -> BOOL
        {
            bool dark = (lParam != 0);
            TCHAR className[64];
            GetClassName(hChild, className, 64);

            // Apply dark scrollbars to listboxes, treeviews, listviews, edit controls
            if (_tcsicmp(className, _T("ListBox")) == 0 ||
                _tcsicmp(className, _T("SysListView32")) == 0 ||
                _tcsicmp(className, _T("SysTreeView32")) == 0 ||
                _tcsicmp(className, _T("Edit")) == 0) {
                SetDarkScrollbar(hChild, dark);
            }

            return TRUE;
        }, (LPARAM)(darkMode ? 1 : 0));
}

// Get Windows accent color from system settings
inline COLORREF GetSystemAccentColor() {
    DWORD color = 0;
    BOOL opaque = FALSE;

    // DwmGetColorizationColor returns the window colorization color
    HRESULT hr = DwmGetColorizationColor(&color, &opaque);
    if (SUCCEEDED(hr)) {
        // ARGB to RGB (discard alpha)
        return RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    }

    // Fallback: read from registry
    DWORD accentColor = 0;
    DWORD size = sizeof(DWORD);
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\DWM",
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        RegQueryValueExW(key, L"AccentColor", nullptr, nullptr,
            reinterpret_cast<BYTE*>(&accentColor), &size);
        RegCloseKey(key);
        // Registry stores as AABBGGRR
        return RGB(accentColor & 0xFF,
            (accentColor >> 8) & 0xFF,
            (accentColor >> 16) & 0xFF);
    }

    // Default accent blue
    return RGB(0, 120, 215);
}

// Create a themed tooltip control with dark mode support
inline void SetDarkTooltip(HWND hTooltip, bool darkMode) {
    if (darkMode) {
        SetWindowTheme(hTooltip, L"DarkMode_Explorer", nullptr);
    }
    else {
        SetWindowTheme(hTooltip, nullptr, nullptr);
    }
}

// ========================================================================
// Windows 11 Mica/Acrylic Backdrop + Rounded Corners
// ========================================================================

// DWM attribute constants (Windows 11 22H2+, may not be in older SDKs)
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE  33
#endif
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE       38
#endif

// Window corner preference values
enum DWM_WINDOW_CORNER_PREFERENCE_VALUES
{
    DWMWCP_DEFAULT = 0,  // Let system decide
    DWMWCP_DONOTROUND = 1,  // Never round corners
    DWMWCP_ROUND = 2,  // Round with default radius
    DWMWCP_ROUNDSMALL = 3   // Round with small radius
};

// System backdrop type values (Mica/Acrylic)
enum DWM_SYSTEMBACKDROP_TYPE_VALUES
{
    DWMSBT_AUTO = 0,  // Let DWM decide
    DWMSBT_NONE = 1,  // No system backdrop
    DWMSBT_MAINWINDOW = 2,  // Mica
    DWMSBT_TRANSIENTWINDOW = 3,  // Acrylic
    DWMSBT_TABBEDWINDOW = 4   // Tabbed (Mica Alt)
};

// Get the Windows build number via registry (safe, no versionhelpers.h)
inline DWORD GetWindowsBuildNumber() {
    DWORD build = 0;
    HKEY key;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        // Try CurrentBuildNumber (string) first
        wchar_t buildStr[32] = {};
        DWORD size = sizeof(buildStr);
        DWORD type = 0;
        if (RegQueryValueExW(key, L"CurrentBuildNumber", nullptr, &type,
            reinterpret_cast<BYTE*>(buildStr), &size) == ERROR_SUCCESS) {
            build = static_cast<DWORD>(_wtoi(buildStr));
        }
        RegCloseKey(key);
    }
    return build;
}

// Check if running on Windows 11 (build 22000+)
inline bool IsWindows11OrLater() {
    static DWORD s_build = GetWindowsBuildNumber();
    return s_build >= 22000;
}

// Check if running on Windows 11 22H2+ (build 22621+, required for Mica)
inline bool IsWindows11_22H2OrLater() {
    static DWORD s_build = GetWindowsBuildNumber();
    return s_build >= 22621;
}

// Enable rounded corners on a window (Windows 11+)
inline bool SetRoundedCorners(HWND hWnd, int preference = DWMWCP_ROUND) {
    if (!IsWindows11OrLater())
        return false;

    HRESULT hr = DwmSetWindowAttribute(hWnd,
        DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
    return SUCCEEDED(hr);
}

// Set system backdrop type (Mica, Acrylic, etc.) on a window
// Requires Windows 11 22H2+ (build 22621+)
inline bool SetSystemBackdrop(HWND hWnd, int backdropType = DWMSBT_MAINWINDOW) {
    if (!IsWindows11_22H2OrLater())
        return false;

    HRESULT hr = DwmSetWindowAttribute(hWnd,
        DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
    return SUCCEEDED(hr);
}

// Convenience: Enable Mica backdrop on a window
inline bool EnableMicaBackdrop(HWND hWnd) {
    return SetSystemBackdrop(hWnd, DWMSBT_MAINWINDOW);
}

// Convenience: Enable Acrylic backdrop on a window
inline bool EnableAcrylicBackdrop(HWND hWnd) {
    return SetSystemBackdrop(hWnd, DWMSBT_TRANSIENTWINDOW);
}

// Convenience: Enable Mica Alt (Tabbed) backdrop on a window
inline bool EnableMicaAltBackdrop(HWND hWnd) {
    return SetSystemBackdrop(hWnd, DWMSBT_TABBEDWINDOW);
}
}

#endif // _DARKMODEHELPER_H_
