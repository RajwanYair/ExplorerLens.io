#pragma once
// ============================================================================
// DecoderPriorityScheduler.h — Priority-queue scheduler for decoder tasks
//
// Purpose:   Priority-queue scheduler for decoder task ordering
// Provides:  DecoderTaskPriority, SchedulerPolicy enums, and
//            DecoderPriorityScheduler class
// Used by:   Batch decode pipeline to prioritize visible thumbnails
// ============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

/// Priority level for a decoder task
enum class DecoderTaskPriority : uint8_t {
    Urgent = 0,   // Visible thumbnail in Explorer's viewport
    Foreground = 1,   // Near-viewport, likely to scroll into view
    Background = 2,   // Pre-fetch or speculative decode
    Idle = 3,   // Low-priority maintenance decode
    Deferred = 4    // Parked until resources free up
};

inline const char* DecoderTaskPriorityName(DecoderTaskPriority p) noexcept {
    switch (p) {
    case DecoderTaskPriority::Urgent:     return "Urgent";
    case DecoderTaskPriority::Foreground: return "Foreground";
    case DecoderTaskPriority::Background: return "Background";
    case DecoderTaskPriority::Idle:       return "Idle";
    case DecoderTaskPriority::Deferred:   return "Deferred";
    default:                              return "Unknown";
    }
}

/// Scheduling policy for the task queue
enum class SchedulerPolicy : uint8_t {
    FIFO = 0,   // First-in first-out, ignoring priority
    Priority = 1,   // Strict priority ordering
    RoundRobin = 2,   // Cycle through priority bands equally
    Deadline = 3,   // Earliest deadline first
    Weighted = 4    // Weighted fair queuing across priorities
};

inline const char* SchedulerPolicyName(SchedulerPolicy p) noexcept {
    switch (p) {
    case SchedulerPolicy::FIFO:       return "FIFO";
    case SchedulerPolicy::Priority:   return "Priority";
    case SchedulerPolicy::RoundRobin: return "RoundRobin";
    case SchedulerPolicy::Deadline:   return "Deadline";
    case SchedulerPolicy::Weighted:   return "Weighted";
    default:                          return "Unknown";
    }
}

/// A single decoder task submitted to the scheduler
struct DecoderTask {
    DecoderTaskPriority priority = DecoderTaskPriority::Background;
    SchedulerPolicy     policy = SchedulerPolicy::Priority;
    std::string         decoderName;   // e.g. "LibRaw", "LibWebP"
    std::wstring        filePath;      // Source file to decode
    double              deadlineMs = 0.0;  // Soft deadline for Deadline policy
    uint64_t            taskId = 0;    // Unique task identifier
};

/// Schedules decoder tasks across the thread pool according to the
/// configured scheduling policy.  Supports cancellation and live
/// queue-depth monitoring.
class DecoderPriorityScheduler {
public:
    DecoderPriorityScheduler() = default;
    ~DecoderPriorityScheduler() = default;

    DecoderPriorityScheduler(const DecoderPriorityScheduler&) = delete;
    DecoderPriorityScheduler& operator=(const DecoderPriorityScheduler&) = delete;
    DecoderPriorityScheduler(DecoderPriorityScheduler&&) noexcept = default;
    DecoderPriorityScheduler& operator=(DecoderPriorityScheduler&&) noexcept = default;

    /// Submit a new task to the queue; returns assigned task ID
    uint64_t Submit(DecoderTask task) {
        task.taskId = m_nextId++;
        m_queue.push_back(std::move(task));
        SortQueue();
        return task.taskId;
    }

    /// Cancel a queued task by ID; returns true if found and removed
    bool Cancel(uint64_t taskId) {
        auto it = std::find_if(m_queue.begin(), m_queue.end(),
            [taskId](const DecoderTask& t) { return t.taskId == taskId; });
        if (it == m_queue.end()) return false;
        m_queue.erase(it);
        return true;
    }

    /// Number of tasks waiting in the queue
    uint32_t GetQueueDepth() const noexcept {
        return static_cast<uint32_t>(m_queue.size());
    }

    /// Number of actively running tasks (incremented by PopNext, decremented by MarkCompleted)
    uint32_t GetActiveCount() const noexcept { return m_activeCount; }

    /// Peek at the next task without removing it
    bool PeekNext(DecoderTask& outTask) const {
        if (m_queue.empty()) return false;
        outTask = m_queue.front();
        return true;
    }

    /// Pop the next task from the queue
    bool PopNext(DecoderTask& outTask) {
        if (m_queue.empty()) return false;
        outTask = m_queue.front();
        m_queue.erase(m_queue.begin());
        m_activeCount++;
        return true;
    }

    /// Mark a task as completed, decrementing active count
    void MarkCompleted() noexcept {
        if (m_activeCount > 0) m_activeCount--;
    }

private:
    void SortQueue() {
        std::stable_sort(m_queue.begin(), m_queue.end(),
            [](const DecoderTask& a, const DecoderTask& b) {
                return static_cast<uint8_t>(a.priority) <
                    static_cast<uint8_t>(b.priority);
            });
    }

    std::vector<DecoderTask> m_queue;
    uint64_t m_nextId = 1;
    uint32_t m_activeCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
