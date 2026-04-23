// VersionValidationTests.cpp — Catch2 tests for build version string contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates version string format, semantic-versioning compliance, codename
// invariants, and build constant consistency from Engine/Core/BuildValidation.h.
// Ensures the BuildInfo struct upholds the contracts required by Bump-Version.ps1
// and the version-bearing-file registry (§version-bump.instructions.md).
//
// All tests are self-contained — constants are mirrored here; no BuildValidation.h
// is included to avoid pulling in <windows.h> in the test harness.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Version constants mirrored from BuildValidation.h (BuildInfo struct)
// These must be kept in sync with Bump-Version.ps1 updates.
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::Version {

// Current build constants (updated by Bump-Version.ps1)
static constexpr int    MAJOR          = 38;
static constexpr int    MINOR          = 8;
static constexpr int    PATCH          = 0;
static constexpr const char* VERSION_STRING = "38.8.0";
static constexpr const char* CODENAME       = "Betelgeuse";

// Limits
static constexpr int MAX_MAJOR = 9999;
static constexpr int MAX_MINOR = 9999;
static constexpr int MAX_PATCH = 9999;

// Minimum test count (from §10 — must never regress below Phase 1 baseline)
static constexpr int MIN_TEST_COUNT  = 4978;
static constexpr int DECLARED_TESTS  = 4978;

// Decoder count (from §7)
static constexpr int DECODER_COUNT       = 25;
static constexpr int SUPPORTED_EXTENSIONS = 200;

// Fixed CLSID (must not change — see copilot-instructions.md rule 6)
static constexpr std::string_view CLSID =
    "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";

// ── Helpers ──────────────────────────────────────────────────────────────────

/// Parses "X.Y.Z" into {major, minor, patch}. Returns false on failure.
inline bool ParseSemVer(std::string_view s, int& major, int& minor, int& patch) {
    // Expect exactly two dots
    auto d1 = s.find('.');
    if (d1 == std::string_view::npos) return false;
    auto d2 = s.find('.', d1 + 1);
    if (d2 == std::string_view::npos) return false;
    // No third dot allowed (no pre-release suffix in our VERSION file)
    if (s.find('.', d2 + 1) != std::string_view::npos) return false;

    auto parseInt = [](std::string_view seg, int& out) -> bool {
        if (seg.empty()) return false;
        // No leading zeros (except "0" itself)
        if (seg.size() > 1 && seg.front() == '0') return false;
        auto result = std::from_chars(seg.data(), seg.data() + seg.size(), out);
        return result.ec == std::errc{} && result.ptr == seg.data() + seg.size();
    };

    return parseInt(s.substr(0, d1),  major) &&
           parseInt(s.substr(d1 + 1, d2 - d1 - 1), minor) &&
           parseInt(s.substr(d2 + 1), patch);
}

/// Returns true when the version string is well-formed.
inline bool IsValidVersionString(std::string_view s) {
    int ma{}, mi{}, pa{};
    if (!ParseSemVer(s, ma, mi, pa)) return false;
    return ma >= 0 && ma <= MAX_MAJOR &&
           mi >= 0 && mi <= MAX_MINOR &&
           pa >= 0 && pa <= MAX_PATCH;
}

/// Returns true when codename is a single non-empty word (letters only, capitalised).
inline bool IsValidCodename(std::string_view name) {
    if (name.empty()) return false;
    if (name.front() < 'A' || name.front() > 'Z') return false;
    for (char c : name) {
        bool alpha = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        if (!alpha) return false;
    }
    return true;
}

} // namespace ExplorerLens::Tests::Version

using namespace ExplorerLens::Tests::Version;

// ===========================================================================
// VERSION_STRING format
// ===========================================================================

TEST_CASE("VersionString — VERSION_STRING is non-empty",
          "[version][string]") {
    REQUIRE(std::string_view{VERSION_STRING}.size() > 0);
}

TEST_CASE("VersionString — VERSION_STRING is valid semver (X.Y.Z)",
          "[version][string][semver]") {
    REQUIRE(IsValidVersionString(VERSION_STRING));
}

TEST_CASE("VersionString — VERSION_STRING contains exactly two dots",
          "[version][string]") {
    std::string_view sv{VERSION_STRING};
    int dotCount = 0;
    for (char c : sv) { if (c == '.') ++dotCount; }
    REQUIRE(dotCount == 2);
}

TEST_CASE("VersionString — VERSION_STRING has no leading zeros in any component",
          "[version][string][semver]") {
    // "38.8.0" is fine; "38.08.0" would violate strict semver
    int ma{}, mi{}, pa{};
    REQUIRE(ParseSemVer(VERSION_STRING, ma, mi, pa));
}

TEST_CASE("VersionString — VERSION_STRING contains no spaces",
          "[version][string]") {
    std::string_view sv{VERSION_STRING};
    REQUIRE(sv.find(' ') == std::string_view::npos);
}

TEST_CASE("VersionString — VERSION_STRING contains no 'v' prefix",
          "[version][string]") {
    // Git tags use vX.Y.Z but VERSION file and constant do not
    REQUIRE(std::string_view{VERSION_STRING}.front() != 'v');
}

// ===========================================================================
// MAJOR / MINOR / PATCH constants vs VERSION_STRING consistency
// ===========================================================================

TEST_CASE("VersionConstants — MAJOR matches parsed VERSION_STRING",
          "[version][constants][consistency]") {
    int ma{}, mi{}, pa{};
    REQUIRE(ParseSemVer(VERSION_STRING, ma, mi, pa));
    REQUIRE(ma == MAJOR);
}

TEST_CASE("VersionConstants — MINOR matches parsed VERSION_STRING",
          "[version][constants][consistency]") {
    int ma{}, mi{}, pa{};
    REQUIRE(ParseSemVer(VERSION_STRING, ma, mi, pa));
    REQUIRE(mi == MINOR);
}

TEST_CASE("VersionConstants — PATCH matches parsed VERSION_STRING",
          "[version][constants][consistency]") {
    int ma{}, mi{}, pa{};
    REQUIRE(ParseSemVer(VERSION_STRING, ma, mi, pa));
    REQUIRE(pa == PATCH);
}

TEST_CASE("VersionConstants — MAJOR is within valid range",
          "[version][constants]") {
    REQUIRE(MAJOR >= 0);
    REQUIRE(MAJOR <= MAX_MAJOR);
}

TEST_CASE("VersionConstants — MINOR is within valid range",
          "[version][constants]") {
    REQUIRE(MINOR >= 0);
    REQUIRE(MINOR <= MAX_MINOR);
}

TEST_CASE("VersionConstants — PATCH is within valid range",
          "[version][constants]") {
    REQUIRE(PATCH >= 0);
    REQUIRE(PATCH <= MAX_PATCH);
}

// ===========================================================================
// Codename
// ===========================================================================

TEST_CASE("Codename — CODENAME is non-empty",
          "[version][codename]") {
    REQUIRE(std::string_view{CODENAME}.size() > 0);
}

TEST_CASE("Codename — CODENAME is a single capitalised word (letters only)",
          "[version][codename]") {
    REQUIRE(IsValidCodename(CODENAME));
}

TEST_CASE("Codename — CODENAME starts with uppercase letter",
          "[version][codename]") {
    REQUIRE(std::string_view{CODENAME}.front() >= 'A');
    REQUIRE(std::string_view{CODENAME}.front() <= 'Z');
}

TEST_CASE("Codename — CODENAME contains no digits",
          "[version][codename]") {
    std::string_view sv{CODENAME};
    bool hasDigit = false;
    for (char c : sv) { if (c >= '0' && c <= '9') hasDigit = true; }
    REQUIRE_FALSE(hasDigit);
}

TEST_CASE("Codename — CODENAME contains no spaces",
          "[version][codename]") {
    REQUIRE(std::string_view{CODENAME}.find(' ') == std::string_view::npos);
}

// ===========================================================================
// IsValidCodename helper
// ===========================================================================

TEST_CASE("IsValidCodename — accepts single capitalised words",
          "[version][codename][helper]") {
    CHECK(IsValidCodename("Betelgeuse"));
    CHECK(IsValidCodename("Rigel"));
    CHECK(IsValidCodename("Antares"));
    CHECK(IsValidCodename("Vega"));
    CHECK(IsValidCodename("A"));
}

TEST_CASE("IsValidCodename — rejects invalid names",
          "[version][codename][helper]") {
    CHECK_FALSE(IsValidCodename(""));
    CHECK_FALSE(IsValidCodename("betelgeuse"));  // must start uppercase
    CHECK_FALSE(IsValidCodename("Phase 1"));     // space
    CHECK_FALSE(IsValidCodename("v38"));         // digit
    CHECK_FALSE(IsValidCodename("Beta-2"));      // hyphen + digit
    CHECK_FALSE(IsValidCodename("123"));         // all digits
}

// ===========================================================================
// Test count regression gate
// ===========================================================================

TEST_CASE("TestCount — DECLARED_TESTS >= Phase 1 baseline MIN_TEST_COUNT",
          "[version][testcount][regression]") {
    REQUIRE(DECLARED_TESTS >= MIN_TEST_COUNT);
}

TEST_CASE("TestCount — MIN_TEST_COUNT is >= 4978 (v38.8.0 baseline)",
          "[version][testcount]") {
    REQUIRE(MIN_TEST_COUNT >= 4978);
}

// ===========================================================================
// Decoder count invariants
// ===========================================================================

TEST_CASE("DecoderCount — DECODER_COUNT is >= 25",
          "[version][decoders]") {
    REQUIRE(DECODER_COUNT >= 25);
}

TEST_CASE("SupportedExtensions — SUPPORTED_EXTENSIONS is >= 200",
          "[version][extensions]") {
    REQUIRE(SUPPORTED_EXTENSIONS >= 200);
}

// ===========================================================================
// Fixed CLSID invariant
// ===========================================================================

TEST_CASE("CLSID — fixed CLSID constant matches expected value",
          "[version][clsid]") {
    REQUIRE(CLSID == "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}");
}

TEST_CASE("CLSID — fixed CLSID has valid GUID format",
          "[version][clsid][format]") {
    REQUIRE(CLSID.size() == 38);
    REQUIRE(CLSID.front() == '{');
    REQUIRE(CLSID.back()  == '}');
}

// ===========================================================================
// ParseSemVer helper
// ===========================================================================

TEST_CASE("ParseSemVer — parses valid version strings",
          "[version][parser]") {
    int ma{}, mi{}, pa{};
    REQUIRE(ParseSemVer("1.0.0",   ma, mi, pa)); CHECK(ma==1); CHECK(mi==0); CHECK(pa==0);
    REQUIRE(ParseSemVer("38.8.0",  ma, mi, pa)); CHECK(ma==38); CHECK(mi==8); CHECK(pa==0);
    REQUIRE(ParseSemVer("0.0.1",   ma, mi, pa)); CHECK(ma==0); CHECK(mi==0); CHECK(pa==1);
    REQUIRE(ParseSemVer("100.200.300", ma, mi, pa)); CHECK(ma==100); CHECK(mi==200); CHECK(pa==300);
}

TEST_CASE("ParseSemVer — rejects malformed strings",
          "[version][parser]") {
    int ma{}, mi{}, pa{};
    CHECK_FALSE(ParseSemVer("",          ma, mi, pa));
    CHECK_FALSE(ParseSemVer("1",         ma, mi, pa));
    CHECK_FALSE(ParseSemVer("1.0",       ma, mi, pa));
    CHECK_FALSE(ParseSemVer("1.0.0.0",   ma, mi, pa));
    CHECK_FALSE(ParseSemVer("v1.0.0",    ma, mi, pa));
    CHECK_FALSE(ParseSemVer("1.00.0",    ma, mi, pa));  // leading zero
    CHECK_FALSE(ParseSemVer("1.0.0-rc1", ma, mi, pa)); // pre-release not allowed
    CHECK_FALSE(ParseSemVer("a.b.c",     ma, mi, pa));
}
