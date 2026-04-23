// ShellNotificationProvider.h — WinRT Toast Notification Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Thin wrapper for sending Windows toast notifications from the decode pipeline.
// Uses the WinRT ToastNotificationManager COM ABI (roapi.h) so it compiles on
// Windows 8.1+ without requiring the WinRT C++/CX or C++/WinRT projections.
// Designed for fire-and-forget: callers do not await the notification.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

// Severity levels for toast notifications.
enum class NotifySeverity : uint8_t {
    INFO         = 0,  // Informational — shown in notification centre only
    NOTIFY_WARN  = 1,  // Warning — shown as a transient toast
    NOTIFY_ERROR = 2,  // Error — shown as persistent toast with reshow on next shell open
};

// ShellNotificationProvider — fire-and-forget WinRT toast bridge.
//
// Usage:
//   ShellNotificationProvider::Notify(L"Decode failed", L"Unable to render PDF thumbnail.", NotifySeverity::NOTIFY_ERROR);
//
// Implementation notes:
//   * Initialises the WinRT apartment on first call (RoInitialize MTA).
//   * Uses the Windows 8.1+ XML toast template toast.xml via IToastNotification.
//   * If WinRT initialisation fails (e.g., running in a non-interactive session)
//     the call silently no-ops — callers must not depend on delivery.
//   * Application User Model ID defaults to "ExplorerLens.Shell" — override via
//     SetAppId() before the first Notify() call.
//
class ShellNotificationProvider {
public:
    // Send a toast notification.
    // title   — notification heading (max 64 chars, truncated silently)
    // body    — notification body text (max 200 chars, truncated silently)
    // severity — INFO, WARNING, or ERROR; affects icon and toast duration
    //
    // Returns true if the notification was dispatched to the OS.
    // Returns false if WinRT is unavailable (server session, minimal desktop, etc.)
    static bool Notify(const std::wstring& title,
                       const std::wstring& body,
                       NotifySeverity     severity = NotifySeverity::INFO);

    // Override the AUMID shown in the notification action centre.
    // Must be called before the first Notify().
    // Default: L"ExplorerLens.Shell"
    static void SetAppId(const std::wstring& appId);

    // Returns true if WinRT notifications are available on this system.
    // Performs a one-shot detection on first call (cheap thereafter).
    static bool IsAvailable();

private:
    ShellNotificationProvider() = delete;

    // Formats the notification XML payload for the WinRT Toast API.
    static std::wstring BuildXml(const std::wstring& title,
                                  const std::wstring& body,
                                  NotifySeverity     severity);

    // Escape XML special characters in toast payload strings.
    static std::wstring XmlEscape(const std::wstring& in);

    static std::wstring s_appId;
    static bool         s_available;
    static bool         s_detectedOnce;
};

} } // namespace ExplorerLens::Engine
