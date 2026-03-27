// ThumbnailPriorityQueue.h — Priority-Based Decode Request Scheduling
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a thread-safe priority queue for thumbnail decode requests.
// Visible viewport items get highest priority; background prefetch items
// get lowest. Supports cancellation and priority boosting.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <algorithm>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class ThumbnailPriority : int {
    Critical = 0,   // User-visible, blocking (shell requesting thumbnail synchronously)
    High     = 1,   // Viewport-visible items
    Normal   = 2,   // Near-viewport prefetch
    Low      = 3,   // Background speculative prefetch
    Idle     = 4    // Cleanup / cache warming
};

using DecodeRequestId = uint64_t;

struct ThumbnailDecodeItem {
    DecodeRequestId    id          = 0;
    std::wstring       filePath;
    ThumbnailPriority     priority    = ThumbnailPriority::Normal;
    uint32_t           thumbWidth  = 256;
    uint32_t           thumbHeight = 256;
    std::function<void(DecodeRequestId, bool /*success*/)> onComplete;
    DWORD              submitTick  = 0; // GetTickCount() at submit time (for aging)
    bool               cancelled   = false;
};

class ThumbnailPriorityQueue {
public:
    ThumbnailPriorityQueue() {
        m_nextId = 1;
    }

    // Submit a decode request; returns assigned ID
    DecodeRequestId Submit(ThumbnailDecodeItem req) {
        std::unique_lock<std::mutex> lk(m_mtx);
        req.id         = m_nextId++;
        req.submitTick = GetTickCount();
        m_queue.push_back(std::move(req));
        // Maintain heap order
        std::push_heap(m_queue.begin(), m_queue.end(), CompareRequests);
        m_cv.notify_one();
        return req.id;
    }

    // Cancel a pending request (if not yet dequeued)
    bool Cancel(DecodeRequestId id) {
        std::lock_guard<std::mutex> lk(m_mtx);
        for (auto& r : m_queue) {
            if (r.id == id) {
                r.cancelled = true;
                return true;
            }
        }
        return false;
    }

    // Boost priority of an existing request (e.g. user scrolled to it)
    bool Boost(DecodeRequestId id, ThumbnailPriority newPriority) {
        std::lock_guard<std::mutex> lk(m_mtx);
        for (auto& r : m_queue) {
            if (r.id == id && r.priority > newPriority) {
                r.priority = newPriority;
                std::make_heap(m_queue.begin(), m_queue.end(), CompareRequests);
                return true;
            }
        }
        return false;
    }

    // Dequeue highest-priority non-cancelled request (blocks if empty)
    bool Dequeue(ThumbnailDecodeItem& out, DWORD timeoutMs = INFINITE) {
        std::unique_lock<std::mutex> lk(m_mtx);
        auto deadline = std::chrono::milliseconds(timeoutMs);
        bool timedOut = !m_cv.wait_for(lk, deadline, [this] {
            return m_shutdown || HasNonCancelledItem();
        });
        if (timedOut || m_shutdown) return false;

        // Pop from heap, skip cancelled
        while (!m_queue.empty()) {
            std::pop_heap(m_queue.begin(), m_queue.end(), CompareRequests);
            ThumbnailDecodeItem req = std::move(m_queue.back());
            m_queue.pop_back();
            if (!req.cancelled) {
                out = std::move(req);
                return true;
            }
        }
        return false;
    }

    void Shutdown() {
        { std::lock_guard<std::mutex> lk(m_mtx); m_shutdown = true; }
        m_cv.notify_all();
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_queue.size();
    }

    // Age-based priority decay: promote requests waiting longer than agingMs
    void ApplyAging(DWORD agingMs = 2000) {
        std::lock_guard<std::mutex> lk(m_mtx);
        DWORD now = GetTickCount();
        bool changed = false;
        for (auto& r : m_queue) {
            if (!r.cancelled && r.priority > ThumbnailPriority::High &&
                (now - r.submitTick) > agingMs) {
                r.priority = static_cast<ThumbnailPriority>(static_cast<int>(r.priority) - 1);
                changed = true;
            }
        }
        if (changed)
            std::make_heap(m_queue.begin(), m_queue.end(), CompareRequests);
    }

    int PendingCount(ThumbnailPriority p) const {
        std::lock_guard<std::mutex> lk(m_mtx);
        int cnt = 0;
        for (const auto& r : m_queue)
            if (r.priority == p && !r.cancelled) cnt++;
        return cnt;
    }

private:
    static bool CompareRequests(const ThumbnailDecodeItem& a, const ThumbnailDecodeItem& b) {
        // Lower priority value = higher precedence; older (lower tick) breaks ties
        if (a.priority != b.priority)
            return static_cast<int>(a.priority) > static_cast<int>(b.priority);
        return a.submitTick > b.submitTick;
    }

    bool HasNonCancelledItem() const {
        for (const auto& r : m_queue)
            if (!r.cancelled) return true;
        return false;
    }

    mutable std::mutex              m_mtx;
    std::condition_variable         m_cv;
    std::vector<ThumbnailDecodeItem>      m_queue; // binary max-heap
    std::atomic<DecodeRequestId>    m_nextId { 1 };
    bool                            m_shutdown = false;
};

}} // namespace ExplorerLens::Engine
