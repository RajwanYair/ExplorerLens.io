#pragma once

#include <windows.h>
#include <uxtheme.h>

namespace DarkMode
{
	// Windows 10 1809+ dark mode detection
	inline bool IsSystemDarkMode()
	{
		// Check registry for dark mode setting
		DWORD value = 0;
		DWORD size = sizeof(value);
		
		if (RegGetValue(HKEY_CURRENT_USER,
			L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
			L"AppsUseLightTheme",
			RRF_RT_REG_DWORD,
			nullptr,
			&value,
			&size) == ERROR_SUCCESS)
		{
			// 0 = Dark mode, 1 = Light mode
			return (value == 0);
		}

		return false; // Default to light mode
	}

	// Get appropriate background color for thumbnails based on system theme
	inline COLORREF GetThumbnailBackgroundColor()
	{
		if (IsSystemDarkMode())
		{
			// Dark mode: use dark gray background
			return RGB(32, 32, 32);
		}
		else
		{
			// Light mode: use white background
			return RGB(255, 255, 255);
		}
	}

	// Get appropriate border color based on system theme
	inline COLORREF GetThumbnailBorderColor()
	{
		if (IsSystemDarkMode())
		{
			// Dark mode: lighter border for contrast
			return RGB(80, 80, 80);
		}
		else
		{
			// Light mode: darker border
			return RGB(200, 200, 200);
		}
	}

	// Check if high contrast mode is enabled
	inline bool IsHighContrastMode()
	{
		HIGHCONTRAST hc = { sizeof(HIGHCONTRAST) };
		if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0))
		{
			return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
		}
		return false;
	}
}
