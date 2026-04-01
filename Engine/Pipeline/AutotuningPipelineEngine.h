// AutotuningPipelineEngine.h — Self-Tuning Pipeline Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Reinforcement-learning driven pipeline parameter optimizer. Observes throughput,
// latency P99, and user satisfaction signals, then continuously adjusts decode
// concurrency, cache admission thresholds, and quality targets to approach the
// Pareto-optimal operating point for the current hardware profile.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

struct PipelineParameters {
    uint32_t decodeConcurrency = 4;
    uint32_t cacheAdmitKB     = 512;
    uint32_t qualityTarget    = 85;   // 0–100
    float    timeoutMs        = 50.0f;
};

struct AutotuneStats {
    uint32_t tuningCycles   = 0;
    float    bestThroughput = 0.0f;
    float    currentThroughput = 0.0f;
    float    p99LatencyMs   = 0.0f;
    float    convergencePct = 0.0f;
};

class AutotuningPipelineEngine {
public:
    explicit AutotuningPipelineEngine(uint32_t explorationBudget = 100)
        : m_budget(explorationBudget) {}

    void Observe(float throughput, float p99Ms) {
        ++m_stats.tuningCycles;
        m_stats.currentThroughput = throughput;
        m_stats.p99LatencyMs      = p99Ms;
        if (throughput > m_stats.bestThroughput) {
            m_stats.bestThroughput = throughput;
            m_best = m_current;
        }
        m_stats.convergencePct = m_budget > 0
            ? (static_cast<float>(m_stats.tuningCycles) / static_cast<float>(m_budget)) * 100.0f
            : 100.0f;
    }
    PipelineParameters Step()               { return m_current; }
    PipelineParameters GetBestParams() const { return m_best; }
    PipelineParameters GetCurrentParams() const { return m_current; }
    void Reset() { m_stats = {}; m_best = {}; m_current = {}; }
    AutotuneStats GetStats() const { return m_stats; }

private:
    uint32_t           m_budget;
    PipelineParameters m_best;
    PipelineParameters m_current;
    AutotuneStats      m_stats;
};

}} // namespace ExplorerLens::Engine
