// =============================================================================
// ExplorerLens Engine — UbsanSuppressor.h
// Sprint S358 | ROADMAP v8.0 Phase 4 (UBSAN clean in CI)
// UBSAN (Undefined Behaviour Sanitizer) annotation helpers and violation
// tracking contract for the CI pipeline.
//
// Phase 4 exit criterion: "UBSAN clean in CI"
// UBSAN is enabled via -fsanitize=undefined in Clang-based CI builds.
// MSVC does not have UBSAN; these annotations are no-ops on MSVC.
//
// Usage:
//   LENS_UBSAN_SUPPRESS_VPTR void VirtualCast() { ... }
//   LENS_UBSAN_SUPPRESS_SHIFT uint32_t ShiftOp() { ... }
//
// The violation tracker accumulates runtime counts during UBSAN CI runs
// so the test harness can assert UbsanViolationCount() == 0.
// =============================================================================
#pragma once

#include <cstdint>
#include <atomic>
#include <string>

#ifndef EXPLORERLENS_ENGINE_UBSANSUPPRESSOR_H
#define EXPLORERLENS_ENGINE_UBSANSUPPRESSOR_H

// ---------------------------------------------------------------------------
// Compiler-specific UBSAN annotation macros
// ---------------------------------------------------------------------------

#if defined(__clang__)
    // Clang: __attribute__((no_sanitize("undefined"))) per-function suppress
    #define LENS_UBSAN_SUPPRESS_ALL    __attribute__((no_sanitize("undefined")))
    #define LENS_UBSAN_SUPPRESS_VPTR   __attribute__((no_sanitize("vptr")))
    #define LENS_UBSAN_SUPPRESS_SHIFT  __attribute__((no_sanitize("shift")))
    #define LENS_UBSAN_SUPPRESS_OVF    __attribute__((no_sanitize("signed-integer-overflow")))
    #define LENS_UBSAN_SUPPRESS_NULL   __attribute__((no_sanitize("null")))
    #define LENS_UBSAN_SUPPRESS_ALIGN  __attribute__((no_sanitize("alignment")))
    #define LENS_UBSAN_HAS_SUPPORT     1
#elif defined(__GNUC__)
    // GCC 5+
    #define LENS_UBSAN_SUPPRESS_ALL    __attribute__((no_sanitize_undefined))
    #define LENS_UBSAN_SUPPRESS_VPTR   __attribute__((no_sanitize_undefined))
    #define LENS_UBSAN_SUPPRESS_SHIFT  __attribute__((no_sanitize_undefined))
    #define LENS_UBSAN_SUPPRESS_OVF    __attribute__((no_sanitize_undefined))
    #define LENS_UBSAN_SUPPRESS_NULL   __attribute__((no_sanitize_undefined))
    #define LENS_UBSAN_SUPPRESS_ALIGN  __attribute__((no_sanitize_undefined))
    #define LENS_UBSAN_HAS_SUPPORT     1
#else
    // MSVC / unknown — no-ops
    #define LENS_UBSAN_SUPPRESS_ALL
    #define LENS_UBSAN_SUPPRESS_VPTR
    #define LENS_UBSAN_SUPPRESS_SHIFT
    #define LENS_UBSAN_SUPPRESS_OVF
    #define LENS_UBSAN_SUPPRESS_NULL
    #define LENS_UBSAN_SUPPRESS_ALIGN
    #define LENS_UBSAN_HAS_SUPPORT     0
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// UbsanViolationType — categories of undefined behaviour
// ---------------------------------------------------------------------------
enum class UbsanViolationType : uint8_t {
    SIGNED_INTEGER_OVERFLOW = 0,  ///< Signed int arithmetic overflow
    UNSIGNED_INTEGER_OVERFLOW = 1,
    SHIFT_EXPONENT           = 2, ///< Shift count OOB
    VPTR                     = 3, ///< Virtual dispatch through wrong type
    NULL_DEREFERENCE         = 4, ///< Null pointer member access
    ALIGNMENT                = 5, ///< Misaligned pointer dereference
    ARRAY_BOUNDS             = 6, ///< Out-of-bounds array subscript
    UNREACHABLE              = 7, ///< __builtin_unreachable() reached
    RETURN                   = 8, ///< Non-void function falls off end
    FLOAT_CAST_OVERFLOW      = 9, ///< Float → int overflow
    FUNCTION                 = 10,///< Invalid function pointer call
    UNKNOWN                  = 15,
};

// ---------------------------------------------------------------------------
// UbsanViolationRecord — a single captured UBSAN event (for CI tracking)
// ---------------------------------------------------------------------------
struct UbsanViolationRecord final {
    UbsanViolationType type{UbsanViolationType::UNKNOWN};
    std::string        sourceFile;   ///< __FILE__ at violation site
    uint32_t           sourceLine{0};///< __LINE__ at violation site
    std::string        message;      ///< UBSAN diagnostic text (if captured)
    uint64_t           timestampNs{0}; ///< Monotonic clock nanoseconds
};

// ---------------------------------------------------------------------------
// UbsanViolationTracker — thread-safe violation accumulator (CI only)
// ---------------------------------------------------------------------------
class UbsanViolationTracker final {
public:
    UbsanViolationTracker()                                  = default;
    UbsanViolationTracker(const UbsanViolationTracker&)      = delete;
    UbsanViolationTracker& operator=(const UbsanViolationTracker&) = delete;

    /// Record a violation (called from UBSAN runtime handler).
    void Record(UbsanViolationType type,
                const char* file,
                uint32_t line,
                const char* msg = nullptr) noexcept;

    /// Total violations recorded since Reset().
    [[nodiscard]] uint64_t Count() const noexcept {
        return m_count.load(std::memory_order_relaxed);
    }

    /// Count for a specific violation type.
    [[nodiscard]] uint64_t CountOf(UbsanViolationType type) const noexcept {
        return m_perType[static_cast<uint8_t>(type)].load(std::memory_order_relaxed);
    }

    /// Returns true iff zero violations have been recorded (CI pass condition).
    [[nodiscard]] bool IsClean() const noexcept { return Count() == 0u; }

    /// Reset all counters (call between test runs).
    void Reset() noexcept;

    /// Global singleton for UBSAN handler callbacks.
    [[nodiscard]] static UbsanViolationTracker& Global() noexcept;

private:
    std::atomic<uint64_t> m_count{0u};
    std::atomic<uint64_t> m_perType[16]{}; // indexed by UbsanViolationType
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kUbsanMaxViolationTypes = 16u;
/// CI pass threshold — any value above 0 fails the UBSAN gate.
static constexpr uint64_t kUbsanPhase4ViolationBudget = 0u;

// ---------------------------------------------------------------------------
// UbsanSuppressor — utility wrappers + CI gate helper
// ---------------------------------------------------------------------------
class UbsanSuppressor final {
public:
    UbsanSuppressor() = delete;

    /// Returns true if this build was compiled with UBSAN enabled.
    [[nodiscard]] static bool IsUbsanActive() noexcept {
#if LENS_UBSAN_HAS_SUPPORT && defined(__SANITIZE_ADDRESS__)
        return true; // Clang/GCC with -fsanitize=undefined
#elif defined(LENS_UBSAN_ENABLED) // manual define from CMake
        return true;
#else
        return false;
#endif
    }

    /// Returns true iff the global tracker has zero violations.
    [[nodiscard]] static bool Phase4GatePassed() noexcept {
        return UbsanViolationTracker::Global().IsClean();
    }

    /// Emit a formatted log line describing the violation count.
    static void ReportToLog() noexcept;
};

// ---------------------------------------------------------------------------
// Inline UbsanViolationTracker implementations
// ---------------------------------------------------------------------------
inline void UbsanViolationTracker::Reset() noexcept {
    m_count.store(0u, std::memory_order_relaxed);
    for (auto& c : m_perType)
        c.store(0u, std::memory_order_relaxed);
}

inline void UbsanViolationTracker::Record(
    UbsanViolationType type,
    const char* /*file*/,
    uint32_t    /*line*/,
    const char* /*msg*/) noexcept
{
    m_count.fetch_add(1u, std::memory_order_relaxed);
    const uint8_t idx = static_cast<uint8_t>(type);
    if (idx < kUbsanMaxViolationTypes)
        m_perType[idx].fetch_add(1u, std::memory_order_relaxed);
}

inline UbsanViolationTracker& UbsanViolationTracker::Global() noexcept {
    static UbsanViolationTracker s_instance;
    return s_instance;
}

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_UBSANSUPPRESSOR_H
