//==============================================================================
// ExplorerLens Engine — Cancellable decode context (Sprint S242)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 A15 — std::jthread + std::stop_token for decode cancel.
//==============================================================================
//
// Background:
//   Windows Explorer can abandon a thumbnail request (user scrolls away,
//   folder closes, file deleted). Today decoders run to completion and throw
//   the result away — wasted CPU + I/O. This header scaffolds a cooperative
//   cancellation context that decoders poll at safe points (stage boundaries,
//   row loops, GPU upload chunks).
//
// Design:
//   * Header-only, no dependencies beyond <stop_token>/<atomic>.
//   * DecodeCancelToken is move-only; decoders take it by reference.
//   * `IsCancelled()` is a cheap atomic load — safe in tight loops.
//   * Deadline-based `IsTimeExpired(now)` allows per-decoder budgets.
//   * Does NOT require C++23 — uses C++20 std::stop_token.
//==============================================================================
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <stop_token>

namespace ExplorerLens {
namespace Engine {

/// <summary>
/// Reason codes reported when a decode is aborted — used by ETW and
/// FormatStatusProvider so the degrading-decoder detector can tell the
/// difference between "user cancelled" and "decoder gave up".
/// </summary>
enum class DecodeCancelReason : std::uint8_t
{
    NONE          = 0,   // still running
    USER          = 1,   // explicit cancel from Explorer
    TIMEOUT       = 2,   // per-format decode budget expired
    SHUTDOWN      = 3,   // DLL_PROCESS_DETACH / extension unload
    MEMORY_BUDGET = 4,   // memory pressure controller pre-empted us
    UPSTREAM      = 5    // parent stop_source requested stop
};

/// <summary>
/// Cancellation token threaded into the 9-stage pipeline. Construct with a
/// stop_token (from a jthread or manual stop_source) and optional deadline.
/// Callers poll <c>IsCancelled</c> at safe points.
/// </summary>
class DecodeCancelToken
{
  public:
    using clock      = std::chrono::steady_clock;
    using time_point = clock::time_point;

    DecodeCancelToken() noexcept = default;

    explicit DecodeCancelToken(std::stop_token token,
                               time_point deadline = time_point::max()) noexcept
        : m_token(std::move(token)), m_deadline(deadline)
    {}

    /// <summary>Fast path — single atomic load + compare.</summary>
    bool IsCancelled() const noexcept
    {
        if (m_reason.load(std::memory_order_relaxed) != DecodeCancelReason::NONE)
            return true;

        if (m_token.stop_requested())
        {
            const_cast<DecodeCancelToken*>(this)->TrySetReason(DecodeCancelReason::UPSTREAM);
            return true;
        }
        return false;
    }

    /// <summary>Deadline check — cheap monotonic clock compare.</summary>
    bool IsTimeExpired(time_point now = clock::now()) const noexcept
    {
        if (m_deadline == time_point::max()) return false;
        const bool expired = now >= m_deadline;
        if (expired)
            const_cast<DecodeCancelToken*>(this)->TrySetReason(DecodeCancelReason::TIMEOUT);
        return expired;
    }

    /// <summary>Explicit cancel from the host side.</summary>
    void Cancel(DecodeCancelReason reason) noexcept { TrySetReason(reason); }

    DecodeCancelReason Reason() const noexcept
    {
        return m_reason.load(std::memory_order_acquire);
    }

  private:
    void TrySetReason(DecodeCancelReason reason) noexcept
    {
        DecodeCancelReason expected = DecodeCancelReason::NONE;
        m_reason.compare_exchange_strong(expected, reason, std::memory_order_release);
    }

    std::stop_token                 m_token;
    time_point                      m_deadline = time_point::max();
    std::atomic<DecodeCancelReason> m_reason{DecodeCancelReason::NONE};
};

} // namespace Engine
} // namespace ExplorerLens
