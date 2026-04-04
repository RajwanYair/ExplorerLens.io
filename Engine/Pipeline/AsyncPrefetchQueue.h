// AsyncPrefetchQueue.h — Async Thumbnail Prefetch Queue
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains a priority queue of upcoming decode requests based on
// Explorer's scroll position and folder navigation patterns. Pre-fetches
// thumbnails before they become visible for sub-millisecond display.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <queue>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PrefetchPriority : uint8_t {
    Critical = 0,
    High = 1,
    Normal = 2,
    Low = 3
};

struct PrefetchRequest
{
    std::wstring filePath;
    uint32_t thumbSize = 256;
    PrefetchPriority priority = PrefetchPriority::Normal;
    uint64_t requestTick = 0;
    double estimatedDecodeMs = 10.0;

    bool operator>(const PrefetchRequest& rhs) const
    {
        return static_cast<uint8_t>(priority) > static_cast<uint8_t>(rhs.priority);
    }
};

struct AsyncPrefetchStats
{
    uint32_t enqueued = 0;
    uint32_t dequeued = 0;
    uint32_t completed = 0;
    uint32_t cancelled = 0;
    uint32_t queueDepth = 0;
    double avgLatencyMs = 0.0;
};

class AsyncPrefetchQueue
{
  public:
    AsyncPrefetchQueue() : m_maxQueueSize(100)
    {
        InitializeSRWLock(&m_lock);
    }
    ~AsyncPrefetchQueue() = default;

    static const wchar_t* GetName()
    {
        return L"AsyncPrefetchQueue";
    }

    void SetMaxQueueSize(uint32_t maxSize)
    {
        m_maxQueueSize = maxSize;
    }

    /// Enqueue a prefetch request.
    bool Enqueue(const PrefetchRequest& req)
    {
        AcquireSRWLockExclusive(&m_lock);
        if (m_queue.size() >= m_maxQueueSize) {
            // Drop lowest-priority item if full
            if (!m_queue.empty()) {
                m_stats.cancelled++;
            }
            ReleaseSRWLockExclusive(&m_lock);
            return false;
        }
        PrefetchRequest r = req;
        r.requestTick = GetTickCount64();
        m_queue.push(r);
        m_stats.enqueued++;
        m_stats.queueDepth = static_cast<uint32_t>(m_queue.size());
        ReleaseSRWLockExclusive(&m_lock);
        return true;
    }

    /// Dequeue the highest-priority request.
    bool Dequeue(PrefetchRequest& out)
    {
        AcquireSRWLockExclusive(&m_lock);
        if (m_queue.empty()) {
            ReleaseSRWLockExclusive(&m_lock);
            return false;
        }
        out = m_queue.top();
        m_queue.pop();
        m_stats.dequeued++;
        m_stats.queueDepth = static_cast<uint32_t>(m_queue.size());
        ReleaseSRWLockExclusive(&m_lock);
        return true;
    }

    /// Report completion of a prefetch request.
    void ReportComplete(double decodeMs)
    {
        m_stats.completed++;
        double total = m_stats.avgLatencyMs * (m_stats.completed - 1) + decodeMs;
        m_stats.avgLatencyMs = total / m_stats.completed;
    }

    /// Clear all pending requests.
    void Clear()
    {
        AcquireSRWLockExclusive(&m_lock);
        while (!m_queue.empty()) {
            m_queue.pop();
            m_stats.cancelled++;
        }
        m_stats.queueDepth = 0;
        ReleaseSRWLockExclusive(&m_lock);
    }

    AsyncPrefetchStats GetStats() const
    {
        return m_stats;
    }

  private:
    using PQ = std::priority_queue<PrefetchRequest, std::vector<PrefetchRequest>, std::greater<PrefetchRequest>>;

    SRWLOCK m_lock{};
    PQ m_queue;
    uint32_t m_maxQueueSize;
    mutable AsyncPrefetchStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
