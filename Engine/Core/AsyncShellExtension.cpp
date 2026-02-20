//==============================================================================
// AsyncShellExtension — Sprint 205
// Non-blocking IThumbnailProvider implementation
//==============================================================================

#include "AsyncShellExtension.h"
#include <algorithm>
#include <chrono>

namespace DarkThumbs { namespace Engine {

AsyncShellExtension::AsyncShellExtension() : m_threadCount(4) {}

AsyncShellExtension::AsyncShellExtension(uint32_t threadCount)
    : m_threadCount(threadCount > 0 ? threadCount : 4) {}

AsyncShellExtension::~AsyncShellExtension() {
    Stop();
}

//------------------------------------------------------------------------------
uint64_t AsyncShellExtension::SubmitRequest(const AsyncThumbnailRequest& request) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto req = request;
    req.requestId = m_nextRequestId++;
    req.state = RequestState::Queued;
    auto now = std::chrono::steady_clock::now();
    req.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    m_queue.push_back(req);
    m_stats.queueDepth = static_cast<uint32_t>(m_queue.size());
    return req.requestId;
}

bool AsyncShellExtension::CancelRequest(uint64_t requestId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& req : m_queue) {
        if (req.requestId == requestId && req.state == RequestState::Queued) {
            req.state = RequestState::Cancelled;
            m_stats.cancelledRequests++;
            return true;
        }
    }
    return false;
}

RequestState AsyncShellExtension::GetRequestState(uint64_t requestId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& req : m_queue) {
        if (req.requestId == requestId) return req.state;
    }
    return RequestState::Failed;
}

//------------------------------------------------------------------------------
void AsyncShellExtension::SetThreadCount(uint32_t count) {
    m_threadCount = count > 0 ? count : 4;
    m_stats.totalThreads = m_threadCount;
}

ThreadPoolStats AsyncShellExtension::GetStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto stats = m_stats;
    stats.totalThreads = m_threadCount;
    return stats;
}

//------------------------------------------------------------------------------
void AsyncShellExtension::BoostPriority(uint64_t requestId, ThumbnailPriority newPriority) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& req : m_queue) {
        if (req.requestId == requestId && req.state == RequestState::Queued) {
            req.priority = newPriority;
            break;
        }
    }
}

uint32_t AsyncShellExtension::GetQueueDepth() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t count = 0;
    for (const auto& req : m_queue) {
        if (req.state == RequestState::Queued) count++;
    }
    return count;
}

uint32_t AsyncShellExtension::GetQueueDepthForPriority(ThumbnailPriority priority) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t count = 0;
    for (const auto& req : m_queue) {
        if (req.state == RequestState::Queued && req.priority == priority) count++;
    }
    return count;
}

//------------------------------------------------------------------------------
void AsyncShellExtension::Start() {
    m_running = true;
    m_stats.totalThreads = m_threadCount;
    m_stats.activeThreads = 0;
}

void AsyncShellExtension::Stop() {
    m_running = false;
    m_stats.activeThreads = 0;
}

void AsyncShellExtension::DrainQueue() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& req : m_queue) {
        if (req.state == RequestState::Queued) {
            req.state = RequestState::Completed;
            m_stats.completedRequests++;
        }
    }
    m_stats.queueDepth = 0;
}

//------------------------------------------------------------------------------
const wchar_t* AsyncShellExtension::GetPriorityName(ThumbnailPriority priority) {
    switch (priority) {
        case ThumbnailPriority::Critical: return L"Critical";
        case ThumbnailPriority::High:     return L"High";
        case ThumbnailPriority::Normal:   return L"Normal";
        case ThumbnailPriority::Low:      return L"Low";
        case ThumbnailPriority::Idle:     return L"Idle";
        default: return L"Unknown";
    }
}

const wchar_t* AsyncShellExtension::GetStateName(RequestState state) {
    switch (state) {
        case RequestState::Queued:     return L"Queued";
        case RequestState::InProgress: return L"InProgress";
        case RequestState::Completed:  return L"Completed";
        case RequestState::Failed:     return L"Failed";
        case RequestState::Cancelled:  return L"Cancelled";
        default: return L"Unknown";
    }
}

}} // namespace DarkThumbs::Engine
