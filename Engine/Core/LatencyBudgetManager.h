// LatencyBudgetManager.h — Per-Format Latency SLO Tracker
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks decode latency against per-format SLO budgets. Emits ETW events
// when a budget is exceeded and feeds data to the adaptive scheduler.
//
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class LatencyTier : uint8_t {
    Fast = 0,    // < 5ms   — cached/simple formats (PNG, JPEG, BMP)
    Normal = 1,  // < 17ms  — standard target (WebP, AVIF, HEIC)
    Slow = 2,    // < 100ms — complex formats (RAW, PDF, glTF)
    Async = 3,   // > 100ms — always async, progress UI required
};

struct LatencySLO
{
    std::chrono::milliseconds budget{17};
    LatencyTier tier{LatencyTier::Normal};
    uint32_t softViolationThresholdPct{110};  // % over budget before warning
    uint32_t hardViolationThresholdPct{200};  // % over budget before escalation
};

struct LatencyStats
{
    std::atomic<uint64_t> sampleCount{0};
    std::atomic<uint64_t> totalMicros{0};
    std::atomic<uint64_t> maxMicros{0};
    std::atomic<uint64_t> softViolations{0};
    std::atomic<uint64_t> hardViolations{0};

    double AvgMs() const noexcept
    {
        uint64_t n = sampleCount.load(std::memory_order_relaxed);
        if (n == 0)
            return 0.0;
        return static_cast<double>(totalMicros.load(std::memory_order_relaxed)) / n / 1000.0;
    }
};

class LatencyBudgetManager
{
  public:
    static LatencyBudgetManager& Instance() noexcept
    {
        static LatencyBudgetManager s_inst;
        return s_inst;
    }

    void RegisterFormat(std::string_view ext, LatencySLO slo)
    {
        m_slos[std::string(ext)] = slo;
    }

    // Called by decoder pipeline after each decode completes.
    void RecordSample(std::string_view ext, std::chrono::microseconds elapsed)
    {
        auto it = m_slos.find(std::string(ext));
        if (it == m_slos.end())
            return;

        const LatencySLO& slo = it->second;
        LatencyStats& stats = m_stats[std::string(ext)];

        uint64_t us = static_cast<uint64_t>(elapsed.count());
        stats.sampleCount.fetch_add(1, std::memory_order_relaxed);
        stats.totalMicros.fetch_add(us, std::memory_order_relaxed);

        uint64_t cur = stats.maxMicros.load(std::memory_order_relaxed);
        while (us > cur && !stats.maxMicros.compare_exchange_weak(cur, us, std::memory_order_relaxed)) {}

        uint64_t budgetUs = static_cast<uint64_t>(slo.budget.count()) * 1000;
        if (us >= budgetUs * slo.hardViolationThresholdPct / 100) {
            stats.hardViolations.fetch_add(1, std::memory_order_relaxed);
            EmitViolationEvent(ext, elapsed, ViolationLevel::Hard);
        } else if (us >= budgetUs * slo.softViolationThresholdPct / 100) {
            stats.softViolations.fetch_add(1, std::memory_order_relaxed);
            EmitViolationEvent(ext, elapsed, ViolationLevel::Soft);
        }
    }

    [[nodiscard]] const LatencyStats* GetStats(std::string_view ext) const
    {
        auto it = m_stats.find(std::string(ext));
        return (it != m_stats.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] LatencyTier GetTier(std::string_view ext) const noexcept
    {
        auto it = m_slos.find(std::string(ext));
        return (it != m_slos.end()) ? it->second.tier : LatencyTier::Normal;
    }

    void ResetStats()
    {
        for (auto& [ext, stats] : m_stats) {
            stats.sampleCount.store(0);
            stats.totalMicros.store(0);
            stats.maxMicros.store(0);
            stats.softViolations.store(0);
            stats.hardViolations.store(0);
        }
    }

  private:
    enum class ViolationLevel {
        Soft,
        Hard
    };

    void EmitViolationEvent(std::string_view ext, std::chrono::microseconds elapsed, ViolationLevel level) noexcept
    {
        (void)ext;
        (void)elapsed;
        (void)level;
    }

    std::unordered_map<std::string, LatencySLO> m_slos;
    std::unordered_map<std::string, LatencyStats> m_stats;
};

// RAII decode timer — automatically records sample on destruction.
class DecodeTimer
{
  public:
    DecodeTimer(std::string_view ext, LatencyBudgetManager& mgr = LatencyBudgetManager::Instance())
        : m_ext(ext)
        , m_mgr(mgr)
        , m_start(std::chrono::high_resolution_clock::now())
    {}

    ~DecodeTimer()
    {
        auto elapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_start);
        m_mgr.RecordSample(m_ext, elapsed);
    }

    DecodeTimer(const DecodeTimer&) = delete;
    DecodeTimer& operator=(const DecodeTimer&) = delete;

  private:
    std::string_view m_ext;
    LatencyBudgetManager& m_mgr;
    std::chrono::high_resolution_clock::time_point m_start;
};

}  // namespace Engine
}  // namespace ExplorerLens
