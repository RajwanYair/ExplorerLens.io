#pragma once

#include <windows.h>
#include <uxtheme.h>

namespace DarkMode
{
// Windows 10 1809+ dark mode detection
inline bool IsSystemDarkMode() {
	// Check registry for dark mode setting
	DWORD value = 0;
	DWORD size = sizeof(value);

	if (RegGetValue(HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		L"AppsUseLightTheme",
		RRF_RT_REG_DWORD,
		nullptr,
		&value,
		&size) == ERROR_SUCCESS) {
		// 0 = Dark mode, 1 = Light mode
		return (value == 0);
	}

	return false; // Default to light mode
}

// Get appropriate background color for thumbnails based on system theme
inline COLORREF GetThumbnailBackgroundColor() {
	if (IsSystemDarkMode()) {
		// Dark mode: use dark gray background
		return RGB(32, 32, 32);
	}
	else {
		// Light mode: use white background
		return RGB(255, 255, 255);
	}
}

// Get appropriate border color based on system theme
inline COLORREF GetThumbnailBorderColor() {
	if (IsSystemDarkMode()) {
		// Dark mode: lighter border for contrast
		return RGB(80, 80, 80);
	}
	else {
		// Light mode: darker border
		return RGB(200, 200, 200);
	}
}

// Get appropriate text color based on system theme
// Fixes: dark theme was rendering black text (default) on dark backgrounds
inline COLORREF GetThumbnailTextColor() {
	if (IsHighContrastMode()) {
		// High contrast: use system window text color
		return GetSysColor(COLOR_WINDOWTEXT);
	}
	if (IsSystemDarkMode()) {
		// Dark mode: white text for readability on dark backgrounds
		return RGB(240, 240, 240);
	}
	else {
		// Light mode: dark text
		return RGB(30, 30, 30);
	}
}

// Get secondary/dimmed text color for metadata/labels
inline COLORREF GetThumbnailSecondaryTextColor() {
	if (IsSystemDarkMode()) {
		return RGB(160, 160, 160);
	}
	else {
		return RGB(100, 100, 100);
	}
}

// Get accent color for highlights and interactive elements
inline COLORREF GetThumbnailAccentColor() {
	return RGB(0, 120, 215); // Windows accent blue
}

// Get overlay background color (semi-transparent info bars on thumbnails)
inline COLORREF GetThumbnailOverlayColor() {
	if (IsSystemDarkMode()) {
		return RGB(48, 48, 48); // Dark overlay
	}
	else {
		return RGB(240, 240, 240); // Light overlay
	}
}

// Check if high contrast mode is enabled
inline bool IsHighContrastMode() {
	HIGHCONTRAST hc = { sizeof(HIGHCONTRAST) };
	if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0)) {
		return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
	}
	return false;
}
}
