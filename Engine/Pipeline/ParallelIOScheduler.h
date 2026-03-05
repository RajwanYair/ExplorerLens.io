// ParallelIOScheduler.h — Parallel File I/O for Batch Decode
// Copyright (c) 2026 ExplorerLens Project
//
// Schedules and manages parallel file I/O requests for batch thumbnail
// generation. Supports priority queuing and concurrency control.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

enum class IOPriority : uint8_t {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

enum class IORequestState : uint8_t {
    Pending = 0,
    InFlight = 1,
    Completed = 2,
    Failed = 3,
    Cancelled = 4
};

struct ParallelIORequest {
    uint64_t            id = 0;
    std::wstring        filePath;
    IOPriority          priority = IOPriority::Normal;
    IORequestState      state = IORequestState::Pending;
    uint64_t            fileSize = 0;
    uint64_t            bytesRead = 0;
    uint64_t            submitTimeMs = 0;
    uint64_t            completeTimeMs = 0;
    std::vector<uint8_t> data;
};

struct IOSchedulerStats {
    uint64_t totalSubmitted = 0;
    uint64_t totalCompleted = 0;
    uint64_t totalFailed = 0;
    uint64_t totalCancelled = 0;
    uint64_t totalBytesRead = 0;
    uint32_t pendingCount = 0;
    uint32_t inFlightCount = 0;
    double   avgLatencyMs = 0.0;
};

class ParallelIOScheduler {
public:
    static ParallelIOScheduler& Instance() { static ParallelIOScheduler s; return s; }

    uint64_t Submit(const std::wstring& filePath, IOPriority priority = IOPriority::Normal) {
        ParallelIORequest req{};
        req.id = m_nextId++;
        req.filePath = filePath;
        req.priority = priority;
        req.state = IORequestState::Pending;
        req.submitTimeMs = GetCurrentTimeMs();
        m_requests.push_back(req);
        m_stats.totalSubmitted++;
        m_stats.pendingCount++;
        return req.id;
    }

    bool Cancel(uint64_t id) {
        for (auto& req : m_requests) {
            if (req.id == id && req.state == IORequestState::Pending) {
                req.state = IORequestState::Cancelled;
                m_stats.totalCancelled++;
                m_stats.pendingCount--;
                return true;
            }
        }
        return false;
    }

    void SetConcurrency(uint32_t maxConcurrent) {
        m_maxConcurrency = (std::max)(uint32_t(1), (std::min)(maxConcurrent, uint32_t(64)));
    }

    uint32_t GetConcurrency() const { return m_maxConcurrency; }

    uint32_t GetPendingCount() const { return m_stats.pendingCount; }
    uint32_t GetInFlightCount() const { return m_stats.inFlightCount; }

    bool ProcessNext() {
        if (m_stats.inFlightCount >= m_maxConcurrency) return false;

        // Find highest priority pending request
        ParallelIORequest* best = nullptr;
        for (auto& req : m_requests) {
            if (req.state == IORequestState::Pending) {
                if (!best || static_cast<uint8_t>(req.priority) > static_cast<uint8_t>(best->priority)) {
                    best = &req;
                }
            }
        }
        if (!best) return false;

        best->state = IORequestState::InFlight;
        m_stats.pendingCount--;
        m_stats.inFlightCount++;
        return true;
    }

    bool CompleteRequest(uint64_t id, bool success, uint64_t bytesRead = 0) {
        for (auto& req : m_requests) {
            if (req.id == id && req.state == IORequestState::InFlight) {
                req.state = success ? IORequestState::Completed : IORequestState::Failed;
                req.bytesRead = bytesRead;
                req.completeTimeMs = GetCurrentTimeMs();
                m_stats.inFlightCount--;
                if (success) {
                    m_stats.totalCompleted++;
                    m_stats.totalBytesRead += bytesRead;
                }
                else {
                    m_stats.totalFailed++;
                }
                if (m_stats.totalCompleted > 0) {
                    double elapsed = static_cast<double>(req.completeTimeMs - req.submitTimeMs);
                    m_stats.avgLatencyMs = (m_stats.avgLatencyMs * (m_stats.totalCompleted - 1) + elapsed)
                        / m_stats.totalCompleted;
                }
                return true;
            }
        }
        return false;
    }

    const IOSchedulerStats& GetStats() const { return m_stats; }
    const ParallelIORequest* GetRequest(uint64_t id) const {
        for (const auto& req : m_requests) {
            if (req.id == id) return &req;
        }
        return nullptr;
    }

    void Reset() {
        m_requests.clear();
        m_stats = IOSchedulerStats{};
        m_nextId = 1;
    }

    bool Validate() const {
        if (m_maxConcurrency == 0 || m_maxConcurrency > 64) return false;
        uint32_t pending = 0, inflight = 0;
        for (const auto& req : m_requests) {
            if (req.state == IORequestState::Pending)  ++pending;
            if (req.state == IORequestState::InFlight)  ++inflight;
        }
        if (pending != m_stats.pendingCount) return false;
        if (inflight != m_stats.inFlightCount) return false;
        return true;
    }

private:
    ParallelIOScheduler() = default;
    ~ParallelIOScheduler() = default;
    ParallelIOScheduler(const ParallelIOScheduler&) = delete;
    ParallelIOScheduler& operator=(const ParallelIOScheduler&) = delete;

    static uint64_t GetCurrentTimeMs() {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    std::vector<ParallelIORequest>  m_requests;
    IOSchedulerStats        m_stats{};
    uint32_t                m_maxConcurrency = 4;
    uint64_t                m_nextId = 1;
};

} // namespace Engine
} // namespace ExplorerLens
