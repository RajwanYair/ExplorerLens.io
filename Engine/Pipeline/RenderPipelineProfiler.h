// RenderPipelineProfiler.h — Render Pipeline Performance Profiler
// Copyright (c) 2026 ExplorerLens Project
//
// Profiles individual stages of the render pipeline to identify bottlenecks.
// Records per-stage timing, hit counts, and provides stage-by-stage breakdown
// reports. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ProfilerStage : uint8_t {
    FileIO,
    HeaderParse,
    Decode,
    ColorConversion,
    Resize,
    GPUUpload,
    CacheLookup,
    CacheStore,
    Composite,
    Output
};

struct StageProfile {
    ProfilerStage stage = ProfilerStage::FileIO;
    float totalMs = 0.0f;
    float minMs = 1e9f;
    float maxMs = 0.0f;
    uint64_t hitCount = 0;

    float AverageMs() const {
        return hitCount > 0 ? totalMs / static_cast<float>(hitCount) : 0.0f;
    }
};

struct PipelineProfileSnapshot {
    std::vector<StageProfile> stages;
    float totalPipelineMs = 0.0f;
    ProfilerStage bottleneckStage = ProfilerStage::FileIO;
    float bottleneckMs = 0.0f;
};

struct PipelineProfilerStats {
    uint64_t totalProfiles = 0;
    uint64_t stagesRecorded = 0;
    ProfilerStage mostFrequentBottleneck = ProfilerStage::FileIO;
    bool initialized = false;
};

class RenderPipelineProfiler {
public:
    static RenderPipelineProfiler& Instance() {
        static RenderPipelineProfiler instance;
        return instance;
    }

    void Initialize() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stageProfiles.clear();
        m_stats = {};
        m_stats.initialized = true;
    }

    void RecordStage(ProfilerStage stage, float durationMs) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.stagesRecorded++;

        auto& profile = m_stageProfiles[stage];
        profile.stage = stage;
        profile.totalMs += durationMs;
        profile.hitCount++;
        profile.minMs = (std::min)(profile.minMs, durationMs);
        profile.maxMs = (std::max)(profile.maxMs, durationMs);
    }

    PipelineProfileSnapshot GetSnapshot() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        PipelineProfileSnapshot snap;
        snap.totalPipelineMs = 0.0f;
        snap.bottleneckMs = 0.0f;

        for (const auto& [stage, profile] : m_stageProfiles) {
            snap.stages.push_back(profile);
            float avgMs = profile.AverageMs();
            snap.totalPipelineMs += profile.totalMs;
            if (avgMs > snap.bottleneckMs) {
                snap.bottleneckMs = avgMs;
                snap.bottleneckStage = stage;
            }
        }

        return snap;
    }

    StageProfile GetStageProfile(ProfilerStage stage) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_stageProfiles.find(stage);
        if (it != m_stageProfiles.end()) return it->second;
        return {};
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    PipelineProfilerStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_stageProfiles.clear();
    }

private:
    RenderPipelineProfiler() = default;
    ~RenderPipelineProfiler() = default;
    RenderPipelineProfiler(const RenderPipelineProfiler&) = delete;
    RenderPipelineProfiler& operator=(const RenderPipelineProfiler&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<ProfilerStage, StageProfile> m_stageProfiles;
    PipelineProfilerStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
