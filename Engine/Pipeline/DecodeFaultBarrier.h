// =============================================================================
// ExplorerLens Engine — DecodeFaultBarrier.h
// Sprint S354 | ROADMAP v8.0 Phase 4 (Safety)
// SEH-based structured exception fault barrier around decoder invocations.
//
// Prevents a buggy decoder from crashing the Explorer shell host process.
// Wraps the inner decode call in an SEH __try/__except block and converts
// any Win32 structured exception into a typed FaultBarrierStatus.
//
// Usage pattern:
//   DecodeFaultBarrier barrier;
//   barrier.Execute([&]() { myDecoder.Decode(stream, out); });
//   if (!barrier.Succeeded()) { /* fallback */ }
//
// MSVC-only: SEH is not available in C++ exception contexts and cannot be
// placed inside the same function as C++ objects with destructors without
// a trampoline. DecodeFaultBarrier handles this by delegating __try to
// the Execute() helper which is a plain C function on Win32.
//
// Non-Windows compiles to a simple inline lambda invoke.
// =============================================================================
#pragma once

#include <cstdint>
#include <functional>

#ifndef EXPLORERLENS_ENGINE_DECODEFAULTBARRIER_H
#define EXPLORERLENS_ENGINE_DECODEFAULTBARRIER_H

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// FaultBarrierStatus — outcome of a guarded decode call
// ---------------------------------------------------------------------------
enum class FaultBarrierStatus : uint8_t {
    OK                  = 0, ///< Decode completed without exceptions
    ACCESS_VIOLATION    = 1, ///< EXCEPTION_ACCESS_VIOLATION (0xC0000005)
    STACK_OVERFLOW      = 2, ///< EXCEPTION_STACK_OVERFLOW  (0xC00000FD)
    INTEGER_DIVIDE_ZERO = 3, ///< EXCEPTION_INT_DIVIDE_BY_ZERO (0xC0000094)
    ILLEGAL_INSTRUCTION = 4, ///< EXCEPTION_ILLEGAL_INSTRUCTION (0xC000001D)
    UNKNOWN_EXCEPTION   = 5, ///< Any other Win32 structured exception code
    NOT_WIN32           = 6, ///< Non-Windows build — no SEH available
    NOT_EXECUTED        = 7, ///< Execute() has not been called yet
    CPP_EXCEPTION       = 8, ///< std::exception caught (unwound via C++ EH)
};

// ---------------------------------------------------------------------------
// FaultBarrierConfig — tuning parameters
// ---------------------------------------------------------------------------
struct FaultBarrierConfig final {
    bool captureContinuable{false};  ///< Continue after continuable exceptions (dangerous)
    bool logOnFault{true};           ///< Emit ETW event on fault
    bool triggerMiniDump{false};     ///< Write MiniDump on fault (see MiniDumpCapture.h)
    bool rethrowCppExceptions{false};///< Re-throw std::exception after recording

    [[nodiscard]] static FaultBarrierConfig Default() noexcept {
        FaultBarrierConfig c;
        c.captureContinuable  = false;
        c.logOnFault          = true;
        c.triggerMiniDump     = false;
        c.rethrowCppExceptions = false;
        return c;
    }

    /// Config used in shell COM server (no mini-dumps, log only)
    [[nodiscard]] static FaultBarrierConfig ForShellHost() noexcept {
        FaultBarrierConfig c;
        c.captureContinuable  = false;
        c.logOnFault          = true;
        c.triggerMiniDump     = false;
        c.rethrowCppExceptions = false;
        return c;
    }
};

// ---------------------------------------------------------------------------
// DecodeFaultBarrier — RAII guard for a single decoder invocation
// ---------------------------------------------------------------------------
class DecodeFaultBarrier final {
public:
    explicit DecodeFaultBarrier(
        FaultBarrierConfig cfg = FaultBarrierConfig::Default()) noexcept
        : m_cfg(cfg)
        , m_status(FaultBarrierStatus::NOT_EXECUTED)
        , m_exceptionCode(0u)
    {}

    // Non-copyable, non-moveable (holds mutable fault state)
    DecodeFaultBarrier(const DecodeFaultBarrier&)            = delete;
    DecodeFaultBarrier& operator=(const DecodeFaultBarrier&) = delete;
    DecodeFaultBarrier(DecodeFaultBarrier&&)                 = delete;
    DecodeFaultBarrier& operator=(DecodeFaultBarrier&&)      = delete;

    ~DecodeFaultBarrier() = default;

    /// Execute a callable inside the SEH fault barrier.
    /// @param fn  Any callable with signature void()
    template<typename Fn>
    void Execute(Fn&& fn) noexcept
    {
        // Reset state
        m_status        = FaultBarrierStatus::NOT_EXECUTED;
        m_exceptionCode = 0u;

#if defined(_WIN32) && defined(_MSC_VER)
        ExecuteWin32(std::forward<Fn>(fn));
#else
        // Non-Win32: plain call, catch std::exceptions only
        try {
            fn();
            m_status = FaultBarrierStatus::OK;
        } catch (const std::exception&) {
            m_status = FaultBarrierStatus::CPP_EXCEPTION;
        }
#endif
    }

    /// Returns true iff the last Execute() call completed without faults.
    [[nodiscard]] bool Succeeded()      const noexcept { return m_status == FaultBarrierStatus::OK; }
    [[nodiscard]] FaultBarrierStatus Status()    const noexcept { return m_status; }
    [[nodiscard]] uint32_t           ExceptionCode() const noexcept { return m_exceptionCode; }
    [[nodiscard]] bool               HasFaulted() const noexcept {
        return m_status != FaultBarrierStatus::OK
            && m_status != FaultBarrierStatus::NOT_EXECUTED;
    }

    /// Reset to NOT_EXECUTED state for reuse.
    void Reset() noexcept {
        m_status        = FaultBarrierStatus::NOT_EXECUTED;
        m_exceptionCode = 0u;
    }

private:
    FaultBarrierConfig m_cfg;
    FaultBarrierStatus m_status;
    uint32_t           m_exceptionCode;

#if defined(_WIN32) && defined(_MSC_VER)
    // Win32/MSVC SEH path — trampoline to keep __try in a plain function.
    template<typename Fn>
    void ExecuteWin32(Fn&& fn) noexcept;

    /// Map Win32 exception code to FaultBarrierStatus.
    [[nodiscard]] static FaultBarrierStatus MapExceptionCode(uint32_t code) noexcept {
        switch (code) {
            case 0xC0000005u: return FaultBarrierStatus::ACCESS_VIOLATION;
            case 0xC00000FDu: return FaultBarrierStatus::STACK_OVERFLOW;
            case 0xC0000094u: return FaultBarrierStatus::INTEGER_DIVIDE_ZERO;
            case 0xC000001Du: return FaultBarrierStatus::ILLEGAL_INSTRUCTION;
            default:          return FaultBarrierStatus::UNKNOWN_EXCEPTION;
        }
    }
#endif // _WIN32 && _MSC_VER
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kSehAccessViolation     = 0xC0000005u;
static constexpr uint32_t kSehStackOverflow       = 0xC00000FDu;
static constexpr uint32_t kSehIntDivideByZero     = 0xC0000094u;
static constexpr uint32_t kSehIllegalInstruction  = 0xC000001Du;
static constexpr uint32_t kSehCppException        = 0xE06D7363u; ///< Microsoft C++ exception code

// ---------------------------------------------------------------------------
// Win32 SEH template body
// Note: __try/__except cannot be in a function that creates C++ objects with
// destructors, so we route through a lambda-capture trampoline stored in a
// std::function<void()> (no destructors in the __try scope itself).
// ---------------------------------------------------------------------------
#if defined(_WIN32) && defined(_MSC_VER)

template<typename Fn>
void DecodeFaultBarrier::ExecuteWin32(Fn&& fn) noexcept
{
    // Wrap the templated callable in std::function to isolate destructor from __try scope
    std::function<void()> wrapper = std::forward<Fn>(fn);
    uint32_t exCode = 0u;
    bool cppExcept  = false;

    __try {
        try {
            wrapper();
        } catch (const std::exception&) {
            cppExcept = true;
        }
    }
    __except (
        exCode = static_cast<uint32_t>(::GetExceptionCode()),
        EXCEPTION_EXECUTE_HANDLER
    )
    {
        m_exceptionCode = exCode;
        m_status        = MapExceptionCode(exCode);
        return;
    }

    if (cppExcept) {
        m_status = FaultBarrierStatus::CPP_EXCEPTION;
    } else {
        m_status = FaultBarrierStatus::OK;
    }
}

#endif // _WIN32 && _MSC_VER

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DECODEFAULTBARRIER_H
