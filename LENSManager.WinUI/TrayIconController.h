// TrayIconController.h — System Tray Icon and Context Menu for Manager.WinUI
// Copyright (c) 2026 ExplorerLens Project
//
// Manages the Windows system tray icon (Shell_NotifyIcon) for the Manager app.
// Provides a context menu with quick-actions: Show, Register/Unregister shell
// extension, Clear cache, and Exit. Also shows balloon notifications.
//
#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <functional>
#include <cstdint>

#pragma comment(lib, "shell32.lib")

namespace ExplorerLens { namespace Engine { namespace WinUI {

// Context menu item commands
enum class TrayCommand : UINT {
    Show           = 1001,
    Register       = 1002,
    Unregister     = 1003,
    ClearCache     = 1004,
    CheckUpdates   = 1005,
    Exit           = 1099,
};

// Callback for tray command activation
using TrayCommandCallback = std::function<void(TrayCommand)>;

class TrayIconController {
public:
    // Initialize tray icon with given parent HWND and notification message
    // notifyMsg: custom WM_ message (WM_APP+n) for tray icon callbacks
    bool Initialize(HWND hwnd, UINT notifyMsg, HICON hIcon = nullptr) {
        m_hwnd      = hwnd;
        m_notifyMsg = notifyMsg;
        m_nid = {};
        m_nid.cbSize           = sizeof(m_nid);
        m_nid.hWnd             = hwnd;
        m_nid.uID              = 1;
        m_nid.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        m_nid.uCallbackMessage = notifyMsg;
        m_nid.hIcon            = hIcon ? hIcon : LoadIconW(nullptr, IDI_APPLICATION);
        wcscpy_s(m_nid.szTip, L"ExplorerLens Manager");
        return Shell_NotifyIconW(NIM_ADD, &m_nid) != 0;
    }

    void Destroy() {
        if (m_hwnd) Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_hwnd = nullptr;
    }

    // Update tooltip text
    void SetTooltip(const std::wstring& tip) {
        wcscpy_s(m_nid.szTip, tip.substr(0, 127).c_str());
        Shell_NotifyIconW(NIM_MODIFY, &m_nid);
    }

    // Show balloon notification
    void ShowBalloon(const std::wstring& title, const std::wstring& text,
                     DWORD timeoutMs = 3000, DWORD infoFlags = NIIF_INFO) {
        m_nid.uFlags     = NIF_INFO;
        m_nid.dwInfoFlags= infoFlags;
        m_nid.uTimeout   = timeoutMs;
        wcscpy_s(m_nid.szInfoTitle, title.substr(0, 63).c_str());
        wcscpy_s(m_nid.szInfo, text.substr(0, 255).c_str());
        Shell_NotifyIconW(NIM_MODIFY, &m_nid);
        m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    }

    // Handle tray notify messages — call from WndProc
    bool HandleMessage(UINT msg, LPARAM lParam, TrayCommandCallback cb) {
        if (msg != m_notifyMsg) return false;
        switch (lParam) {
        case WM_LBUTTONDBLCLK:
            if (cb) cb(TrayCommand::Show);
            break;
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU:
            ShowContextMenu(cb);
            break;
        }
        return true;
    }

private:
    HWND          m_hwnd      = nullptr;
    UINT          m_notifyMsg = 0;
    NOTIFYICONDATAW m_nid     = {};

    void ShowContextMenu(TrayCommandCallback cb) {
        POINT pt = {};
        GetCursorPos(&pt);
        HMENU hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, MF_STRING, (UINT)TrayCommand::Show,         L"Open Manager");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, (UINT)TrayCommand::Register,     L"Register Shell Extension");
        AppendMenuW(hMenu, MF_STRING, (UINT)TrayCommand::Unregister,   L"Unregister Shell Extension");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, (UINT)TrayCommand::ClearCache,   L"Clear Thumbnail Cache");
        AppendMenuW(hMenu, MF_STRING, (UINT)TrayCommand::CheckUpdates, L"Check for Updates");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, (UINT)TrayCommand::Exit,         L"Exit");

        SetForegroundWindow(m_hwnd);
        UINT cmd = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                    pt.x, pt.y, m_hwnd, nullptr);
        DestroyMenu(hMenu);
        if (cmd && cb) cb(static_cast<TrayCommand>(cmd));
    }
};

}}} // namespace ExplorerLens::Engine::WinUI
