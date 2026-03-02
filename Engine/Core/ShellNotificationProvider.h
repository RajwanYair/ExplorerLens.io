// ShellNotificationProvider.h — Explorer Shell Notification Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Provides notification infrastructure for cross-communication with
// Windows Explorer shell. Generates progress notifications, decode
// error alerts, and batch completion reports via registered callbacks.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class NotificationType : uint8_t {
    DecodeComplete, DecodeError, BatchComplete, CacheCleared,
    MemoryWarning, GPUFault, FormatUnsupported, Info
};

struct ShellNotifyEvent {
    NotificationType type = NotificationType::Info;
    std::wstring     title;
    std::wstring     message;
    std::wstring     filePath;
    uint32_t         errorCode = 0;
    uint64_t         timestamp = 0;
};

struct NotificationConfig {
    bool     enableDecodeComplete = false;  // Too noisy by default
    bool     enableErrors = true;
    bool     enableBatchComplete = true;
    bool     enableMemoryWarnings = true;
    bool     enableGPUFaults = true;
    uint32_t maxRecentNotifications = 50;
    uint32_t throttleMs = 1000; // Min time between same-type notifications
};

struct NotificationStats {
    uint32_t totalNotifications = 0;
    uint32_t suppressed = 0;
    uint32_t errorNotifications = 0;
    uint32_t warningNotifications = 0;
};

class ShellNotificationProvider {
public:
    ShellNotificationProvider() {
        InitializeSRWLock(&m_lock);
    }
    ~ShellNotificationProvider() = default;

    static const wchar_t* GetName() { return L"ShellNotificationProvider"; }

    void Configure(const NotificationConfig& config) { m_config = config; }

    /// Post a notification (thread-safe).
    bool Post(const ShellNotifyEvent& notif) {
        // Check if this type is enabled
        if (!IsEnabled(notif.type)) {
            m_stats.suppressed++;
            return false;
        }

        // Throttle check
        uint64_t now = GetTickCount64();
        bool shouldThrottle = false;

        AcquireSRWLockExclusive(&m_lock);
        auto typeIdx = static_cast<uint8_t>(notif.type);
        if (typeIdx < 8) {
            if (now - m_lastNotifTick[typeIdx] < m_config.throttleMs) {
                shouldThrottle = true;
            }
            else {
                m_lastNotifTick[typeIdx] = now;
            }
        }

        if (shouldThrottle) {
            m_stats.suppressed++;
            ReleaseSRWLockExclusive(&m_lock);
            return false;
        }

        // Store notification
        ShellNotifyEvent n = notif;
        n.timestamp = now;
        m_recent.push_back(n);
        if (m_recent.size() > m_config.maxRecentNotifications)
            m_recent.erase(m_recent.begin());

        m_stats.totalNotifications++;
        if (notif.type == NotificationType::DecodeError || notif.type == NotificationType::GPUFault)
            m_stats.errorNotifications++;
        if (notif.type == NotificationType::MemoryWarning)
            m_stats.warningNotifications++;

        ReleaseSRWLockExclusive(&m_lock);
        return true;
    }

    /// Get recent notifications.
    std::vector<ShellNotifyEvent> GetRecent(uint32_t count = 10) const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        uint32_t n = std::min(count, static_cast<uint32_t>(m_recent.size()));
        std::vector<ShellNotifyEvent> result(m_recent.end() - n, m_recent.end());
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return result;
    }

    NotificationStats GetStats() const { return m_stats; }

private:
    bool IsEnabled(NotificationType type) const {
        switch (type) {
        case NotificationType::DecodeComplete:   return m_config.enableDecodeComplete;
        case NotificationType::DecodeError:      return m_config.enableErrors;
        case NotificationType::BatchComplete:    return m_config.enableBatchComplete;
        case NotificationType::MemoryWarning:    return m_config.enableMemoryWarnings;
        case NotificationType::GPUFault:         return m_config.enableGPUFaults;
        default: return true;
        }
    }

    SRWLOCK m_lock{};
    NotificationConfig m_config;
    std::vector<ShellNotifyEvent> m_recent;
    uint64_t m_lastNotifTick[8] = {};
    mutable NotificationStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
