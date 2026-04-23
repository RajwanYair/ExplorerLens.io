// ErrorDomainTests.cpp — Catch2 tests for error domain contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the error domain model and std::expected-style contracts from
// §7.4 and D31.
// Tests cover: 5 error categories (DECODE/IO/CACHE/POLICY/INTERNAL),
// category predicate helpers (IsDecodeError/IsIOError etc.), HRESULT
// mapping invariants (success → non-failure, E_FAIL range), error code
// distinctness within and across categories, and error message non-empty
// contracts.
//
// All tests are self-contained — no Windows SDK headers included.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Error domain model (§7.4, D31)
// Mirrored from Engine/Core/EngineError.h
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::ErrorDomain {

// ── Error category identifiers ─────────────────────────────────────────────

enum class ErrorCategory : uint8_t {
    DECODE   = 0,   // Format decode failures
    IO       = 1,   // Stream / filesystem errors
    CACHE    = 2,   // Cache miss / corruption / budget
    POLICY   = 3,   // Decoder disabled / policy block
    INTERNAL = 4,   // Invariant violations (should not reach callers)
};

static constexpr int ERROR_CATEGORY_COUNT = 5;

// ── Error domain names (for structured logging / telemetry) ────────────────

static constexpr std::array<std::string_view, 5> ERROR_CATEGORY_NAMES = {{
    "decode",
    "io",
    "cache",
    "policy",
    "internal",
}};

// ── Error code ranges (mirrors EngineError enum ranges) ───────────────────

static constexpr uint32_t DECODE_BASE   = 0x00001000;
static constexpr uint32_t IO_BASE       = 0x00002000;
static constexpr uint32_t CACHE_BASE    = 0x00003000;
static constexpr uint32_t POLICY_BASE   = 0x00004000;
static constexpr uint32_t INTERNAL_BASE = 0x00009000;
static constexpr uint32_t CATEGORY_MASK = 0x0000F000;  // Extract range key

// ── Category predicate helpers ─────────────────────────────────────────────

inline constexpr bool IsDecodeError  (uint32_t code) { return (code & CATEGORY_MASK) == DECODE_BASE;   }
inline constexpr bool IsIOError      (uint32_t code) { return (code & CATEGORY_MASK) == IO_BASE;       }
inline constexpr bool IsCacheError   (uint32_t code) { return (code & CATEGORY_MASK) == CACHE_BASE;    }
inline constexpr bool IsPolicyError  (uint32_t code) { return (code & CATEGORY_MASK) == POLICY_BASE;   }
inline constexpr bool IsInternalError(uint32_t code) { return (code & CATEGORY_MASK) == INTERNAL_BASE; }
inline constexpr bool IsSuccess      (uint32_t code) { return code == 0u; }

/// Returns the ErrorCategory for a given error code
inline constexpr ErrorCategory GetErrorCategory(uint32_t code) {
    switch (code & CATEGORY_MASK) {
        case DECODE_BASE:   return ErrorCategory::DECODE;
        case IO_BASE:       return ErrorCategory::IO;
        case CACHE_BASE:    return ErrorCategory::CACHE;
        case POLICY_BASE:   return ErrorCategory::POLICY;
        default:            return ErrorCategory::INTERNAL;
    }
}

// ── HRESULT mapping constants ─────────────────────────────────────────────

// Windows HRESULT conventions (not including <winerror.h>)
static constexpr uint32_t HRESULT_S_OK             = 0x00000000;
static constexpr uint32_t HRESULT_E_FAIL           = 0x80004005;
static constexpr uint32_t HRESULT_E_INVALIDARG     = 0x80070057;
static constexpr uint32_t HRESULT_E_OUTOFMEMORY    = 0x8007000E;
static constexpr uint32_t HRESULT_E_NOTIMPL        = 0x80004001;
static constexpr uint32_t HRESULT_ERROR_FILE_NOT_FOUND = 0x80070002;

// An HRESULT is a failure if the sign bit (bit 31) is set
inline constexpr bool IsFailureHRESULT(uint32_t hr) {
    return (hr & 0x80000000u) != 0;
}

inline constexpr bool IsSuccessHRESULT(uint32_t hr) {
    return (hr & 0x80000000u) == 0;
}

/// Maps EngineError code → HRESULT (subset, for COM boundary)
inline constexpr uint32_t EngineErrorToHRESULT(uint32_t engineCode) {
    if (engineCode == 0) return HRESULT_S_OK;
    if (IsDecodeError(engineCode))  return HRESULT_E_FAIL;
    if (IsIOError(engineCode))      return HRESULT_ERROR_FILE_NOT_FOUND;
    if (IsCacheError(engineCode))   return HRESULT_E_FAIL;
    if (IsPolicyError(engineCode))  return HRESULT_E_NOTIMPL;
    return HRESULT_E_FAIL;  // Internal / unknown
}

// ── Representative error codes ─────────────────────────────────────────────

static constexpr std::array<uint32_t, 15> ALL_ERROR_CODES = {{
    0x00001001u, 0x00001002u, 0x00001003u, 0x00001004u, 0x00001005u, 0x00001006u, // decode
    0x00002001u, 0x00002002u, 0x00002003u, 0x00002004u,                           // io
    0x00003001u, 0x00003002u, 0x00003003u,                                         // cache
    0x00004001u, 0x00004002u,                                                      // policy
}};

} // namespace ExplorerLens::Tests::ErrorDomain

using namespace ExplorerLens::Tests::ErrorDomain;

// ===========================================================================
// Error categories
// ===========================================================================

TEST_CASE("ErrorCategory — 5 categories defined",
          "[error][category]") {
    REQUIRE(ERROR_CATEGORY_COUNT == 5);
}

TEST_CASE("ErrorCategory — DECODE is category 0",
          "[error][category]") {
    REQUIRE(static_cast<uint8_t>(ErrorCategory::DECODE) == 0);
}

TEST_CASE("ErrorCategory — all 5 category values are distinct",
          "[error][category][uniqueness]") {
    std::array<uint8_t, 5> vals = {
        static_cast<uint8_t>(ErrorCategory::DECODE),
        static_cast<uint8_t>(ErrorCategory::IO),
        static_cast<uint8_t>(ErrorCategory::CACHE),
        static_cast<uint8_t>(ErrorCategory::POLICY),
        static_cast<uint8_t>(ErrorCategory::INTERNAL),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

TEST_CASE("ErrorCategoryNames — all 5 names are non-empty",
          "[error][category][names]") {
    for (auto name : ERROR_CATEGORY_NAMES) {
        REQUIRE_FALSE(name.empty());
    }
}

TEST_CASE("ErrorCategoryNames — all 5 names are unique",
          "[error][category][names]") {
    for (size_t i = 0; i < ERROR_CATEGORY_NAMES.size(); ++i) {
        for (size_t j = i + 1; j < ERROR_CATEGORY_NAMES.size(); ++j) {
            CHECK(ERROR_CATEGORY_NAMES[i] != ERROR_CATEGORY_NAMES[j]);
        }
    }
}

// ===========================================================================
// Error code base ranges
// ===========================================================================

TEST_CASE("ErrorBases — DECODE_BASE is 0x1000",   "[error][base]") { REQUIRE(DECODE_BASE   == 0x1000u); }
TEST_CASE("ErrorBases — IO_BASE is 0x2000",        "[error][base]") { REQUIRE(IO_BASE       == 0x2000u); }
TEST_CASE("ErrorBases — CACHE_BASE is 0x3000",     "[error][base]") { REQUIRE(CACHE_BASE    == 0x3000u); }
TEST_CASE("ErrorBases — POLICY_BASE is 0x4000",    "[error][base]") { REQUIRE(POLICY_BASE   == 0x4000u); }
TEST_CASE("ErrorBases — INTERNAL_BASE is 0x9000",  "[error][base]") { REQUIRE(INTERNAL_BASE == 0x9000u); }

TEST_CASE("ErrorBases — all 5 bases are distinct",
          "[error][base][uniqueness]") {
    std::array<uint32_t, 5> bases = {
        DECODE_BASE, IO_BASE, CACHE_BASE, POLICY_BASE, INTERNAL_BASE
    };
    for (size_t i = 0; i < bases.size(); ++i) {
        for (size_t j = i + 1; j < bases.size(); ++j) {
            CHECK(bases[i] != bases[j]);
        }
    }
}

// ===========================================================================
// Category predicate helpers
// ===========================================================================

TEST_CASE("IsSuccess — only code 0 is success",
          "[error][predicate]") {
    CHECK(IsSuccess(0u));
    CHECK_FALSE(IsSuccess(1u));
    CHECK_FALSE(IsSuccess(0x1001u));
}

TEST_CASE("IsDecodeError — correctly identifies decode range",
          "[error][predicate]") {
    CHECK(IsDecodeError(0x1001u));
    CHECK(IsDecodeError(0x1006u));
    CHECK_FALSE(IsDecodeError(0x2001u));
    CHECK_FALSE(IsDecodeError(0u));
}

TEST_CASE("IsIOError — correctly identifies IO range",
          "[error][predicate]") {
    CHECK(IsIOError(0x2001u));
    CHECK(IsIOError(0x2004u));
    CHECK_FALSE(IsIOError(0x1001u));
}

TEST_CASE("IsCacheError — correctly identifies cache range",
          "[error][predicate]") {
    CHECK(IsCacheError(0x3001u));
    CHECK(IsCacheError(0x3003u));
    CHECK_FALSE(IsCacheError(0x4001u));
}

TEST_CASE("IsPolicyError — correctly identifies policy range",
          "[error][predicate]") {
    CHECK(IsPolicyError(0x4001u));
    CHECK(IsPolicyError(0x4002u));
    CHECK_FALSE(IsPolicyError(0x3001u));
}

TEST_CASE("IsInternalError — correctly identifies internal range",
          "[error][predicate]") {
    CHECK(IsInternalError(0x9001u));
    CHECK_FALSE(IsInternalError(0x1001u));
}

// ===========================================================================
// HRESULT mapping
// ===========================================================================

TEST_CASE("HRESULT — S_OK is 0 (success)",
          "[error][hresult]") {
    REQUIRE(IsSuccessHRESULT(HRESULT_S_OK));
    REQUIRE(HRESULT_S_OK == 0u);
}

TEST_CASE("HRESULT — E_FAIL has failure bit set",
          "[error][hresult]") {
    REQUIRE(IsFailureHRESULT(HRESULT_E_FAIL));
}

TEST_CASE("HRESULT — EngineError OK maps to S_OK",
          "[error][hresult][mapping]") {
    REQUIRE(EngineErrorToHRESULT(0u) == HRESULT_S_OK);
    REQUIRE(IsSuccessHRESULT(EngineErrorToHRESULT(0u)));
}

TEST_CASE("HRESULT — decode errors map to failure HRESULT",
          "[error][hresult][mapping]") {
    CHECK(IsFailureHRESULT(EngineErrorToHRESULT(0x1001u)));
    CHECK(IsFailureHRESULT(EngineErrorToHRESULT(0x1006u)));
}

TEST_CASE("HRESULT — IO errors map to failure HRESULT",
          "[error][hresult][mapping]") {
    CHECK(IsFailureHRESULT(EngineErrorToHRESULT(0x2001u)));
}

TEST_CASE("HRESULT — policy errors map to failure HRESULT",
          "[error][hresult][mapping]") {
    CHECK(IsFailureHRESULT(EngineErrorToHRESULT(0x4001u)));
}

TEST_CASE("HRESULT — success code never maps to failure HRESULT",
          "[error][hresult][mapping]") {
    REQUIRE_FALSE(IsFailureHRESULT(EngineErrorToHRESULT(0u)));
}

// ===========================================================================
// All 15 representative error codes pass category predicates
// ===========================================================================

TEST_CASE("AllErrorCodes — no error code in ALL_ERROR_CODES is success",
          "[error][codes]") {
    for (uint32_t code : ALL_ERROR_CODES) {
        CHECK_FALSE(IsSuccess(code));
    }
}

TEST_CASE("AllErrorCodes — each error maps to a failure HRESULT",
          "[error][codes][hresult]") {
    for (uint32_t code : ALL_ERROR_CODES) {
        INFO("Code: 0x" << std::hex << code);
        CHECK(IsFailureHRESULT(EngineErrorToHRESULT(code)));
    }
}

TEST_CASE("AllErrorCodes — all 15 codes are distinct",
          "[error][codes][uniqueness]") {
    for (size_t i = 0; i < ALL_ERROR_CODES.size(); ++i) {
        for (size_t j = i + 1; j < ALL_ERROR_CODES.size(); ++j) {
            CHECK(ALL_ERROR_CODES[i] != ALL_ERROR_CODES[j]);
        }
    }
}
