// ToastNotifier.h — WinRT Toast Notification Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Sends Windows Runtime toast notifications from the ExplorerLens engine
// without requiring a packaged app identity when running as a COM server.
// Falls back to Shell_NotifyIcon balloon when WinRT is unavailable.
//
#pragma once

#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

// Toast urgency level maps to WinRT ToastScenario.
enum class ToastUrgency : uint8_t {
    Default  = 0,  // Standard info notification
    Reminder = 1,  // Persistent until dismissed
    Alarm    = 2,  // Looping + prominent
    Warning  = 3,  // Yellow-accent alert
};

// ToastNotifier — Sends WinRT toast notifications from an unpackaged context.
//
// Registers ExplorerLens as a COM activatable server via the app-ID approach
// (no MSIX required). Falls back to tray balloon if WinRT COM is unavailable.
//
// Design notes:
//   - AppId must match the registered COM server CLSID in the registry.
//   - All methods are safe to call before WinRT is initialised (will no-op gracefully).
//   - Thread-safe: all public methods acquire m_lock.
class ToastNotifier {
public:
    ToastNotifier() noexcept;
    ~ToastNotifier() noexcept;

    ToastNotifier(const ToastNotifier&)            = delete;
    ToastNotifier& operator=(const ToastNotifier&) = delete;

    // Initialise the WinRT notifier for a specific app-ID ProgID.
    // Call once before ShowToast(). Returns false if WinRT unavailable.
    bool Initialize(const std::wstring& appId) noexcept;

    // Show a simple title+body toast.
    void ShowToast(const std::wstring& title,
                   const std::wstring& body,
                   ToastUrgency        urgency = ToastUrgency::Default) noexcept;

    // Show a toast with an action button.
    void ShowToastWithAction(const std::wstring& title,
                             const std::wstring& body,
                             const std::wstring& actionLabel,
                             const std::wstring& actionArgs,
                             ToastUrgency        urgency = ToastUrgency::Default) noexcept;

    // Show a progress-bar toast.  Call UpdateProgress() to update the bar.
    void ShowProgressToast(const std::wstring& title,
                           const std::wstring& tag,
                           double              initialValue = 0.0) noexcept;

    // Update an in-progress toast progress bar (0.0 – 1.0).
    void UpdateProgress(const std::wstring& tag,
                        double              value,
                        const std::wstring& statusText = {}) noexcept;

    // Dismiss a specific toast by tag.
    void Dismiss(const std::wstring& tag) noexcept;

    bool IsAvailable() const noexcept { return m_available; }

private:
    bool        m_available { false };
    std::wstring m_appId;
    void*        m_pNotifier { nullptr };  // Opaque WinRT handle (avoids WinRT headers)
};

}} // namespace ExplorerLens::Engine
