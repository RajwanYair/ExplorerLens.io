// RequestDeduplicator.h — Concurrent Thumbnail Request Coalescing
// Copyright (c) 2026 ExplorerLens Project
//
// Coalesces concurrent thumbnail requests for the same file path into
// a single decode operation.  All waiters share the result through a
// shared future, eliminating redundant decodes when Explorer rapidly
// requests the same thumbnail.
//
#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Result of a deduplicated decode operation
struct DeduplicatedResult {
    bool        success = false;
    uint32_t    width = 0;
    uint32_t    height = 0;
    std::vector<uint8_t> pixels;
    std::string error;
    bool        wasCoalesced = false;  // True if this was a joined request
};

/// Deduplication statistics
struct DeduplicationStats {
    uint64_t totalRequests = 0;
    uint64_t uniqueDecodes = 0;
    uint64_t coalescedRequests = 0;
    uint64_t activeInflight = 0;
    double   avgWaitersPerKey = 0.0;
    double   deduplicationRate = 0.0;  // coalescedRequests / totalRequests
};

/// Internal: represents one in-flight decode with its waiters
struct InflightEntry {
    std::shared_ptr<DeduplicatedResult> result;
    std::shared_ptr<std::condition_variable> cv;
    std::shared_ptr<std::mutex> mtx;
    bool completed = false;
    uint32_t waiterCount = 1;  // First requester counts as 1
};

/// Thread-safe request deduplicator for thumbnail generation
class RequestDeduplicator {
public:
    static RequestDeduplicator& Instance() {
        static RequestDeduplicator inst;
        return inst;
    }

    /// Submit a decode request; returns existing result if already in-flight.
    /// @param filePath  The file to decode
    /// @param decoder   Callback invoked only for the first request
    /// @return Shared result (blocking for coalesced requests)
    DeduplicatedResult Submit(const std::wstring& filePath,
        std::function<DeduplicatedResult()> decoder) {
        std::unique_lock<std::mutex> globalLock(m_mutex);
        m_stats.totalRequests++;

        auto it = m_inflight.find(filePath);
        if (it != m_inflight.end() && !it->second.completed) {
            // Another decode is in-flight — join it
            it->second.waiterCount++;
            m_stats.coalescedRequests++;

            auto cv = it->second.cv;
            auto mtx = it->second.mtx;
            auto resultPtr = it->second.result;
            globalLock.unlock();

            // Wait for the primary decode to finish
            std::unique_lock<std::mutex> waitLock(*mtx);
            cv->wait(waitLock, [&resultPtr]() { return resultPtr->success || !resultPtr->error.empty(); });

            DeduplicatedResult joined = *resultPtr;
            joined.wasCoalesced = true;
            return joined;
        }

        // First request for this path — create inflight entry
        InflightEntry entry;
        entry.result = std::make_shared<DeduplicatedResult>();
        entry.cv = std::make_shared<std::condition_variable>();
        entry.mtx = std::make_shared<std::mutex>();
        m_inflight[filePath] = entry;
        m_stats.uniqueDecodes++;
        m_stats.activeInflight++;
        globalLock.unlock();

        // Execute the actual decode (outside the global lock)
        DeduplicatedResult decoded = decoder();

        // Publish result and wake waiters
        {
            std::lock_guard<std::mutex> entryLock(*entry.mtx);
            *entry.result = decoded;
        }
        entry.cv->notify_all();

        // Clean up inflight map
        {
            std::lock_guard<std::mutex> cleanLock(m_mutex);
            m_inflight.erase(filePath);
            if (m_stats.activeInflight > 0) m_stats.activeInflight--;
        }

        decoded.wasCoalesced = false;
        return decoded;
    }

    /// Get deduplication statistics
    DeduplicationStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto stats = m_stats;
        if (stats.totalRequests > 0) {
            stats.deduplicationRate = static_cast<double>(stats.coalescedRequests) /
                static_cast<double>(stats.totalRequests);
        }
        if (stats.uniqueDecodes > 0) {
            stats.avgWaitersPerKey = static_cast<double>(stats.totalRequests) /
                static_cast<double>(stats.uniqueDecodes);
        }
        return stats;
    }

    /// Get number of currently in-flight decodes
    size_t InflightCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_inflight.size();
    }

private:
    RequestDeduplicator() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, InflightEntry> m_inflight;
    mutable DeduplicationStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
