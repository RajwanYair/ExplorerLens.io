#pragma once
// =============================================================================
// NotificationEngine.h — Toast Notifications for Batch/Update Events
// ExplorerLens Engine — Utils Module
// =============================================================================

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens {

/// Notification priority level
enum class NotifyPriority : uint32_t {
    Low         = 0,   ///< Informational, dismissable
    Normal      = 1,   ///< Standard notification
    High        = 2,   ///< Important, stays visible longer
    Critical    = 3    ///< Error/failure, requires acknowledgment
};

/// Notification type
enum class NotifyType : uint32_t {
    BatchComplete       = 0,   ///< Batch processing finished
    UpdateAvailable     = 1,   ///< New version available
    DecoderError        = 2,   ///< Decoder failure
    CacheCleared        = 3,   ///< Cache was purged
    PluginLoaded        = 4,   ///< Plugin successfully loaded
    LicenseExpiring     = 5,   ///< License expiration warning
    SystemWarning       = 6,   ///< System resource warning
    Count               = 7
};

/// Notification state
enum class NotifyState : uint32_t {
    Pending     = 0,
    Displayed   = 1,
    Dismissed   = 2,
    Expired     = 3,
    Clicked     = 4
};

/// A single notification record
struct Notification {
    uint64_t        id          = 0;
    NotifyType      type        = NotifyType::BatchComplete;
    NotifyPriority  priority    = NotifyPriority::Normal;
    NotifyState     state       = NotifyState::Pending;
    std::wstring    title;
    std::wstring    message;
    std::wstring    actionUrl;  ///< Optional action on click
    uint64_t        createdAt   = 0;
    uint64_t        expiresAt   = 0;
    uint32_t        durationMs  = 5000;   ///< Display duration
};

/// Notification callback
using NotifyCallback = std::function<void(const Notification&)>;

/// NotificationEngine — manages toast notifications for ExplorerLens events
class NotificationEngine {
public:
    NotificationEngine();

    // Notification lifecycle
    uint64_t Send(NotifyType type, const std::wstring& title, const std::wstring& message,
                  NotifyPriority priority = NotifyPriority::Normal);
    bool Dismiss(uint64_t notificationId);
    bool MarkClicked(uint64_t notificationId);
    uint32_t ClearExpired();

    // Queue management
    uint32_t GetPendingCount() const;
    uint32_t GetTotalCount() const { return static_cast<uint32_t>(m_notifications.size()); }
    const Notification* GetNotification(uint64_t id) const;
    std::vector<Notification> GetByType(NotifyType type) const;
    std::vector<Notification> GetByState(NotifyState state) const;

    // Callbacks
    void SetOnNotify(NotifyCallback callback) { m_onNotify = callback; }

    // Configuration
    void SetDefaultDuration(uint32_t ms) { m_defaultDurationMs = ms; }
    void SetMaxNotifications(uint32_t max) { m_maxNotifications = max; }

    // Static
    static const wchar_t* GetTypeName(NotifyType type);
    static const wchar_t* GetPriorityName(NotifyPriority priority);
    static const wchar_t* GetStateName(NotifyState state);
    static constexpr uint32_t GetTypeCount() { return static_cast<uint32_t>(NotifyType::Count); }

private:
    std::vector<Notification>   m_notifications;
    NotifyCallback              m_onNotify;
    uint64_t                    m_nextId = 1;
    uint32_t                    m_defaultDurationMs = 5000;
    uint32_t                    m_maxNotifications = 100;
};

} // namespace ExplorerLens

