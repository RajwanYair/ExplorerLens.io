//==============================================================================
// AsyncShellExtension
// Non-blocking IThumbnailProvider implementation
//==============================================================================

#include "AsyncShellExtension.h"
#include <algorithm>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

AsyncShellExtension::AsyncShellExtension() : m_threadCount(4) {}

AsyncShellExtension::AsyncShellExtension(uint32_t threadCount)
    : m_threadCount(threadCount > 0 ? threadCount : 4) {}

AsyncShellExtension::~AsyncShellExtension() { Stop(); }

//------------------------------------------------------------------------------
uint64_t
AsyncShellExtension::SubmitRequest(const AsyncThumbnailRequest &request) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto req = request;
  req.requestId = m_nextRequestId++;
  req.state = AsyncDecodeState::Queued;
  req.queuedAt = std::chrono::steady_clock::now();
  m_queue.push_back(req);
  m_stats.queueDepth = static_cast<uint32_t>(m_queue.size());
  return req.requestId;
}

bool AsyncShellExtension::CancelRequest(uint64_t requestId) {
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto &req : m_queue) {
    if (req.requestId == requestId && req.state == AsyncDecodeState::Queued) {
      req.state = AsyncDecodeState::Cancelled;
      m_stats.cancelledRequests++;
      return true;
    }
  }
  return false;
}

AsyncDecodeState
AsyncShellExtension::GetRequestState(uint64_t requestId) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  for (const auto &req : m_queue) {
    if (req.requestId == requestId)
      return req.state;
  }
  return AsyncDecodeState::Failed;
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
void AsyncShellExtension::BoostPriority(uint64_t requestId,
                                        DecodePriority newPriority) {
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto &req : m_queue) {
    if (req.requestId == requestId && req.state == AsyncDecodeState::Queued) {
      req.priority = newPriority;
      break;
    }
  }
}

uint32_t AsyncShellExtension::GetQueueDepth() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  uint32_t count = 0;
  for (const auto &req : m_queue) {
    if (req.state == AsyncDecodeState::Queued)
      count++;
  }
  return count;
}

uint32_t
AsyncShellExtension::GetQueueDepthForPriority(DecodePriority priority) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  uint32_t count = 0;
  for (const auto &req : m_queue) {
    if (req.state == AsyncDecodeState::Queued && req.priority == priority)
      count++;
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
  for (auto &req : m_queue) {
    if (req.state == AsyncDecodeState::Queued) {
      req.state = AsyncDecodeState::Completed;
      m_stats.completedRequests++;
    }
  }
  m_stats.queueDepth = 0;
}

//------------------------------------------------------------------------------
const wchar_t *AsyncShellExtension::GetPriorityName(DecodePriority priority) {
  switch (priority) {
  case DecodePriority::Critical:
    return L"Critical";
  case DecodePriority::High:
    return L"High";
  case DecodePriority::Normal:
    return L"Normal";
  case DecodePriority::Low:
    return L"Low";
  case DecodePriority::Idle:
    return L"Idle";
  default:
    return L"Unknown";
  }
}

const wchar_t *AsyncShellExtension::GetStateName(AsyncDecodeState state) {
  switch (state) {
  case AsyncDecodeState::Queued:
    return L"Queued";
  case AsyncDecodeState::Dispatched:
    return L"Dispatched";
  case AsyncDecodeState::Decoding:
    return L"Decoding";
  case AsyncDecodeState::GPUProcessing:
    return L"GPU Processing";
  case AsyncDecodeState::Completed:
    return L"Completed";
  case AsyncDecodeState::Failed:
    return L"Failed";
  case AsyncDecodeState::TimedOut:
    return L"Timed Out";
  case AsyncDecodeState::Cancelled:
    return L"Cancelled";
  default:
    return L"Unknown";
  }
}

} // namespace Engine
} // namespace ExplorerLens
