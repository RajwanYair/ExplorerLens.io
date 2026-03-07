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
        // ForceDark ensures all common controls render white-on-dark text.
        // AllowDark only permits it but does not guarantee it for every
        // control class (group boxes, checkboxes sometimes fall through).
        SetPreferredAppMode(enable ? ForceDark : ForceLight);
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
// NOTE: Per-child EnableDarkModeForWindow + SetWindowTheme is handled
// by ApplyDarkScrollbars() in a single consistent pass. We avoid
// sending WM_THEMECHANGED here to prevent controls painting with
// stale visual-style state before ApplyDarkScrollbars applies the
// correct dark/light theme.
inline void ApplyThemeToDialog(HWND hDlg, const ThemeColors& theme, bool isDarkMode = false) {
    // Set dialog background brush (delete old brush to avoid GDI leak)
    HBRUSH oldBrush = reinterpret_cast<HBRUSH>(
        SetClassLongPtr(hDlg, GCLP_HBRBACKGROUND,
            reinterpret_cast<LONG_PTR>(CreateSolidBrush(theme.background))));
    if (oldBrush) DeleteObject(oldBrush);
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

// Apply dark theme to ALL child controls (scrollbars, buttons, statics, etc.)
//
// KEY INSIGHT: "DarkMode_Explorer" tells the visual-style renderer to use
// white text on dark backgrounds for Button-class controls (checkboxes,
// radio buttons, group boxes, push buttons). However, Static-class controls
// (LTEXT/CTEXT labels) do NOT respond to DarkMode_Explorer for text color.
// For Static controls we DISABLE visual styles entirely, which forces the
// GDI renderer to use the WM_CTLCOLORSTATIC text color (white in dark mode).
inline void ApplyDarkScrollbars(HWND hDlg, bool darkMode) {
    // Apply DarkMode_Explorer to the dialog window itself
    SetDarkScrollbar(hDlg, darkMode);

    EnumChildWindows(hDlg, [](HWND hChild, LPARAM lParam) -> BOOL
        {
            bool dark = (lParam != 0);
            TCHAR className[64];
            GetClassName(hChild, className, 64);

            // CRITICAL: AllowDarkModeForWindow (ordinal 133) MUST be
            // called BEFORE SetWindowTheme("DarkMode_Explorer") — otherwise
            // the visual-style renderer ignores the dark theme and paints
            // black text on the dark background.
            EnableDarkModeForWindow(hChild, dark);

            // Track whether we disable visual styles for this control.
            // Controls with disabled styles must NOT receive WM_THEMECHANGED
            // because that message re-enables the visual style renderer,
            // overriding the text color we set in WM_CTLCOLORSTATIC.
            bool disabledVisualStyles = false;

            // ── Static controls (LTEXT, CTEXT, RTEXT, icons) ──
            // DarkMode_Explorer does NOT change text color for the Static
            // window class. Disabling visual styles (empty theme) forces
            // the classic GDI renderer which respects SetTextColor() from
            // our WM_CTLCOLORSTATIC handler (white in dark, black in light).
            if (_tcsicmp(className, _T("Static")) == 0) {
                if (dark) {
                    SetWindowTheme(hChild, L"", L"");
                    disabledVisualStyles = true;
                }
                else {
                    SetWindowTheme(hChild, nullptr, nullptr);
                }
            }
            // ── Button controls (checkbox, radio, group box, push button) ──
            // DarkMode_Explorer does NOT reliably render white text for
            // checkbox, radio-button, and group-box sub-styles across all
            // Windows 10/11 builds.  Disable visual styles for every Button
            // sub-type EXCEPT push buttons so the classic GDI renderer uses
            // the white text color we set in WM_CTLCOLORSTATIC / WM_CTLCOLORBTN.
            // Push buttons (BS_PUSHBUTTON / BS_DEFPUSHBUTTON) keep
            // DarkMode_Explorer for proper dark button chrome.
            else if (_tcsicmp(className, _T("Button")) == 0) {
                LONG style = GetWindowLong(hChild, GWL_STYLE);
                LONG btnType = style & 0x0FL; // BS_TYPEMASK
                bool isPushButton = (btnType == BS_PUSHBUTTON ||
                    btnType == BS_DEFPUSHBUTTON);
                if (!isPushButton) {
                    // Groupbox, checkbox, radio — disable visual styles.
                    // GDI draws text using WM_CTLCOLORSTATIC colors (white).
                    if (dark) {
                        SetWindowTheme(hChild, L"", L"");
                        disabledVisualStyles = true;
                    }
                    else {
                        SetWindowTheme(hChild, nullptr, nullptr);
                    }
                }
                else {
                    SetDarkScrollbar(hChild, dark);
                }
            }
            // ── All other themed controls ──
            else if (_tcsicmp(className, _T("ListBox")) == 0 ||
                _tcsicmp(className, _T("SysListView32")) == 0 ||
                _tcsicmp(className, _T("SysTreeView32")) == 0 ||
                _tcsicmp(className, _T("Edit")) == 0 ||
                _tcsicmp(className, _T("ComboBox")) == 0 ||
                _tcsicmp(className, _T("SysTabControl32")) == 0 ||
                _tcsicmp(className, _T("msctls_trackbar32")) == 0 ||
                _tcsicmp(className, _T("msctls_progress32")) == 0 ||
                _tcsicmp(className, _T("msctls_statusbar32")) == 0 ||
                _tcsicmp(className, _T("tooltips_class32")) == 0) {
                SetDarkScrollbar(hChild, dark);
            }

            // Only send WM_THEMECHANGED to controls that still use visual
            // styles.  For controls where we disabled styles (Static labels,
            // checkboxes, radio buttons, group boxes in dark mode),
            // WM_THEMECHANGED would re-enable the visual style renderer and
            // cause it to paint black text, overriding our SetTextColor().
            if (!disabledVisualStyles) {
                SendMessage(hChild, WM_THEMECHANGED, 0, 0);
            }
            InvalidateRect(hChild, nullptr, TRUE);

            // Status bar needs explicit color messages (SB_SETBKCOLOR)
            if (_tcsicmp(className, _T("msctls_statusbar32")) == 0) {
                ThemeColors t = dark ? GetDarkTheme() : GetLightTheme();
                ::SendMessage(hChild, SB_SETBKCOLOR, 0, (LPARAM)t.background);
            }

            return TRUE;
        }, (LPARAM)(darkMode ? 1 : 0));

    // Force a full dialog + children repaint after all theming is done
    RedrawWindow(hDlg, nullptr, nullptr,
        RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

// Set themed text/background on a status bar control
inline void ApplyDarkStatusBar(HWND hStatusBar, bool darkMode) {
    if (!hStatusBar || !IsWindow(hStatusBar))
        return;
    ThemeColors theme = darkMode ? GetDarkTheme() : GetLightTheme();
    SetDarkScrollbar(hStatusBar, darkMode);
    ::SendMessage(hStatusBar, SB_SETBKCOLOR, 0, (LPARAM)theme.background);
    InvalidateRect(hStatusBar, nullptr, TRUE);
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
