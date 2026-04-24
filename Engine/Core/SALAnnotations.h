//==============================================================================
// ExplorerLens Engine — SAL annotation helpers (Sprint S247)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 A12 / ADR — /analyze + SAL annotations (Phase 2).
//==============================================================================
//
// Shim layer over the SAL 2.0 macros so Engine/Core code can be annotated
// today without pulling the entire `<sal.h>` surface everywhere, and without
// breaking builds on non-MSVC toolchains (GCC/Clang CI smoke jobs).
//
// Usage example:
//
//     LENS_SAL_IN_READS(len) const std::uint8_t* bytes,
//     LENS_SAL_IN_RANGE(1, 65536) std::uint32_t thumbPx
//
// On MSVC these expand to real _In_reads_() / _In_range_(). Elsewhere they
// expand to nothing. The header is header-only and costs nothing at runtime.
//==============================================================================
#pragma once

#if defined(_MSC_VER)
    #include <sal.h>

    #define LENS_SAL_IN                      _In_
    #define LENS_SAL_IN_OPT                  _In_opt_
    #define LENS_SAL_OUT                     _Out_
    #define LENS_SAL_OUT_OPT                 _Out_opt_
    #define LENS_SAL_INOUT                   _Inout_
    #define LENS_SAL_IN_READS(n)             _In_reads_(n)
    #define LENS_SAL_IN_READS_BYTES(n)       _In_reads_bytes_(n)
    #define LENS_SAL_OUT_WRITES(n)           _Out_writes_(n)
    #define LENS_SAL_OUT_WRITES_BYTES(n)     _Out_writes_bytes_(n)
    #define LENS_SAL_IN_RANGE(lo, hi)        _In_range_(lo, hi)
    #define LENS_SAL_RETURN_VALID            _Ret_valid_
    #define LENS_SAL_RETURN_NOTNULL          _Ret_notnull_
    #define LENS_SAL_RETURN_MAYBE_NULL       _Ret_maybenull_
    #define LENS_SAL_NULLTERMINATED          _Null_terminated_
    #define LENS_SAL_SUCCESS(expr)           _Success_(expr)
    #define LENS_SAL_PRINTF_FORMAT(n)        _Printf_format_string_params_(n)
    #define LENS_SAL_USE_DECL_ANNOTATIONS    _Use_decl_annotations_
#else
    #define LENS_SAL_IN
    #define LENS_SAL_IN_OPT
    #define LENS_SAL_OUT
    #define LENS_SAL_OUT_OPT
    #define LENS_SAL_INOUT
    #define LENS_SAL_IN_READS(n)
    #define LENS_SAL_IN_READS_BYTES(n)
    #define LENS_SAL_OUT_WRITES(n)
    #define LENS_SAL_OUT_WRITES_BYTES(n)
    #define LENS_SAL_IN_RANGE(lo, hi)
    #define LENS_SAL_RETURN_VALID
    #define LENS_SAL_RETURN_NOTNULL
    #define LENS_SAL_RETURN_MAYBE_NULL
    #define LENS_SAL_NULLTERMINATED
    #define LENS_SAL_SUCCESS(expr)
    #define LENS_SAL_PRINTF_FORMAT(n)
    #define LENS_SAL_USE_DECL_ANNOTATIONS
#endif

// Compile-time probe: callers can include this header in an isolated TU and
// reference `kLensSalEnabled` to verify SAL is active in their build config.
namespace ExplorerLens {
namespace Engine {

#if defined(_MSC_VER)
    inline constexpr bool kLensSalEnabled = true;
#else
    inline constexpr bool kLensSalEnabled = false;
#endif

} // namespace Engine
} // namespace ExplorerLens
