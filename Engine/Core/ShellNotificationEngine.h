#pragma once
// ============================================================================
// ShellNotificationEngine.h — Shell change notification broadcasting
//                             (SHChangeNotify wrapper)
//
// Purpose:   Shell change notification broadcasting (SHChangeNotify wrapper)
// Provides:  NotifyEvent, NotifyScope enums, ShellNotification struct,
//            ShellNotificationEngine class
// Used by:   Registration and cache invalidation
// ============================================================================

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Enums ────────────────────────────────────────────────────────────────────

enum class ShellNotifyType : uint8_t {
    AssocChanged = 0,
    UpdateDir = 1,
    UpdateItem = 2,
    RenameItem = 3,
    Delete = 4
};

inline const char* ShellNotifyTypeName(ShellNotifyType t) {
    switch (t) {
    case ShellNotifyType::AssocChanged: return "AssocChanged";
    case ShellNotifyType::UpdateDir:    return "UpdateDir";
    case ShellNotifyType::UpdateItem:   return "UpdateItem";
    case ShellNotifyType::RenameItem:   return "RenameItem";
    case ShellNotifyType::Delete:       return "Delete";
    default:                            return "Unknown";
    }
}

enum class ShellNotifyPriority : uint8_t {
    Immediate = 0,
    High = 1,
    Normal = 2,
    Low = 3,
    Batched = 4
};

inline const char* ShellNotifyPriorityName(ShellNotifyPriority p) {
    switch (p) {
    case ShellNotifyPriority::Immediate: return "Immediate";
    case ShellNotifyPriority::High:      return "High";
    case ShellNotifyPriority::Normal:    return "Normal";
    case ShellNotifyPriority::Low:       return "Low";
    case ShellNotifyPriority::Batched:   return "Batched";
    default:                             return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct ShellNotification {
    ShellNotifyType     type = ShellNotifyType::UpdateItem;
    ShellNotifyPriority priority = ShellNotifyPriority::Normal;
    std::string         itemPath;
    uint64_t            timestamp = 0;
};

// ── Class ────────────────────────────────────────────────────────────────────

class ShellNotificationEngine {
public:
    ShellNotificationEngine() = default;
    ~ShellNotificationEngine() = default;

    // Queue or immediately send a shell notification
    bool SendNotification(const ShellNotification& notification) {
        if (notification.itemPath.empty())
            return false;

        if (notification.priority == ShellNotifyPriority::Immediate) {
            // Immediate dispatch (simulated)
            m_dispatchedCount++;
            m_totalSent++;
            return true;
        }
        // Otherwise queue for batching
        m_pendingQueue.push_back(notification);
        m_totalSent++;
        return true;
    }

    // Flush all pending batched notifications
    uint32_t BatchFlush() {
        uint32_t flushed = static_cast<uint32_t>(m_pendingQueue.size());
        m_dispatchedCount += flushed;
        m_pendingQueue.clear();
        m_flushCount++;
        return flushed;
    }

    size_t GetPendingCount() const { return m_pendingQueue.size(); }
    uint64_t GetTotalSent() const { return m_totalSent; }
    uint64_t GetDispatchedCount() const { return m_dispatchedCount; }
    uint32_t GetFlushCount() const { return m_flushCount; }

    // Peek at the next pending notification
    const ShellNotification* PeekPending() const {
        if (m_pendingQueue.empty())
            return nullptr;
        return &m_pendingQueue.front();
    }

    bool HasPending() const { return !m_pendingQueue.empty(); }

    void ClearPending() {
        m_pendingQueue.clear();
    }

    void Reset() {
        m_pendingQueue.clear();
        m_totalSent = 0;
        m_dispatchedCount = 0;
        m_flushCount = 0;
    }

private:
    std::vector<ShellNotification> m_pendingQueue;
    uint64_t                       m_totalSent = 0;
    uint64_t                       m_dispatchedCount = 0;
    uint32_t                       m_flushCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
