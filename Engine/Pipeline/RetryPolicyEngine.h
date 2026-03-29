// RetryPolicyEngine.h — Silent Retry Policy Engine with Exponential Backoff + Jitter
// Copyright (c) 2026 ExplorerLens Project
//
// Configurable retry policy: exponential backoff with optional decorrelated jitter,
// per-operation retry budget, and result callbacks. Used by the decoder pipeline
// to silently absorb transient I/O and decode errors before surfacing them.
//
#pragma once
#include <functional>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class RetryResult { Success, ExhaustedRetries, PermanentFailure };

struct RetryConfig {
    int     maxRetries       = 3;
    double  baseDelayMs      = 50.0;
    double  maxDelayMs       = 5000.0;
    double  backoffMulti     = 2.0;
    bool    enableJitter     = true;
    double  jitterFactor     = 0.25;  // ±25% of computed delay
};

struct RetryEngineStats {
    int    totalAttempts    = 0;
    int    successCount     = 0;
    int    retryCount       = 0;
    int    exhaustedCount   = 0;
    double totalDelayMs     = 0.0;
};

class RetryPolicyEngine {
public:
    explicit RetryPolicyEngine(const RetryConfig& cfg = {}) : m_cfg(cfg) {}

    template<typename Fn>
    RetryResult Execute(Fn&& op, const std::string& /*tag*/ = {}) {
        int attempt = 0;
        while (true) {
            m_stats.totalAttempts++;
            bool ok = op();
            if (ok) { m_stats.successCount++; return RetryResult::Success; }
            attempt++;
            if (attempt > m_cfg.maxRetries) { m_stats.exhaustedCount++; return RetryResult::ExhaustedRetries; }
            m_stats.retryCount++;
            double delay = ComputeDelay(attempt);
            m_stats.totalDelayMs += delay;
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(delay));
        }
    }

    RetryConfig&       Config()          noexcept { return m_cfg; }
    const RetryConfig& Config()    const noexcept { return m_cfg; }
    const RetryEngineStats&  Stats()     const noexcept { return m_stats; }
    void               ResetStats()      noexcept { m_stats = {}; }
    int                MaxRetries() const noexcept { return m_cfg.maxRetries; }
    double             BaseDelay()  const noexcept { return m_cfg.baseDelayMs; }

private:
    double ComputeDelay(int attempt) {
        double d = m_cfg.baseDelayMs * std::pow(m_cfg.backoffMulti, attempt - 1);
        d = std::min(d, m_cfg.maxDelayMs);
        if (m_cfg.enableJitter) {
            std::uniform_real_distribution<double> dist(
                d * (1.0 - m_cfg.jitterFactor), d * (1.0 + m_cfg.jitterFactor));
            d = dist(m_rng);
        }
        return d;
    }

    RetryConfig  m_cfg;
    RetryEngineStats   m_stats;
    std::mt19937 m_rng{ std::random_device{}() };
};

} // namespace Engine
} // namespace ExplorerLens
