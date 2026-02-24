// SystemTrayIcon.h — System Tray Integration for LENSManager
// ExplorerLens Manager v15.0.0 "Zenith" — Sprint 375
// Copyright (c) 2026 ExplorerLens Project
//
// Provides notification area icon with context menu, balloon tips,
// and quick-access to common operations.
#pragma once

#include "resource.h"
#include <shellapi.h>
#include <string>
#include <windows.h>

namespace ExplorerLens {

// ============================================================================
// System tray message IDs
// ============================================================================
constexpr UINT WM_TRAYICON = WM_APP + 100;

// Context menu item IDs
constexpr UINT ID_TRAY_OPEN = 40001;
constexpr UINT ID_TRAY_REFRESH = 40002;
constexpr UINT ID_TRAY_CLEARACHE = 40003;
constexpr UINT ID_TRAY_ABOUT = 40004;
constexpr UINT ID_TRAY_EXIT = 40005;

// ============================================================================
// SystemTrayIcon — Notification area icon management
// ============================================================================

class SystemTrayIcon {
public:
  // ====================================================================
  // Create tray icon (call from OnInitDialog or similar)
  // ====================================================================
  bool Create(HWND hOwner, HICON hIcon) {
    m_hOwner = hOwner;
    m_nid = {};
    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = hOwner;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = hIcon;
    wcscpy_s(m_nid.szTip, L"ExplorerLens v15.0.0 — Thumbnail Handler");

    // Enable modern GUID-based tray icon (Windows 7+)
    m_nid.uVersion = NOTIFYICON_VERSION_4;

    BOOL ok = Shell_NotifyIconW(NIM_ADD, &m_nid);
    if (ok) {
      Shell_NotifyIconW(NIM_SETVERSION, &m_nid);
      m_created = true;
    }
    return m_created;
  }

  // ====================================================================
  // Remove tray icon (call from OnDestroy)
  // ====================================================================
  void Remove() {
    if (m_created) {
      Shell_NotifyIconW(NIM_DELETE, &m_nid);
      m_created = false;
    }
  }

  // ====================================================================
  // Show balloon notification
  // ====================================================================
  void ShowBalloon(const wchar_t *title, const wchar_t *text,
                   DWORD infoFlags = NIIF_INFO, UINT timeoutMs = 3000) {
    if (!m_created)
      return;
    m_nid.uFlags = NIF_INFO;
    m_nid.dwInfoFlags = infoFlags;
    wcscpy_s(m_nid.szInfoTitle, title);
    wcscpy_s(m_nid.szInfo, text);
    m_nid.uTimeout = timeoutMs;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
  }

  // ====================================================================
  // Update tooltip text
  // ====================================================================
  void SetTooltip(const wchar_t *tip) {
    if (!m_created)
      return;
    m_nid.uFlags = NIF_TIP | NIF_SHOWTIP;
    wcscpy_s(m_nid.szTip, tip);
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
  }

  // ====================================================================
  // Handle WM_TRAYICON message — show context menu or restore
  // ====================================================================
  bool OnTrayMessage(WPARAM wParam, LPARAM lParam) {
    UINT msg = LOWORD(lParam);

    switch (msg) {
    case WM_LBUTTONDBLCLK:
      // Double-click — show/restore main window
      ShowWindow(m_hOwner, SW_RESTORE);
      SetForegroundWindow(m_hOwner);
      return true;

    case WM_RBUTTONUP:
    case WM_CONTEXTMENU:
      // Right-click — show context menu
      ShowContextMenu();
      return true;
    }
    return false;
  }

  // ====================================================================
  // Minimize to tray instead of closing
  // ====================================================================
  static bool HandleMinimize(HWND hWnd) {
    ShowWindow(hWnd, SW_HIDE);
    return true; // Handled — don't destroy
  }

  bool IsCreated() const { return m_created; }

private:
  void ShowContextMenu() {
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu)
      return;

    AppendMenuW(hMenu, MF_STRING, ID_TRAY_OPEN, L"&Open ExplorerLens");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_REFRESH, L"&Refresh Thumbnails");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_CLEARACHE, L"&Clear Cache");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_ABOUT, L"&About...");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"E&xit");

    // Bold the default item
    SetMenuDefaultItem(hMenu, ID_TRAY_OPEN, FALSE);

    // Required: SetForegroundWindow before TrackPopupMenu (Win32 quirk)
    SetForegroundWindow(m_hOwner);

    POINT pt;
    GetCursorPos(&pt);
    TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y,
                     m_hOwner, nullptr);

    DestroyMenu(hMenu);
  }

  HWND m_hOwner = nullptr;
  NOTIFYICONDATAW m_nid = {};
  bool m_created = false;
};

} // namespace ExplorerLens
