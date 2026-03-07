// RateLimitedDecodeQueue.h — Backpressure-Aware Decode Queue
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a rate-limited decode work queue with configurable throughput
// caps, backpressure signaling, and fair scheduling across requestors.
//
#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class RateLimitPriority : uint8_t {
    Critical,
    High,
    Normal,
    Low,
    Background
};

struct DecodeWorkItem {
    uint64_t itemId = 0;
    std::wstring filePath;
    RateLimitPriority priority = RateLimitPriority::Normal;
    uint32_t requestedSize = 256;
    uint64_t enqueuedTimestamp = 0;
};

struct RateLimitQueueStats {
    uint64_t totalEnqueued = 0;
    uint64_t totalDequeued = 0;
    uint64_t totalDropped = 0;
    uint32_t currentDepth = 0;
    uint32_t maxDepthReached = 0;
    double avgWaitTimeMs = 0.0;
};

class RateLimitedDecodeQueue {
public:
    explicit RateLimitedDecodeQueue(uint32_t maxQueueDepth = 256, uint32_t maxPerSecond = 100)
        : m_maxQueueDepth(maxQueueDepth), m_maxPerSecond(maxPerSecond) {
    }

    bool Enqueue(const DecodeWorkItem& item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.size() >= m_maxQueueDepth) {
            m_stats.totalDropped++;
            return false;
        }
        m_queue.push(item);
        m_stats.totalEnqueued++;
        m_stats.currentDepth = static_cast<uint32_t>(m_queue.size());
        if (m_stats.currentDepth > m_stats.maxDepthReached)
            m_stats.maxDepthReached = m_stats.currentDepth;
        return true;
    }

    bool Dequeue(DecodeWorkItem& item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) return false;
        item = m_queue.front();
        m_queue.pop();
        m_stats.totalDequeued++;
        m_stats.currentDepth = static_cast<uint32_t>(m_queue.size());
        return true;
    }

    bool IsBackpressured() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size() > (m_maxQueueDepth * 3 / 4);
    }

    RateLimitQueueStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    uint32_t GetDepth() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_queue.size());
    }

    void SetRateLimit(uint32_t maxPerSecond) { m_maxPerSecond = maxPerSecond; }
    uint32_t GetRateLimit() const { return m_maxPerSecond; }

private:
    mutable std::mutex m_mutex;
    std::queue<DecodeWorkItem> m_queue;
    uint32_t m_maxQueueDepth;
    uint32_t m_maxPerSecond;
    RateLimitQueueStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
