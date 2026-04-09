// PredictivePreGenEngine.cpp — Predictive Thumbnail Pre-Generation Engine
// Copyright (c) 2026 ExplorerLens Project
//
#include "Pipeline/PredictivePreGenEngine.h"
#include "Pipeline/DirectoryPreScanQueue.h"
#include "Pipeline/AdjacencyPredictor.h"
#include "Pipeline/ScrollVelocityTracker.h"
#include "Pipeline/IdleTimePreGenerator.h"
#include <atomic>
#include <mutex>
#include <chrono>

namespace ExplorerLens { namespace Engine {

struct PredictivePreGenEngine::Impl {
    DirectoryPreScanQueue  scanQueue;
    AdjacencyPredictor     adjPredictor;
    ScrollVelocityTracker  scrollTracker;
    IdleTimePreGenerator   idleGen;

    std::atomic<uint64_t> totalQueued    { 0 };
    std::atomic<uint64_t> totalGenerated { 0 };
    std::atomic<uint64_t> cacheHits      { 0 };
    std::atomic<uint64_t> cacheMisses    { 0 };
    std::mutex            statsLock;

    // Current directory state for scroll predictions.
    std::vector<std::wstring> currentDirFiles;
    std::mutex                dirLock;
};

PredictivePreGenEngine::PredictivePreGenEngine(
    const PreGenEngineConfig& config) noexcept
    : m_config(config)
{
    DirectoryPreScanConfig scanCfg{};
    scanCfg.maxQueueDepth    = config.maxQueueDepth;
    scanCfg.workerThreadCount = config.backgroundThreads;
    scanCfg.skipNetworkDrives = config.skipNetworkPaths;

    m_impl = new Impl{
        DirectoryPreScanQueue{scanCfg},
        AdjacencyPredictor{},
        ScrollVelocityTracker{},
        IdleTimePreGenerator{},
    };
}

PredictivePreGenEngine::~PredictivePreGenEngine()
{
    Stop();
    delete m_impl;
}

void PredictivePreGenEngine::SetCallbacks(
    CacheQueryCallback  cacheQuery,
    GenerateCallback    generate) noexcept
{
    m_cacheQuery = std::move(cacheQuery);
    m_generate   = std::move(generate);
}

void PredictivePreGenEngine::OnFolderOpened(const std::wstring& dirPath) noexcept
{
    m_impl->adjPredictor.RecordNavigation(dirPath);
    const uint32_t queued = m_impl->scanQueue.EnqueueDirectory(dirPath);
    m_impl->totalQueued.fetch_add(queued);

    if (!m_config.enableAdjacency) return;
    auto preds = m_impl->adjPredictor.Predict(dirPath, m_config.backgroundThreads);
    for (const auto& p : preds) {
        const uint32_t q = m_impl->scanQueue.EnqueueDirectory(p.directoryPath);
        m_impl->totalQueued.fetch_add(q);
    }
}

void PredictivePreGenEngine::OnScrollEvent(
    int32_t topIndex, int32_t deltaItems, uint32_t viewportRows) noexcept
{
    if (!m_config.enableScrollPredict) return;

    ScrollSample s{};
    s.timestampUs  = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::steady_clock::now().time_since_epoch()).count();
    s.deltaItems   = deltaItems;
    s.viewportRows = viewportRows;
    m_impl->scrollTracker.AddSample(s);

    m_impl->scrollTracker.Evaluate(topIndex);
}

void PredictivePreGenEngine::OnThumbnailRequested(
    const std::wstring& filePath) noexcept
{
    if (m_cacheQuery && m_cacheQuery(filePath)) {
        m_impl->cacheHits.fetch_add(1);
    } else {
        m_impl->cacheMisses.fetch_add(1);
        // Promote in the scan queue so it is generated sooner.
        m_impl->scanQueue.Promote(filePath);
    }
}

void PredictivePreGenEngine::Start() noexcept
{
    if (m_running) return;
    m_running = true;

    // Wire up the pre-gen callback.
    m_impl->scanQueue.SetPreGenCallback([this](const std::wstring& path) {
        if (m_cacheQuery && m_cacheQuery(path)) return; // already cached
        if (m_generate) m_generate(path);
        m_impl->totalGenerated.fetch_add(1);
    });
    m_impl->scanQueue.Start();

    if (m_config.enableScrollPredict) {
        m_impl->scrollTracker.SetTriggerCallback(
            [this](int32_t startIdx, uint32_t count, int32_t /*dir*/) {
                m_impl->totalQueued.fetch_add(count);
                // Scroll prediction just bumps the queuing counter; actual files
                // are queued by OnFolderOpened so we just log here.
            });
    }

    if (m_config.enableIdleGen) {
        m_impl->idleGen.SetWorkCallback([this]() -> bool {
            return m_impl->scanQueue.QueueDepth() > 0;
        });
        m_impl->idleGen.Start();
    }
}

void PredictivePreGenEngine::Stop() noexcept
{
    if (!m_running) return;
    m_impl->idleGen.Stop();
    m_impl->scanQueue.Stop();
    m_running = false;
}

bool PredictivePreGenEngine::IsRunning() const noexcept { return m_running; }

PreGenEngineStats PredictivePreGenEngine::GetStats() const noexcept
{
    PreGenEngineStats s{};
    s.totalQueued       = m_impl->totalQueued.load();
    s.totalGenerated    = m_impl->totalGenerated.load();
    s.cacheHits         = m_impl->cacheHits.load();
    s.cacheMisses       = m_impl->cacheMisses.load();
    const uint64_t total = s.cacheHits + s.cacheMisses;
    s.cacheHitRate      = (total > 0) ? static_cast<float>(s.cacheHits) / total : 0.0f;
    s.throughputPerSec  = 0.0f;  // Would need timing integration
    s.activeThreads     = m_impl->scanQueue.IsRunning() ? m_config.backgroundThreads : 0;
    s.currentQueueDepth = m_impl->scanQueue.QueueDepth();
    return s;
}

void PredictivePreGenEngine::ResetStats() noexcept
{
    m_impl->totalQueued.store(0);
    m_impl->totalGenerated.store(0);
    m_impl->cacheHits.store(0);
    m_impl->cacheMisses.store(0);
}

}} // namespace ExplorerLens::Engine
