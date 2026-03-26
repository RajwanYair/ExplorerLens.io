// TrayAgent.h — System Tray Icon + Context Menu Agent
// Copyright (c) 2026 ExplorerLens Project
//
// Registers a Windows notification tray icon for ExplorerLens, exposing
// quick actions (enable/disable, clear cache, open manager) via a context menu.
// Uses Shell_NotifyIcon (Win32) with WM_TASKBARCREATED re-registration support.
//
#pragma once

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <functional>

namespace ExplorerLens { namespace Engine {

// Context-menu item identifiers for the tray agent.
enum class TrayMenuAction : UINT {
    EnableExtension  = 1001,
    DisableExtension = 1002,
    ClearCache       = 1003,
    OpenManager      = 1004,
    ShowDashboard    = 1005,
    Separator1       = 0,
    ExitAgent        = 1099,
};

// Tray agent lifecycle callbacks supplied by the host.
struct TrayCallbacks {
    std::function<void()> onEnable;
    std::function<void()> onDisable;
    std::function<void()> onClearCache;
    std::function<void()> onOpenManager;
    std::function<void()> onShowDashboard;
    std::function<void()> onExit;
};

// TrayAgent — Manages the shell notification area icon for ExplorerLens.
//
// Typical usage:
//   TrayAgent agent;
//   agent.SetCallbacks(cbs);
//   agent.Create(hwndHost, L"ExplorerLens Active");
//   // ... message loop ...
//   agent.Destroy();
class TrayAgent {
public:
    TrayAgent() noexcept;
    ~TrayAgent() noexcept;

    TrayAgent(const TrayAgent&)            = delete;
    TrayAgent& operator=(const TrayAgent&) = delete;

    // Create the tray icon.  hwndHost receives WM_USER+1 for tray messages.
    bool Create(HWND hwndHost, const std::wstring& tooltip);

    // Remove the tray icon immediately.
    void Destroy() noexcept;

    // Update tooltip text without recreating the icon.
    void SetTooltip(const std::wstring& tooltip) noexcept;

    // Show a balloon/toast notification from the tray icon.
    void ShowBalloon(const std::wstring& title,
                     const std::wstring& message,
                     DWORD               flags = NIIF_INFO) noexcept;

    // Register caller callbacks for menu actions.
    void SetCallbacks(const TrayCallbacks& cbs) noexcept;

    // Must be called from the host's WndProc for WM_USER+1 messages.
    LRESULT HandleTrayMessage(WPARAM wParam, LPARAM lParam);

    // Call after WM_TASKBARCREATED to re-register on Explorer restart.
    void OnTaskbarCreated() noexcept;

    // Returns the WM_TASKBARCREATED message ID registered at construction.
    static UINT TaskbarCreatedMsg() noexcept { return s_wmTaskbarCreated; }

    bool IsCreated() const noexcept { return m_created; }

private:
    void ShowContextMenu() noexcept;
    void DispatchAction(TrayMenuAction action) noexcept;

    HWND              m_hwnd      { nullptr };
    NOTIFYICONDATAW   m_nid       {};
    TrayCallbacks     m_callbacks {};
    bool              m_created   { false };
    bool              m_enabled   { true };

    static UINT       s_wmTaskbarCreated;
};

}} // namespace ExplorerLens::Engine
