// DecodeRetryPolicy.h — Configurable Retry with Exponential Backoff
// Copyright (c) 2026 ExplorerLens Project
//
// Encapsulates retry logic for transient decode failures, categorized into
// six failure kinds (IOTimeout, GPUDeviceLost, MemoryPressure, CodecBusy,
// NetworkTimeout, FileLocked) each independently togglable. Supports four
// backoff strategies from fixed delay through exponential-with-jitter.
// Evaluate() returns a RetryDecision with computed delay, and cumulative
// stats are maintained for observability.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <atomic>
#include <cmath>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class DecodeFailureKind : uint32_t {
    IOTimeout = 0,
    GPUDeviceLost = 1,
    MemoryPressure = 2,
    CodecBusy = 3,
    NetworkTimeout = 4,
    FileLocked = 5,
    Unknown = 6,
    Count = 7
};

static const wchar_t* DecodeFailureKindName(DecodeFailureKind k) {
    static const wchar_t* names[] = {
        L"IOTimeout", L"GPUDeviceLost", L"MemoryPressure",
        L"CodecBusy", L"NetworkTimeout", L"FileLocked", L"Unknown"
    };
    auto idx = static_cast<uint32_t>(k);
    return (idx < static_cast<uint32_t>(DecodeFailureKind::Count)) ? names[idx] : L"Unknown";
}

enum class BackoffStrategy : uint32_t {
    Fixed = 0,  // Same delay every time
    Linear = 1,  // delay * attempt
    Exponential = 2,  // delay * 2^attempt
    ExponentialWithJitter = 3  // Exponential + random jitter
};

struct DecodeRetryConfig {
    uint32_t        maxAttempts = 3;
    uint32_t        baseDelayMs = 50;
    uint32_t        maxDelayMs = 5000;
    BackoffStrategy strategy = BackoffStrategy::ExponentialWithJitter;
    bool            retryOnIOTimeout = true;
    bool            retryOnGPULost = true;
    bool            retryOnMemoryPressure = true;
    bool            retryOnCodecBusy = true;
    bool            retryOnNetworkTimeout = true;
    bool            retryOnFileLocked = true;
};

struct RetryDecision {
    bool     shouldRetry = false;
    uint32_t delayMs = 0;
    uint32_t attemptNumber = 0;
    DecodeFailureKind failureKind = DecodeFailureKind::Unknown;
};

struct DecodeRetryStats {
    uint64_t totalAttempts = 0;
    uint64_t totalRetries = 0;
    uint64_t totalExhausted = 0;
    uint64_t totalSuccessAfterRetry = 0;
    uint64_t retriesByKind[static_cast<size_t>(DecodeFailureKind::Count)] = {};
};

// ========================================================================
// DecodeRetryPolicy — Retry logic with backoff for decode pipeline
// ========================================================================
class DecodeRetryPolicy {
public:
    static DecodeRetryPolicy& Instance() {
        static DecodeRetryPolicy instance;
        return instance;
    }

    void Initialize(const DecodeRetryConfig& config = {}) {
        m_config = config;
        ResetStats();
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Check if a failure kind is retryable
    bool IsRetryable(DecodeFailureKind kind) const {
        switch (kind) {
        case DecodeFailureKind::IOTimeout:       return m_config.retryOnIOTimeout;
        case DecodeFailureKind::GPUDeviceLost:   return m_config.retryOnGPULost;
        case DecodeFailureKind::MemoryPressure:  return m_config.retryOnMemoryPressure;
        case DecodeFailureKind::CodecBusy:       return m_config.retryOnCodecBusy;
        case DecodeFailureKind::NetworkTimeout:   return m_config.retryOnNetworkTimeout;
        case DecodeFailureKind::FileLocked:      return m_config.retryOnFileLocked;
        default: return false;
        }
    }

    // Evaluate whether to retry
    RetryDecision Evaluate(DecodeFailureKind kind, uint32_t currentAttempt) {
        m_stats.totalAttempts++;
        RetryDecision decision;
        decision.failureKind = kind;
        decision.attemptNumber = currentAttempt;

        if (!IsRetryable(kind) || currentAttempt >= m_config.maxAttempts) {
            decision.shouldRetry = false;
            if (currentAttempt >= m_config.maxAttempts) {
                m_stats.totalExhausted++;
            }
            return decision;
        }

        decision.shouldRetry = true;
        decision.delayMs = ComputeDelay(currentAttempt);
        m_stats.totalRetries++;
        m_stats.retriesByKind[static_cast<size_t>(kind)]++;

        return decision;
    }

    // Record success after retry
    void RecordRetrySuccess() {
        m_stats.totalSuccessAfterRetry++;
    }

    // Compute delay for a given attempt
    uint32_t ComputeDelay(uint32_t attempt) const {
        uint32_t delay = 0;
        switch (m_config.strategy) {
        case BackoffStrategy::Fixed:
            delay = m_config.baseDelayMs;
            break;
        case BackoffStrategy::Linear:
            delay = m_config.baseDelayMs * (attempt + 1);
            break;
        case BackoffStrategy::Exponential:
            delay = m_config.baseDelayMs * (1u << attempt);
            break;
        case BackoffStrategy::ExponentialWithJitter: {
            uint32_t expDelay = m_config.baseDelayMs * (1u << attempt);
            // Simple jitter: +/- 25%
            uint32_t jitter = (GetTickCount() % (expDelay / 4 + 1));
            delay = expDelay + jitter;
            break;
        }
        }
        return (std::min)(delay, m_config.maxDelayMs);
    }

    // Get config
    const DecodeRetryConfig& GetConfig() const { return m_config; }

    // Get stats
    DecodeRetryStats GetStats() const { return m_stats; }

    void ResetStats() {
        m_stats = {};
    }

    // Get max attempts
    uint32_t GetMaxAttempts() const { return m_config.maxAttempts; }

private:
    DecodeRetryPolicy() = default;

    DecodeRetryConfig m_config;
    DecodeRetryStats  m_stats;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
