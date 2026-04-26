// Engine/Core/AsyncDecodeToken.h
// ExplorerLens — Cancel-aware decode token (H36 / ROADMAP v7.0 Phase 2)
// Sprint S307 — std::stop_token wrapper for cooperative decode cancellation.
//
// Cooperative cancellation protocol:
//   1. Caller creates AsyncDecodeToken; passes to decode pipeline.
//   2. Decode stages periodically call IsCancellationRequested().
//   3. Caller calls RequestCancellation() to signal stop.
//   4. On true, decode stage returns DecodeResult::Cancelled early.
//
// Phase 2 wiring:
//   - DecodePipeline will accept AsyncDecodeToken& and propagate it.
//   - Replaces ad-hoc bool* cancel flags currently used in ~12 decode paths.
//
// NOTE: Requires C++23 (/std:c++latest, ADR A23). std::stop_token /
//       std::stop_source were standardised in C++20 but this file uses the
//       C++23 jthread-compatible overloads and [[nodiscard]] on every getter.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_ASYNC_DECODE_TOKEN_H
#define EXPLORERLENS_ENGINE_ASYNC_DECODE_TOKEN_H

#include <stop_token>   // std::stop_source, std::stop_token (C++20/23)
#include <atomic>
#include <cstdint>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// AsyncDecodeToken
// ---------------------------------------------------------------------------
// Lightweight wrapper around std::stop_source / std::stop_token that carries
// additional decode-context metadata (request ID, priority hint) alongside
// the cancellation signal.
//
// Ownership model:
//   - The *initiating* call site holds the AsyncDecodeToken (owns stop_source).
//   - Each pipeline stage receives a DecodeTokenView (read-only, no ownership).
//
class AsyncDecodeToken final {
public:
    // Default-construct: cancellation not yet requested.
    AsyncDecodeToken() noexcept
        : m_source{}
        , m_requestId{ s_nextId.fetch_add(1u, std::memory_order_relaxed) }
        , m_priorityHint{ 0 }
    {}

    // Non-copyable (owns the stop_source).
    AsyncDecodeToken(const AsyncDecodeToken&)            = delete;
    AsyncDecodeToken& operator=(const AsyncDecodeToken&) = delete;

    // Movable.
    AsyncDecodeToken(AsyncDecodeToken&&)            noexcept = default;
    AsyncDecodeToken& operator=(AsyncDecodeToken&&) noexcept = default;

    ~AsyncDecodeToken() = default;

    // ── Cancellation ─────────────────────────────────────────────────────────

    /// Signal all downstream stages to abort decode at their next check point.
    void RequestCancellation() noexcept
    {
        m_source.request_stop();
    }

    /// True once RequestCancellation() has been called.
    [[nodiscard]] bool IsCancellationRequested() const noexcept
    {
        return m_source.get_token().stop_requested();
    }

    // ── Token / source accessors ──────────────────────────────────────────────

    /// Obtain a lightweight std::stop_token to pass into pipeline stages.
    /// The token remains valid for the lifetime of this AsyncDecodeToken.
    [[nodiscard]] std::stop_token GetToken() const noexcept
    {
        return m_source.get_token();
    }

    [[nodiscard]] const std::stop_source& StopSource() const noexcept
    {
        return m_source;
    }

    // ── Metadata ──────────────────────────────────────────────────────────────

    /// Monotonically-increasing decode request ID (process-wide).
    [[nodiscard]] std::uint64_t RequestId() const noexcept { return m_requestId; }

    /// Optional priority hint (0 = normal, >0 = elevated, <0 = background).
    [[nodiscard]] int PriorityHint() const noexcept { return m_priorityHint; }
    void SetPriorityHint(int hint) noexcept { m_priorityHint = hint; }

    // ── Factory ───────────────────────────────────────────────────────────────

    /// Create a pre-cancelled token (useful for unit tests that exercise the
    /// early-exit paths without running real decode work).
    [[nodiscard]] static AsyncDecodeToken AlreadyCancelled() noexcept
    {
        AsyncDecodeToken t;
        t.RequestCancellation();
        return t;
    }

private:
    std::stop_source        m_source;
    std::uint64_t           m_requestId;
    int                     m_priorityHint;

    inline static std::atomic<std::uint64_t> s_nextId{ 1u };
};

// ---------------------------------------------------------------------------
// DecodeTokenView
// ---------------------------------------------------------------------------
// Non-owning, read-only view passed into pipeline stages.  Holds a copy of
// the std::stop_token (cheap: internally a shared_ptr ref-count bump).
//
struct DecodeTokenView final {
    DecodeTokenView() noexcept = default;

    explicit DecodeTokenView(const AsyncDecodeToken& tok) noexcept
        : stopToken{ tok.GetToken() }
        , requestId{ tok.RequestId() }
        , priorityHint{ tok.PriorityHint() }
    {}

    [[nodiscard]] bool IsCancellationRequested() const noexcept
    {
        return stopToken.stop_requested();
    }

    std::stop_token  stopToken{};
    std::uint64_t    requestId{};
    int              priorityHint{};
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_ASYNC_DECODE_TOKEN_H
