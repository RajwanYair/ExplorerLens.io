#pragma once
//==============================================================================
// AsyncShellExtension — Sprint 205
// Non-blocking IThumbnailProvider with thread pool and priority queue
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>

namespace DarkThumbs { namespace Engine {

/// Priority levels for thumbnail requests
enum class ThumbnailPriority : uint8_t {
    Critical = 0,   // Visible items in current view
    High     = 1,   // Items about to scroll into view
    Normal   = 2,   // Background prefetch
    Low      = 3,   // Speculative decode
    Idle     = 4    // Cache warming
};

/// Request state tracking
enum class RequestState : uint8_t {
    Queued,
    InProgress,
    Completed,
    Failed,
    Cancelled
};

/// Async thumbnail request
struct AsyncThumbnailRequest {
    uint64_t         requestId = 0;
    std::wstring     filePath;
    uint32_t         width = 256;
    uint32_t         height = 256;
    ThumbnailPriority priority = ThumbnailPriority::Normal;
    RequestState     state = RequestState::Queued;
    uint64_t         timestampMs = 0;
    bool             useGPU = true;
};

/// Thread pool statistics
struct ThreadPoolStats {
    uint32_t activeThreads = 0;
    uint32_t totalThreads = 0;
    uint32_t queueDepth = 0;
    uint64_t completedRequests = 0;
    uint64_t failedRequests = 0;
    uint64_t cancelledRequests = 0;
    double   avgLatencyMs = 0.0;
    double   throughputPerSec = 0.0;
};

//------------------------------------------------------------------------------
class AsyncShellExtension {
public:
    AsyncShellExtension();
    explicit AsyncShellExtension(uint32_t threadCount);
    ~AsyncShellExtension();

    // Queue management
    uint64_t SubmitRequest(const AsyncThumbnailRequest& request);
    bool     CancelRequest(uint64_t requestId);
    RequestState GetRequestState(uint64_t requestId) const;

    // Thread pool
    void SetThreadCount(uint32_t count);
    uint32_t GetThreadCount() const { return m_threadCount; }
    ThreadPoolStats GetStats() const;

    // Priority management
    void BoostPriority(uint64_t requestId, ThumbnailPriority newPriority);
    uint32_t GetQueueDepth() const;
    uint32_t GetQueueDepthForPriority(ThumbnailPriority priority) const;

    // Lifecycle
    void Start();
    void Stop();
    void DrainQueue();
    bool IsRunning() const { return m_running; }

    // Static helpers
    static const wchar_t* GetPriorityName(ThumbnailPriority priority);
    static const wchar_t* GetStateName(RequestState state);

private:
    uint32_t m_threadCount = 4;
    bool m_running = false;
    uint64_t m_nextRequestId = 1;
    mutable std::mutex m_mutex;
    std::vector<AsyncThumbnailRequest> m_queue;
    ThreadPoolStats m_stats;
};

}} // namespace DarkThumbs::Engine
