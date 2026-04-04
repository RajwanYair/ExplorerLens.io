// BatchProgressReporter.h — Batch Operation Progress Reporting
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks progress of batch thumbnail generation operations. Reports
// throughput, estimated time remaining, per-format completion rates,
// and error summaries for large folder scans or batch exports.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ReporterBatchProgress
{
    uint32_t totalItems = 0;
    uint32_t completedItems = 0;
    uint32_t failedItems = 0;
    uint32_t skippedItems = 0;
    double progressPercent = 0.0;
    double throughputPerSec = 0.0;
    double estimatedRemainingMs = 0.0;
    double elapsedMs = 0.0;
    bool isComplete = false;
    std::wstring currentFile;
};

struct ReporterItemResult
{
    std::wstring filePath;
    bool success = false;
    double decodeMs = 0.0;
    uint32_t errorCode = 0;
};

struct BatchReporterStats
{
    uint32_t batchesStarted = 0;
    uint32_t batchesCompleted = 0;
    uint64_t totalItemsProcessed = 0;
    double bestThroughput = 0.0;
    double avgThroughput = 0.0;
};

class BatchProgressReporter
{
  public:
    BatchProgressReporter()
    {
        InitializeSRWLock(&m_lock);
        QueryPerformanceFrequency(&m_freq);
    }
    ~BatchProgressReporter() = default;

    static const wchar_t* GetName()
    {
        return L"BatchProgressReporter";
    }

    /// Start a new batch operation.
    void BeginBatch(uint32_t totalItems)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_progress = {};
        m_progress.totalItems = totalItems;
        m_results.clear();
        m_results.reserve(totalItems);
        QueryPerformanceCounter(&m_startTime);
        m_isActive = true;
        m_stats.batchesStarted++;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Report an individual item completion.
    void ReportItem(const ReporterItemResult& result)
    {
        AcquireSRWLockExclusive(&m_lock);
        if (!m_isActive) {
            ReleaseSRWLockExclusive(&m_lock);
            return;
        }

        m_results.push_back(result);
        m_progress.completedItems++;
        if (!result.success)
            m_progress.failedItems++;
        m_progress.currentFile = result.filePath;

        // Update metrics
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        m_progress.elapsedMs = 1000.0 * (now.QuadPart - m_startTime.QuadPart) / m_freq.QuadPart;

        if (m_progress.elapsedMs > 0) {
            m_progress.throughputPerSec = 1000.0 * m_progress.completedItems / m_progress.elapsedMs;
        }

        uint32_t remaining = m_progress.totalItems - m_progress.completedItems;
        if (m_progress.throughputPerSec > 0) {
            m_progress.estimatedRemainingMs = 1000.0 * remaining / m_progress.throughputPerSec;
        }

        m_progress.progressPercent =
            m_progress.totalItems > 0 ? 100.0 * m_progress.completedItems / m_progress.totalItems : 0.0;

        m_progress.isComplete = (m_progress.completedItems + m_progress.skippedItems >= m_progress.totalItems);
        if (m_progress.isComplete) {
            m_isActive = false;
            m_stats.batchesCompleted++;
            m_stats.totalItemsProcessed += m_progress.completedItems;
            if (m_progress.throughputPerSec > m_stats.bestThroughput)
                m_stats.bestThroughput = m_progress.throughputPerSec;
        }

        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Skip an item (already cached, unsupported, etc.)
    void SkipItem([[maybe_unused]] const std::wstring& filePath)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_progress.skippedItems++;
        m_progress.progressPercent =
            m_progress.totalItems > 0
                ? 100.0 * (m_progress.completedItems + m_progress.skippedItems) / m_progress.totalItems
                : 0.0;
        m_progress.isComplete = (m_progress.completedItems + m_progress.skippedItems >= m_progress.totalItems);
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Get current progress snapshot.
    ReporterBatchProgress GetProgress() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        ReporterBatchProgress p = m_progress;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return p;
    }

    /// Format remaining time as human-readable string.
    static std::wstring FormatETA(double remainingMs)
    {
        if (remainingMs < 1000)
            return std::to_wstring(static_cast<int>(remainingMs)) + L" ms";
        if (remainingMs < 60000)
            return std::to_wstring(static_cast<int>(remainingMs / 1000)) + L" sec";
        double mins = remainingMs / 60000.0;
        return std::to_wstring(static_cast<int>(mins)) + L" min";
    }

    BatchReporterStats GetStats() const
    {
        return m_stats;
    }

  private:
    SRWLOCK m_lock{};
    LARGE_INTEGER m_freq{};
    LARGE_INTEGER m_startTime{};
    bool m_isActive = false;
    ReporterBatchProgress m_progress;
    std::vector<ReporterItemResult> m_results;
    mutable BatchReporterStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
