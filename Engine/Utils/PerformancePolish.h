// PerformancePolish.h - Performance Polish & Optimization (Sprint 32)
// DarkThumbs Engine v7.0.0+
// Copyright (c) 2026 DarkThumbs Project
//
// Features:
// - Micro-profiler with scope-based timing & percentile tracking
// - Memory pool allocator for thumbnail buffers
// - Startup time optimization (cold <500ms, warm <100ms)
// - Soak test framework (100k files, 0 leaks, 0 crashes)
// - Bottleneck detection (>10ms p95 threshold)
// - Per-decoder performance breakdown
//
// Extends existing PerformanceProfiler.h with Sprint 32 targets

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <mutex>
#include <atomic>
#include <cmath>

namespace DarkThumbs {
namespace Engine {
namespace Sprint32 {

// ============================================================================
// Scope-Based Micro-Profiler with Percentile Tracking
// ============================================================================

/// Aggregate statistics for a profiled scope
struct ProfileStats {
    std::string scopeName;
    uint64_t sampleCount = 0;
    double totalUs = 0.0;
    double minUs = 1e12;
    double maxUs = 0.0;
    double avgUs = 0.0;
    double p50Us = 0.0;
    double p95Us = 0.0;
    double p99Us = 0.0;
    double stdDevUs = 0.0;

    bool ExceedsBudget(double budgetUs) const { return p95Us > budgetUs; }

    void ComputeFromSamples(const std::vector<double>& samples) {
        if (samples.empty()) return;
        sampleCount = samples.size();
        totalUs = std::accumulate(samples.begin(), samples.end(), 0.0);
        avgUs = totalUs / sampleCount;
        minUs = *std::min_element(samples.begin(), samples.end());
        maxUs = *std::max_element(samples.begin(), samples.end());

        std::vector<double> sorted = samples;
        std::sort(sorted.begin(), sorted.end());
        auto pct = [&sorted](double p) -> double {
            size_t idx = static_cast<size_t>(p * (sorted.size() - 1));
            return sorted[std::min(idx, sorted.size() - 1)];
        };
        p50Us = pct(0.50);
        p95Us = pct(0.95);
        p99Us = pct(0.99);

        double sumSq = 0.0;
        for (double v : samples) sumSq += (v - avgUs) * (v - avgUs);
        stdDevUs = std::sqrt(sumSq / sampleCount);
    }
};

/// Thread-safe profiler registry
class ProfilerRegistry {
public:
    void Record(const std::string& scopeName, double durationUs) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_samples[scopeName].push_back(durationUs);
    }

    ProfileStats GetStats(const std::string& scopeName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        ProfileStats stats;
        stats.scopeName = scopeName;
        auto it = m_samples.find(scopeName);
        if (it != m_samples.end()) stats.ComputeFromSamples(it->second);
        return stats;
    }

    /// Find all scopes exceeding the performance budget (default 10ms = 10000us)
    std::vector<ProfileStats> FindBottlenecks(double thresholdUs = 10000.0) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<ProfileStats> bottlenecks;
        for (const auto& [name, samples] : m_samples) {
            ProfileStats stats;
            stats.scopeName = name;
            stats.ComputeFromSamples(samples);
            if (stats.p95Us > thresholdUs) bottlenecks.push_back(stats);
        }
        std::sort(bottlenecks.begin(), bottlenecks.end(),
                  [](const ProfileStats& a, const ProfileStats& b) { return a.p95Us > b.p95Us; });
        return bottlenecks;
    }

    void Reset() { std::lock_guard<std::mutex> lock(m_mutex); m_samples.clear(); }
    size_t GetScopeCount() const { std::lock_guard<std::mutex> lock(m_mutex); return m_samples.size(); }

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<double>> m_samples;
};

/// RAII scope-based profiler
class ScopeProfiler {
public:
    ScopeProfiler(ProfilerRegistry& registry, const std::string& name)
        : m_registry(registry), m_name(name), m_start(std::chrono::steady_clock::now()) {}
    ~ScopeProfiler() {
        auto elapsed = std::chrono::steady_clock::now() - m_start;
        m_registry.Record(m_name, std::chrono::duration<double, std::micro>(elapsed).count());
    }
private:
    ProfilerRegistry& m_registry;
    std::string m_name;
    std::chrono::steady_clock::time_point m_start;
};


// ============================================================================
// Memory Pool Allocator
// ============================================================================

/// Fixed-size block pool for thumbnail RGBA buffers — avoids heap fragmentation
class MemoryPool {
public:
    struct PoolConfig {
        size_t blockSize = 256 * 256 * 4;   ///< 256x256 RGBA = 256KB
        size_t maxBlocks = 512;
        size_t initialBlocks = 32;
    };

    explicit MemoryPool(PoolConfig config = {}) : m_config(config) {
        for (size_t i = 0; i < config.initialBlocks; ++i)
            m_freeBlocks.push_back(new uint8_t[config.blockSize]);
        m_totalAllocated = config.initialBlocks;
    }

    ~MemoryPool() { for (auto* b : m_freeBlocks) delete[] b; }

    uint8_t* Allocate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_allocationCount++;
        if (!m_freeBlocks.empty()) {
            auto* block = m_freeBlocks.back();
            m_freeBlocks.pop_back();
            m_poolHits++; m_inUse++;
            return block;
        }
        m_poolMisses++;
        if (m_totalAllocated < m_config.maxBlocks) {
            m_totalAllocated++; m_inUse++;
            return new uint8_t[m_config.blockSize];
        }
        return nullptr;
    }

    void Deallocate(uint8_t* block) {
        if (!block) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_inUse--;
        if (m_freeBlocks.size() < m_config.maxBlocks) m_freeBlocks.push_back(block);
        else { delete[] block; m_totalAllocated--; }
    }

    struct PoolStats {
        size_t blockSize, totalAllocated, inUse, freeCount;
        uint64_t allocationCount, poolHits, poolMisses;
        double hitRate;
        size_t totalMemoryBytes;
    };

    PoolStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return {m_config.blockSize, m_totalAllocated, m_inUse, m_freeBlocks.size(),
                m_allocationCount, m_poolHits, m_poolMisses,
                m_allocationCount > 0 ? double(m_poolHits) / m_allocationCount : 0.0,
                m_totalAllocated * m_config.blockSize};
    }

private:
    PoolConfig m_config;
    std::vector<uint8_t*> m_freeBlocks;
    mutable std::mutex m_mutex;
    size_t m_totalAllocated = 0, m_inUse = 0;
    uint64_t m_allocationCount = 0, m_poolHits = 0, m_poolMisses = 0;
};


// ============================================================================
// Startup Timer
// ============================================================================

class StartupTimer {
public:
    enum class Phase { ProcessStart, COMInit, ConfigLoad, DecoderInit, CacheWarm, GPUInit, Ready };

    void RecordPhase(Phase phase) {
        auto now = std::chrono::steady_clock::now();
        if (m_phases.empty()) m_processStart = now;
        m_phases[phase] = now;
    }

    double GetPhaseTimeMs(Phase phase) const {
        auto it = m_phases.find(phase);
        if (it == m_phases.end()) return -1.0;
        return std::chrono::duration<double, std::milli>(it->second - m_processStart).count();
    }

    double GetTotalStartupMs() const { return GetPhaseTimeMs(Phase::Ready); }
    bool MeetsColdTarget(double targetMs = 500.0) const { auto t = GetTotalStartupMs(); return t >= 0 && t <= targetMs; }
    bool MeetsWarmTarget(double targetMs = 100.0) const { auto t = GetTotalStartupMs(); return t >= 0 && t <= targetMs; }

private:
    std::chrono::steady_clock::time_point m_processStart;
    std::map<Phase, std::chrono::steady_clock::time_point> m_phases;
};


// ============================================================================
// Soak Test Framework
// ============================================================================

struct SoakTestConfig {
    uint32_t totalFiles = 100000;
    uint32_t concurrentThreads = 8;
    uint32_t batchSize = 1000;
    double maxMemoryGrowthMB = 50.0;
    uint32_t maxCrashCount = 0;
    double maxP95LatencyMs = 100.0;
};

struct SoakTestResult {
    bool passed = false;
    uint32_t filesProcessed = 0, filesSucceeded = 0, filesFailed = 0;
    uint32_t crashCount = 0, leakCount = 0;
    double elapsedSeconds = 0.0, throughputFilesPerSec = 0.0;
    double peakMemoryMB = 0.0, memoryGrowthMB = 0.0, p95LatencyMs = 0.0;

    struct DecoderPerf {
        std::string name;
        uint32_t filesDecoded = 0, failures = 0;
        double avgLatencyMs = 0.0, p95LatencyMs = 0.0;
    };
    std::vector<DecoderPerf> decoderBreakdown;

    bool Evaluate(const SoakTestConfig& config) {
        passed = (crashCount <= config.maxCrashCount) &&
                 (memoryGrowthMB <= config.maxMemoryGrowthMB) &&
                 (p95LatencyMs <= config.maxP95LatencyMs) &&
                 (leakCount == 0);
        return passed;
    }
};


// ============================================================================
// Performance Report
// ============================================================================

struct PerformanceReport {
    std::vector<ProfileStats> allScopes;
    std::vector<ProfileStats> bottlenecks;
    double currentMemoryMB = 0.0, peakMemoryMB = 0.0, memoryGrowthMB = 0.0;
    MemoryPool::PoolStats poolStats;
    double coldStartMs = 0.0, warmStartMs = 0.0;
    SoakTestResult soakResult;

    bool AllTargetsMet() const {
        return coldStartMs <= 500.0 && warmStartMs <= 100.0 &&
               bottlenecks.empty() && memoryGrowthMB <= 50.0 && soakResult.passed;
    }
};

} // namespace Sprint32
} // namespace Engine
} // namespace DarkThumbs
