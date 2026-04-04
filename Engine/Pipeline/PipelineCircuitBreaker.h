// PipelineCircuitBreaker.h — Pipeline-Level Aggregate Circuit Breaker
// Copyright (c) 2026 ExplorerLens Project
//
// Implements Closed -> Open -> HalfOpen circuit-breaker semantics at the
// aggregate pipeline level. A sliding window tracks the failure rate; once
// it exceeds a configurable threshold the breaker trips, rejecting all
// incoming requests for a cooldown period before allowing a small number
// of probe requests. Successful probes close the circuit; any probe failure
// re-trips it.
//
// Thread-safe singleton via atomics.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

// Circuit breaker states (Pipeline-scoped to avoid conflict with DecoderCircuitBreaker)
enum class PipelineCircuitState : uint32_t {
    Closed = 0,   // Normal operation — requests flow through
    Open = 1,     // Tripped — all requests rejected immediately
    HalfOpen = 2  // Testing — limited requests allowed to probe recovery
};

static const wchar_t* PipelineCircuitStateName(PipelineCircuitState s)
{
    switch (s) {
        case PipelineCircuitState::Closed:
            return L"Closed";
        case PipelineCircuitState::Open:
            return L"Open";
        case PipelineCircuitState::HalfOpen:
            return L"HalfOpen";
        default:
            return L"Unknown";
    }
}

struct PipelineBreakerConfig
{
    double failureRateThreshold = 0.50;  // Trip at 50% failure rate
    uint32_t minimumRequests = 10;       // Minimum requests before evaluating
    uint32_t openDurationMs = 30000;     // Stay open for 30s before half-open
    uint32_t halfOpenMaxRequests = 3;    // Max probes in half-open
    double recoveryThreshold = 0.80;     // Success rate to close from half-open
    uint32_t slidingWindowSize = 100;    // Window size for rate calculation
};

struct PipelineCircuitBreakerStatus
{
    PipelineCircuitState state = PipelineCircuitState::Closed;
    uint64_t totalRequests = 0;
    uint64_t totalFailures = 0;
    double currentFailureRate = 0.0;
    uint32_t tripCount = 0;
    uint64_t lastTripTimestamp = 0;
    uint64_t lastRecoveryTimestamp = 0;
    uint32_t halfOpenProbes = 0;
};

// ========================================================================
// PipelineCircuitBreaker — Aggregate failure protection
// ========================================================================
class PipelineCircuitBreaker
{
  public:
    static PipelineCircuitBreaker& Instance()
    {
        static PipelineCircuitBreaker instance;
        return instance;
    }

    void Initialize(const PipelineBreakerConfig& config = {})
    {
        m_config = config;
        Reset();
        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

    // Check if a request should be allowed
    bool AllowRequest()
    {
        PipelineCircuitState state = static_cast<PipelineCircuitState>(m_state.load(std::memory_order_acquire));

        switch (state) {
            case PipelineCircuitState::Closed:
                return true;

            case PipelineCircuitState::Open: {
                // Check if enough time has passed to try half-open
                uint64_t now = GetTickCount64();
                uint64_t tripped = m_lastTripTime.load(std::memory_order_relaxed);
                if (now - tripped >= m_config.openDurationMs) {
                    // Transition to half-open
                    uint32_t expected = static_cast<uint32_t>(PipelineCircuitState::Open);
                    if (m_state.compare_exchange_strong(expected, static_cast<uint32_t>(PipelineCircuitState::HalfOpen),
                                                        std::memory_order_acq_rel)) {
                        m_halfOpenProbes.store(0, std::memory_order_relaxed);
                    }
                    return true;
                }
                return false;
            }

            case PipelineCircuitState::HalfOpen: {
                uint32_t probes = m_halfOpenProbes.fetch_add(1, std::memory_order_relaxed);
                return probes < m_config.halfOpenMaxRequests;
            }

            default:
                return false;
        }
    }

    // Record a successful request
    void RecordSuccess()
    {
        m_windowSuccesses.fetch_add(1, std::memory_order_relaxed);
        m_windowTotal.fetch_add(1, std::memory_order_relaxed);
        m_totalRequests.fetch_add(1, std::memory_order_relaxed);

        PipelineCircuitState state = static_cast<PipelineCircuitState>(m_state.load(std::memory_order_acquire));
        if (state == PipelineCircuitState::HalfOpen) {
            TryRecover();
        }

        MaybeRollWindow();
    }

    // Record a failed request
    void RecordFailure()
    {
        m_windowFailures.fetch_add(1, std::memory_order_relaxed);
        m_windowTotal.fetch_add(1, std::memory_order_relaxed);
        m_totalRequests.fetch_add(1, std::memory_order_relaxed);
        m_totalFailures.fetch_add(1, std::memory_order_relaxed);

        PipelineCircuitState state = static_cast<PipelineCircuitState>(m_state.load(std::memory_order_acquire));
        if (state == PipelineCircuitState::HalfOpen) {
            TripOpen();
        } else if (state == PipelineCircuitState::Closed) {
            EvaluateTrip();
        }

        MaybeRollWindow();
    }

    // Get current state
    PipelineCircuitState GetState() const
    {
        return static_cast<PipelineCircuitState>(m_state.load(std::memory_order_acquire));
    }

    // Force close (manual recovery)
    void ForceClose()
    {
        m_state.store(static_cast<uint32_t>(PipelineCircuitState::Closed), std::memory_order_release);
        m_lastRecoveryTime.store(GetTickCount64(), std::memory_order_relaxed);
        m_windowFailures.store(0, std::memory_order_relaxed);
        m_windowSuccesses.store(0, std::memory_order_relaxed);
        m_windowTotal.store(0, std::memory_order_relaxed);
    }

    // Get full status
    PipelineCircuitBreakerStatus GetStatus() const
    {
        PipelineCircuitBreakerStatus status;
        status.state = GetState();
        status.totalRequests = m_totalRequests.load(std::memory_order_relaxed);
        status.totalFailures = m_totalFailures.load(std::memory_order_relaxed);
        status.tripCount = m_tripCount.load(std::memory_order_relaxed);
        status.lastTripTimestamp = m_lastTripTime.load(std::memory_order_relaxed);
        status.lastRecoveryTimestamp = m_lastRecoveryTime.load(std::memory_order_relaxed);
        status.halfOpenProbes = m_halfOpenProbes.load(std::memory_order_relaxed);

        uint64_t wTotal = m_windowTotal.load(std::memory_order_relaxed);
        uint64_t wFail = m_windowFailures.load(std::memory_order_relaxed);
        status.currentFailureRate = (wTotal > 0) ? (static_cast<double>(wFail) / static_cast<double>(wTotal)) : 0.0;

        return status;
    }

    // Reset everything
    void Reset()
    {
        m_state.store(static_cast<uint32_t>(PipelineCircuitState::Closed), std::memory_order_release);
        m_totalRequests.store(0, std::memory_order_relaxed);
        m_totalFailures.store(0, std::memory_order_relaxed);
        m_windowTotal.store(0, std::memory_order_relaxed);
        m_windowFailures.store(0, std::memory_order_relaxed);
        m_windowSuccesses.store(0, std::memory_order_relaxed);
        m_tripCount.store(0, std::memory_order_relaxed);
        m_halfOpenProbes.store(0, std::memory_order_relaxed);
        m_lastTripTime.store(0, std::memory_order_relaxed);
        m_lastRecoveryTime.store(0, std::memory_order_relaxed);
    }

  private:
    PipelineCircuitBreaker() = default;

    void EvaluateTrip()
    {
        uint64_t wTotal = m_windowTotal.load(std::memory_order_relaxed);
        if (wTotal < m_config.minimumRequests)
            return;

        uint64_t wFail = m_windowFailures.load(std::memory_order_relaxed);
        double rate = static_cast<double>(wFail) / static_cast<double>(wTotal);
        if (rate >= m_config.failureRateThreshold) {
            TripOpen();
        }
    }

    void TripOpen()
    {
        uint32_t expected = m_state.load(std::memory_order_relaxed);
        uint32_t openVal = static_cast<uint32_t>(PipelineCircuitState::Open);
        if (expected != openVal) {
            if (m_state.compare_exchange_strong(expected, openVal, std::memory_order_acq_rel)) {
                m_tripCount.fetch_add(1, std::memory_order_relaxed);
                m_lastTripTime.store(GetTickCount64(), std::memory_order_relaxed);
            }
        }
    }

    void TryRecover()
    {
        uint32_t probes = m_halfOpenProbes.load(std::memory_order_relaxed);
        if (probes < m_config.halfOpenMaxRequests)
            return;

        uint64_t wSucc = m_windowSuccesses.load(std::memory_order_relaxed);
        double successRate = static_cast<double>(wSucc) / static_cast<double>(probes);
        if (successRate >= m_config.recoveryThreshold) {
            ForceClose();
        }
    }

    void MaybeRollWindow()
    {
        uint64_t wTotal = m_windowTotal.load(std::memory_order_relaxed);
        if (wTotal >= m_config.slidingWindowSize * 2) {
            // Halve the window
            m_windowTotal.store(wTotal / 2, std::memory_order_relaxed);
            uint64_t wFail = m_windowFailures.load(std::memory_order_relaxed);
            m_windowFailures.store(wFail / 2, std::memory_order_relaxed);
            uint64_t wSucc = m_windowSuccesses.load(std::memory_order_relaxed);
            m_windowSuccesses.store(wSucc / 2, std::memory_order_relaxed);
        }
    }

    PipelineBreakerConfig m_config;
    std::atomic<uint32_t> m_state{static_cast<uint32_t>(PipelineCircuitState::Closed)};
    std::atomic<uint64_t> m_totalRequests{0};
    std::atomic<uint64_t> m_totalFailures{0};
    std::atomic<uint64_t> m_windowTotal{0};
    std::atomic<uint64_t> m_windowFailures{0};
    std::atomic<uint64_t> m_windowSuccesses{0};
    std::atomic<uint32_t> m_tripCount{0};
    std::atomic<uint32_t> m_halfOpenProbes{0};
    std::atomic<uint64_t> m_lastTripTime{0};
    std::atomic<uint64_t> m_lastRecoveryTime{0};
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
