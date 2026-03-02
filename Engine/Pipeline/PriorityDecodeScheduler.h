// PriorityDecodeScheduler.h — Priority-Based Decode Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Schedules concurrent decode operations using a priority system that
// factors in file visibility, format complexity, and system load.
// Cooperates with Windows thread pool for optimal CPU utilization.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class DecodeUrgency : uint8_t {
    Immediate = 0,  // Currently visible thumbnail
    Soon = 1,       // About to scroll into view
    Deferred = 2,   // Prefetch / background
    Idle = 3        // Low-priority background processing
};

struct ScheduledDecodeTask {
    uint64_t       taskId = 0;
    std::wstring   filePath;
    DecodeUrgency  urgency = DecodeUrgency::Deferred;
    float          complexityScore = 1.0f; // 0.0-10.0, higher = more complex
    uint32_t       thumbSize = 256;
    uint64_t       enqueueTick = 0;
    uint64_t       startTick = 0;
    uint64_t       endTick = 0;
    bool           completed = false;
    bool           cancelled = false;
};

struct DecodeSchedulerConfig {
    uint32_t maxConcurrentTasks = 4;
    uint32_t maxQueueSize = 256;
    bool     respectSystemLoad = true;
    float    cpuThrottleThreshold = 0.85f; // Throttle at 85% CPU
};

struct DecodeSchedulerStats {
    uint32_t totalScheduled = 0;
    uint32_t totalCompleted = 0;
    uint32_t totalCancelled = 0;
    uint32_t activeTasks = 0;
    uint32_t queuedTasks = 0;
    double   avgWaitMs = 0.0;
    double   avgDecodeMs = 0.0;
    double   throughputPerSec = 0.0;
};

class PriorityDecodeScheduler {
public:
    PriorityDecodeScheduler() : m_nextTaskId(1) {
        InitializeSRWLock(&m_lock);
    }
    ~PriorityDecodeScheduler() = default;

    static const wchar_t* GetName() { return L"PriorityDecodeScheduler"; }

    void Configure(const DecodeSchedulerConfig& config) { m_config = config; }

    /// Submit a decode task, returns task ID.
    uint64_t Submit(const std::wstring& path, DecodeUrgency urgency,
        uint32_t thumbSize = 256, float complexity = 1.0f) {
        AcquireSRWLockExclusive(&m_lock);
        ScheduledDecodeTask task;
        task.taskId = m_nextTaskId++;
        task.filePath = path;
        task.urgency = urgency;
        task.thumbSize = thumbSize;
        task.complexityScore = complexity;
        task.enqueueTick = GetTickCount64();
        m_queue.push_back(task);
        // Sort by urgency, then by complexity (lighter first)
        std::sort(m_queue.begin(), m_queue.end(), [](const ScheduledDecodeTask& a, const ScheduledDecodeTask& b) {
            if (a.urgency != b.urgency)
                return static_cast<uint8_t>(a.urgency) < static_cast<uint8_t>(b.urgency);
            return a.complexityScore < b.complexityScore;
            });
        m_stats.totalScheduled++;
        m_stats.queuedTasks = static_cast<uint32_t>(m_queue.size());
        uint64_t id = task.taskId;
        ReleaseSRWLockExclusive(&m_lock);
        return id;
    }

    /// Cancel a pending task by ID. returns true if found & cancelled.
    bool Cancel(uint64_t taskId) {
        AcquireSRWLockExclusive(&m_lock);
        for (auto it = m_queue.begin(); it != m_queue.end(); ++it) {
            if (it->taskId == taskId) {
                m_queue.erase(it);
                m_stats.totalCancelled++;
                m_stats.queuedTasks = static_cast<uint32_t>(m_queue.size());
                ReleaseSRWLockExclusive(&m_lock);
                return true;
            }
        }
        ReleaseSRWLockExclusive(&m_lock);
        return false;
    }

    /// Get next task to execute (highest priority).
    bool GetNext(ScheduledDecodeTask& out) {
        AcquireSRWLockExclusive(&m_lock);
        if (m_queue.empty() || m_stats.activeTasks >= m_config.maxConcurrentTasks) {
            ReleaseSRWLockExclusive(&m_lock);
            return false;
        }
        out = m_queue.front();
        m_queue.erase(m_queue.begin());
        out.startTick = GetTickCount64();
        m_stats.activeTasks++;
        m_stats.queuedTasks = static_cast<uint32_t>(m_queue.size());
        ReleaseSRWLockExclusive(&m_lock);
        return true;
    }

    /// Report task completion.
    void Complete([[maybe_unused]] uint64_t taskId) {
        m_stats.activeTasks = (m_stats.activeTasks > 0) ? m_stats.activeTasks - 1 : 0;
        m_stats.totalCompleted++;
    }

    DecodeSchedulerStats GetStats() const { return m_stats; }

private:
    SRWLOCK m_lock{};
    std::vector<ScheduledDecodeTask> m_queue;
    DecodeSchedulerConfig m_config;
    uint64_t m_nextTaskId;
    mutable DecodeSchedulerStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
