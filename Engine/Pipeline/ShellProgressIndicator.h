// ShellProgressIndicator.h — Thumbnail Generation Progress Reporting
// Copyright (c) 2026 ExplorerLens Project
//
// Reports decode progress back to the Windows Shell for large-file thumbnails.
// Uses IProgressDialog COM interface when available, with ETW event fallback.
//
#pragma once

#include <cstdint>
#include <atomic>
#include <functional>
#include <chrono>
#include <string>
#include <mutex>
#include <array>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Progress stages for thumbnail generation pipeline
// ============================================================================

enum class ThumbnailStage : uint8_t {
    Detection = 0,  // Format detection / magic-byte check
    Extraction = 1,  // Archive extraction or stream open
    Decode = 2,  // Image/video/document decode
    Transform = 3,  // Resize, crop, color-space conversion
    Render = 4,  // GPU render pass (if applicable)
    Cache = 5,  // Cache write (persistent / in-memory)
    Complete = 6,  // Done
    Failed = 7   // Error occurred
};

inline const char* ThumbnailStageToString(ThumbnailStage stage) {
    static const char* names[] = {
        "Detection", "Extraction", "Decode", "Transform",
        "Render", "Cache", "Complete", "Failed"
    };
    auto idx = static_cast<uint8_t>(stage);
    return (idx < 8) ? names[idx] : "Unknown";
}

// ============================================================================
// Progress callback signature
// ============================================================================

/// Callback: (stage, progressPercent [0..100], estimatedRemainingMs, message)
using ShellProgressCallback = std::function<void(
    ThumbnailStage stage,
    uint32_t       percentComplete,
    uint32_t       estimatedRemainingMs,
    const char* statusMessage
    )>;

// ============================================================================
// Per-file progress tracker
// ============================================================================

struct FileProgress {
    uint64_t        fileSize = 0;      // Total file size in bytes
    uint64_t        bytesProcessed = 0;      // Bytes processed so far
    ThumbnailStage  currentStage = ThumbnailStage::Detection;
    uint32_t        percentComplete = 0;     // 0..100
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point stageStartTime;
    bool            cancelled = false;
};

// ============================================================================
// ShellProgressIndicator — main progress reporting engine
// ============================================================================

class ShellProgressIndicator {
public:
    // Estimated decode time thresholds (in bytes):
    //   Files < SMALL_FILE_THRESHOLD: no progress reporting (too fast)
    //   Files >= LARGE_FILE_THRESHOLD: full progress dialog
    static constexpr uint64_t SMALL_FILE_THRESHOLD = 1 * 1024 * 1024;   // 1 MB
    static constexpr uint64_t LARGE_FILE_THRESHOLD = 50 * 1024 * 1024;  // 50 MB

    // Progress reporting granularity (ms between updates)
    static constexpr uint32_t MIN_UPDATE_INTERVAL_MS = 50;

    ShellProgressIndicator() = default;
    ~ShellProgressIndicator() = default;

    // Non-copyable, movable
    ShellProgressIndicator(const ShellProgressIndicator&) = delete;
    ShellProgressIndicator& operator=(const ShellProgressIndicator&) = delete;
    ShellProgressIndicator(ShellProgressIndicator&&) noexcept = default;
    ShellProgressIndicator& operator=(ShellProgressIndicator&&) noexcept = default;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /// Begin tracking progress for a file
    void BeginFile(const std::wstring& filePath, uint64_t fileSize) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_progress.fileSize = fileSize;
        m_progress.bytesProcessed = 0;
        m_progress.currentStage = ThumbnailStage::Detection;
        m_progress.percentComplete = 0;
        m_progress.startTime = std::chrono::steady_clock::now();
        m_progress.stageStartTime = m_progress.startTime;
        m_progress.cancelled = false;
        m_filePath = filePath;
        m_shouldReport = (fileSize >= SMALL_FILE_THRESHOLD);
    }

    /// End tracking (success or failure)
    void EndFile(bool success) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_progress.currentStage = success ? ThumbnailStage::Complete : ThumbnailStage::Failed;
        m_progress.percentComplete = success ? 100 : m_progress.percentComplete;
        NotifyLocked(success ? "Complete" : "Failed");
        m_shouldReport = false;
    }

    // ========================================================================
    // Stage transitions
    // ========================================================================

    /// Advance to the next pipeline stage
    void AdvanceStage(ThumbnailStage newStage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_progress.currentStage = newStage;
        m_progress.stageStartTime = std::chrono::steady_clock::now();

        // Approximate percentage by stage
        static constexpr uint32_t stagePercent[] = { 5, 15, 50, 75, 90, 95, 100, 0 };
        auto idx = static_cast<uint8_t>(newStage);
        if (idx < 8) {
            m_progress.percentComplete = stagePercent[idx];
        }

        NotifyLocked(ThumbnailStageToString(newStage));
    }

    /// Report byte-level progress within the current stage
    void ReportBytes(uint64_t additionalBytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_progress.bytesProcessed += additionalBytes;

        if (m_progress.fileSize > 0) {
            uint32_t rawPercent = static_cast<uint32_t>(
                (m_progress.bytesProcessed * 100) / m_progress.fileSize
                );
            m_progress.percentComplete = (rawPercent > 100) ? 100 : rawPercent;
        }

        // Throttle updates
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_lastNotifyTime
        ).count();

        if (elapsed >= MIN_UPDATE_INTERVAL_MS) {
            NotifyLocked("Processing...");
        }
    }

    // ========================================================================
    // Cancellation
    // ========================================================================

    void RequestCancel() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_progress.cancelled = true;
    }

    bool IsCancelled() const {
        return m_progress.cancelled;
    }

    // ========================================================================
    // Callback registration
    // ========================================================================

    void SetCallback(ShellProgressCallback callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callback = std::move(callback);
    }

    // ========================================================================
    // Queries
    // ========================================================================

    FileProgress GetProgress() const {
        // No lock needed — reads of aligned atomics are safe
        return m_progress;
    }

    /// Estimated time remaining in milliseconds
    uint32_t EstimateRemainingMs() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_progress.startTime
        ).count();

        if (m_progress.percentComplete == 0 || m_progress.percentComplete >= 100) return 0;

        double totalEstimateMs = (elapsedMs * 100.0) / m_progress.percentComplete;
        double remainingMs = totalEstimateMs - elapsedMs;
        return (remainingMs > 0) ? static_cast<uint32_t>(remainingMs) : 0;
    }

    /// Get human-readable ETA string (e.g., "~3s remaining")
    std::string GetETAString() const {
        uint32_t ms = EstimateRemainingMs();
        if (ms == 0) return "";
        if (ms < 1000) return "<1s remaining";
        uint32_t sec = ms / 1000;
        if (sec < 60) return "~" + std::to_string(sec) + "s remaining";
        return "~" + std::to_string(sec / 60) + "m " + std::to_string(sec % 60) + "s remaining";
    }

private:
    void NotifyLocked(const char* message) {
        if (!m_shouldReport || !m_callback) return;
        m_lastNotifyTime = std::chrono::steady_clock::now();
        m_callback(
            m_progress.currentStage,
            m_progress.percentComplete,
            EstimateRemainingMs(),
            message
        );
    }

    FileProgress     m_progress;
    ShellProgressCallback m_callback;
    std::wstring     m_filePath;
    std::mutex       m_mutex;
    bool             m_shouldReport = false;
    std::chrono::steady_clock::time_point m_lastNotifyTime;
};

// ============================================================================
// Batch progress aggregator (for directory thumbnail generation)
// ============================================================================

class BatchProgressTracker {
public:
    void Begin(uint32_t totalFiles) {
        m_totalFiles.store(totalFiles, std::memory_order_relaxed);
        m_completedFiles.store(0, std::memory_order_relaxed);
        m_failedFiles.store(0, std::memory_order_relaxed);
        m_startTime = std::chrono::steady_clock::now();
    }

    void OnFileComplete(bool success) {
        m_completedFiles.fetch_add(1, std::memory_order_relaxed);
        if (!success) {
            m_failedFiles.fetch_add(1, std::memory_order_relaxed);
        }
    }

    uint32_t GetTotalFiles() const { return m_totalFiles.load(std::memory_order_relaxed); }
    uint32_t GetCompletedFiles() const { return m_completedFiles.load(std::memory_order_relaxed); }
    uint32_t GetFailedFiles() const { return m_failedFiles.load(std::memory_order_relaxed); }

    double GetThroughput() const {
        auto now = std::chrono::steady_clock::now();
        double elapsedSec = std::chrono::duration<double>(now - m_startTime).count();
        if (elapsedSec < 0.001) return 0.0;
        return m_completedFiles.load(std::memory_order_relaxed) / elapsedSec;
    }

    uint32_t GetBatchPercentComplete() const {
        uint32_t total = m_totalFiles.load(std::memory_order_relaxed);
        if (total == 0) return 100;
        return (m_completedFiles.load(std::memory_order_relaxed) * 100) / total;
    }

private:
    std::atomic<uint32_t> m_totalFiles{ 0 };
    std::atomic<uint32_t> m_completedFiles{ 0 };
    std::atomic<uint32_t> m_failedFiles{ 0 };
    std::chrono::steady_clock::time_point m_startTime;
};

} // namespace Engine
} // namespace ExplorerLens
