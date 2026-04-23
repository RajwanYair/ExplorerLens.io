// ResultTypeTests.cpp — Catch2 tests for Result<T,E> error monad
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the fundamental Result<T,E> type used throughout the decode
// pipeline for structured error propagation (ADR-010, D31, §10.2).
//
// Self-contained: uses Result<int,std::string> and Result<std::string,int>
// to avoid windows.h header ordering sensitivity in cross-platform builds.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

// Include the production header
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../Core/ResultType.h"

#include <string>
#include <vector>

using namespace ExplorerLens::Engine;

// Convenience factories for tests
static Result<int, std::string> OkInt(int v)  { return Result<int,std::string>(OkTag{}, v); }
static Result<int, std::string> ErrStr(const std::string& e)
    { return Result<int,std::string>(ErrTag{}, e); }

// =============================================================================
// §1 — Basic construction and state inspection
// =============================================================================

TEST_CASE("Result: Ok result IsOk=true, IsErr=false", "[result][construction]") {
    auto r = OkInt(42);
    REQUIRE(r.IsOk());
    REQUIRE_FALSE(r.IsErr());
}

TEST_CASE("Result: Err result IsOk=false, IsErr=true", "[result][construction]") {
    auto r = ErrStr("decode failed");
    REQUIRE_FALSE(r.IsOk());
    REQUIRE(r.IsErr());
}

TEST_CASE("Result: Ok value is accessible", "[result][access]") {
    auto r = OkInt(99);
    REQUIRE(r.Value() == 99);
}

TEST_CASE("Result: Err message is accessible", "[result][access]") {
    auto r = ErrStr("out of memory");
    REQUIRE(r.Error() == "out of memory");
}

TEST_CASE("Result: ValueOr returns value when Ok", "[result][valueor]") {
    auto r = OkInt(7);
    REQUIRE(r.ValueOr(0) == 7);
}

TEST_CASE("Result: ValueOr returns default when Err", "[result][valueor]") {
    auto r = ErrStr("missing");
    REQUIRE(r.ValueOr(-1) == -1);
}

TEST_CASE("Result: Ok with zero value", "[result][edge]") {
    auto r = OkInt(0);
    REQUIRE(r.IsOk());
    REQUIRE(r.Value() == 0);
}

TEST_CASE("Result: Ok with negative value", "[result][edge]") {
    auto r = OkInt(-1);
    REQUIRE(r.IsOk());
    REQUIRE(r.Value() == -1);
}

// =============================================================================
// §2 — Copy and move semantics
// =============================================================================

TEST_CASE("Result: copy construction preserves Ok state", "[result][copy]") {
    auto r1 = OkInt(5);
    auto r2 = r1;  // copy
    REQUIRE(r2.IsOk());
    REQUIRE(r2.Value() == 5);
}

TEST_CASE("Result: copy construction preserves Err state", "[result][copy]") {
    auto r1 = ErrStr("bad data");
    auto r2 = r1;
    REQUIRE(r2.IsErr());
    REQUIRE(r2.Error() == "bad data");
}

TEST_CASE("Result: move construction takes value", "[result][move]") {
    auto r1 = OkInt(123);
    auto r2 = std::move(r1);
    REQUIRE(r2.IsOk());
    REQUIRE(r2.Value() == 123);
}

TEST_CASE("Result: copy assignment Ok", "[result][assignment]") {
    auto r1 = OkInt(10);
    auto r2 = ErrStr("old error");
    r2 = r1;
    REQUIRE(r2.IsOk());
    REQUIRE(r2.Value() == 10);
}

TEST_CASE("Result: copy assignment Err", "[result][assignment]") {
    auto r1 = ErrStr("new error");
    auto r2 = OkInt(5);
    r2 = r1;
    REQUIRE(r2.IsErr());
    REQUIRE(r2.Error() == "new error");
}

TEST_CASE("Result: self-assignment is safe", "[result][assignment]") {
    auto r = OkInt(42);
    r = r;  // NOLINT(self-assign)
    REQUIRE(r.IsOk());
    REQUIRE(r.Value() == 42);
}

// =============================================================================
// §3 — Map: transform success value, pass error through
// =============================================================================

TEST_CASE("Result: Map transforms Ok value", "[result][map]") {
    auto r = OkInt(4);
    auto mapped = r.Map([](int v) { return v * v; });
    REQUIRE(mapped.IsOk());
    REQUIRE(mapped.Value() == 16);
}

TEST_CASE("Result: Map passes Err through unchanged", "[result][map]") {
    auto r = ErrStr("error propagated");
    auto mapped = r.Map([](int v) { return v * 2; });
    REQUIRE(mapped.IsErr());
    REQUIRE(mapped.Error() == "error propagated");
}

TEST_CASE("Result: Map can change the value type", "[result][map]") {
    auto r = OkInt(256);
    auto mapped = r.Map([](int v) { return std::to_string(v); });
    REQUIRE(mapped.IsOk());
    REQUIRE(mapped.Value() == "256");
}

// =============================================================================
// §4 — MapErr: transform error, pass value through
// =============================================================================

TEST_CASE("Result: MapErr transforms Err value", "[result][maperr]") {
    auto r = ErrStr("raw error");
    auto mapped = r.MapErr([](const std::string& s) { return s.size(); });
    REQUIRE(mapped.IsErr());
    REQUIRE(mapped.Error() == 9u);  // "raw error".size()
}

TEST_CASE("Result: MapErr passes Ok through unchanged", "[result][maperr]") {
    auto r = OkInt(77);
    auto mapped = r.MapErr([](const std::string&) { return 0; });
    REQUIRE(mapped.IsOk());
    REQUIRE(mapped.Value() == 77);
}

// =============================================================================
// §5 — AndThen: flatMap / chain of fallible operations
// =============================================================================

TEST_CASE("Result: AndThen chains two Ok operations", "[result][andthen]") {
    auto r = OkInt(10);
    auto chained = r.AndThen([](int v) -> Result<int,std::string> {
        return Result<int,std::string>(OkTag{}, v + 5);
    });
    REQUIRE(chained.IsOk());
    REQUIRE(chained.Value() == 15);
}

TEST_CASE("Result: AndThen short-circuits on first Err", "[result][andthen]") {
    auto r = ErrStr("short circuit");
    auto chained = r.AndThen([](int v) -> Result<int,std::string> {
        return Result<int,std::string>(OkTag{}, v + 99);
    });
    REQUIRE(chained.IsErr());
    REQUIRE(chained.Error() == "short circuit");
}

TEST_CASE("Result: AndThen propagates inner Err", "[result][andthen]") {
    auto r = OkInt(5);
    auto chained = r.AndThen([](int) -> Result<int,std::string> {
        return Result<int,std::string>(ErrTag{}, std::string("inner failure"));
    });
    REQUIRE(chained.IsErr());
    REQUIRE(chained.Error() == "inner failure");
}

TEST_CASE("Result: AndThen three-step pipeline all Ok", "[result][andthen]") {
    auto r = OkInt(1);
    auto result = r
        .AndThen([](int v) -> Result<int,std::string> {
            return {OkTag{}, v * 2};
        })
        .AndThen([](int v) -> Result<int,std::string> {
            return {OkTag{}, v + 10};
        })
        .AndThen([](int v) -> Result<int,std::string> {
            return {OkTag{}, v * v};
        });
    REQUIRE(result.IsOk());
    REQUIRE(result.Value() == 144);  // ((1*2)+10)^2 = 12^2
}

// =============================================================================
// §6 — Result with string value type
// =============================================================================

TEST_CASE("Result<string,int>: Ok string", "[result][string]") {
    Result<std::string, int> r{OkTag{}, std::string("hello")};
    REQUIRE(r.IsOk());
    REQUIRE(r.Value() == "hello");
}

TEST_CASE("Result<string,int>: Err int code", "[result][string]") {
    Result<std::string, int> r{ErrTag{}, 404};
    REQUIRE(r.IsErr());
    REQUIRE(r.Error() == 404);
}

// =============================================================================
// §7 — ValueOr with rvalue default
// =============================================================================

TEST_CASE("Result: ValueOr with rvalue default when Ok", "[result][valueor]") {
    auto r = OkInt(3);
    std::string fallback = "fallback";
    // Use int ValueOr
    REQUIRE(r.ValueOr(0) == 3);
}

TEST_CASE("Result: ValueOr with rvalue default when Err", "[result][valueor]") {
    auto r = ErrStr("none");
    REQUIRE(r.ValueOr(999) == 999);
}

// =============================================================================
// §8 — Realistic decode-pipeline use case
// =============================================================================

namespace {

Result<int, std::string> ParseWidth(const std::string& token) {
    try {
        int v = std::stoi(token);
        if (v <= 0) return {ErrTag{}, std::string("width must be positive")};
        if (v > 65535) return {ErrTag{}, std::string("width exceeds 65535")};
        return {OkTag{}, v};
    } catch (...) {
        return {ErrTag{}, std::string("not a number: ") + token};
    }
}

Result<int, std::string> ParseHeight(const std::string& token) {
    return ParseWidth(token);  // same rules
}

Result<std::pair<int,int>, std::string> ParseDimensions(
    const std::string& w, const std::string& h)
{
    return ParseWidth(w).AndThen([&h](int width) {
        return ParseHeight(h).Map([width](int height) {
            return std::make_pair(width, height);
        });
    });
}

} // anonymous namespace

TEST_CASE("Pipeline: parse valid dimensions", "[result][pipeline]") {
    auto r = ParseDimensions("256", "256");
    REQUIRE(r.IsOk());
    auto [w, h] = r.Value();
    REQUIRE(w == 256);
    REQUIRE(h == 256);
}

TEST_CASE("Pipeline: invalid width rejects pipeline", "[result][pipeline]") {
    auto r = ParseDimensions("abc", "256");
    REQUIRE(r.IsErr());
    REQUIRE_THAT(r.Error(), Catch::Matchers::ContainsSubstring("not a number"));
}

TEST_CASE("Pipeline: negative width rejects pipeline", "[result][pipeline]") {
    auto r = ParseDimensions("-1", "256");
    REQUIRE(r.IsErr());
    REQUIRE_THAT(r.Error(), Catch::Matchers::ContainsSubstring("positive"));
}

TEST_CASE("Pipeline: zero width rejects pipeline", "[result][pipeline]") {
    auto r = ParseDimensions("0", "100");
    REQUIRE(r.IsErr());
}

TEST_CASE("Pipeline: oversized width rejects pipeline", "[result][pipeline]") {
    auto r = ParseDimensions("100000", "100");
    REQUIRE(r.IsErr());
    REQUIRE_THAT(r.Error(), Catch::Matchers::ContainsSubstring("65535"));
}

TEST_CASE("Pipeline: valid width but invalid height", "[result][pipeline]") {
    auto r = ParseDimensions("640", "bad");
    REQUIRE(r.IsErr());
    REQUIRE_THAT(r.Error(), Catch::Matchers::ContainsSubstring("not a number"));
}
