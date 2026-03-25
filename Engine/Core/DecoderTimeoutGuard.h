// DecoderTimeoutGuard.h — Per-Decoder Watchdog Timeout Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 11 (v15.3.0 "Zenith-T"): Enforces a hard per-decoder timeout so that
// a misbehaving or adversarially crafted file cannot block the Shell Extension
// thread indefinitely.  After the configured deadline the watchdog fires and the
// caller receives a DecodeErrorCategory::Timeout result.
//
#pragma once

#include <chrono>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class TimeoutResult
{
    Completed,
    TimedOut
};

class DecoderTimeoutGuard
{
public:
    using Clock    = std::chrono::steady_clock;
    using Duration = std::chrono::milliseconds;

    static constexpr Duration DEFAULT_TIMEOUT{ 5000 };

    explicit DecoderTimeoutGuard(Duration timeout = DEFAULT_TIMEOUT) noexcept
        : m_timeout(timeout)
        , m_deadline(Clock::now() + timeout)
    {
    }

    bool IsExpired() const noexcept
    {
        return Clock::now() >= m_deadline;
    }

    Duration Remaining() const noexcept
    {
        auto remaining = m_deadline - Clock::now();
        if (remaining.count() <= 0) return Duration{ 0 };
        return std::chrono::duration_cast<Duration>(remaining);
    }

    void Reset() noexcept
    {
        m_deadline = Clock::now() + m_timeout;
    }

    Duration GetTimeout() const noexcept { return m_timeout; }

    TimeoutResult RunWithTimeout(const std::function<void()>& work) const;

private:
    Duration             m_timeout;
    Clock::time_point    m_deadline;
};

} // namespace Engine
} // namespace ExplorerLens
