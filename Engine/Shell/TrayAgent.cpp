// TrayAgent.cpp — System Tray Icon + Context Menu Agent
// Copyright (c) 2026 ExplorerLens Project
//
// Implementation of TrayAgent: Win32 Shell_NotifyIcon lifecycle, context menu
// construction and dispatch, balloon notifications, Explorer-restart recovery.
//
#include "TrayAgent.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>

namespace ExplorerLens { namespace Engine {

// Static: registered once per process at first construction.
UINT TrayAgent::s_wmTaskbarCreated = ::RegisterWindowMessageW(L"TaskbarCreated");

TrayAgent::TrayAgent() noexcept = default;

TrayAgent::~TrayAgent() noexcept {
    Destroy();
}

bool TrayAgent::Create(HWND hwndHost, const std::wstring& tooltip) {
    if (m_created) return true;
    m_hwnd = hwndHost;

    ::ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize           = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd             = hwndHost;
    m_nid.uID              = 1;
    m_nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
    m_nid.uCallbackMessage = WM_USER + 1;
    m_nid.hIcon            = ::LoadIconW(::GetModuleHandleW(nullptr),
                                         MAKEINTRESOURCEW(1));
    if (!m_nid.hIcon) {
        m_nid.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
    }
    ::wcsncpy_s(m_nid.szTip, tooltip.c_str(), _TRUNCATE);

    m_created = (::Shell_NotifyIconW(NIM_ADD, &m_nid) == TRUE);
    if (m_created) {
        m_nid.uVersion = NOTIFYICON_VERSION_4;
        ::Shell_NotifyIconW(NIM_SETVERSION, &m_nid);
    }
    return m_created;
}

void TrayAgent::Destroy() noexcept {
    if (m_created) {
        ::Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_created = false;
    }
}

void TrayAgent::SetTooltip(const std::wstring& tooltip) noexcept {
    if (!m_created) return;
    ::wcsncpy_s(m_nid.szTip, tooltip.c_str(), _TRUNCATE);
    m_nid.uFlags = NIF_TIP;
    ::Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayAgent::ShowBalloon(const std::wstring& title,
                             const std::wstring& message,
                             DWORD               flags) noexcept {
    if (!m_created) return;
    m_nid.uFlags    = NIF_INFO;
    m_nid.dwInfoFlags = flags;
    ::wcsncpy_s(m_nid.szInfoTitle, title.c_str(),   _TRUNCATE);
    ::wcsncpy_s(m_nid.szInfo,      message.c_str(),  _TRUNCATE);
    ::Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayAgent::SetCallbacks(const TrayCallbacks& cbs) noexcept {
    m_callbacks = cbs;
}

void TrayAgent::OnTaskbarCreated() noexcept {
    if (!m_created) return;
    m_nid.uVersion = NOTIFYICON_VERSION_4;
    ::Shell_NotifyIconW(NIM_ADD, &m_nid);
    ::Shell_NotifyIconW(NIM_SETVERSION, &m_nid);
}

LRESULT TrayAgent::HandleTrayMessage(WPARAM /*wParam*/, LPARAM lParam) {
    const UINT msg = LOWORD(lParam);
    if (msg == WM_RBUTTONUP || msg == WM_CONTEXTMENU) {
        ShowContextMenu();
    } else if (msg == WM_LBUTTONDBLCLK) {
        DispatchAction(TrayMenuAction::OpenManager);
    }
    return 0;
}

void TrayAgent::ShowContextMenu() noexcept {
    HMENU hMenu = ::CreatePopupMenu();
    if (!hMenu) return;

    const UINT enabledFlag = m_enabled
        ? (MF_GRAYED | MF_STRING)
        : (MF_STRING);
    const UINT disabledFlag = m_enabled
        ? (MF_STRING)
        : (MF_GRAYED | MF_STRING);

    ::AppendMenuW(hMenu, MF_STRING,
                  static_cast<UINT_PTR>(TrayMenuAction::OpenManager),
                  L"Open LENSManager…");
    ::AppendMenuW(hMenu, MF_STRING,
                  static_cast<UINT_PTR>(TrayMenuAction::ShowDashboard),
                  L"Show Dashboard");
    ::AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    ::AppendMenuW(hMenu, enabledFlag,
                  static_cast<UINT_PTR>(TrayMenuAction::EnableExtension),
                  L"Enable Extension");
    ::AppendMenuW(hMenu, disabledFlag,
                  static_cast<UINT_PTR>(TrayMenuAction::DisableExtension),
                  L"Disable Extension");
    ::AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    ::AppendMenuW(hMenu, MF_STRING,
                  static_cast<UINT_PTR>(TrayMenuAction::ClearCache),
                  L"Clear Cache");
    ::AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    ::AppendMenuW(hMenu, MF_STRING,
                  static_cast<UINT_PTR>(TrayMenuAction::ExitAgent),
                  L"Exit");

    POINT pt{};
    ::GetCursorPos(&pt);
    ::SetForegroundWindow(m_hwnd);

    const UINT cmd = static_cast<UINT>(
        ::TrackPopupMenu(hMenu,
                         TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                         pt.x, pt.y, 0, m_hwnd, nullptr));
    ::DestroyMenu(hMenu);

    if (cmd) {
        DispatchAction(static_cast<TrayMenuAction>(cmd));
    }
}

void TrayAgent::DispatchAction(TrayMenuAction action) noexcept {
    switch (action) {
    case TrayMenuAction::EnableExtension:
        m_enabled = true;
        if (m_callbacks.onEnable) m_callbacks.onEnable();
        break;
    case TrayMenuAction::DisableExtension:
        m_enabled = false;
        if (m_callbacks.onDisable) m_callbacks.onDisable();
        break;
    case TrayMenuAction::ClearCache:
        if (m_callbacks.onClearCache) m_callbacks.onClearCache();
        break;
    case TrayMenuAction::OpenManager:
        if (m_callbacks.onOpenManager) m_callbacks.onOpenManager();
        break;
    case TrayMenuAction::ShowDashboard:
        if (m_callbacks.onShowDashboard) m_callbacks.onShowDashboard();
        break;
    case TrayMenuAction::ExitAgent:
        if (m_callbacks.onExit) m_callbacks.onExit();
        break;
    default:
        break;
    }
}

}} // namespace ExplorerLens::Engine
