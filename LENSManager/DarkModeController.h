// DarkModeController.h — Unified Dark Mode Controller for LENSManager
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates dark mode across the entire Manager UI using DarkModeHelper.h.
// Handles theme change notifications, WM_SETTINGCHANGE, per-control theming.
#pragma once

#include "DarkModeHelper.h"
#include <algorithm>
#include <vector>
#include <windows.h>

namespace ExplorerLens {

// ============================================================================
// DarkModeController — Manages full-app dark mode lifecycle
// ============================================================================

class DarkModeController {
public:
  static DarkModeController& Instance() {
    static DarkModeController s_instance;
    return s_instance;
  }

  // ====================================================================
  // Initialize dark mode for the application
  // Call once at app startup before creating main window
  // ====================================================================
  void Initialize() {
    // Detect current system theme
    m_isDarkMode = DarkMode::IsSystemDarkMode();
    m_colors =
      m_isDarkMode ? DarkMode::GetDarkTheme() : DarkMode::GetLightTheme();
    m_accentColor = DarkMode::GetSystemAccentColor();

    // Set preferred app mode
    DarkMode::SetAppDarkMode(m_isDarkMode);
    m_initialized = true;
  }

  // ====================================================================
  // Apply dark mode to a top-level window (call after CreateWindow)
  // ====================================================================
  void ApplyToWindow(HWND hWnd) {
    if (!m_initialized)
      Initialize();

    // Dark title bar
    DarkMode::SetDarkModeForTitleBar(hWnd, m_isDarkMode);
    DarkMode::EnableDarkModeForWindow(hWnd, m_isDarkMode);

    // Apply colors to dialog and children
    DarkMode::ApplyThemeToDialog(hWnd, m_colors);
    DarkMode::ApplyDarkScrollbars(hWnd, m_isDarkMode);

    // Windows 11: Rounded corners
    DarkMode::SetRoundedCorners(hWnd, DarkMode::DWMWCP_ROUND);

    // Windows 11 22H2+: Mica backdrop (only in dark mode for best effect)
    if (m_isDarkMode) {
      DarkMode::EnableMicaBackdrop(hWnd);
    }
    else {
      // In light mode, use Mica Alt for a subtle tinted backdrop
      DarkMode::EnableMicaAltBackdrop(hWnd);
    }

    // Track managed windows
    m_managedWindows.push_back(hWnd);
  }

  // ====================================================================
  // Toggle between dark and light mode (user-initiated)
  // Flips the current theme and re-applies to all managed windows.
  // ====================================================================
  void ToggleTheme(HWND hWnd) {
    m_isDarkMode = !m_isDarkMode;
    m_colors = m_isDarkMode ? DarkMode::GetDarkTheme()
                            : DarkMode::GetLightTheme();
    m_accentColor = DarkMode::GetSystemAccentColor();
    DarkMode::SetAppDarkMode(m_isDarkMode);

    for (HWND hw : m_managedWindows) {
      if (IsWindow(hw)) {
        ApplyToWindow(hw);
      }
    }
    // Also apply to the calling window if not already tracked
    if (std::find(m_managedWindows.begin(), m_managedWindows.end(), hWnd) ==
        m_managedWindows.end()) {
      ApplyToWindow(hWnd);
    }
  }

  // ====================================================================
  // Handle WM_SETTINGCHANGE — system theme changed
  // ====================================================================
  bool OnSettingChange(HWND hWnd, LPARAM lParam) {
    // Check if the "ImmersiveColorSet" setting changed
    if (lParam) {
      const wchar_t* setting = reinterpret_cast<const wchar_t*>(lParam);
      if (wcscmp(setting, L"ImmersiveColorSet") == 0) {
        bool wasDark = m_isDarkMode;
        m_isDarkMode = DarkMode::IsSystemDarkMode();

        if (wasDark != m_isDarkMode) {
          // Theme changed — refresh everything
          m_colors = m_isDarkMode ? DarkMode::GetDarkTheme()
            : DarkMode::GetLightTheme();
          m_accentColor = DarkMode::GetSystemAccentColor();

          DarkMode::SetAppDarkMode(m_isDarkMode);

          for (HWND hw : m_managedWindows) {
            if (IsWindow(hw)) {
              ApplyToWindow(hw);
            }
          }
          return true; // Theme changed
        }
      }
    }
    return false;
  }

  // ====================================================================
  // Handle WM_CTLCOLOR* messages — return themed brushes
  // ====================================================================
  HBRUSH OnCtlColor(HDC hdc, HWND hControl) {
    return DarkMode::GetControlBrush(hdc, hControl, m_isDarkMode);
  }

  // ====================================================================
  // Owner-draw helpers for WTL controls
  // ====================================================================

  // Draw a themed group box
  void DrawGroupBox(HDC hdc, RECT& rc, const wchar_t* text) {
    // Fill background
    HBRUSH bgBrush = CreateSolidBrush(m_colors.groupBox);
    FillRect(hdc, &rc, bgBrush);
    DeleteObject(bgBrush);

    // Draw border
    HPEN pen = CreatePen(PS_SOLID, 1, m_colors.border);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, rc.left, rc.top + 8, rc.right, rc.bottom, 6, 6);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);

    // Draw group text
    if (text && text[0]) {
      SetTextColor(hdc, m_colors.text);
      SetBkColor(hdc, m_colors.groupBox);
      RECT textRc = { rc.left + 10, rc.top, rc.left + 200, rc.top + 16 };
      DrawTextW(hdc, text, -1, &textRc, DT_LEFT | DT_SINGLELINE);
    }
  }

  // Draw a themed checkbox
  void DrawCheckBox(DRAWITEMSTRUCT* dis, bool checked, const wchar_t* text) {
    // Background
    HBRUSH bgBrush = CreateSolidBrush(m_colors.background);
    FillRect(dis->hDC, &dis->rcItem, bgBrush);
    DeleteObject(bgBrush);

    // Checkbox square
    RECT boxRc = { dis->rcItem.left, dis->rcItem.top + 2, dis->rcItem.left + 13,
                  dis->rcItem.top + 15 };
    HPEN pen = CreatePen(PS_SOLID, 1, m_colors.border);
    HPEN oldPen = (HPEN)SelectObject(dis->hDC, pen);
    HBRUSH boxBrush =
      CreateSolidBrush(checked ? m_accentColor : m_colors.buttonFace);
    HBRUSH oldBrush = (HBRUSH)SelectObject(dis->hDC, boxBrush);
    Rectangle(dis->hDC, boxRc.left, boxRc.top, boxRc.right, boxRc.bottom);
    SelectObject(dis->hDC, oldBrush);
    SelectObject(dis->hDC, oldPen);
    DeleteObject(pen);
    DeleteObject(boxBrush);

    // Checkmark
    if (checked) {
      SetTextColor(dis->hDC, RGB(255, 255, 255));
      RECT chkRc = boxRc;
      DrawTextW(dis->hDC, L"\x2713", -1, &chkRc,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Label text
    RECT textRc = { boxRc.right + 5, dis->rcItem.top, dis->rcItem.right,
                   dis->rcItem.bottom };
    SetTextColor(dis->hDC, (dis->itemState & ODS_DISABLED)
      ? m_colors.disabledText
      : m_colors.text);
    SetBkMode(dis->hDC, TRANSPARENT);
    DrawTextW(dis->hDC, text, -1, &textRc,
      DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  }

  // ====================================================================
  // Accessors
  // ====================================================================
  bool IsDarkMode() const { return m_isDarkMode; }
  const DarkMode::ThemeColors& Colors() const { return m_colors; }
  COLORREF AccentColor() const { return m_accentColor; }

private:
  DarkModeController() = default;

  bool m_initialized = false;
  bool m_isDarkMode = false;
  DarkMode::ThemeColors m_colors{};
  COLORREF m_accentColor = RGB(0, 120, 215);
  std::vector<HWND> m_managedWindows;
};

} // namespace ExplorerLens
