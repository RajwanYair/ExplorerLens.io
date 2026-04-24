//==============================================================================
// ExplorerLens Engine — [[nodiscard]] reason macro (Sprint S232)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §5 — Phase 1: LENS_NODISCARD_EXPECTED on all std::expected sites
//==============================================================================
//
// C++23 allows `[[nodiscard("message")]]`. For std::expected<T,E> return
// values we want a uniform reason string so the compiler diagnostic tells
// callers exactly why dropping the result is a bug.
//
// Usage:
//   LENS_NODISCARD_EXPECTED
//   std::expected<int, EngineError> Decode(...);
//
// Callers that discard the result will get:
//   warning: ignoring return value of '...', declared with attribute
//   'nodiscard': 'std::expected must be inspected — dropping it silently
//   hides decode errors.'
//==============================================================================
#pragma once

#if defined(__cplusplus) && __cplusplus >= 202002L
    #define LENS_NODISCARD_EXPECTED \
        [[nodiscard("std::expected must be inspected — dropping it silently hides decode errors.")]]
#else
    #define LENS_NODISCARD_EXPECTED [[nodiscard]]
#endif

// Short alias for hot paths
#define LENS_NODISCARD_RESULT LENS_NODISCARD_EXPECTED

namespace ExplorerLens {
namespace Engine {

/// <summary>
/// Compile-time guard: when invoked from a test, verifies the macro
/// expands to a valid attribute sequence on the current compiler.
/// </summary>
struct NodiscardExpectedProbe
{
    LENS_NODISCARD_EXPECTED
    static int Probe() noexcept { return 0; }
};

} // namespace Engine
} // namespace ExplorerLens
