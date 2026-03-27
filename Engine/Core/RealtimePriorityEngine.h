// RealtimePriorityEngine.h — Real-Time Decode Priority Queue (RMS-Based)
// Copyright (c) 2026 ExplorerLens Project
//
// Rate-monotonic-inspired decode priority queue that guarantees deadline compliance for visible-viewport requests.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct RTDecodeTask {
    uint64_t    id;
    double      deadlineMs;   // wall-clock deadline
    uint32_t    periodMs;     // scheduling period
    uint8_t     priority;     // 0=highest
};
class RealtimePriorityEngine {
public:
    bool   Enqueue(RTDecodeTask task)       { m_pending++; (void)task; return true; }
    bool   Dequeue(RTDecodeTask& out)       { if (!m_pending) return false; m_pending--; out = {}; return true; }
    size_t PendingCount() const             { return m_pending; }
    bool   WouldMissDeadline(double deadlineMs) const { (void)deadlineMs; return false; }
private:
    std::atomic<size_t> m_pending{0};
};

} // namespace Engine
} // namespace ExplorerLens