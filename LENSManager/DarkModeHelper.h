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
	inline ThemeColors GetLightTheme()
	{
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

	inline ThemeColors GetDarkTheme()
	{
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
	inline bool IsSystemDarkMode()
	{
		// Check registry: HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme
		DWORD value = 1; // Default to light mode
		DWORD size = sizeof(DWORD);
		HKEY key;

		if (RegOpenKeyExW(HKEY_CURRENT_USER,
			L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
			0, KEY_READ, &key) == ERROR_SUCCESS)
		{
			RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, nullptr,
				reinterpret_cast<BYTE*>(&value), &size);
			RegCloseKey(key);
		}

		return (value == 0); // 0 = dark mode, 1 = light mode
	}

	// Enable dark mode for a window (Windows 10 1809+)
	inline bool EnableDarkModeForWindow(HWND hWnd, bool enable)
	{
		static fnAllowDarkModeForWindow AllowDarkModeForWindow = nullptr;

		if (!AllowDarkModeForWindow)
		{
			HMODULE hUxtheme = LoadLibraryW(L"uxtheme.dll");
			if (hUxtheme)
			{
				// Use ordinal 133 for AllowDarkModeForWindow
				AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(
					GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));
			}
		}

		if (AllowDarkModeForWindow)
		{
			return AllowDarkModeForWindow(hWnd, enable ? TRUE : FALSE) != 0;
		}

		return false;
	}

	// Set preferred app mode (Windows 10 1903+)
	inline bool SetAppDarkMode(bool enable)
	{
		static fnSetPreferredAppMode SetPreferredAppMode = nullptr;
		static fnRefreshImmersiveColorPolicyState RefreshImmersiveColorPolicyState = nullptr;

		if (!SetPreferredAppMode)
		{
			HMODULE hUxtheme = LoadLibraryW(L"uxtheme.dll");
			if (hUxtheme)
			{
				// Use ordinal 135 for SetPreferredAppMode
				SetPreferredAppMode = reinterpret_cast<fnSetPreferredAppMode>(
					GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135)));

				// Use ordinal 104 for RefreshImmersiveColorPolicyState
				RefreshImmersiveColorPolicyState = reinterpret_cast<fnRefreshImmersiveColorPolicyState>(
					GetProcAddress(hUxtheme, MAKEINTRESOURCEA(104)));
			}
		}

		if (SetPreferredAppMode && RefreshImmersiveColorPolicyState)
		{
			SetPreferredAppMode(enable ? AllowDark : Default);
			RefreshImmersiveColorPolicyState();
			return true;
		}

		return false;
	}

	// Set dark mode for window title bar (Windows 11 and Windows 10 20H1+)
	inline bool SetDarkModeForTitleBar(HWND hWnd, bool enable)
	{
		// DWMWA_USE_IMMERSIVE_DARK_MODE (20) - Windows 10 20H1+
		// DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 (19) - Windows 10 1903-2004
		BOOL value = enable ? TRUE : FALSE;
		HRESULT hr = DwmSetWindowAttribute(hWnd, 20, &value, sizeof(value));
		
		if (FAILED(hr))
		{
			// Try the older attribute for Windows 10 1903-2004
			hr = DwmSetWindowAttribute(hWnd, 19, &value, sizeof(value));
		}

		return SUCCEEDED(hr);
	}

	// Apply theme colors to dialog
	inline void ApplyThemeToDialog(HWND hDlg, const ThemeColors& theme)
	{
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

	// Custom draw brush for controls
	inline HBRUSH GetControlBrush(HDC hdc, HWND hControl, bool isDarkMode)
	{
		static HBRUSH hBrushLight = nullptr;
		static HBRUSH hBrushDark = nullptr;

		if (!hBrushLight)
		{
			hBrushLight = CreateSolidBrush(GetLightTheme().background);
			hBrushDark = CreateSolidBrush(GetDarkTheme().background);
		}

		ThemeColors theme = isDarkMode ? GetDarkTheme() : GetLightTheme();

		// Set text color and background mode
		SetTextColor(hdc, theme.text);
		SetBkColor(hdc, theme.background);
		SetBkMode(hdc, TRANSPARENT);

		return isDarkMode ? hBrushDark : hBrushLight;
	}

	// Set dark mode scrollbar theme for a window (Windows 10 1903+)
	inline void SetDarkScrollbar(HWND hWnd, bool darkMode)
	{
		// Explorer theme enables dark scrollbars on Windows 10 1903+
		if (darkMode)
		{
			SetWindowTheme(hWnd, L"DarkMode_Explorer", nullptr);
		}
		else
		{
			SetWindowTheme(hWnd, L"Explorer", nullptr);
		}
	}

	// Apply dark scrollbars to all child scrollable controls
	inline void ApplyDarkScrollbars(HWND hDlg, bool darkMode)
	{
		EnumChildWindows(hDlg, [](HWND hChild, LPARAM lParam) -> BOOL
		{
			bool dark = (lParam != 0);
			TCHAR className[64];
			GetClassName(hChild, className, 64);

			// Apply dark scrollbars to listboxes, treeviews, listviews, edit controls
			if (_tcsicmp(className, _T("ListBox")) == 0 ||
				_tcsicmp(className, _T("SysListView32")) == 0 ||
				_tcsicmp(className, _T("SysTreeView32")) == 0 ||
				_tcsicmp(className, _T("Edit")) == 0)
			{
				SetDarkScrollbar(hChild, dark);
			}
			
			return TRUE;
		}, (LPARAM)(darkMode ? 1 : 0));
	}

	// Get Windows accent color from system settings
	inline COLORREF GetSystemAccentColor()
	{
		DWORD color = 0;
		BOOL opaque = FALSE;

		// DwmGetColorizationColor returns the window colorization color
		HRESULT hr = DwmGetColorizationColor(&color, &opaque);
		if (SUCCEEDED(hr))
		{
			// ARGB to RGB (discard alpha)
			return RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
		}
		
		// Fallback: read from registry
		DWORD accentColor = 0;
		DWORD size = sizeof(DWORD);
		HKEY key;
		if (RegOpenKeyExW(HKEY_CURRENT_USER,
			L"Software\\Microsoft\\Windows\\DWM",
			0, KEY_READ, &key) == ERROR_SUCCESS)
		{
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
	inline void SetDarkTooltip(HWND hTooltip, bool darkMode)
	{
		if (darkMode)
		{
			SetWindowTheme(hTooltip, L"DarkMode_Explorer", nullptr);
		}
		else
		{
			SetWindowTheme(hTooltip, nullptr, nullptr);
		}
	}
}

#endif // _DARKMODEHELPER_H_
