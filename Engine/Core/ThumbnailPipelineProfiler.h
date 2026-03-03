// ThumbnailPipelineProfiler.h — Performance Profiling Instrumentation
// Copyright (c) 2026 ExplorerLens Project
//
// Lightweight profiling system for the thumbnail pipeline. Instruments each
// pipeline stage (file I/O, decode, GPU resize, cache store) with precise
// timing using QueryPerformanceCounter. Generates flame-chart compatible
// output and integrates with ETW tracing.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

/// Pipeline stage identifiers for profiling
enum class ProfileStage : uint8_t {
    Total = 0,    // Entire thumbnail pipeline
    CacheLookup = 1,    // Cache lookup
    FileOpen = 2,    // File open + header read
    FileRead = 3,    // Full file I/O
    FormatDetect = 4,    // Magic byte detection
    Decode = 5,    // Format-specific decode
    ColorConvert = 6,    // Color space conversion
    GPUUpload = 7,    // Upload to GPU texture
    GPUResize = 8,    // GPU-accelerated resize
    CPUResize = 9,    // CPU fallback resize
    QualityGate = 10,   // Output quality validation
    CacheStore = 11,   // Store result in cache
    BitmapCreate = 12,   // Create HBITMAP for shell
    StageCount = 13
};

/// Single timing entry (start + end QPC ticks)
struct ProfileEntry {
    int64_t     startTick = 0;
    int64_t     endTick = 0;
    ProfileStage stage = ProfileStage::Total;
    const char* label = nullptr;
    uint32_t    threadId = 0;
    uint32_t    depth = 0;        // Nesting depth for flame chart
};

/// Aggregated statistics for a pipeline stage
struct StageStatistics {
    ProfileStage stage = ProfileStage::Total;
    uint64_t    count = 0;
    double      totalMs = 0.0;
    double      avgMs = 0.0;
    double      minMs = 1e9;
    double      maxMs = 0.0;
    double      p50Ms = 0.0;
    double      p95Ms = 0.0;
    double      p99Ms = 0.0;
    double      percentOfTotal = 0.0;   // % of Total stage time
};

/// Complete profile for one thumbnail generation
struct ThumbnailProfile {
    std::vector<ProfileEntry>   entries;
    double                      totalMs = 0.0;
    std::string                 filePath;
    uint32_t                    thumbnailSize = 0;
    std::string                 decoderUsed;
    bool                        cacheHit = false;
    bool                        gpuUsed = false;
};

/// RAII scope for timing a pipeline stage
class ProfileScope {
public:
    ProfileScope(class ThumbnailPipelineProfiler* profiler, ProfileStage stage, const char* label = nullptr);
    ~ProfileScope();

    // Non-copyable, non-movable
    ProfileScope(const ProfileScope&) = delete;
    ProfileScope& operator=(const ProfileScope&) = delete;

private:
    class ThumbnailPipelineProfiler* m_profiler;
    ProfileStage    m_stage;
    int64_t         m_startTick;
    const char* m_label;
};

/// Maximum entries per thread-local profile
constexpr uint32_t MAX_PROFILE_ENTRIES = 64;

/// High-precision pipeline profiler. Captures per-stage timing for individual
/// thumbnail generations and maintains aggregate statistics across all requests.
///
/// Usage:
///   auto& profiler = ThumbnailPipelineProfiler::Instance();
///   profiler.BeginProfile("photo.jpg", 256);
///   { ProfileScope s(&profiler, ProfileStage::FileRead); /* read file */ }
///   { ProfileScope s(&profiler, ProfileStage::Decode); /* decode */ }
///   auto profile = profiler.EndProfile();
///
class ThumbnailPipelineProfiler {
public:
    static ThumbnailPipelineProfiler& Instance() {
        static ThumbnailPipelineProfiler instance;
        return instance;
    }

    /// Initialize profiler (calibrate QPC frequency)
    void Initialize() {
        QueryPerformanceFrequency(&m_qpcFreq);
        m_enabled = true;
    }

    /// Enable/disable profiling at runtime
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

    /// Begin profiling a new thumbnail generation
    void BeginProfile(const char* filePath, uint32_t thumbnailSize) {
        if (!m_enabled) return;
        auto& active = GetActiveProfile();
        active.entries.clear();
        active.filePath = filePath ? filePath : "";
        active.thumbnailSize = thumbnailSize;
        active.totalMs = 0;
        active.cacheHit = false;
        active.gpuUsed = false;
    }

    /// Record start of a pipeline stage
    int64_t RecordStageStart(ProfileStage stage, const char* label = nullptr) {
        if (!m_enabled) return 0;
        LARGE_INTEGER tick;
        QueryPerformanceCounter(&tick);

        ProfileEntry entry;
        entry.startTick = tick.QuadPart;
        entry.stage = stage;
        entry.label = label;
        entry.threadId = GetCurrentThreadId();
        entry.depth = m_currentDepth++;

        auto& active = GetActiveProfile();
        active.entries.push_back(entry);
        return tick.QuadPart;
    }

    /// Record end of a pipeline stage
    void RecordStageEnd(ProfileStage stage, int64_t startTick) {
        if (!m_enabled) return;
        LARGE_INTEGER tick;
        QueryPerformanceCounter(&tick);

        m_currentDepth--;

        auto& active = GetActiveProfile();
        for (auto it = active.entries.rbegin(); it != active.entries.rend(); ++it) {
            if (it->stage == stage && it->startTick == startTick && it->endTick == 0) {
                it->endTick = tick.QuadPart;
                double ms = TicksToMs(it->endTick - it->startTick);

                // Update aggregate stats
                UpdateAggregateStats(stage, ms);
                break;
            }
        }
    }

    /// End profiling and return the captured profile
    ThumbnailProfile EndProfile() {
        auto& active = GetActiveProfile();
        ThumbnailProfile result = active;

        // Calculate total time
        for (const auto& entry : result.entries) {
            if (entry.stage == ProfileStage::Total && entry.endTick > 0) {
                result.totalMs = TicksToMs(entry.endTick - entry.startTick);
            }
        }

        active = ThumbnailProfile{}; // Reset
        return result;
    }

    /// Get aggregate statistics for all stages
    std::array<StageStatistics, static_cast<size_t>(ProfileStage::StageCount)> GetAggregateStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_aggregateStats;
    }

    /// Get aggregate stats for a specific stage
    StageStatistics GetStageStats(ProfileStage stage) const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_aggregateStats[static_cast<size_t>(stage)];
    }

    /// Reset all aggregate statistics
    void ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        for (auto& s : m_aggregateStats) {
            s = StageStatistics{};
        }
    }

    /// Generate a human-readable report
    std::string GenerateReport() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        std::ostringstream ss;
        ss << "=== Thumbnail Pipeline Profiler Report ===\n\n";

        double totalAvg = m_aggregateStats[0].avgMs;
        const char* stageNames[] = {
            "Total", "CacheLookup", "FileOpen", "FileRead", "FormatDetect",
            "Decode", "ColorConvert", "GPUUpload", "GPUResize", "CPUResize",
            "QualityGate", "CacheStore", "BitmapCreate"
        };

        for (size_t i = 0; i < static_cast<size_t>(ProfileStage::StageCount); i++) {
            const auto& s = m_aggregateStats[i];
            if (s.count == 0) continue;

            double pct = (totalAvg > 0) ? (s.avgMs / totalAvg * 100.0) : 0.0;
            ss << stageNames[i] << ":\n"
                << "  Count: " << s.count
                << "  Avg: " << s.avgMs << " ms"
                << "  Min: " << s.minMs << " ms"
                << "  Max: " << s.maxMs << " ms"
                << "  P95: " << s.p95Ms << " ms"
                << "  (" << pct << "% of total)\n";
        }
        return ss.str();
    }

    double TicksToMs(int64_t ticks) const {
        return m_qpcFreq.QuadPart > 0 ?
            static_cast<double>(ticks * 1000) / m_qpcFreq.QuadPart : 0.0;
    }

private:
    ThumbnailPipelineProfiler() { Initialize(); }

    ThumbnailProfile& GetActiveProfile() {
        // Thread-local active profile
        thread_local ThumbnailProfile tl_profile;
        return tl_profile;
    }

    void UpdateAggregateStats(ProfileStage stage, double ms) {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        auto& s = m_aggregateStats[static_cast<size_t>(stage)];
        s.stage = stage;
        s.count++;
        s.totalMs += ms;
        s.avgMs = s.totalMs / s.count;
        s.minMs = (std::min)(s.minMs, ms);
        s.maxMs = (std::max)(s.maxMs, ms);

        // Approximate P95/P99 using exponential moving average
        if (s.count == 1) {
            s.p50Ms = ms; s.p95Ms = ms; s.p99Ms = ms;
        }
        else {
            // EMA with different weights
            s.p50Ms = s.p50Ms * 0.95 + ms * 0.05;
            s.p95Ms = (ms > s.p95Ms) ? s.p95Ms * 0.9 + ms * 0.1 : s.p95Ms * 0.99 + ms * 0.01;
            s.p99Ms = (ms > s.p99Ms) ? s.p99Ms * 0.85 + ms * 0.15 : s.p99Ms * 0.995 + ms * 0.005;
        }
    }

    bool                        m_enabled = false;
    LARGE_INTEGER               m_qpcFreq = {};
    thread_local static inline uint32_t m_currentDepth = 0;
    mutable std::mutex          m_statsMutex;
    std::array<StageStatistics, static_cast<size_t>(ProfileStage::StageCount)> m_aggregateStats = {};
};

// ProfileScope implementation
inline ProfileScope::ProfileScope(ThumbnailPipelineProfiler* profiler, ProfileStage stage, const char* label)
    : m_profiler(profiler), m_stage(stage), m_label(label) {
    m_startTick = profiler ? profiler->RecordStageStart(stage, label) : 0;
}

inline ProfileScope::~ProfileScope() {
    if (m_profiler) m_profiler->RecordStageEnd(m_stage, m_startTick);
}

} // namespace Engine
} // namespace ExplorerLens
