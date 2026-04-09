// PredictivePreGenEngine.h — Predictive Thumbnail Pre-Generation Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Top-level coordinator for the T4 predictive pre-generation subsystem.
// Integrates DirectoryPreScanQueue, AdjacencyPredictor, ScrollVelocityTracker,
// and IdleTimePreGenerator into a single engine that drives the cache
// fill strategy. Target: > 95% cache-hit rate during folder browsing,
// 800 img/s background throughput, < 3 s to fill a 2000-file folder.
//
#pragma once
#include <cstdint>
#include <string>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct PreGenEngineConfig {
    uint32_t maxQueueDepth        = 4096;
    uint32_t backgroundThreads    = 4;
    bool     enableScrollPredict  = true;
    bool     enableAdjacency      = true;
    bool     enableIdleGen        = true;
    bool     skipNetworkPaths     = true;
    uint32_t batteryDisableThresh = 20;   // % battery — disable below this
};

struct PreGenEngineStats {
    uint64_t totalQueued       = 0;
    uint64_t totalGenerated    = 0;
    uint64_t cacheHits         = 0;
    uint64_t cacheMisses       = 0;
    float    cacheHitRate      = 0.0f;
    float    throughputPerSec  = 0.0f;
    uint32_t activeThreads     = 0;
    uint32_t currentQueueDepth = 0;
};

class PredictivePreGenEngine {
public:
    // Callback that the engine calls to check if a file is already cached.
    using CacheQueryCallback  = std::function<bool(const std::wstring& path)>;
    // Callback that the engine calls to trigger thumbnail generation.
    using GenerateCallback    = std::function<void(const std::wstring& path)>;

    explicit PredictivePreGenEngine(
        const PreGenEngineConfig& config = {}) noexcept;
    ~PredictivePreGenEngine();

    // Wire up the cache query and generate callbacks.
    void SetCallbacks(CacheQueryCallback cacheQuery,
                      GenerateCallback   generate) noexcept;

    // Called by IThumbnailProvider when Explorer opens a folder.
    void OnFolderOpened(const std::wstring& dirPath) noexcept;

    // Called by IThumbnailProvider when the user scrolls.
    void OnScrollEvent(int32_t topIndex, int32_t deltaItems,
                       uint32_t viewportRows) noexcept;

    // Called when a thumbnail is requested (hits counter + promotes queue entry).
    void OnThumbnailRequested(const std::wstring& filePath) noexcept;

    // Start/stop all background threads.
    void Start() noexcept;
    void Stop()  noexcept;
    bool IsRunning() const noexcept;

    PreGenEngineStats GetStats() const noexcept;
    void              ResetStats() noexcept;

private:
    struct Impl;
    Impl*              m_impl = nullptr;
    PreGenEngineConfig m_config;
    CacheQueryCallback m_cacheQuery;
    GenerateCallback   m_generate;
    bool               m_running = false;
};

}} // namespace ExplorerLens::Engine
