// ContextualRenderingEngine.h — Context-Aware Thumbnail Rendering
// Copyright (c) 2026 ExplorerLens Project
//
// Adjusts rendering quality and parameters based on contextual signals such as
// display DPI, available GPU memory, user viewing patterns, and time-of-day
// energy profiles. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class RenderContextType : uint8_t {
    Default,
    HighDPI,
    LowMemory,
    BatterySaver,
    BackgroundTask,
    UserBrowsing,
    SearchResult
};

enum class RenderQualityTier : uint8_t {
    Minimal,
    Low,
    Medium,
    High,
    Ultra
};

struct ContextualRenderParams {
    RenderContextType contextType = RenderContextType::Default;
    RenderQualityTier qualityTier = RenderQualityTier::Medium;
    uint32_t targetWidthPx = 256;
    uint32_t targetHeightPx = 256;
    float dpiScale = 1.0f;
    bool useGPU = true;
    bool allowCaching = true;
};

struct RenderContextSignals {
    float displayDPI = 96.0f;
    uint64_t availableGPUMemoryBytes = 0;
    bool onBattery = false;
    bool isBackgroundProcess = false;
    uint32_t concurrentRenderCount = 0;
};

struct ContextualRenderStats {
    uint64_t totalEvaluations = 0;
    uint64_t contextSwitches = 0;
    uint64_t qualityUpgrades = 0;
    uint64_t qualityDowngrades = 0;
    bool initialized = false;
};

class ContextualRenderingEngine {
public:
    static ContextualRenderingEngine& Instance() {
        static ContextualRenderingEngine instance;
        return instance;
    }

    void Initialize(RenderQualityTier defaultTier = RenderQualityTier::Medium) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_defaultTier = defaultTier;
        m_currentParams = {};
        m_currentParams.qualityTier = defaultTier;
        m_stats = {};
        m_stats.initialized = true;
    }

    ContextualRenderParams Evaluate(const RenderContextSignals& signals) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalEvaluations++;

        ContextualRenderParams params;
        params.dpiScale = signals.displayDPI / 96.0f;
        params.targetWidthPx = static_cast<uint32_t>(256 * params.dpiScale);
        params.targetHeightPx = static_cast<uint32_t>(256 * params.dpiScale);

        if (signals.onBattery) {
            params.contextType = RenderContextType::BatterySaver;
            params.qualityTier = RenderQualityTier::Low;
            params.useGPU = false;
        } else if (signals.availableGPUMemoryBytes < 128 * 1024 * 1024) {
            params.contextType = RenderContextType::LowMemory;
            params.qualityTier = RenderQualityTier::Low;
            params.useGPU = false;
        } else if (signals.isBackgroundProcess) {
            params.contextType = RenderContextType::BackgroundTask;
            params.qualityTier = RenderQualityTier::Medium;
        } else if (signals.displayDPI >= 192.0f) {
            params.contextType = RenderContextType::HighDPI;
            params.qualityTier = RenderQualityTier::High;
        } else {
            params.contextType = RenderContextType::UserBrowsing;
            params.qualityTier = m_defaultTier;
        }

        if (params.qualityTier != m_currentParams.qualityTier) {
            m_stats.contextSwitches++;
            if (params.qualityTier > m_currentParams.qualityTier)
                m_stats.qualityUpgrades++;
            else
                m_stats.qualityDowngrades++;
        }

        m_currentParams = params;
        return params;
    }

    ContextualRenderParams GetCurrentParams() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_currentParams;
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    ContextualRenderStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_currentParams = {};
    }

private:
    ContextualRenderingEngine() = default;
    ~ContextualRenderingEngine() = default;
    ContextualRenderingEngine(const ContextualRenderingEngine&) = delete;
    ContextualRenderingEngine& operator=(const ContextualRenderingEngine&) = delete;

    mutable std::mutex m_mutex;
    RenderQualityTier m_defaultTier = RenderQualityTier::Medium;
    ContextualRenderParams m_currentParams;
    ContextualRenderStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
