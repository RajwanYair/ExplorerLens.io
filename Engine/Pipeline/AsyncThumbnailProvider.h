#pragma once
//==============================================================================
// AsyncThumbnailProvider
// Non-blocking IThumbnailProvider with background decode thread pool
//
// Architecture:
// 1. Shell calls GetThumbnail() on COM STA thread
// 2. Decode request queued to background thread pool
// 3. Completion callback signals shell with result
// 4. Timeout enforcement prevents Explorer hangs
//
// Key design decisions:
// - Uses Windows Thread Pool API (TP_WORK) for efficiency
// - Request deduplication prevents redundant decodes
// - Priority queue: visible thumbnails decode first
// - Graceful degradation: falls back to sync on pool exhaustion
//==============================================================================

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
// Use canonical DecodePriority from AsyncShellActivation
#include "../Core/AsyncShellActivation.h"

namespace ExplorerLens {
namespace Engine {

/// Priority levels for decode requests are defined in AsyncShellActivation.h
/// (DecodePriority)

/// State of an async decode request
enum class RequestState : uint8_t {
 Queued, ///< Waiting in thread pool
 InProgress, ///< Currently decoding
 Completed, ///< Decode finished successfully
 Failed, ///< Decode failed
 Cancelled, ///< Request was cancelled
 TimedOut ///< Request exceeded timeout
};

/// Result of an async decode operation
struct AsyncDecodeResult {
 RequestState state = RequestState::Queued;
 uint32_t width = 0;
 uint32_t height = 0;
 std::vector<uint8_t> pixelData; ///< BGRA32 pixel data
 std::wstring errorMessage;
 double decodeTimeMs = 0.0;
 bool fromCache = false;
};

/// Callback signature for async decode completion
using AsyncDecodeCallback =
 std::function<void(uint64_t requestId, const AsyncDecodeResult &result)>;

/// Statistics for the async provider
struct AsyncProviderStats {
 uint64_t totalRequests = 0;
 uint64_t completedRequests = 0;
 uint64_t failedRequests = 0;
 uint64_t cancelledRequests = 0;
 uint64_t timedOutRequests = 0;
 uint64_t deduplicatedRequests = 0;
 uint64_t activeWorkers = 0;
 uint64_t queueDepth = 0;
 double avgDecodeTimeMs = 0.0;
 double p99DecodeTimeMs = 0.0;
 bool fallbackToSync = false;
};

/// Configuration for the async thumbnail provider (distinct from
/// AsyncShellActivation::AsyncProviderConfig)
struct ThumbnailProviderConfig {
 uint32_t minThreads = 2; ///< Minimum thread pool size
 uint32_t maxThreads = 8; ///< Maximum thread pool size
 uint32_t maxQueueDepth = 256; ///< Max pending requests before rejection
 uint32_t timeoutMs = 5000; ///< Per-request decode timeout
 uint32_t shellTimeoutMs = 10000; ///< Total shell call timeout
 bool enableDeduplication = true; ///< Merge duplicate file requests
 bool enablePriorityQueue = true; ///< Use priority ordering
 bool enableFallbackSync = true; ///< Fall back to sync on pool exhaustion
 uint32_t thumbnailSize = 256; ///< Default thumbnail dimension
};

/// Internal decode request
struct AsyncProviderDecodeItem {
 uint64_t requestId = 0;
 std::wstring filePath;
 uint32_t requestedSize = 256;
 DecodePriority priority = DecodePriority::Normal;
 RequestState state = RequestState::Queued;
 AsyncDecodeCallback callback;
 std::chrono::steady_clock::time_point submitTime;
 std::chrono::steady_clock::time_point startTime;
};

//==============================================================================
// AsyncThumbnailProvider
//==============================================================================
class AsyncThumbnailProvider {
public:
 AsyncThumbnailProvider();
 explicit AsyncThumbnailProvider(const ThumbnailProviderConfig &config);
 ~AsyncThumbnailProvider();

 // Non-copyable, non-movable (thread pool resources)
 AsyncThumbnailProvider(const AsyncThumbnailProvider &) = delete;
 AsyncThumbnailProvider &operator=(const AsyncThumbnailProvider &) = delete;

 /// Initialize the thread pool and start accepting requests
 bool Initialize();

 /// Shutdown the provider, cancelling pending requests
 void Shutdown();

 /// Submit an async decode request
 /// @return Request ID for tracking, or 0 on failure
 uint64_t SubmitRequest(const std::wstring &filePath, uint32_t requestedSize,
 DecodePriority priority, AsyncDecodeCallback callback);

 /// Cancel a pending request
 bool CancelRequest(uint64_t requestId);

 /// Cancel all pending requests for a file
 uint32_t CancelRequestsForFile(const std::wstring &filePath);

 /// Wait for a specific request to complete (with timeout)
 bool WaitForRequest(uint64_t requestId, uint32_t timeoutMs);

 /// Synchronous fallback — decode immediately on calling thread
 AsyncDecodeResult DecodeSynchronous(const std::wstring &filePath,
 uint32_t requestedSize);

 /// Get current statistics
 AsyncProviderStats GetStats() const;

 /// Check if provider is initialized and accepting requests
 bool IsRunning() const;

 /// Get the current queue depth
 uint32_t GetQueueDepth() const;

 /// Get configuration
 const ThumbnailProviderConfig &GetConfig() const { return m_config; }

 /// Get priority name string
 static const wchar_t *GetPriorityName(DecodePriority priority);

 /// Get state name string
 static const wchar_t *GetStateName(RequestState state);

private:
 /// Thread pool work callback
 void WorkerCallback(AsyncProviderDecodeItem &request);

 /// Check for timed-out requests
 void CheckTimeouts();

 /// Generate unique request ID
 uint64_t GenerateRequestId();

 /// Find duplicate request for deduplication
 uint64_t FindDuplicateRequest(const std::wstring &filePath, uint32_t size);

 ThumbnailProviderConfig m_config;
 std::atomic<bool> m_running{false};
 std::atomic<uint64_t> m_nextRequestId{1};
 mutable std::mutex m_requestMutex;
 std::unordered_map<uint64_t, AsyncProviderDecodeItem> m_requests;

 // Statistics
 std::atomic<uint64_t> m_totalRequests{0};
 std::atomic<uint64_t> m_completedRequests{0};
 std::atomic<uint64_t> m_failedRequests{0};
 std::atomic<uint64_t> m_cancelledRequests{0};
 std::atomic<uint64_t> m_timedOutRequests{0};
 std::atomic<uint64_t> m_deduplicatedRequests{0};
 std::atomic<uint64_t> m_activeWorkers{0};
 std::atomic<double> m_totalDecodeTimeMs{0.0};
};

} // namespace Engine
} // namespace ExplorerLens
