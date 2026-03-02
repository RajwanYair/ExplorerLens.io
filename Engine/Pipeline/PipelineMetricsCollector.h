#pragma once
// ============================================================================
// PipelineMetricsCollector.h — Unified pipeline-wide metrics aggregation
// ExplorerLens Engine v15.0.0 "Zenith"
// ============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <array>
#include <atomic>
#include <chrono>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// Pipeline stages tracked
enum class PipelineStageId : uint32_t {
    FormatDetection = 0,
    Decode = 1,
    GPURender = 2,
    CacheLookup = 3,
    CacheStore = 4,
    PostProcess = 5,
    IORead = 6,
    Validation = 7,
    Count = 8
};

static const wchar_t* PipelineStageName(PipelineStageId id) {
    static const wchar_t* names[] = {
        L"FormatDetection", L"Decode", L"GPURender", L"CacheLookup",
        L"CacheStore", L"PostProcess", L"IORead", L"Validation"
    };
    auto idx = static_cast<uint32_t>(id);
    return (idx < static_cast<uint32_t>(PipelineStageId::Count)) ? names[idx] : L"Unknown";
}

// Per-stage metrics snapshot
struct StageMetricsSnapshot {
    uint64_t totalInvocations = 0;
    uint64_t totalErrors = 0;
    double   avgLatencyMs = 0.0;
    double   p99LatencyMs = 0.0;
    double   maxLatencyMs = 0.0;
    double   throughputPerSec = 0.0;
    uint32_t queueDepth = 0;
};

// Full pipeline metrics snapshot
struct PipelineMetricsSnapshot {
    std::array<StageMetricsSnapshot, static_cast<size_t>(PipelineStageId::Count)> stages;
    uint64_t totalRequests = 0;
    uint64_t totalErrors = 0;
    double   overallLatencyMs = 0.0;
    double   overallThroughput = 0.0;
    uint64_t uptimeSeconds = 0;
    uint64_t snapshotTimestamp = 0;
};

// Per-stage atomic counters
struct StageCounters {
    std::atomic<uint64_t> invocations{ 0 };
    std::atomic<uint64_t> errors{ 0 };
    std::atomic<uint64_t> totalLatencyUs{ 0 };
    std::atomic<uint64_t> maxLatencyUs{ 0 };
    std::atomic<uint32_t> activeCount{ 0 };

    void RecordLatency(uint64_t microseconds) {
        invocations.fetch_add(1, std::memory_order_relaxed);
        totalLatencyUs.fetch_add(microseconds, std::memory_order_relaxed);
        uint64_t curMax = maxLatencyUs.load(std::memory_order_relaxed);
        while (microseconds > curMax) {
            if (maxLatencyUs.compare_exchange_weak(curMax, microseconds, std::memory_order_relaxed))
                break;
        }
    }

    void RecordError() {
        errors.fetch_add(1, std::memory_order_relaxed);
    }

    StageMetricsSnapshot ToSnapshot(double elapsedSec) const {
        StageMetricsSnapshot s;
        s.totalInvocations = invocations.load(std::memory_order_relaxed);
        s.totalErrors = errors.load(std::memory_order_relaxed);
        s.maxLatencyMs = static_cast<double>(maxLatencyUs.load(std::memory_order_relaxed)) / 1000.0;
        s.avgLatencyMs = (s.totalInvocations > 0)
            ? (static_cast<double>(totalLatencyUs.load(std::memory_order_relaxed)) / 1000.0 / static_cast<double>(s.totalInvocations))
            : 0.0;
        s.p99LatencyMs = s.maxLatencyMs * 0.95; // Approximation without full histogram
        s.throughputPerSec = (elapsedSec > 0.0) ? (static_cast<double>(s.totalInvocations) / elapsedSec) : 0.0;
        s.queueDepth = activeCount.load(std::memory_order_relaxed);
        return s;
    }

    void Reset() {
        invocations.store(0, std::memory_order_relaxed);
        errors.store(0, std::memory_order_relaxed);
        totalLatencyUs.store(0, std::memory_order_relaxed);
        maxLatencyUs.store(0, std::memory_order_relaxed);
        activeCount.store(0, std::memory_order_relaxed);
    }
};

// RAII timing scope for a stage
class StageTiming {
public:
    StageTiming(StageCounters& counters) : m_counters(counters) {
        m_counters.activeCount.fetch_add(1, std::memory_order_relaxed);
        QueryPerformanceCounter(&m_start);
    }
    ~StageTiming() {
        LARGE_INTEGER end, freq;
        QueryPerformanceCounter(&end);
        QueryPerformanceFrequency(&freq);
        uint64_t us = static_cast<uint64_t>((end.QuadPart - m_start.QuadPart) * 1000000 / freq.QuadPart);
        m_counters.RecordLatency(us);
        m_counters.activeCount.fetch_sub(1, std::memory_order_relaxed);
    }
private:
    StageCounters& m_counters;
    LARGE_INTEGER m_start{};
};

// ========================================================================
// PipelineMetricsCollector — Singleton aggregating all stage metrics
// ========================================================================
class PipelineMetricsCollector {
public:
    static PipelineMetricsCollector& Instance() {
        static PipelineMetricsCollector instance;
        return instance;
    }

    void Initialize() {
        QueryPerformanceCounter(&m_startTime);
        m_totalRequests.store(0, std::memory_order_relaxed);
        m_totalErrors.store(0, std::memory_order_relaxed);
        for (auto& c : m_stageCounters) c.Reset();
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Get counters for direct stage timing
    StageCounters& GetStageCounters(PipelineStageId stage) {
        return m_stageCounters[static_cast<size_t>(stage)];
    }

    // Record a completed request
    void RecordRequest() {
        m_totalRequests.fetch_add(1, std::memory_order_relaxed);
    }

    void RecordRequestError() {
        m_totalErrors.fetch_add(1, std::memory_order_relaxed);
    }

    // Capture full snapshot
    PipelineMetricsSnapshot CaptureSnapshot() const {
        PipelineMetricsSnapshot snap;
        double elapsed = GetElapsedSeconds();

        for (size_t i = 0; i < static_cast<size_t>(PipelineStageId::Count); ++i) {
            snap.stages[i] = m_stageCounters[i].ToSnapshot(elapsed);
        }

        snap.totalRequests = m_totalRequests.load(std::memory_order_relaxed);
        snap.totalErrors = m_totalErrors.load(std::memory_order_relaxed);
        snap.uptimeSeconds = static_cast<uint64_t>(elapsed);
        snap.overallThroughput = (elapsed > 0.0) ? (static_cast<double>(snap.totalRequests) / elapsed) : 0.0;

        // Overall latency = sum of stage averages
        double totalAvg = 0.0;
        for (auto& s : snap.stages) totalAvg += s.avgLatencyMs;
        snap.overallLatencyMs = totalAvg;

        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        snap.snapshotTimestamp = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

        return snap;
    }

    // Get stage error rate (0.0 - 1.0)
    double GetStageErrorRate(PipelineStageId stage) const {
        auto& c = m_stageCounters[static_cast<size_t>(stage)];
        uint64_t inv = c.invocations.load(std::memory_order_relaxed);
        uint64_t err = c.errors.load(std::memory_order_relaxed);
        return (inv > 0) ? (static_cast<double>(err) / static_cast<double>(inv)) : 0.0;
    }

    // Get overall error rate
    double GetOverallErrorRate() const {
        uint64_t req = m_totalRequests.load(std::memory_order_relaxed);
        uint64_t err = m_totalErrors.load(std::memory_order_relaxed);
        return (req > 0) ? (static_cast<double>(err) / static_cast<double>(req)) : 0.0;
    }

    // Reset all counters
    void Reset() {
        for (auto& c : m_stageCounters) c.Reset();
        m_totalRequests.store(0, std::memory_order_relaxed);
        m_totalErrors.store(0, std::memory_order_relaxed);
        QueryPerformanceCounter(&m_startTime);
    }

private:
    PipelineMetricsCollector() = default;

    double GetElapsedSeconds() const {
        LARGE_INTEGER now, freq;
        QueryPerformanceCounter(&now);
        QueryPerformanceFrequency(&freq);
        return static_cast<double>(now.QuadPart - m_startTime.QuadPart) / static_cast<double>(freq.QuadPart);
    }

    mutable std::array<StageCounters, static_cast<size_t>(PipelineStageId::Count)> m_stageCounters;
    std::atomic<uint64_t> m_totalRequests{ 0 };
    std::atomic<uint64_t> m_totalErrors{ 0 };
    LARGE_INTEGER m_startTime{};
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
