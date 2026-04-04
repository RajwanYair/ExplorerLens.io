// ErrorRecoveryEngineV2.h — Automated error recovery with retry, backoff, fallback
// Copyright (c) 2026 ExplorerLens Project
//
// Provides RecoveryStrategyV2 enum, RecoveryRetryPolicy, RecoveryAction, and
// ErrorRecoveryEngineV2 that executes operations with automatic retry,
// exponential backoff with jitter, and configurable fallback strategies.
//
#pragma once

#include <windows.h>
#include <chrono>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include "ResultType.h"
#include "StructuredErrorDomain.h"

namespace ExplorerLens {
namespace Engine {

/// Recovery strategy to apply when an error matches a pattern
enum class RecoveryStrategyV2 : uint8_t {
    None = 0,              // No recovery — propagate error as-is
    Retry = 1,             // Retry immediately
    RetryWithBackoff = 2,  // Retry with exponential backoff + jitter
    Fallback = 3,          // Use a fallback operation
    Skip = 4,              // Skip and continue (return default)
    Abort = 5,             // Abort the pipeline
    COUNT = 6
};

/// Human-readable name for RecoveryStrategyV2
inline const char* RecoveryStrategyV2Name(RecoveryStrategyV2 s) noexcept
{
    switch (s) {
        case RecoveryStrategyV2::None:
            return "None";
        case RecoveryStrategyV2::Retry:
            return "Retry";
        case RecoveryStrategyV2::RetryWithBackoff:
            return "RetryWithBackoff";
        case RecoveryStrategyV2::Fallback:
            return "Fallback";
        case RecoveryStrategyV2::Skip:
            return "Skip";
        case RecoveryStrategyV2::Abort:
            return "Abort";
        default:
            return "Unknown";
    }
}

/// Configurable retry policy for backoff strategies
struct RecoveryRetryPolicy
{
    uint32_t maxAttempts = 3;        // Maximum number of retry attempts
    uint32_t baseDelayMs = 100;      // Base delay in milliseconds
    double backoffMultiplier = 2.0;  // Exponential backoff multiplier
    double jitterFraction = 0.25;    // Random jitter as fraction of delay (0.0 to 1.0)
    uint32_t maxDelayMs = 10000;     // Maximum delay cap in milliseconds

    /// Calculate delay for a given attempt (0-based), including jitter
    uint32_t CalculateDelayMs(uint32_t attempt) const noexcept
    {
        double delay = static_cast<double>(baseDelayMs);
        for (uint32_t i = 0; i < attempt; ++i) {
            delay *= backoffMultiplier;
        }
        // Cap to max delay
        if (delay > static_cast<double>(maxDelayMs)) {
            delay = static_cast<double>(maxDelayMs);
        }
        // Add jitter
        if (jitterFraction > 0.0) {
            thread_local std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<double> dist(-jitterFraction, jitterFraction);
            double jitter = delay * dist(rng);
            delay += jitter;
            if (delay < 0.0)
                delay = 0.0;
        }
        return static_cast<uint32_t>(delay);
    }
};

/// Pairs an error-matching predicate with a recovery strategy
struct ErrorRecoveryAction
{
    std::string name;                                     // Descriptive name for logging
    std::function<bool(const StructuredError&)> matcher;  // Returns true if this action applies
    RecoveryStrategyV2 strategy = RecoveryStrategyV2::None;
    RecoveryRetryPolicy retryPolicy;  // Used if strategy involves retry

    /// Convenience: match by domain
    static ErrorRecoveryAction ForDomain(StructuredErrorDomain domain, RecoveryStrategyV2 strategy,
                                         RecoveryRetryPolicy policy = {})
    {
        ErrorRecoveryAction action;
        action.name = std::string("Domain:") + StructuredErrorDomainName(domain);
        action.matcher = [domain](const StructuredError& err) {
            return err.GetDomain() == domain;
        };
        action.strategy = strategy;
        action.retryPolicy = policy;
        return action;
    }

    /// Convenience: match by HRESULT code
    static ErrorRecoveryAction ForHResult(HRESULT code, RecoveryStrategyV2 strategy, RecoveryRetryPolicy policy = {})
    {
        ErrorRecoveryAction action;
        action.name = "HRESULT:0x" + std::to_string(static_cast<uint32_t>(code));
        action.matcher = [code](const StructuredError& err) {
            return err.GetCode() == code;
        };
        action.strategy = strategy;
        action.retryPolicy = policy;
        return action;
    }
};

/// Statistics for recovery operations
struct RecoveryStats
{
    uint64_t totalAttempts = 0;   // Total operations attempted
    uint64_t totalRetries = 0;    // Total retries across all operations
    uint64_t totalSuccesses = 0;  // Successful recoveries
    uint64_t totalFailures = 0;   // Operations that exhausted retries
    uint64_t totalSkipped = 0;    // Operations skipped
    uint64_t totalAborted = 0;    // Operations aborted
    uint64_t totalFallbacks = 0;  // Fallback operations invoked

    void Reset() noexcept
    {
        totalAttempts = totalRetries = totalSuccesses = 0;
        totalFailures = totalSkipped = totalAborted = totalFallbacks = 0;
    }
};

/// Executes operations with automatic recovery (retry, backoff, fallback).
/// Thread-safe: each call to Execute is self-contained; stats are approximate
/// under concurrent access (acceptable for diagnostics).
class ErrorRecoveryEngineV2
{
  public:
    ErrorRecoveryEngineV2() = default;

    /// Register a recovery action (checked in order of registration)
    void RegisterAction(const ErrorRecoveryAction& action)
    {
        m_actions.push_back(action);
    }

    /// Clear all registered actions
    void ClearActions() noexcept
    {
        m_actions.clear();
    }

    /// Number of registered actions
    size_t GetActionCount() const noexcept
    {
        return m_actions.size();
    }

    /// Execute an operation with automatic recovery.
    /// @param operation  Function that returns Result<T> — the primary operation
    /// @param fallback   Optional fallback that returns Result<T> (used for Fallback strategy)
    /// @return The final Result after recovery attempts
    template <typename T>
    Result<T> Execute(std::function<Result<T>()> operation, std::function<Result<T>()> fallback = nullptr)
    {
        ++m_stats.totalAttempts;

        // First attempt
        auto result = operation();
        if (result.IsOk()) {
            ++m_stats.totalSuccesses;
            return result;
        }

        // Find matching recovery action
        const ErrorRecoveryAction* matchedAction = FindMatchingAction(result.Error());
        if (!matchedAction) {
            ++m_stats.totalFailures;
            return result;
        }

        switch (matchedAction->strategy) {
            case RecoveryStrategyV2::None:
                ++m_stats.totalFailures;
                return result;

            case RecoveryStrategyV2::Retry:
                return ExecuteRetry<T>(operation, matchedAction->retryPolicy, false);

            case RecoveryStrategyV2::RetryWithBackoff:
                return ExecuteRetry<T>(operation, matchedAction->retryPolicy, true);

            case RecoveryStrategyV2::Fallback:
                if (fallback) {
                    ++m_stats.totalFallbacks;
                    auto fallbackResult = fallback();
                    if (fallbackResult.IsOk())
                        ++m_stats.totalSuccesses;
                    else
                        ++m_stats.totalFailures;
                    return fallbackResult;
                }
                ++m_stats.totalFailures;
                return result;

            case RecoveryStrategyV2::Skip:
                ++m_stats.totalSkipped;
                return result;

            case RecoveryStrategyV2::Abort:
                ++m_stats.totalAborted;
                return result;

            default:
                ++m_stats.totalFailures;
                return result;
        }
    }

    /// Get current recovery statistics
    const RecoveryStats& GetStats() const noexcept
    {
        return m_stats;
    }

    /// Reset statistics
    void ResetStats() noexcept
    {
        m_stats.Reset();
    }

  private:
    /// Find the first matching recovery action for the given error
    const ErrorRecoveryAction* FindMatchingAction(const StructuredError& error) const
    {
        for (const auto& action : m_actions) {
            if (action.matcher && action.matcher(error)) {
                return &action;
            }
        }
        return nullptr;
    }

    /// Execute with retry (optionally with backoff)
    template <typename T>
    Result<T> ExecuteRetry(std::function<Result<T>()>& operation, const RecoveryRetryPolicy& policy, bool useBackoff)
    {
        for (uint32_t attempt = 0; attempt < policy.maxAttempts; ++attempt) {
            ++m_stats.totalRetries;

            if (useBackoff && attempt > 0) {
                uint32_t delayMs = policy.CalculateDelayMs(attempt - 1);
                if (delayMs > 0) {
                    ::Sleep(delayMs);
                }
            }

            auto retryResult = operation();
            if (retryResult.IsOk()) {
                ++m_stats.totalSuccesses;
                return retryResult;
            }
        }

        // All retries exhausted — return the last failure
        ++m_stats.totalFailures;
        auto lastResult = operation();
        return lastResult;
    }

    std::vector<ErrorRecoveryAction> m_actions;
    RecoveryStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
