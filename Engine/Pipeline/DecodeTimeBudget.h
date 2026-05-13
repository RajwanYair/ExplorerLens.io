// Engine/Pipeline/DecodeTimeBudget.h
// ExplorerLens — per-decode CPU time budget enforcer (ROADMAP v8.0 Phase 3)
// Sprint S349.
//
// Purpose:
//   Phase 3 performance exit criterion: "Single thumbnail p95 < 12 ms".
//   DecodeTimeBudget provides:
//     1. A scoped RAII timer that measures wall-clock time for each decode pass.
//     2. Per-format soft and hard budget constants (ms).
//     3. A BudgetViolation record capturing the overage for telemetry.
//     4. A static table of per-format budgets keyed by a format tag (4-char code).
//
//   Integration: AsyncDecodeContext.h (S339) owns a DecodeTimeBudget;
//   the pipeline checks it at stage boundaries.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_DECODE_TIME_BUDGET_H
#define EXPLORERLENS_ENGINE_DECODE_TIME_BUDGET_H

#include <chrono>
#include <cstdint>
#include <cstddef>
#include <string>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// DecodeTimeBudgetStatus
// ---------------------------------------------------------------------------

enum class DecodeTimeBudgetStatus : std::uint8_t {
    OK               = 0,  ///< Within soft budget
    SOFT_EXCEEDED    = 1,  ///< Between soft and hard budgets (log + telemetry)
    HARD_EXCEEDED    = 2,  ///< Exceeded hard budget (abort decode)
    NOT_STARTED      = 3,  ///< Budget timer not yet armed
    ALREADY_STOPPED  = 4,  ///< Stop() called twice
};

// ---------------------------------------------------------------------------
// DecodeTimeBudgetConfig — soft / hard limits per decode task
// ---------------------------------------------------------------------------

struct DecodeTimeBudgetConfig final {
    /// Soft budget in milliseconds (log warning when exceeded).
    std::uint32_t softBudgetMs  = 12u;

    /// Hard budget in milliseconds (abort decode + fallback when exceeded).
    std::uint32_t hardBudgetMs  = 500u;

    /// Format tag string (up to 8 chars) for telemetry label.
    char formatTag[8]           = {};

    // Per-format preset factory methods:

    [[nodiscard]] static DecodeTimeBudgetConfig ForJpeg() noexcept
    {
        DecodeTimeBudgetConfig c;
        c.softBudgetMs = 8u;
        c.hardBudgetMs = 200u;
        c.formatTag[0] = 'J'; c.formatTag[1] = 'P'; c.formatTag[2] = 'E'; c.formatTag[3] = 'G';
        return c;
    }

    [[nodiscard]] static DecodeTimeBudgetConfig ForRaw() noexcept
    {
        DecodeTimeBudgetConfig c;
        c.softBudgetMs = 50u;
        c.hardBudgetMs = 1'200u;
        c.formatTag[0] = 'R'; c.formatTag[1] = 'A'; c.formatTag[2] = 'W'; c.formatTag[3] = 0;
        return c;
    }

    [[nodiscard]] static DecodeTimeBudgetConfig ForPdf() noexcept
    {
        DecodeTimeBudgetConfig c;
        c.softBudgetMs  = 80u;
        c.hardBudgetMs  = 2'000u;
        c.formatTag[0] = 'P'; c.formatTag[1] = 'D'; c.formatTag[2] = 'F'; c.formatTag[3] = 0;
        return c;
    }

    [[nodiscard]] static DecodeTimeBudgetConfig ForGeneric() noexcept
    {
        return {};  // defaults: soft 12 ms, hard 500 ms
    }
};

// ---------------------------------------------------------------------------
// BudgetViolationRecord — captured when soft or hard budget is exceeded
// ---------------------------------------------------------------------------

struct BudgetViolationRecord final {
    DecodeTimeBudgetStatus status     = DecodeTimeBudgetStatus::NOT_STARTED;
    std::uint64_t          elapsedMs  = 0u;
    std::uint32_t          softBudget = 0u;
    std::uint32_t          hardBudget = 0u;

    [[nodiscard]] std::int64_t OverageSoftMs() const noexcept
    {
        return static_cast<std::int64_t>(elapsedMs) -
               static_cast<std::int64_t>(softBudget);
    }

    [[nodiscard]] std::int64_t OverageHardMs() const noexcept
    {
        return static_cast<std::int64_t>(elapsedMs) -
               static_cast<std::int64_t>(hardBudget);
    }
};

// ---------------------------------------------------------------------------
// DecodeTimeBudget — RAII scoped budget timer
// ---------------------------------------------------------------------------

class DecodeTimeBudget final {
public:
    // -----------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------

    /// Default soft budget (p95 Phase 3 target).
    static constexpr std::uint32_t kDefaultSoftMs = 12u;

    /// Default hard budget (abort threshold).
    static constexpr std::uint32_t kDefaultHardMs = 500u;

    /// Maximum sensible hard budget (safety cap).
    static constexpr std::uint32_t kAbsoluteMaxMs = 30'000u;

    // -----------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------

    DecodeTimeBudget() noexcept = default;

    explicit DecodeTimeBudget(const DecodeTimeBudgetConfig& cfg) noexcept
        : m_config(cfg) {}

    ~DecodeTimeBudget() noexcept = default;

    DecodeTimeBudget(const DecodeTimeBudget&)            = delete;
    DecodeTimeBudget& operator=(const DecodeTimeBudget&) = delete;

    // -----------------------------------------------------------------
    // Start / Stop / Check
    // -----------------------------------------------------------------

    /// Arm the timer.  Must be called before the first Check().
    void Start() noexcept
    {
        m_startTime = std::chrono::steady_clock::now();
        m_started   = true;
        m_stopped   = false;
    }

    /// Stop the timer.
    void Stop() noexcept
    {
        if (m_started) {
            m_stopTime = std::chrono::steady_clock::now();
            m_stopped  = true;
        }
    }

    /// Check current elapsed time against the budget.
    [[nodiscard]] DecodeTimeBudgetStatus Check() const noexcept
    {
        if (!m_started)  return DecodeTimeBudgetStatus::NOT_STARTED;
        if (m_stopped)   return DecodeTimeBudgetStatus::ALREADY_STOPPED;

        const auto now     = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 now - m_startTime).count();
        const auto elapsedU = static_cast<std::uint64_t>(elapsed);

        if (elapsedU > m_config.hardBudgetMs)
            return DecodeTimeBudgetStatus::HARD_EXCEEDED;
        if (elapsedU > m_config.softBudgetMs)
            return DecodeTimeBudgetStatus::SOFT_EXCEEDED;
        return DecodeTimeBudgetStatus::OK;
    }

    /// Finalise and return a violation record (valid after Stop() or Check()).
    [[nodiscard]] BudgetViolationRecord Finalise() const noexcept
    {
        BudgetViolationRecord r;
        r.softBudget = m_config.softBudgetMs;
        r.hardBudget = m_config.hardBudgetMs;
        if (!m_started) {
            r.status    = DecodeTimeBudgetStatus::NOT_STARTED;
            r.elapsedMs = 0u;
            return r;
        }
        const auto end     = m_stopped ? m_stopTime : std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 end - m_startTime).count();
        r.elapsedMs = static_cast<std::uint64_t>(elapsed);

        if (r.elapsedMs > m_config.hardBudgetMs)
            r.status = DecodeTimeBudgetStatus::HARD_EXCEEDED;
        else if (r.elapsedMs > m_config.softBudgetMs)
            r.status = DecodeTimeBudgetStatus::SOFT_EXCEEDED;
        else
            r.status = DecodeTimeBudgetStatus::OK;
        return r;
    }

    // -----------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------

    [[nodiscard]] bool IsStarted() const noexcept { return m_started; }
    [[nodiscard]] bool IsStopped() const noexcept { return m_stopped; }
    [[nodiscard]] const DecodeTimeBudgetConfig& Config() const noexcept { return m_config; }

    [[nodiscard]] std::uint64_t ElapsedMs() const noexcept
    {
        if (!m_started) return 0u;
        const auto end = m_stopped ? m_stopTime : std::chrono::steady_clock::now();
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                end - m_startTime).count());
    }

private:
    DecodeTimeBudgetConfig                        m_config{};
    std::chrono::steady_clock::time_point         m_startTime{};
    std::chrono::steady_clock::time_point         m_stopTime{};
    bool                                          m_started = false;
    bool                                          m_stopped = false;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DECODE_TIME_BUDGET_H
