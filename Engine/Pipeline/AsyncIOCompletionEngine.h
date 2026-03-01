#pragma once
// ============================================================================
// AsyncIOCompletionEngine.h — Windows I/O completion port async file ops
//
// Purpose:   Windows I/O completion port based async file operations
// Provides:  AsyncIOPriority, AsyncIOStatus enums, and
//            AsyncIOCompletionEngine class
// Used by:   Decode pipeline for non-blocking file reads
// ============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

/// Priority level for I/O operations
enum class AsyncIOPriority : uint8_t {
    Critical = 0,  // Must complete immediately (cache flush)
    High = 1,  // High priority (active thumbnail request)
    Normal = 2,  // Standard I/O operations
    Low = 3,  // Background prefetch
    Background = 4   // Idle-time maintenance
};

inline const char* AsyncIOPriorityName(AsyncIOPriority p) noexcept {
    switch (p) {
    case AsyncIOPriority::Critical:   return "Critical";
    case AsyncIOPriority::High:       return "High";
    case AsyncIOPriority::Normal:     return "Normal";
    case AsyncIOPriority::Low:        return "Low";
    case AsyncIOPriority::Background: return "Background";
    default:                     return "Unknown";
    }
}

/// Completion status of an asynchronous I/O operation
enum class AsyncIOStatus : uint8_t {
    Pending = 0,  // Operation submitted, awaiting completion
    Completed = 1,  // Successfully completed
    Failed = 2,  // I/O error occurred
    Cancelled = 3,  // Cancelled by user or timeout
    Timeout = 4   // Operation timed out
};

inline const char* AsyncIOStatusName(AsyncIOStatus s) noexcept {
    switch (s) {
    case AsyncIOStatus::Pending:   return "Pending";
    case AsyncIOStatus::Completed: return "Completed";
    case AsyncIOStatus::Failed:    return "Failed";
    case AsyncIOStatus::Cancelled: return "Cancelled";
    case AsyncIOStatus::Timeout:   return "Timeout";
    default:                            return "Unknown";
    }
}

/// Represents a single I/O request in the completion engine
struct IORequest {
    uint64_t           id = 0;
    AsyncIOPriority         priority = AsyncIOPriority::Normal;
    AsyncIOStatus status = AsyncIOStatus::Pending;
    std::wstring       filePath;
    uint64_t           offset = 0;
    uint32_t           sizeBytes = 0;
    uint32_t           bytesTransferred = 0;
};

/// Configuration for the IOCP engine
struct IOCPConfig {
    static constexpr uint32_t IOCP_THREADS = 4;
    uint32_t maxConcurrent = IOCP_THREADS;
    uint32_t timeoutMs = 10000;
    uint32_t maxQueueDepth = 1024;
    bool     priorityBoost = true;
};

/// Manages Windows I/O Completion Ports (IOCP) for overlapped
/// file reads used by the thumbnail decode pipeline, supporting
/// prioritized submission and batch polling.
class AsyncIOCompletionEngine {
public:
    AsyncIOCompletionEngine() = default;
    ~AsyncIOCompletionEngine() = default;

    AsyncIOCompletionEngine(const AsyncIOCompletionEngine&) = delete;
    AsyncIOCompletionEngine& operator=(const AsyncIOCompletionEngine&) = delete;
    AsyncIOCompletionEngine(AsyncIOCompletionEngine&&) noexcept = default;
    AsyncIOCompletionEngine& operator=(AsyncIOCompletionEngine&&) noexcept = default;

    /// Submit an I/O request to the completion port
    uint64_t Submit(const std::wstring& path, uint64_t offset, uint32_t size,
        AsyncIOPriority priority = AsyncIOPriority::Normal) {
        if (path.empty() || size == 0) return 0;
        uint64_t id = ++m_nextId;
        IORequest req;
        req.id = id;
        req.filePath = path;
        req.offset = offset;
        req.sizeBytes = size;
        req.priority = priority;
        req.status = AsyncIOStatus::Pending;
        m_pendingCount++;
        m_totalSubmitted++;
        return id;
    }

    /// Poll for completed operations (returns number completed)
    uint32_t Poll() {
        uint32_t completed = m_pendingCount;
        m_completedCount += completed;
        m_pendingCount = 0;
        return completed;
    }

    /// Cancel a pending I/O request
    bool Cancel(uint64_t requestId) {
        if (requestId == 0 || m_pendingCount == 0) return false;
        m_pendingCount--;
        m_cancelledCount++;
        return true;
    }

    /// Get number of pending operations
    uint32_t GetPending() const noexcept { return m_pendingCount; }

    /// Get total submitted requests
    uint64_t GetTotalSubmitted() const noexcept { return m_totalSubmitted; }

    /// Get completed request count
    uint64_t GetCompletedCount() const noexcept { return m_completedCount; }

    /// Get cancelled request count
    uint64_t GetCancelledCount() const noexcept { return m_cancelledCount; }

    /// Apply configuration
    void SetConfig(const IOCPConfig& cfg) noexcept { m_config = cfg; }

private:
    IOCPConfig m_config;
    uint64_t   m_nextId = 0;
    uint32_t   m_pendingCount = 0;
    uint64_t   m_totalSubmitted = 0;
    uint64_t   m_completedCount = 0;
    uint64_t   m_cancelledCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
