// DecodeCancellationEngine.h — Cooperative Decode Cancellation
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a cooperative cancellation token system for in-flight decode
// operations.  When a thumbnail scrolls out of the visible region, the
// shell extension cancels the token, and the decoder checks it at safe
// yield points to abort early without resource leaks.
//
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Reason a decode was cancelled
enum class CancelReason : uint8_t {
    None = 0,
    ScrolledAway,  // Thumbnail left visible region
    Timeout,       // Deadline exceeded
    LowPriority,   // Preempted by higher-priority request
    Shutdown,      // Engine shutting down
    UserRequested  // Explicit user cancel
};

/// Thread-safe cancellation token checked by decoders at yield points
class CancellationToken
{
  public:
    CancellationToken() = default;

    /// Request cancellation with a reason
    void Cancel(CancelReason reason = CancelReason::UserRequested)
    {
        m_reason.store(reason, std::memory_order_release);
        m_cancelled.store(true, std::memory_order_release);
    }

    /// Check if cancellation was requested (hot path — no lock)
    bool IsCancelled() const
    {
        return m_cancelled.load(std::memory_order_acquire);
    }

    /// Get the cancellation reason
    CancelReason Reason() const
    {
        return m_reason.load(std::memory_order_acquire);
    }

    /// Reset token for reuse (only safe when no decode is in flight)
    void Reset()
    {
        m_cancelled.store(false, std::memory_order_release);
        m_reason.store(CancelReason::None, std::memory_order_release);
    }

    /// Throw-if-cancelled helper for decoders (check at yield points)
    bool ThrowIfCancelled() const
    {
        if (IsCancelled())
            return true;  // Caller should return early
        return false;
    }

  private:
    std::atomic<bool> m_cancelled{false};
    std::atomic<CancelReason> m_reason{CancelReason::None};
};

using CancellationTokenPtr = std::shared_ptr<CancellationToken>;

/// Statistics for cancellation tracking
struct CancellationStats
{
    uint64_t tokensIssued = 0;
    uint64_t tokensCancelled = 0;
    uint64_t tokensCompleted = 0;
    uint64_t bytesReclaimed = 0;  // Memory freed by early cancellation
    double avgCancelLatencyMs = 0.0;
};

/// Manages cancellation tokens for all in-flight decode operations
class DecodeCancellationEngine
{
  public:
    static DecodeCancellationEngine& Instance()
    {
        static DecodeCancellationEngine instance;
        return instance;
    }

    /// Issue a new cancellation token for a decode request
    CancellationTokenPtr CreateToken(const std::wstring& filePath)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto token = std::make_shared<CancellationToken>();
        m_activeTokens[filePath] = token;
        m_stats.tokensIssued++;
        return token;
    }

    /// Cancel a specific file's decode
    bool CancelFile(const std::wstring& filePath, CancelReason reason)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_activeTokens.find(filePath);
        if (it == m_activeTokens.end())
            return false;
        auto token = it->second.lock();
        if (!token) {
            m_activeTokens.erase(it);
            return false;
        }
        token->Cancel(reason);
        m_stats.tokensCancelled++;
        return true;
    }

    /// Cancel all in-flight decodes (e.g., on shutdown)
    void CancelAll(CancelReason reason = CancelReason::Shutdown)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [path, weak] : m_activeTokens) {
            if (auto token = weak.lock()) {
                token->Cancel(reason);
                m_stats.tokensCancelled++;
            }
        }
        m_activeTokens.clear();
    }

    /// Mark a file's decode as completed (removes token)
    void Complete(const std::wstring& filePath)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeTokens.erase(filePath);
        m_stats.tokensCompleted++;
    }

    /// Purge expired weak pointers
    void PurgeExpired()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_activeTokens.begin(); it != m_activeTokens.end();) {
            if (it->second.expired())
                it = m_activeTokens.erase(it);
            else
                ++it;
        }
    }

    /// Get active token count
    size_t ActiveCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeTokens.size();
    }

    CancellationStats GetStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

  private:
    DecodeCancellationEngine() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, std::weak_ptr<CancellationToken>> m_activeTokens;
    CancellationStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
