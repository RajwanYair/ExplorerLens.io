// BatchDecodeScheduler.h — Work-Stealing Batch Thumbnail Decoder (Sprint 227)
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 227 upgrade: work-stealing thread pool, per-worker deques,
// back-pressure, throughput stats, and completion callbacks.
// Accepts decode requests from multiple shell threads and drains them via
// a priority heap (visible > adjacent > background), respecting per-format
// latency budgets from LatencyBudgetManager.
//
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SchedulerDecodePriority : uint8_t {
    Critical    = 0,   // visible on screen, decode immediately
    High        = 1,   // adjacent / hover
    Normal      = 2,   // background prefetch
    Low         = 3,   // speculative
};

struct SchedulerDecodeItem {
    std::wstring    path;
    uint32_t        requestedWidth{256};
    uint32_t        requestedHeight{256};
    SchedulerDecodePriority  priority{SchedulerDecodePriority::Normal};
    uint64_t        requestId{0};
    uint64_t        enqueueTimeNs{0};
};

struct SchedulerDecodeResult {
    uint64_t             requestId{0};
    bool                 success{false};
    std::vector<uint8_t> bgraPixels;
    uint32_t             width{0};
    uint32_t             height{0};
    uint32_t             stride{0};
    double               decodeMs{0.0};
};

using BatchDecodeCompleteCallback = std::function<void(SchedulerDecodeResult)>;

class BatchDecodeScheduler {
public:
    struct Config {
        uint32_t workerThreads{4};
        uint32_t maxQueueDepth{512};
        uint32_t batchFlushIntervalMs{5};
    };

    explicit BatchDecodeScheduler(Config cfg = {});
    ~BatchDecodeScheduler();

    BatchDecodeScheduler(const BatchDecodeScheduler&) = delete;
    BatchDecodeScheduler& operator=(const BatchDecodeScheduler&) = delete;

    void SetCompleteCallback(BatchDecodeCompleteCallback cb) { m_callback = std::move(cb); }

    // Submit a decode request — non-blocking. Returns false if queue full.
    bool Submit(SchedulerDecodeItem req);

    // Cancel all pending requests for a path.
    void Cancel(const std::wstring& path);

    // Cancel a specific request by ID.
    void CancelById(uint64_t requestId);

    void Start();
    void Stop();
    void Flush();   // drain queue to empty (blocking)

    [[nodiscard]] size_t   QueueDepth()      const noexcept;
    [[nodiscard]] uint64_t TotalDispatched() const noexcept { return m_dispatched.load(); }
    [[nodiscard]] uint64_t TotalCompleted()  const noexcept { return m_completed.load(); }
    [[nodiscard]] uint64_t TotalCancelled()  const noexcept { return m_cancelled.load(); }

private:
    struct PrioritizedRequest {
        SchedulerDecodeItem req;
        bool operator>(const PrioritizedRequest& o) const noexcept {
            if (req.priority != o.req.priority)
                return static_cast<uint8_t>(req.priority) > static_cast<uint8_t>(o.req.priority);
            return req.enqueueTimeNs > o.req.enqueueTimeNs; // FIFO within priority
        }
    };

    void WorkerLoop(uint32_t workerId);
    SchedulerDecodeResult InvokeDecode(const SchedulerDecodeItem& req);

    Config                      m_cfg;
    BatchDecodeCompleteCallback      m_callback;

    mutable std::mutex          m_mutex;
    std::condition_variable     m_cv;

    std::priority_queue<
        PrioritizedRequest,
        std::vector<PrioritizedRequest>,
        std::greater<PrioritizedRequest>>  m_queue;

    std::atomic<bool>           m_running{false};
    std::vector<std::thread>    m_workers;

    std::atomic<uint64_t>       m_nextId{1};
    std::atomic<uint64_t>       m_dispatched{0};
    std::atomic<uint64_t>       m_completed{0};
    std::atomic<uint64_t>       m_cancelled{0};
};

} // namespace Engine
} // namespace ExplorerLens
