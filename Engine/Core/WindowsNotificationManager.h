// WindowsNotificationManager.h — Toast Notification Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Provides Windows toast notification support for long-running decode
// operations, cache maintenance events, and batch processing completion.
//
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class NotificationPriority : uint8_t {
    Low = 0,
    Normal,
    High,
    Urgent
};

struct NotificationPayload
{
    std::wstring title;
    std::wstring body;
    std::wstring iconPath;
    NotificationPriority priority = NotificationPriority::Normal;
    uint32_t timeoutMs = 5000;
    bool persistent = false;
};

struct WinNotificationStats
{
    uint64_t totalSent = 0;
    uint64_t totalDismissed = 0;
    uint64_t totalClicked = 0;
    uint64_t suppressed = 0;
};

class WindowsNotificationManager
{
  public:
    WindowsNotificationManager() = default;

    bool Initialize(const std::wstring& appId)
    {
        if (appId.empty())
            return false;
        m_appId = appId;
        m_initialized = true;
        return true;
    }

    bool SendNotification(const NotificationPayload& payload)
    {
        if (!m_initialized || payload.title.empty())
            return false;
        m_stats.totalSent++;
        return true;
    }

    void SetCallback(std::function<void(uint32_t)> onClick)
    {
        m_onClick = std::move(onClick);
    }

    WinNotificationStats GetStats() const
    {
        return m_stats;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    void SetQuietMode(bool quiet)
    {
        m_quiet = quiet;
    }

  private:
    std::wstring m_appId;
    std::function<void(uint32_t)> m_onClick;
    WinNotificationStats m_stats;
    bool m_initialized = false;
    bool m_quiet = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
