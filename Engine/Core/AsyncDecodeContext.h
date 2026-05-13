// Engine/Core/AsyncDecodeContext.h
// ExplorerLens — Unified async decode context: connects Phase 2 pipeline components
// Sprint S339.
//
// Purpose:
//   Phase 2 introduced multiple independent subsystems:
//     - DecodeTimeoutGuard  (S322) — per-operation timeout + stop_token
//     - PerFormatMemoryBudget (S323) — per-format heap budget
//     - DecodeErrorTracker  (S326) — permanent-fail circuit breaker
//     - ThumbnailPlaceholderBroker (S329) — placeholder delivery
//
//   These subsystems currently have no shared context; each decode call must
//   construct them separately and coordinate manually.
//
//   AsyncDecodeContext provides a single context object passed through the
//   entire decode pipeline.  It:
//     1. Owns a stop_source for cooperative cancellation
//     2. Carries per-call configuration for timeout, memory, and error tracking
//     3. Records pipeline stage transitions (state machine)
//     4. Provides a lightweight diagnostics snapshot after decode completes
//
//   Usage:
//     AsyncDecodeContext ctx{ AsyncDecodeContextConfig{ .timeoutMs = 800 } };
//     ctx.SetState(DecodeContextState::DECODING);
//     // ... decode work ...
//     ctx.SetState(DecodeContextState::CACHE_WRITE);
//     // ... cache write ...
//     ctx.SetState(DecodeContextState::COMPLETE);
//     auto diag = ctx.Snapshot();
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_ASYNC_DECODE_CONTEXT_H
#define EXPLORERLENS_ENGINE_ASYNC_DECODE_CONTEXT_H

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <chrono>
#include <string>
#include <stop_token>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// DecodeContextState — pipeline stage machine
// ---------------------------------------------------------------------------
enum class DecodeContextState : std::uint8_t {
    CREATED        = 0,  ///< Context constructed, not yet started
    READING        = 1,  ///< Reading file bytes from disk
    DECODING       = 2,  ///< Decoder running (CPU or GPU)
    POSTPROCESS    = 3,  ///< Resize / orientation / colour management
    CACHE_WRITE    = 4,  ///< Writing to L2 cache
    COMPLETE       = 5,  ///< Successfully finished
    CANCELLED      = 6,  ///< Stop requested via stop_token
    TIMEOUT        = 7,  ///< Deadline exceeded
    FAILED         = 8,  ///< Unrecoverable decode error
};

// ---------------------------------------------------------------------------
// AsyncDecodeContextConfig
// ---------------------------------------------------------------------------
struct AsyncDecodeContextConfig final {
    /// Per-call decode deadline (ms). 0 = no timeout.
    std::uint32_t timeoutMs{ 1200u };

    /// Per-call memory budget for the decoder (bytes). 0 = no limit.
    std::size_t   memoryBudgetBytes{ 128u * 1024u * 1024u }; // 128 MB

    /// If true, record this call in the persistent DecodeErrorTracker on failure
    bool trackErrors{ true };

    /// If true, deliver a placeholder if decode fails (via ThumbnailPlaceholderBroker)
    bool usePlaceholderOnFail{ true };

    /// Caller-supplied file extension for error tracking (e.g., ".cr3")
    std::string fileExtension;
};

// ---------------------------------------------------------------------------
// AsyncDecodeSnapshot — lightweight diagnostics after decode completes
// ---------------------------------------------------------------------------
struct AsyncDecodeSnapshot final {
    DecodeContextState finalState{};
    std::uint32_t      elapsedMs{};
    bool               wasCancelled{};
    bool               wasTimeout{};
    bool               succeeded{};
};

// ---------------------------------------------------------------------------
// AsyncDecodeContext
// ---------------------------------------------------------------------------
class AsyncDecodeContext final {
public:
    explicit AsyncDecodeContext(AsyncDecodeContextConfig cfg = {}) noexcept
        : m_cfg(std::move(cfg))
        , m_state(DecodeContextState::CREATED)
        , m_startTime(std::chrono::steady_clock::now())
    {}

    // Non-copyable (owns stop_source)
    AsyncDecodeContext(const AsyncDecodeContext&)            = delete;
    AsyncDecodeContext& operator=(const AsyncDecodeContext&) = delete;
    AsyncDecodeContext(AsyncDecodeContext&&)                 = default;
    AsyncDecodeContext& operator=(AsyncDecodeContext&&)      = default;

    ~AsyncDecodeContext() noexcept = default;

    // ------------------------------------------------------------------
    // State management
    // ------------------------------------------------------------------
    void SetState(DecodeContextState s) noexcept
    {
        m_state.store(s, std::memory_order_release);
    }

    [[nodiscard]]
    DecodeContextState State() const noexcept
    {
        return m_state.load(std::memory_order_acquire);
    }

    // ------------------------------------------------------------------
    // Cancellation
    // ------------------------------------------------------------------
    void RequestStop() noexcept { m_stopSource.request_stop(); }

    [[nodiscard]] std::stop_token StopToken() const noexcept
    {
        return m_stopSource.get_token();
    }

    [[nodiscard]] bool StopRequested() const noexcept
    {
        return m_stopSource.stop_requested();
    }

    // ------------------------------------------------------------------
    // Timeout check
    // ------------------------------------------------------------------
    [[nodiscard]] bool IsDeadlineExceeded() const noexcept
    {
        if (m_cfg.timeoutMs == 0u) return false;
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_startTime).count();
        return elapsed >= static_cast<std::int64_t>(m_cfg.timeoutMs);
    }

    // ------------------------------------------------------------------
    // Elapsed time
    // ------------------------------------------------------------------
    [[nodiscard]] std::uint32_t ElapsedMs() const noexcept
    {
        const auto now = std::chrono::steady_clock::now();
        const auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_startTime).count();
        return static_cast<std::uint32_t>((std::max)(0LL, ms));
    }

    // ------------------------------------------------------------------
    // Config access
    // ------------------------------------------------------------------
    [[nodiscard]] const AsyncDecodeContextConfig& Config() const noexcept
    {
        return m_cfg;
    }

    // ------------------------------------------------------------------
    // Snapshot() — capture diagnostics at end of decode
    // ------------------------------------------------------------------
    [[nodiscard]]
    AsyncDecodeSnapshot Snapshot() const noexcept
    {
        const auto s = State();
        AsyncDecodeSnapshot snap{};
        snap.finalState   = s;
        snap.elapsedMs    = ElapsedMs();
        snap.wasCancelled = (s == DecodeContextState::CANCELLED);
        snap.wasTimeout   = (s == DecodeContextState::TIMEOUT);
        snap.succeeded    = (s == DecodeContextState::COMPLETE);
        return snap;
    }

    // ------------------------------------------------------------------
    // Constants
    // ------------------------------------------------------------------
    static constexpr std::uint32_t kDefaultTimeoutMs     = 1200u;
    static constexpr std::size_t   kDefaultMemoryBudget  = 128u * 1024u * 1024u;

private:
    AsyncDecodeContextConfig                         m_cfg;
    std::atomic<DecodeContextState>                  m_state;
    std::stop_source                                 m_stopSource;
    std::chrono::steady_clock::time_point            m_startTime;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_ASYNC_DECODE_CONTEXT_H
