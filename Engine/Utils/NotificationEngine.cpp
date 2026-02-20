// =============================================================================
// NotificationEngine.cpp — Sprint 243: Toast Notifications for Batch/Update Events
// DarkThumbs Engine — Utils Module
// =============================================================================

#include "NotificationEngine.h"
#include <chrono>
#include <algorithm>

namespace DarkThumbs {

static uint64_t NowMs() {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

NotificationEngine::NotificationEngine() {}

uint64_t NotificationEngine::Send(NotifyType type, const std::wstring& title,
                                   const std::wstring& message, NotifyPriority priority) {
    // Enforce max notifications
    if (m_notifications.size() >= m_maxNotifications) {
        ClearExpired();
        // If still at max, remove oldest dismissed
        if (m_notifications.size() >= m_maxNotifications) {
            auto it = std::find_if(m_notifications.begin(), m_notifications.end(),
                [](const Notification& n) { return n.state == NotifyState::Dismissed; });
            if (it != m_notifications.end()) m_notifications.erase(it);
        }
    }

    Notification n;
    n.id = m_nextId++;
    n.type = type;
    n.priority = priority;
    n.state = NotifyState::Pending;
    n.title = title;
    n.message = message;
    n.createdAt = NowMs();
    n.durationMs = m_defaultDurationMs;
    // Critical notifications last longer
    if (priority == NotifyPriority::Critical) n.durationMs *= 3;
    else if (priority == NotifyPriority::High) n.durationMs *= 2;
    n.expiresAt = n.createdAt + n.durationMs;

    m_notifications.push_back(n);

    // Fire callback
    if (m_onNotify) m_onNotify(n);

    return n.id;
}

bool NotificationEngine::Dismiss(uint64_t notificationId) {
    for (auto& n : m_notifications) {
        if (n.id == notificationId && n.state != NotifyState::Dismissed) {
            n.state = NotifyState::Dismissed;
            return true;
        }
    }
    return false;
}

bool NotificationEngine::MarkClicked(uint64_t notificationId) {
    for (auto& n : m_notifications) {
        if (n.id == notificationId) {
            n.state = NotifyState::Clicked;
            return true;
        }
    }
    return false;
}

uint32_t NotificationEngine::ClearExpired() {
    uint64_t now = NowMs();
    uint32_t cleared = 0;
    for (auto& n : m_notifications) {
        if (n.state == NotifyState::Pending || n.state == NotifyState::Displayed) {
            if (now >= n.expiresAt) {
                n.state = NotifyState::Expired;
                cleared++;
            }
        }
    }
    return cleared;
}

uint32_t NotificationEngine::GetPendingCount() const {
    uint32_t count = 0;
    for (const auto& n : m_notifications) {
        if (n.state == NotifyState::Pending) count++;
    }
    return count;
}

const Notification* NotificationEngine::GetNotification(uint64_t id) const {
    for (const auto& n : m_notifications) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

std::vector<Notification> NotificationEngine::GetByType(NotifyType type) const {
    std::vector<Notification> result;
    for (const auto& n : m_notifications) {
        if (n.type == type) result.push_back(n);
    }
    return result;
}

std::vector<Notification> NotificationEngine::GetByState(NotifyState state) const {
    std::vector<Notification> result;
    for (const auto& n : m_notifications) {
        if (n.state == state) result.push_back(n);
    }
    return result;
}

const wchar_t* NotificationEngine::GetTypeName(NotifyType type) {
    switch (type) {
        case NotifyType::BatchComplete:     return L"Batch Complete";
        case NotifyType::UpdateAvailable:   return L"Update Available";
        case NotifyType::DecoderError:      return L"Decoder Error";
        case NotifyType::CacheCleared:      return L"Cache Cleared";
        case NotifyType::PluginLoaded:      return L"Plugin Loaded";
        case NotifyType::LicenseExpiring:   return L"License Expiring";
        case NotifyType::SystemWarning:     return L"System Warning";
        default:                            return L"Unknown";
    }
}

const wchar_t* NotificationEngine::GetPriorityName(NotifyPriority priority) {
    switch (priority) {
        case NotifyPriority::Low:       return L"Low";
        case NotifyPriority::Normal:    return L"Normal";
        case NotifyPriority::High:      return L"High";
        case NotifyPriority::Critical:  return L"Critical";
        default:                        return L"Unknown";
    }
}

const wchar_t* NotificationEngine::GetStateName(NotifyState state) {
    switch (state) {
        case NotifyState::Pending:      return L"Pending";
        case NotifyState::Displayed:    return L"Displayed";
        case NotifyState::Dismissed:    return L"Dismissed";
        case NotifyState::Expired:      return L"Expired";
        case NotifyState::Clicked:      return L"Clicked";
        default:                        return L"Unknown";
    }
}

} // namespace DarkThumbs
