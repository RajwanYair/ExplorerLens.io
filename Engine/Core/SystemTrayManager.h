// SystemTrayManager.h — System Tray Icon Management
// Copyright (c) 2026 ExplorerLens Project
//
// Manages system tray (notification area) icons for ExplorerLens background
// service notifications. Uses Shell_NotifyIconW with a hidden message-only
// window, background message pump thread, balloon notifications, and
// context-menu support via TrackPopupMenu. Thread-safe via SRWLOCK.

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <shellapi.h>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// System tray icon manager using Shell_NotifyIconW.
class SystemTrayManager
{
  public:
    SystemTrayManager()
    {
        InitializeSRWLock(&m_lock);
    }

    ~SystemTrayManager()
    {
        Shutdown();
    }

    SystemTrayManager(const SystemTrayManager&) = delete;
    SystemTrayManager& operator=(const SystemTrayManager&) = delete;

    // ── Initialization ────────────────────────────────────────────────────
    /// Creates a hidden message-only window for tray icon notifications.
    /// Must be called before any other method.
    inline bool Initialize(HINSTANCE hInstance)
    {
        if (m_initialized.load())
            return true;

        m_hInstance = hInstance;

        // Register a unique window class for tray messages
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = TrayWndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = kWindowClass;

        m_atom = RegisterClassExW(&wc);
        if (m_atom == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }

        // Create message-only window (HWND_MESSAGE parent)
        m_hwnd = CreateWindowExW(0, kWindowClass, L"ExplorerLensTray", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance,
                                 this);

        if (!m_hwnd)
            return false;

        // Store 'this' in window user data for the WndProc
        SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        // Start the message pump on a background thread
        m_running.store(true);
        m_pumpThread = std::thread([this]() { MessagePumpLoop(); });

        m_initialized.store(true);
        return true;
    }

    // ── Icon management ───────────────────────────────────────────────────
    /// Add a new tray icon with the given tooltip text.
    inline bool AddTrayIcon(uint32_t iconId, const std::wstring& tooltip)
    {
        if (!m_initialized.load() || !m_hwnd)
            return false;

        NOTIFYICONDATAW nid{};
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_hwnd;
        nid.uID = iconId;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        nid.uCallbackMessage = kCallbackMsg;
        nid.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));

        size_t maxLen = (std::min)(tooltip.size(), static_cast<size_t>(127));
        wcsncpy_s(nid.szTip, tooltip.c_str(), maxLen);

        BOOL ok = Shell_NotifyIconW(NIM_ADD, &nid);
        if (ok) {
            AcquireSRWLockExclusive(&m_lock);
            m_icons[iconId] = nid;
            ReleaseSRWLockExclusive(&m_lock);
        }
        return ok != FALSE;
    }

    /// Update the tooltip text of an existing tray icon.
    inline bool UpdateTooltip(uint32_t iconId, const std::wstring& tooltip)
    {
        if (!m_initialized.load())
            return false;

        AcquireSRWLockShared(&m_lock);
        auto it = m_icons.find(iconId);
        if (it == m_icons.end()) {
            ReleaseSRWLockShared(&m_lock);
            return false;
        }
        NOTIFYICONDATAW nid = it->second;
        ReleaseSRWLockShared(&m_lock);

        nid.uFlags = NIF_TIP;
        size_t maxLen = (std::min)(tooltip.size(), static_cast<size_t>(127));
        wcsncpy_s(nid.szTip, tooltip.c_str(), maxLen);

        BOOL ok = Shell_NotifyIconW(NIM_MODIFY, &nid);
        if (ok) {
            AcquireSRWLockExclusive(&m_lock);
            m_icons[iconId] = nid;
            ReleaseSRWLockExclusive(&m_lock);
        }
        return ok != FALSE;
    }

    /// Show a balloon notification on the specified tray icon.
    inline bool ShowBalloon(uint32_t iconId, const std::wstring& title, const std::wstring& message,
                            uint32_t timeoutMs = 5000)
    {
        if (!m_initialized.load())
            return false;

        AcquireSRWLockShared(&m_lock);
        auto it = m_icons.find(iconId);
        if (it == m_icons.end()) {
            ReleaseSRWLockShared(&m_lock);
            return false;
        }
        NOTIFYICONDATAW nid = it->second;
        ReleaseSRWLockShared(&m_lock);

        nid.uFlags = NIF_INFO;
        nid.dwInfoFlags = NIIF_INFO;
        nid.uTimeout = timeoutMs;

        size_t titleMax = (std::min)(title.size(), static_cast<size_t>(63));
        wcsncpy_s(nid.szInfoTitle, title.c_str(), titleMax);

        size_t msgMax = (std::min)(message.size(), static_cast<size_t>(255));
        wcsncpy_s(nid.szInfo, message.c_str(), msgMax);

        return Shell_NotifyIconW(NIM_MODIFY, &nid) != FALSE;
    }

    /// Remove a tray icon.
    inline bool RemoveTrayIcon(uint32_t iconId)
    {
        if (!m_initialized.load())
            return false;

        AcquireSRWLockExclusive(&m_lock);
        auto it = m_icons.find(iconId);
        if (it == m_icons.end()) {
            ReleaseSRWLockExclusive(&m_lock);
            return false;
        }
        NOTIFYICONDATAW nid = it->second;
        m_icons.erase(it);

        // Also remove context menu if any
        m_contextMenus.erase(iconId);
        ReleaseSRWLockExclusive(&m_lock);

        return Shell_NotifyIconW(NIM_DELETE, &nid) != FALSE;
    }

    /// Check if a tray icon with the given ID exists.
    inline bool IsVisible(uint32_t iconId) const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        bool found = m_icons.find(iconId) != m_icons.end();
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return found;
    }

    // ── Callbacks & menus ─────────────────────────────────────────────────
    using ClickCallback = std::function<void(uint32_t iconId, uint32_t mouseMsg)>;
    using MenuItem = std::pair<std::wstring, std::function<void()>>;

    /// Set a callback invoked when the tray icon is clicked.
    inline void SetClickCallback(ClickCallback fn)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_clickCallback = std::move(fn);
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Set context-menu items shown on right-click of the specified icon.
    inline void SetContextMenu(uint32_t iconId, std::vector<MenuItem> items)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_contextMenus[iconId] = std::move(items);
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Shutdown ──────────────────────────────────────────────────────────
    /// Remove all icons, destroy the hidden window, and stop the message pump.
    inline void Shutdown()
    {
        if (!m_initialized.exchange(false))
            return;

        m_running.store(false);

        // Post WM_QUIT to break the message loop
        if (m_hwnd) {
            PostMessageW(m_hwnd, WM_QUIT, 0, 0);
        }

        if (m_pumpThread.joinable()) {
            m_pumpThread.join();
        }

        // Remove all tray icons
        AcquireSRWLockExclusive(&m_lock);
        for (auto& pair : m_icons) {
            Shell_NotifyIconW(NIM_DELETE, &pair.second);
        }
        m_icons.clear();
        m_contextMenus.clear();
        ReleaseSRWLockExclusive(&m_lock);

        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        if (m_atom != 0) {
            UnregisterClassW(kWindowClass, m_hInstance);
            m_atom = 0;
        }
    }

  private:
    static constexpr UINT kCallbackMsg = WM_APP + 0x100;
    static constexpr UINT kMenuBaseId = 40000;
    static constexpr const wchar_t* kWindowClass = L"ExplorerLens_TrayMsgWnd";

    // ── Message pump ──────────────────────────────────────────────────────
    inline void MessagePumpLoop()
    {
        MSG msg{};
        while (m_running.load()) {
            BOOL ret = GetMessageW(&msg, nullptr, 0, 0);
            if (ret == 0 || ret == -1)
                break;  // WM_QUIT or error
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // ── WndProc ───────────────────────────────────────────────────────────
    static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto* self = reinterpret_cast<SystemTrayManager*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        if (msg == kCallbackMsg && self) {
            uint32_t iconId = static_cast<uint32_t>(wParam);
            uint32_t mouseMsg = LOWORD(lParam);

            // Invoke the generic click callback
            AcquireSRWLockShared(&self->m_lock);
            auto cb = self->m_clickCallback;
            ReleaseSRWLockShared(&self->m_lock);
            if (cb)
                cb(iconId, mouseMsg);

            // Show context menu on right-click
            if (mouseMsg == WM_RBUTTONUP) {
                self->ShowContextMenuForIcon(hwnd, iconId);
            }
            return 0;
        }

        if (msg == WM_COMMAND && self) {
            uint32_t cmdId = LOWORD(wParam);
            self->HandleMenuCommand(cmdId);
            return 0;
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    /// Display the context menu for the given icon at the cursor position.
    inline void ShowContextMenuForIcon(HWND hwnd, uint32_t iconId)
    {
        AcquireSRWLockShared(&m_lock);
        auto it = m_contextMenus.find(iconId);
        if (it == m_contextMenus.end() || it->second.empty()) {
            ReleaseSRWLockShared(&m_lock);
            return;
        }
        auto items = it->second;  // copy under lock
        ReleaseSRWLockShared(&m_lock);

        HMENU hMenu = CreatePopupMenu();
        if (!hMenu)
            return;

        // Store callbacks for this menu invocation
        AcquireSRWLockExclusive(&m_lock);
        m_pendingMenuCallbacks.clear();
        for (size_t i = 0; i < items.size(); ++i) {
            UINT id = kMenuBaseId + static_cast<UINT>(i);
            AppendMenuW(hMenu, MF_STRING, id, items[i].first.c_str());
            m_pendingMenuCallbacks[id] = items[i].second;
        }
        ReleaseSRWLockExclusive(&m_lock);

        POINT pt{};
        GetCursorPos(&pt);

        // Required for TrackPopupMenu to work from a notification icon
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
        PostMessageW(hwnd, WM_NULL, 0, 0);

        DestroyMenu(hMenu);
    }

    /// Dispatch a menu command to its callback.
    inline void HandleMenuCommand(uint32_t cmdId)
    {
        AcquireSRWLockShared(&m_lock);
        auto it = m_pendingMenuCallbacks.find(cmdId);
        std::function<void()> fn;
        if (it != m_pendingMenuCallbacks.end()) {
            fn = it->second;
        }
        ReleaseSRWLockShared(&m_lock);

        if (fn)
            fn();
    }

    // ── State ─────────────────────────────────────────────────────────────
    HINSTANCE m_hInstance = nullptr;
    HWND m_hwnd = nullptr;
    ATOM m_atom = 0;

    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_running{false};
    std::thread m_pumpThread;

    mutable SRWLOCK m_lock = SRWLOCK_INIT;

    std::unordered_map<uint32_t, NOTIFYICONDATAW> m_icons;
    std::unordered_map<uint32_t, std::vector<MenuItem>> m_contextMenus;
    ClickCallback m_clickCallback;
    std::unordered_map<UINT, std::function<void()>> m_pendingMenuCallbacks;
};

}  // namespace Engine
}  // namespace ExplorerLens
