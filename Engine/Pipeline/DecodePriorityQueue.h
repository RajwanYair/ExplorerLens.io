// DecodePriorityQueue.h — Priority-Aware Decode Request Queue
// Copyright (c) 2026 ExplorerLens Project
//
// Manages pending thumbnail requests with priority based on visibility,
// user scroll position, and file type. Ensures visible thumbnails are
// decoded first and off-screen requests are deprioritized.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class QueuePriority : uint8_t {
    Critical,   // Currently visible in Explorer
    High,       // About to become visible (prefetch)
    Normal,     // Background batch request
    Low,        // Off-screen or speculative
    Idle,       // Only when system is idle
    COUNT
};

struct QueuedDecodeRequest {
    std::wstring filePath;
    uint32_t requestId = 0;
    QueuePriority priority = QueuePriority::Normal;
    uint32_t thumbnailSize = 256;
    uint64_t fileSize = 0;
    double submittedAtMs = 0.0;
    bool cancelled = false;
};

struct QueueStats {
    uint32_t totalQueued = 0;
    uint32_t criticalCount = 0;
    uint32_t highCount = 0;
    uint32_t normalCount = 0;
    uint32_t lowCount = 0;
    uint32_t cancelledCount = 0;
};

class DecodePriorityQueue {
public:
    void Enqueue(const QueuedDecodeRequest& req) {
        m_requests.push_back(req);
        m_stats.totalQueued++;
        switch (req.priority) {
        case QueuePriority::Critical: m_stats.criticalCount++; break;
        case QueuePriority::High:     m_stats.highCount++; break;
        case QueuePriority::Normal:   m_stats.normalCount++; break;
        case QueuePriority::Low:      m_stats.lowCount++; break;
        default: break;
        }
    }

    QueuedDecodeRequest Dequeue() {
        if (m_requests.empty()) return {};
        // Sort by priority (lower enum = higher priority)
        std::sort(m_requests.begin(), m_requests.end(),
            [](const QueuedDecodeRequest& a, const QueuedDecodeRequest& b) {
                return static_cast<uint8_t>(a.priority) < static_cast<uint8_t>(b.priority);
            });
        auto top = m_requests.front();
        m_requests.erase(m_requests.begin());
        return top;
    }

    void Cancel(uint32_t requestId) {
        for (auto& r : m_requests) {
            if (r.requestId == requestId) {
                r.cancelled = true;
                m_stats.cancelledCount++;
                break;
            }
        }
    }

    bool IsEmpty() const { return m_requests.empty(); }
    size_t Size() const { return m_requests.size(); }
    const QueueStats& Stats() const { return m_stats; }

    static const wchar_t* PriorityName(QueuePriority p) {
        switch (p) {
        case QueuePriority::Critical: return L"Critical";
        case QueuePriority::High:     return L"High";
        case QueuePriority::Normal:   return L"Normal";
        case QueuePriority::Low:      return L"Low";
        case QueuePriority::Idle:     return L"Idle";
        default: return L"Unknown";
        }
    }
    static size_t PriorityCount() { return static_cast<size_t>(QueuePriority::COUNT); }

private:
    std::vector<QueuedDecodeRequest> m_requests;
    QueueStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
