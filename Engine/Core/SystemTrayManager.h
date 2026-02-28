// SystemTrayManager.h — NotifyIcon System Tray Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a system tray (notification area) icon for LENSManager.
// Provides: minimize-to-tray, status tooltip, context menu with
// quick actions (pause/resume, open manager, exit), and balloon
// notifications for build/registration events.

#pragma once

#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <shellapi.h>
#include <shlobj.h>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Tray icon state
enum class TrayIconState : uint8_t {
 Normal = 0, ///< Idle, ready
 Active = 1, ///< Processing thumbnails
 Paused = 2, ///< Temporarily paused
 Error = 3, ///< Error occurred
 Updating = 4 ///< Registration update in progress
};

/// Tray menu commands
enum class TrayCommand : uint16_t {
 OpenManager = 1001,
 PauseResume = 1002,
 RefreshShell = 1003,
 ClearCache = 1004,
 ShowPerformance = 1005,
 CheckForUpdates = 1006,
 About = 1007,
 Exit = 1008
};

/// Balloon notification type
enum class BalloonType : uint8_t {
 Info = NIIF_INFO,
 Warning = NIIF_WARNING,
 Error = NIIF_ERROR,
 None = NIIF_NONE,
 Custom = NIIF_USER ///< Use custom icon
};

/// Tray quick-action for test/UI surface
enum class TrayAction : uint8_t {
 OpenSettings = 0,
 ShowStatus,
 RefreshShell,
 ExitApp,
 COUNT
};

/// System tray manager
class SystemTrayManager {
public:
 static SystemTrayManager &Instance() {
 static SystemTrayManager inst;
 return inst;
 }

 /// Initialize tray icon
 bool Initialize(HWND hwndParent, HICON hIcon,
 UINT callbackMessage = WM_USER + 100) {
 if (m_initialized)
 return true;

 m_hwnd = hwndParent;
 m_callbackMsg = callbackMessage;

 NOTIFYICONDATAW nid = {};
 nid.cbSize = sizeof(nid);
 nid.hWnd = hwndParent;
 nid.uID = TRAY_ICON_ID;
 nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
 nid.uCallbackMessage = callbackMessage;
 nid.hIcon = hIcon;
 wcscpy_s(nid.szTip, L"ExplorerLens v15.0 — Ready");
 nid.uVersion = NOTIFYICON_VERSION_4;

 if (Shell_NotifyIconW(NIM_ADD, &nid)) {
 Shell_NotifyIconW(NIM_SETVERSION, &nid);
 m_initialized = true;
 m_hIcon = hIcon;
 return true;
 }
 return false;
 }

 /// Remove tray icon
 void Shutdown() {
 if (!m_initialized)
 return;

 NOTIFYICONDATAW nid = {};
 nid.cbSize = sizeof(nid);
 nid.hWnd = m_hwnd;
 nid.uID = TRAY_ICON_ID;
 Shell_NotifyIconW(NIM_DELETE, &nid);
 m_initialized = false;
 }

 /// Update tooltip text
 void SetTooltip(const wchar_t *text) {
 if (!m_initialized || !text)
 return;

 NOTIFYICONDATAW nid = {};
 nid.cbSize = sizeof(nid);
 nid.hWnd = m_hwnd;
 nid.uID = TRAY_ICON_ID;
 nid.uFlags = NIF_TIP | NIF_SHOWTIP;
 wcsncpy_s(nid.szTip, text, _TRUNCATE);
 Shell_NotifyIconW(NIM_MODIFY, &nid);
 }

 /// Update icon (e.g., to show state change)
 void SetIcon(HICON hIcon) {
 if (!m_initialized || !hIcon)
 return;

 NOTIFYICONDATAW nid = {};
 nid.cbSize = sizeof(nid);
 nid.hWnd = m_hwnd;
 nid.uID = TRAY_ICON_ID;
 nid.uFlags = NIF_ICON;
 nid.hIcon = hIcon;
 Shell_NotifyIconW(NIM_MODIFY, &nid);
 m_hIcon = hIcon;
 }

 /// Show balloon notification
 void ShowBalloon(const wchar_t *title, const wchar_t *message,
 BalloonType type = BalloonType::Info,
 DWORD timeoutMs = 5000) {
 if (!m_initialized)
 return;

 NOTIFYICONDATAW nid = {};
 nid.cbSize = sizeof(nid);
 nid.hWnd = m_hwnd;
 nid.uID = TRAY_ICON_ID;
 nid.uFlags = NIF_INFO;
 nid.dwInfoFlags = static_cast<DWORD>(type);
 if (title)
 wcsncpy_s(nid.szInfoTitle, title, _TRUNCATE);
 if (message)
 wcsncpy_s(nid.szInfo, message, _TRUNCATE);
 // Note: timeout is ignored in modern Windows, but set it anyway
 (void)timeoutMs;
 Shell_NotifyIconW(NIM_MODIFY, &nid);
 }

 /// Show context menu at cursor position
 void ShowContextMenu() {
 if (!m_hwnd)
 return;

 HMENU hMenu = CreatePopupMenu();
 if (!hMenu)
 return;

 AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayCommand::OpenManager),
 L"&Open ExplorerLens Manager");
 AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);

 if (m_state == TrayIconState::Paused) {
 AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayCommand::PauseResume),
 L"&Resume Processing");
 } else {
 AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayCommand::PauseResume),
 L"&Pause Processing");
 }

 AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayCommand::RefreshShell),
 L"Re&fresh Shell");
 AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayCommand::ClearCache),
 L"&Clear Cache");
 AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
 AppendMenuW(hMenu, MF_STRING,
 static_cast<UINT>(TrayCommand::ShowPerformance),
 L"Show &Performance");
 AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayCommand::About),
 L"&About...");
 AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
 AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayCommand::Exit),
 L"E&xit");

 // Set first item as bold default
 SetMenuDefaultItem(hMenu, static_cast<UINT>(TrayCommand::OpenManager),
 FALSE);

 POINT pt;
 GetCursorPos(&pt);
 SetForegroundWindow(m_hwnd);
 TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hwnd,
 nullptr);
 DestroyMenu(hMenu);
 }

 /// Process WM_COMMAND from tray menu
 bool HandleCommand(TrayCommand cmd) {
 switch (cmd) {
 case TrayCommand::PauseResume:
 m_state = (m_state == TrayIconState::Paused) ? TrayIconState::Normal
 : TrayIconState::Paused;
 UpdateTooltipFromState();
 return true;
 case TrayCommand::RefreshShell:
 SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
 ShowBalloon(L"ExplorerLens", L"Shell refreshed successfully.");
 return true;
 case TrayCommand::ClearCache:
 ShowBalloon(L"ExplorerLens", L"Cache cleared.", BalloonType::Info);
 return true;
 default:
 return false;
 }
 }

 /// Set state and update icon/tooltip
 void SetState(TrayIconState state) {
 m_state = state;
 UpdateTooltipFromState();
 }

 TrayIconState GetState() const { return m_state; }
 bool IsInitialized() const { return m_initialized; }
 UINT GetCallbackMessage() const { return m_callbackMsg; }

 /// Action count
 static constexpr size_t ActionCount() {
 return static_cast<size_t>(TrayAction::COUNT);
 }
 /// Action display name
 static const wchar_t *ActionName(TrayAction a) {
 switch (a) {
 case TrayAction::OpenSettings:
 return L"Open Settings";
 case TrayAction::ShowStatus:
 return L"Show Status";
 case TrayAction::RefreshShell:
 return L"Refresh Shell";
 case TrayAction::ExitApp:
 return L"Exit";
 default:
 return L"Unknown";
 }
 }

private:
 SystemTrayManager() = default;
 ~SystemTrayManager() { Shutdown(); }

 static constexpr UINT TRAY_ICON_ID = 1;

 void UpdateTooltipFromState() {
 const wchar_t *tip = L"ExplorerLens v15.0";
 switch (m_state) {
 case TrayIconState::Normal:
 tip = L"ExplorerLens v15.0 — Ready";
 break;
 case TrayIconState::Active:
 tip = L"ExplorerLens v15.0 — Processing...";
 break;
 case TrayIconState::Paused:
 tip = L"ExplorerLens v15.0 — Paused";
 break;
 case TrayIconState::Error:
 tip = L"ExplorerLens v15.0 — Error";
 break;
 case TrayIconState::Updating:
 tip = L"ExplorerLens v15.0 — Updating...";
 break;
 }
 SetTooltip(tip);
 }

 HWND m_hwnd = nullptr;
 HICON m_hIcon = nullptr;
 UINT m_callbackMsg = 0;
 TrayIconState m_state = TrayIconState::Normal;
 bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
