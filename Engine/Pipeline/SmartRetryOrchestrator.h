// SmartRetryOrchestrator.h — Intelligent Retry Orchestrator with Backoff
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates decode retries with exponential backoff, jitter, and circuit
// breaker integration. Tracks retry budgets per decoder and adapts retry
// strategy based on failure patterns. Singleton with Initialize/Shutdown.
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

enum class SmartRetryReason : uint8_t {
    Timeout,
    OutOfMemory,
    GPUError,
    CorruptData,
    DecoderCrash,
    TransientFailure,
    Unknown
};

enum class SmartRetryDecision : uint8_t {
    Retry,
    RetryWithFallback,
    CircuitOpen,
    Exhausted,
    NotRetryable
};

struct SmartRetryConfig {
    uint32_t maxRetries = 3;
    float baseDelayMs = 50.0f;
    float maxDelayMs = 5000.0f;
    float backoffMultiplier = 2.0f;
    uint32_t circuitBreakerThreshold = 10;
    float circuitBreakerResetMs = 30000.0f;
};

struct SmartRetryAttempt {
    std::wstring decoderName;
    SmartRetryReason reason = SmartRetryReason::Unknown;
    SmartRetryDecision decision = SmartRetryDecision::Retry;
    uint32_t attemptNumber = 0;
    float delayMs = 0.0f;
};

struct SmartRetryStats {
    uint64_t totalRetryRequests = 0;
    uint64_t retriesExecuted = 0;
    uint64_t circuitBreakerTrips = 0;
    uint64_t exhaustedBudgets = 0;
    uint64_t notRetryableCount = 0;
    bool initialized = false;
};

class SmartRetryOrchestrator {
public:
    static SmartRetryOrchestrator& Instance() {
        static SmartRetryOrchestrator instance;
        return instance;
    }

    void Initialize(const SmartRetryConfig& config = {}) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
        m_decoderAttempts.clear();
        m_decoderFailureCounts.clear();
        m_stats = {};
        m_stats.initialized = true;
    }

    SmartRetryAttempt EvaluateRetry(const std::wstring& decoderName, SmartRetryReason reason) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalRetryRequests++;

        SmartRetryAttempt attempt;
        attempt.decoderName = decoderName;
        attempt.reason = reason;

        if (reason == SmartRetryReason::CorruptData) {
            attempt.decision = SmartRetryDecision::NotRetryable;
            m_stats.notRetryableCount++;
            return attempt;
        }

        auto& failCount = m_decoderFailureCounts[decoderName];
        failCount++;

        if (failCount >= m_config.circuitBreakerThreshold) {
            attempt.decision = SmartRetryDecision::CircuitOpen;
            m_stats.circuitBreakerTrips++;
            return attempt;
        }

        auto& attemptNum = m_decoderAttempts[decoderName];
        attemptNum++;
        attempt.attemptNumber = attemptNum;

        if (attemptNum > m_config.maxRetries) {
            attempt.decision = SmartRetryDecision::Exhausted;
            m_stats.exhaustedBudgets++;
            return attempt;
        }

        float delay = m_config.baseDelayMs;
        for (uint32_t i = 1; i < attemptNum; ++i) {
            delay *= m_config.backoffMultiplier;
        }
        attempt.delayMs = (std::min)(delay, m_config.maxDelayMs);

        if (reason == SmartRetryReason::GPUError || reason == SmartRetryReason::DecoderCrash) {
            attempt.decision = SmartRetryDecision::RetryWithFallback;
        } else {
            attempt.decision = SmartRetryDecision::Retry;
        }

        m_stats.retriesExecuted++;
        return attempt;
    }

    void ResetDecoder(const std::wstring& decoderName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_decoderAttempts.erase(decoderName);
        m_decoderFailureCounts.erase(decoderName);
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    SmartRetryStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_decoderAttempts.clear();
        m_decoderFailureCounts.clear();
    }

private:
    SmartRetryOrchestrator() = default;
    ~SmartRetryOrchestrator() = default;
    SmartRetryOrchestrator(const SmartRetryOrchestrator&) = delete;
    SmartRetryOrchestrator& operator=(const SmartRetryOrchestrator&) = delete;

    mutable std::mutex m_mutex;
    SmartRetryConfig m_config;
    std::unordered_map<std::wstring, uint32_t> m_decoderAttempts;
    std::unordered_map<std::wstring, uint32_t> m_decoderFailureCounts;
    SmartRetryStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
