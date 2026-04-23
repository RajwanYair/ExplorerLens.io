// StdExpectedContractTests.cpp — Catch2 tests for std::expected<T,E> contract patterns
// Copyright (c) 2026 ExplorerLens Project
//
// Tests the std::expected<T,E> contract patterns used throughout ExplorerLens
// (ADR-015: std::expected as the primary error-propagation mechanism).
//
// All tests are self-contained — no Engine headers required.
// The EngineError enum and helper types below mirror the production design.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ExplorerLens::Tests::StdExpectedContract {

//==============================================================================
// Test-local EngineError — mirrors the production enum design (ADR-015)
//==============================================================================
enum class EngineError : uint32_t {
    OK                 = 0,
    INVALID_ARG        = 1,
    IO_ERROR           = 2,
    UNSUPPORTED_FORMAT = 3,
    DECODE_FAILED      = 4,
    OUT_OF_MEMORY      = 5,
    TIMEOUT            = 6,
    NOT_INITIALIZED    = 7,
    ACCESS_DENIED      = 8,
    CORRUPT_DATA       = 9,
};

// Map EngineError to HRESULT (mirrors Engine production mapping)
static HRESULT ToHRESULT(EngineError e) noexcept
{
    switch (e) {
        case EngineError::OK:                 return S_OK;
        case EngineError::INVALID_ARG:        return E_INVALIDARG;
        case EngineError::IO_ERROR:           return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
        case EngineError::UNSUPPORTED_FORMAT: return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        case EngineError::DECODE_FAILED:      return E_FAIL;
        case EngineError::OUT_OF_MEMORY:      return E_OUTOFMEMORY;
        case EngineError::TIMEOUT:            return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        case EngineError::NOT_INITIALIZED:    return E_NOT_VALID_STATE;
        case EngineError::ACCESS_DENIED:      return E_ACCESSDENIED;
        case EngineError::CORRUPT_DATA:       return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
        default:                              return E_UNEXPECTED;
    }
}

// std::expected short-hand alias
template<typename T>
using Expected = std::expected<T, EngineError>;

// Helper: simulated operation that may fail
static Expected<int> SafeDivide(int a, int b)
{
    if (b == 0) return std::unexpected(EngineError::INVALID_ARG);
    return a / b;
}

static Expected<std::string> OpenFile(std::string_view path)
{
    if (path.empty()) return std::unexpected(EngineError::INVALID_ARG);
    if (path == "missing.raw") return std::unexpected(EngineError::IO_ERROR);
    if (path == "bad.xyz")  return std::unexpected(EngineError::UNSUPPORTED_FORMAT);
    return std::string(path);
}

//==============================================================================
// Basic construction and observation
//==============================================================================

TEST_CASE("std::expected: default success construction", "[expected][contract]")
{
    Expected<int> e{42};
    REQUIRE(e.has_value());
    REQUIRE(e.value() == 42);
    REQUIRE(*e == 42);
}

TEST_CASE("std::expected: error construction via std::unexpected", "[expected][contract]")
{
    Expected<int> e = std::unexpected(EngineError::IO_ERROR);
    REQUIRE_FALSE(e.has_value());
    REQUIRE(e.error() == EngineError::IO_ERROR);
}

TEST_CASE("std::expected: value_or returns value when ok", "[expected][contract]")
{
    Expected<int> e{100};
    REQUIRE(e.value_or(-1) == 100);
}

TEST_CASE("std::expected: value_or returns fallback when error", "[expected][contract]")
{
    Expected<int> e = std::unexpected(EngineError::DECODE_FAILED);
    REQUIRE(e.value_or(-1) == -1);
}

//==============================================================================
// EngineError → HRESULT mapping
//==============================================================================

TEST_CASE("ToHRESULT: OK maps to S_OK", "[expected][hresult]")
{
    REQUIRE(ToHRESULT(EngineError::OK) == S_OK);
}

TEST_CASE("ToHRESULT: INVALID_ARG maps to E_INVALIDARG", "[expected][hresult]")
{
    REQUIRE(ToHRESULT(EngineError::INVALID_ARG) == E_INVALIDARG);
}

TEST_CASE("ToHRESULT: OUT_OF_MEMORY maps to E_OUTOFMEMORY", "[expected][hresult]")
{
    REQUIRE(ToHRESULT(EngineError::OUT_OF_MEMORY) == E_OUTOFMEMORY);
}

TEST_CASE("ToHRESULT: DECODE_FAILED maps to E_FAIL", "[expected][hresult]")
{
    REQUIRE(ToHRESULT(EngineError::DECODE_FAILED) == E_FAIL);
}

TEST_CASE("ToHRESULT: ACCESS_DENIED maps to E_ACCESSDENIED", "[expected][hresult]")
{
    REQUIRE(ToHRESULT(EngineError::ACCESS_DENIED) == E_ACCESSDENIED);
}

TEST_CASE("ToHRESULT: all error codes produce FAILED() HRESULT", "[expected][hresult]")
{
    const EngineError errors[] = {
        EngineError::INVALID_ARG, EngineError::IO_ERROR,
        EngineError::UNSUPPORTED_FORMAT, EngineError::DECODE_FAILED,
        EngineError::OUT_OF_MEMORY, EngineError::TIMEOUT,
        EngineError::NOT_INITIALIZED, EngineError::ACCESS_DENIED,
        EngineError::CORRUPT_DATA,
    };
    for (auto err : errors) {
        REQUIRE(FAILED(ToHRESULT(err)));
    }
}

//==============================================================================
// SafeDivide — representative monadic use
//==============================================================================

TEST_CASE("SafeDivide: success case", "[expected][monadic]")
{
    auto r = SafeDivide(10, 2);
    REQUIRE(r.has_value());
    REQUIRE(r.value() == 5);
}

TEST_CASE("SafeDivide: divide by zero returns INVALID_ARG", "[expected][monadic]")
{
    auto r = SafeDivide(10, 0);
    REQUIRE_FALSE(r.has_value());
    REQUIRE(r.error() == EngineError::INVALID_ARG);
}

TEST_CASE("SafeDivide: negative dividend", "[expected][monadic]")
{
    auto r = SafeDivide(-20, 4);
    REQUIRE(r.has_value());
    REQUIRE(r.value() == -5);
}

//==============================================================================
// and_then / or_else / transform composability
//==============================================================================

TEST_CASE("std::expected: and_then chains on success", "[expected][compose]")
{
    auto result = SafeDivide(10, 2)
        .and_then([](int v) -> Expected<int> { return v * 3; });
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 15);
}

TEST_CASE("std::expected: and_then short-circuits on error", "[expected][compose]")
{
    bool chainCalled = false;
    auto result = SafeDivide(10, 0)
        .and_then([&chainCalled](int) -> Expected<int> {
            chainCalled = true;
            return 999;
        });
    REQUIRE_FALSE(result.has_value());
    REQUIRE_FALSE(chainCalled);
    REQUIRE(result.error() == EngineError::INVALID_ARG);
}

TEST_CASE("std::expected: transform maps value", "[expected][compose]")
{
    auto r = SafeDivide(8, 2)
        .transform([](int v) { return std::to_string(v); });
    REQUIRE(r.has_value());
    REQUIRE(r.value() == "4");
}

TEST_CASE("std::expected: transform does not fire on error", "[expected][compose]")
{
    auto r = SafeDivide(8, 0)
        .transform([](int v) { return std::to_string(v); });
    REQUIRE_FALSE(r.has_value());
}

TEST_CASE("std::expected: or_else recovers from error", "[expected][compose]")
{
    auto r = SafeDivide(10, 0)
        .or_else([](EngineError) -> Expected<int> { return -1; });
    REQUIRE(r.has_value());
    REQUIRE(r.value() == -1);
}

TEST_CASE("std::expected: or_else does not fire on success", "[expected][compose]")
{
    bool calledFallback = false;
    auto r = SafeDivide(6, 2)
        .or_else([&calledFallback](EngineError) -> Expected<int> {
            calledFallback = true;
            return -999;
        });
    REQUIRE(r.has_value());
    REQUIRE(r.value() == 3);
    REQUIRE_FALSE(calledFallback);
}

//==============================================================================
// OpenFile — realistic pipeline pattern
//==============================================================================

TEST_CASE("OpenFile: valid path returns path string", "[expected][pipeline]")
{
    auto r = OpenFile("photo.arw");
    REQUIRE(r.has_value());
    REQUIRE(r.value() == "photo.arw");
}

TEST_CASE("OpenFile: empty path returns INVALID_ARG", "[expected][pipeline]")
{
    auto r = OpenFile("");
    REQUIRE_FALSE(r.has_value());
    REQUIRE(r.error() == EngineError::INVALID_ARG);
}

TEST_CASE("OpenFile: missing file returns IO_ERROR", "[expected][pipeline]")
{
    auto r = OpenFile("missing.raw");
    REQUIRE_FALSE(r.has_value());
    REQUIRE(r.error() == EngineError::IO_ERROR);
}

TEST_CASE("OpenFile: unknown extension returns UNSUPPORTED_FORMAT", "[expected][pipeline]")
{
    auto r = OpenFile("bad.xyz");
    REQUIRE_FALSE(r.has_value());
    REQUIRE(r.error() == EngineError::UNSUPPORTED_FORMAT);
}

TEST_CASE("OpenFile: pipeline chain open -> length -> convert", "[expected][pipeline]")
{
    // Simulate: open → get length → map to int
    auto r = OpenFile("img.heic")
        .and_then([](const std::string& s) -> Expected<size_t> {
            if (s.empty()) return std::unexpected(EngineError::INVALID_ARG);
            return s.size();
        })
        .transform([](size_t n) -> int { return static_cast<int>(n); });

    REQUIRE(r.has_value());
    REQUIRE(r.value() == static_cast<int>(std::string("img.heic").size()));
}

//==============================================================================
// Edge cases: Expected<void, E>
//==============================================================================

TEST_CASE("std::expected<void,E>: success default", "[expected][void]")
{
    std::expected<void, EngineError> e;
    REQUIRE(e.has_value());
}

TEST_CASE("std::expected<void,E>: error propagates", "[expected][void]")
{
    std::expected<void, EngineError> e = std::unexpected(EngineError::NOT_INITIALIZED);
    REQUIRE_FALSE(e.has_value());
    REQUIRE(e.error() == EngineError::NOT_INITIALIZED);
}

TEST_CASE("std::expected<void,E>: HRESULT mapping", "[expected][void]")
{
    std::expected<void, EngineError> e = std::unexpected(EngineError::NOT_INITIALIZED);
    REQUIRE(ToHRESULT(e.error()) == E_NOT_VALID_STATE);
}

//==============================================================================
// Copy / move semantics
//==============================================================================

TEST_CASE("std::expected: copy construction preserves value", "[expected][semantics]")
{
    Expected<std::string> a{"hello"};
    Expected<std::string> b = a; // NOLINT
    REQUIRE(b.has_value());
    REQUIRE(b.value() == "hello");
}

TEST_CASE("std::expected: move construction transfers value", "[expected][semantics]")
{
    Expected<std::string> a{"world"};
    Expected<std::string> b = std::move(a);
    REQUIRE(b.has_value());
    REQUIRE(b.value() == "world");
}

TEST_CASE("std::expected: copy construction preserves error", "[expected][semantics]")
{
    Expected<int> a = std::unexpected(EngineError::CORRUPT_DATA);
    Expected<int> b = a; // NOLINT
    REQUIRE_FALSE(b.has_value());
    REQUIRE(b.error() == EngineError::CORRUPT_DATA);
}

} // namespace ExplorerLens::Tests::StdExpectedContract
