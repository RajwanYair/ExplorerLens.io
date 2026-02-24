#pragma once
//==============================================================================
// AsyncShellExtension
// Non-blocking IThumbnailProvider with thread pool and priority queue
//==============================================================================

#include "AsyncShellActivation.h"
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Thread pool statistics
struct ThreadPoolStats {
  uint32_t activeThreads = 0;
  uint32_t totalThreads = 0;
  uint32_t queueDepth = 0;
  uint64_t completedRequests = 0;
  uint64_t failedRequests = 0;
  uint64_t cancelledRequests = 0;
  double avgLatencyMs = 0.0;
  double throughputPerSec = 0.0;
};

//------------------------------------------------------------------------------
class AsyncShellExtension {
public:
  AsyncShellExtension();
  explicit AsyncShellExtension(uint32_t threadCount);
  ~AsyncShellExtension();

  // Queue management
  uint64_t SubmitRequest(const AsyncThumbnailRequest &request);
  bool CancelRequest(uint64_t requestId);
  AsyncDecodeState GetRequestState(uint64_t requestId) const;

  // Thread pool
  void SetThreadCount(uint32_t count);
  uint32_t GetThreadCount() const { return m_threadCount; }
  ThreadPoolStats GetStats() const;

  // Priority management
  void BoostPriority(uint64_t requestId, DecodePriority newPriority);
  uint32_t GetQueueDepth() const;
  uint32_t GetQueueDepthForPriority(DecodePriority priority) const;

  // Lifecycle
  void Start();
  void Stop();
  void DrainQueue();
  bool IsRunning() const { return m_running; }

  // Static helpers
  static const wchar_t *GetPriorityName(DecodePriority priority);
  static const wchar_t *GetStateName(AsyncDecodeState state);

private:
  uint32_t m_threadCount = 4;
  bool m_running = false;
  uint64_t m_nextRequestId = 1;
  mutable std::mutex m_mutex;
  std::vector<AsyncThumbnailRequest> m_queue;
  ThreadPoolStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
