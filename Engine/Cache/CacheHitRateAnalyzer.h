// CacheHitRateAnalyzer.h — Analyzes and reports cache hit/miss ratios
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks cache hits, misses, and evictions over sliding time windows to
// provide actionable metrics for cache tuning and capacity planning.
//
#pragma once
#include <string>
#include <cstdint>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

struct CacheHitRateAnalyzerConfig {
    bool enabled = true;
    uint32_t windowSize = 1000;
    std::string label = "CacheHitRateAnalyzer";
};

class CacheHitRateAnalyzer {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_hits.store(0); m_misses.store(0); m_evictions.store(0);
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheHitRateAnalyzerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    void RecordHit() { m_hits.fetch_add(1, std::memory_order_relaxed); }
    void RecordMiss() { m_misses.fetch_add(1, std::memory_order_relaxed); }
    void RecordEviction() { m_evictions.fetch_add(1, std::memory_order_relaxed); }

    double GetHitRate() const {
        uint64_t h = m_hits.load(std::memory_order_relaxed);
        uint64_t total = h + m_misses.load(std::memory_order_relaxed);
        return (total > 0) ? static_cast<double>(h) / static_cast<double>(total) : 0.0;
    }

    uint64_t GetTotalRequests() const {
        return m_hits.load(std::memory_order_relaxed) + m_misses.load(std::memory_order_relaxed);
    }

private:
    bool m_initialized = false;
    CacheHitRateAnalyzerConfig m_config;
    std::atomic<uint64_t> m_hits{ 0 };
    std::atomic<uint64_t> m_misses{ 0 };
    std::atomic<uint64_t> m_evictions{ 0 };
};

}
} // namespace ExplorerLens::Engine
