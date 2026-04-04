// DecodeStrategyOptimizer.h — Decode Strategy Selection Optimizer
// Copyright (c) 2026 ExplorerLens Project
//
// Selects the optimal decode strategy based on file format, system state,
// GPU availability, and historical performance data. Implements a
// multi-armed bandit approach to learn the best pipeline per format.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DecodeStrategy : uint8_t {
    CPUSingleThread,
    CPUMultiThread,
    GPUDirect,        // GPU decode pipeline
    GPUWithFallback,  // GPU with CPU fallback
    MemoryMapped,     // mmap + CPU
    Streaming,        // Progressive decode
    Cached            // Sub-ms cache return
};

struct StrategyPerformance
{
    DecodeStrategy strategy = DecodeStrategy::CPUSingleThread;
    uint64_t trialCount = 0;
    double avgLatencyMs = 0.0;
    double successRate = 1.0;
    double score = 0.0;  // UCB1 score
};

struct StrategyRecommendation
{
    DecodeStrategy bestStrategy = DecodeStrategy::CPUSingleThread;
    double confidence = 0.0;
    double expectedLatencyMs = 0.0;
    std::vector<StrategyPerformance> alternatives;
};

struct OptimizerStats
{
    uint32_t decisionsRendered = 0;
    uint32_t formatsTracked = 0;
    uint32_t strategySwitches = 0;
    double avgLatencyImprovement = 0.0;
};

class DecodeStrategyOptimizer
{
  public:
    DecodeStrategyOptimizer()
    {
        InitializeSRWLock(&m_lock);
    }
    ~DecodeStrategyOptimizer() = default;

    static const wchar_t* GetName()
    {
        return L"DecodeStrategyOptimizer";
    }

    /// Record a trial result for a format+strategy combination.
    void RecordTrial(const std::string& format, DecodeStrategy strategy, double latencyMs, bool success)
    {
        AcquireSRWLockExclusive(&m_lock);
        auto& perfs = m_formatStrategies[format];
        auto stratIdx = static_cast<uint8_t>(strategy);

        // Find or create entry
        StrategyPerformance* sp = nullptr;
        for (auto& p : perfs) {
            if (p.strategy == strategy) {
                sp = &p;
                break;
            }
        }
        if (!sp) {
            perfs.push_back({strategy, 0, 0.0, 1.0, 0.0});
            sp = &perfs.back();
        }

        sp->trialCount++;
        // Exponential moving average for latency
        double alpha = 2.0 / (sp->trialCount + 1);
        sp->avgLatencyMs = alpha * latencyMs + (1.0 - alpha) * sp->avgLatencyMs;
        // Success rate update
        sp->successRate = ((sp->successRate * (sp->trialCount - 1)) + (success ? 1.0 : 0.0)) / sp->trialCount;

        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Recommend best strategy for a format using UCB1 algorithm.
    StrategyRecommendation Recommend(const std::string& format) const
    {
        StrategyRecommendation rec;

        AcquireSRWLockShared(&m_lock);
        auto it = m_formatStrategies.find(format);
        if (it == m_formatStrategies.end() || it->second.empty()) {
            ReleaseSRWLockShared(&m_lock);
            rec.bestStrategy = DecodeStrategy::CPUSingleThread;
            rec.confidence = 0.0;
            return rec;
        }

        auto perfs = it->second;  // Copy while locked
        ReleaseSRWLockShared(&m_lock);

        // Compute UCB1 scores
        uint64_t totalTrials = 0;
        for (const auto& p : perfs)
            totalTrials += p.trialCount;
        double logTotal = std::log(static_cast<double>(totalTrials + 1));

        for (auto& p : perfs) {
            // Normalize latency to 0-1 reward (lower latency = higher reward)
            double reward = p.avgLatencyMs > 0 ? (1.0 / (1.0 + p.avgLatencyMs / 100.0)) : 0.5;
            reward *= p.successRate;  // Penalize failures
            double exploration = std::sqrt(2.0 * logTotal / (p.trialCount + 1));
            p.score = reward + exploration;
        }

        // Pick highest UCB1 score
        auto best = &perfs[0];
        for (auto& p : perfs) {
            if (p.score > best->score)
                best = &p;
        }

        rec.bestStrategy = best->strategy;
        rec.expectedLatencyMs = best->avgLatencyMs;
        rec.confidence = best->trialCount > 10 ? 0.9 : (best->trialCount / 10.0) * 0.9;
        rec.alternatives = perfs;

        m_stats.decisionsRendered++;
        return rec;
    }

    static const char* StrategyName(DecodeStrategy s)
    {
        switch (s) {
            case DecodeStrategy::CPUSingleThread:
                return "CPUSingleThread";
            case DecodeStrategy::CPUMultiThread:
                return "CPUMultiThread";
            case DecodeStrategy::GPUDirect:
                return "GPUDirect";
            case DecodeStrategy::GPUWithFallback:
                return "GPUWithFallback";
            case DecodeStrategy::MemoryMapped:
                return "MemoryMapped";
            case DecodeStrategy::Streaming:
                return "Streaming";
            case DecodeStrategy::Cached:
                return "Cached";
            default:
                return "Unknown";
        }
    }

    OptimizerStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable SRWLOCK m_lock{};
    std::unordered_map<std::string, std::vector<StrategyPerformance>> m_formatStrategies;
    mutable OptimizerStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
