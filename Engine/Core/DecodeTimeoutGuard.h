// Engine/Core/DecodeTimeoutGuard.h
// ExplorerLens — 500 ms hard decode timeout with std::jthread watchdog (H39 / ROADMAP v8.0 Phase 2)
// Sprint S322.
//
// Purpose:
//   Windows Explorer calls IThumbnailProvider::GetThumbnail() on a UI-adjacent
//   thread.  A decoder that hangs (corrupt file, infinite loop, stalled I/O)
//   stalls the Explorer window until it times out internally — typically 30 s.
//
//   DecodeTimeoutGuard solves this by:
//     1. Launching a std::jthread watchdog at construction time.
//     2. Setting an atomic flag when the deadline (default 500 ms) expires.
//     3. Signalling the bound std::stop_token so the decoder's cancel path fires.
//     4. Providing IsExpired() for poll-based decoders that cannot use stop_token.
//
//   RAII semantics: the watchdog thread is stopped (via stop_source) and
//   joined in the destructor — no thread leaks even on exception paths.
//
// Integration with AsyncDecodeToken:
//   Pass the stop_source from DecodeTimeoutGuard to AsyncDecodeToken so the
//   cancellation flows through a single stop_token across both mechanisms.
//
// Usage:
//   {
//       DecodeTimeoutGuard guard{ std::stop_source{}, DecodeTimeoutConfig{} };
//       guard.Start();
//       auto result = decoder.Decode(stream, cx, guard.StopToken());
//       if (guard.IsExpired()) return E_ABORT;
//   }  // ~DecodeTimeoutGuard stops watchdog thread
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_DECODE_TIMEOUT_GUARD_H
#define EXPLORERLENS_ENGINE_DECODE_TIMEOUT_GUARD_H

#include <cstdint>
#include <atomic>
#include <chrono>
#include <stop_token>
#include <thread>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// DecodeTimeoutConfig
// ---------------------------------------------------------------------------
struct DecodeTimeoutConfig final {
    /// Hard timeout for any single decode operation.
    /// IThumbnailProvider callers expect a response within this window.
    static constexpr std::uint32_t kDefaultTimeoutMs = 500u;

    /// Absolute minimum — never set below this or Explorer flickers.
    static constexpr std::uint32_t kMinTimeoutMs = 100u;

    /// Maximum allowed — above this Explorer's own timeout fires first.
    static constexpr std::uint32_t kMaxTimeoutMs = 5000u;

    /// Watchdog poll granularity.  Smaller = more responsive cancel,
    /// larger = fewer kernel wakes.  32 ms is one Explorer frame.
    static constexpr std::uint32_t kWatchdogPollMs = 32u;

    std::uint32_t timeoutMs = kDefaultTimeoutMs;

    /// When true, request stop on the stop_source when the deadline fires.
    /// When false, only set the IsExpired() flag (poll mode).
    bool requestStopOnTimeout = true;
};

// ---------------------------------------------------------------------------
// DecodeTimeoutStatus
// ---------------------------------------------------------------------------
enum class DecodeTimeoutStatus : std::uint8_t {
    IDLE            = 0,   ///< Guard constructed, not yet started
    RUNNING         = 1,   ///< Watchdog active; deadline not reached
    EXPIRED         = 2,   ///< Deadline fired; stop was requested
    COMPLETED       = 3,   ///< Decode finished before deadline
    INVALID_CONFIG  = 4,   ///< timeoutMs out of [kMin, kMax] range
};

// ---------------------------------------------------------------------------
// DecodeTimeoutGuard
// ---------------------------------------------------------------------------
class DecodeTimeoutGuard final {
public:
    explicit DecodeTimeoutGuard(DecodeTimeoutConfig cfg = {}) noexcept
        : m_cfg(cfg)
        , m_status(DecodeTimeoutStatus::IDLE)
        , m_expired(false)
    {
        if (cfg.timeoutMs < DecodeTimeoutConfig::kMinTimeoutMs ||
            cfg.timeoutMs > DecodeTimeoutConfig::kMaxTimeoutMs) {
            m_status.store(DecodeTimeoutStatus::INVALID_CONFIG,
                           std::memory_order_relaxed);
        }
    }

    ~DecodeTimeoutGuard() noexcept { Stop(); }

    // Non-copyable, movable
    DecodeTimeoutGuard(const DecodeTimeoutGuard&)            = delete;
    DecodeTimeoutGuard& operator=(const DecodeTimeoutGuard&) = delete;
    DecodeTimeoutGuard(DecodeTimeoutGuard&&)                 = default;
    DecodeTimeoutGuard& operator=(DecodeTimeoutGuard&&)      = default;

    // ------------------------------------------------------------------
    // Start() — arm the watchdog.  Call once per decode operation.
    // ------------------------------------------------------------------
    bool Start() noexcept
    {
        const auto s = m_status.load(std::memory_order_relaxed);
        if (s == DecodeTimeoutStatus::INVALID_CONFIG) return false;
        if (s == DecodeTimeoutStatus::RUNNING)        return false;

        m_expired.store(false, std::memory_order_relaxed);
        m_stopSource = std::stop_source{};
        m_status.store(DecodeTimeoutStatus::RUNNING, std::memory_order_relaxed);

        const std::uint32_t timeoutMs  = m_cfg.timeoutMs;
        const std::uint32_t pollMs     = m_cfg.watchdogPollMs;
        const bool          doStop     = m_cfg.requestStopOnTimeout;
        std::stop_source&   ss         = m_stopSource;
        std::atomic<bool>&  expired    = m_expired;
        auto&               status     = m_status;

        m_watchdog = std::jthread([=, &ss, &expired, &status](std::stop_token st) {
            const auto deadline = std::chrono::steady_clock::now()
                                  + std::chrono::milliseconds(timeoutMs);
            while (!st.stop_requested()) {
                if (std::chrono::steady_clock::now() >= deadline) {
                    expired.store(true,  std::memory_order_release);
                    status.store(DecodeTimeoutStatus::EXPIRED,
                                 std::memory_order_release);
                    if (doStop) ss.request_stop();
                    return;
                }
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(pollMs));
            }
        });

        return true;
    }

    // ------------------------------------------------------------------
    // Complete() — signal successful decode before deadline.
    // ------------------------------------------------------------------
    void Complete() noexcept
    {
        m_status.store(DecodeTimeoutStatus::COMPLETED,
                       std::memory_order_release);
        Stop();
    }

    // ------------------------------------------------------------------
    // Stop() — stop the watchdog; called by Complete() and destructor.
    // ------------------------------------------------------------------
    void Stop() noexcept
    {
        if (m_watchdog.joinable()) {
            m_watchdog.request_stop();
            m_watchdog.join();
        }
    }

    // ------------------------------------------------------------------
    // Accessors
    // ------------------------------------------------------------------
    [[nodiscard]] bool IsExpired() const noexcept
    {
        return m_expired.load(std::memory_order_acquire);
    }

    [[nodiscard]] DecodeTimeoutStatus Status() const noexcept
    {
        return m_status.load(std::memory_order_acquire);
    }

    /// Returns a stop_token tied to the internal stop_source.
    /// Pass this to the decoder so it can cooperate with cancellation.
    [[nodiscard]] std::stop_token StopToken() const noexcept
    {
        return m_stopSource.get_token();
    }

    // ------------------------------------------------------------------
    // Constants (public for tests)
    // ------------------------------------------------------------------
    static constexpr std::uint32_t kDefaultTimeoutMs  = DecodeTimeoutConfig::kDefaultTimeoutMs;
    static constexpr std::uint32_t kMinTimeoutMs       = DecodeTimeoutConfig::kMinTimeoutMs;
    static constexpr std::uint32_t kMaxTimeoutMs       = DecodeTimeoutConfig::kMaxTimeoutMs;
    static constexpr std::uint32_t kWatchdogPollMs     = DecodeTimeoutConfig::kWatchdogPollMs;

private:
    DecodeTimeoutConfig              m_cfg;
    std::atomic<DecodeTimeoutStatus> m_status;
    std::atomic<bool>                m_expired;
    std::stop_source                 m_stopSource;
    std::jthread                     m_watchdog;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DECODE_TIMEOUT_GUARD_H
