// ObservabilityTests.cpp — Catch2 tests for ETW provider and logging contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the observability layer contracts: ETW provider name/GUID strings,
// log level enum ordering, event category uniqueness, structured log field
// naming conventions, and event ID assignment rules (§15.2 Observability).
//
// All tests are self-contained — no ETW sessions are opened; contracts are
// validated through type inspection and naming convention checks.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Observability contracts mirroring engine conventions (§15.2)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::Observability {

// ── ETW provider constants ───────────────────────────────────────────────────
//
// Published in ETWSinkComplete.h / ETWTraceProvider.h
static constexpr std::string_view ETW_PROVIDER_NAME     = "ExplorerLens-Engine";
static constexpr std::string_view ETW_PROVIDER_GUID_STR =
    "{E9A3B4C2-D456-7890-ABCD-EF1234567890}";

// ── Windows Event Log source ─────────────────────────────────────────────────
static constexpr std::string_view EVENT_LOG_SOURCE      = "ExplorerLens";
static constexpr std::string_view EVENT_LOG_APPLICATION = "Application";

// ── Log level enum ────────────────────────────────────────────────────────────
enum class LogLevel : uint8_t {
    CRITICAL = 0,   // decode crash, COM failure
    ERROR_   = 1,   // decode failure (per-file)
    WARNING  = 2,   // fallback used, performance miss
    INFO     = 3,   // decode success, cache hit/miss
    VERBOSE  = 4,   // per-format probe result
    DEBUG_   = 5,   // byte-level trace
};

// ── Event categories ─────────────────────────────────────────────────────────
enum class EventCategory : uint16_t {
    DECODE_SUCCESS  = 0x0001,
    DECODE_FAILURE  = 0x0002,
    CACHE_HIT       = 0x0010,
    CACHE_MISS      = 0x0011,
    CACHE_EVICT     = 0x0012,
    PROBE_SUCCESS   = 0x0020,
    PROBE_FAILURE   = 0x0021,
    FORMAT_DETECT   = 0x0030,
    PERF_BUDGET_HIT = 0x0040,
    PERF_BUDGET_MISS= 0x0041,
    COM_REGISTER    = 0x0100,
    COM_UNREGISTER  = 0x0101,
    STARTUP         = 0x0200,
    SHUTDOWN        = 0x0201,
};

// ── Structured log fields ─────────────────────────────────────────────────────
// Field names follow snake_case and must stay stable across versions.
static constexpr std::string_view FIELD_FILE_PATH     = "file_path";
static constexpr std::string_view FIELD_FORMAT_ID     = "format_id";
static constexpr std::string_view FIELD_DECODE_MS     = "decode_ms";
static constexpr std::string_view FIELD_THUMB_SIZE    = "thumb_size";
static constexpr std::string_view FIELD_DECODER_ID    = "decoder_id";
static constexpr std::string_view FIELD_CACHE_KEY     = "cache_key";
static constexpr std::string_view FIELD_ERROR_CODE    = "error_code";
static constexpr std::string_view FIELD_SESSION_ID    = "session_id";
static constexpr std::string_view FIELD_THREAD_ID     = "thread_id";
static constexpr std::string_view FIELD_TIMESTAMP_US  = "timestamp_us";

/// Returns true when a GUID string matches the canonical format {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
inline bool IsValidGuidFormat(std::string_view s) {
    // Pattern: {8-4-4-4-12} hex digits plus braces and hyphens = 38 chars
    if (s.size() != 38) return false;
    if (s.front() != '{' || s.back() != '}') return false;
    // Positions of hyphens: 9, 14, 19, 24
    if (s[9] != '-' || s[14] != '-' || s[19] != '-' || s[24] != '-') return false;
    // All other chars must be hex digits
    const std::string_view inner = s.substr(1, 36);
    for (size_t i = 0; i < inner.size(); ++i) {
        char c = inner[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') return false;
        } else {
            bool hex = (c >= '0' && c <= '9') ||
                       (c >= 'A' && c <= 'F') ||
                       (c >= 'a' && c <= 'f');
            if (!hex) return false;
        }
    }
    return true;
}

/// Returns true when a log field name follows snake_case (a-z, 0-9, underscore, starts with letter)
inline bool IsValidFieldName(std::string_view name) {
    if (name.empty()) return false;
    char first = name.front();
    if (first < 'a' || first > 'z') return false;
    for (char c : name) {
        bool ok = (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  (c == '_');
        if (!ok) return false;
    }
    return true;
}

} // namespace ExplorerLens::Tests::Observability

using namespace ExplorerLens::Tests::Observability;

// ===========================================================================
// ETW provider identity
// ===========================================================================

TEST_CASE("ETWProvider — name is ExplorerLens-Engine",
          "[observability][etw][provider]") {
    REQUIRE(ETW_PROVIDER_NAME == "ExplorerLens-Engine");
}

TEST_CASE("ETWProvider — name is non-empty",
          "[observability][etw][provider]") {
    REQUIRE_FALSE(ETW_PROVIDER_NAME.empty());
}

TEST_CASE("ETWProvider — GUID string has valid format",
          "[observability][etw][guid]") {
    REQUIRE(IsValidGuidFormat(ETW_PROVIDER_GUID_STR));
}

TEST_CASE("ETWProvider — GUID starts with brace",
          "[observability][etw][guid]") {
    REQUIRE(ETW_PROVIDER_GUID_STR.front() == '{');
    REQUIRE(ETW_PROVIDER_GUID_STR.back()  == '}');
}

// ===========================================================================
// Windows Event Log
// ===========================================================================

TEST_CASE("EventLog — source name is ExplorerLens",
          "[observability][eventlog]") {
    REQUIRE(EVENT_LOG_SOURCE == "ExplorerLens");
}

TEST_CASE("EventLog — log name is Application",
          "[observability][eventlog]") {
    REQUIRE(EVENT_LOG_APPLICATION == "Application");
}

TEST_CASE("EventLog — source name is non-empty and < 256 chars",
          "[observability][eventlog][limits]") {
    REQUIRE_FALSE(EVENT_LOG_SOURCE.empty());
    REQUIRE(EVENT_LOG_SOURCE.size() < 256);
}

// ===========================================================================
// LogLevel enum ordering
// ===========================================================================

TEST_CASE("LogLevel — CRITICAL is lowest (0), DEBUG is highest (5)",
          "[observability][loglevel]") {
    REQUIRE(static_cast<uint8_t>(LogLevel::CRITICAL) == 0);
    REQUIRE(static_cast<uint8_t>(LogLevel::DEBUG_)   == 5);
}

TEST_CASE("LogLevel — levels are strictly ordered",
          "[observability][loglevel]") {
    REQUIRE(static_cast<uint8_t>(LogLevel::CRITICAL) <
            static_cast<uint8_t>(LogLevel::ERROR_));
    REQUIRE(static_cast<uint8_t>(LogLevel::ERROR_)   <
            static_cast<uint8_t>(LogLevel::WARNING));
    REQUIRE(static_cast<uint8_t>(LogLevel::WARNING)  <
            static_cast<uint8_t>(LogLevel::INFO));
    REQUIRE(static_cast<uint8_t>(LogLevel::INFO)     <
            static_cast<uint8_t>(LogLevel::VERBOSE));
    REQUIRE(static_cast<uint8_t>(LogLevel::VERBOSE)  <
            static_cast<uint8_t>(LogLevel::DEBUG_));
}

TEST_CASE("LogLevel — all 6 levels have distinct values",
          "[observability][loglevel][uniqueness]") {
    std::array<uint8_t, 6> vals = {
        static_cast<uint8_t>(LogLevel::CRITICAL),
        static_cast<uint8_t>(LogLevel::ERROR_),
        static_cast<uint8_t>(LogLevel::WARNING),
        static_cast<uint8_t>(LogLevel::INFO),
        static_cast<uint8_t>(LogLevel::VERBOSE),
        static_cast<uint8_t>(LogLevel::DEBUG_),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            REQUIRE(vals[i] != vals[j]);
        }
    }
}

// ===========================================================================
// EventCategory uniqueness and grouping
// ===========================================================================

TEST_CASE("EventCategory — decode events have distinct IDs",
          "[observability][events][uniqueness]") {
    REQUIRE(static_cast<uint16_t>(EventCategory::DECODE_SUCCESS) !=
            static_cast<uint16_t>(EventCategory::DECODE_FAILURE));
}

TEST_CASE("EventCategory — cache events have distinct IDs",
          "[observability][events][uniqueness]") {
    auto hit   = static_cast<uint16_t>(EventCategory::CACHE_HIT);
    auto miss  = static_cast<uint16_t>(EventCategory::CACHE_MISS);
    auto evict = static_cast<uint16_t>(EventCategory::CACHE_EVICT);
    REQUIRE(hit != miss);
    REQUIRE(hit != evict);
    REQUIRE(miss != evict);
}

TEST_CASE("EventCategory — perf budget events have distinct IDs",
          "[observability][events][uniqueness]") {
    REQUIRE(static_cast<uint16_t>(EventCategory::PERF_BUDGET_HIT) !=
            static_cast<uint16_t>(EventCategory::PERF_BUDGET_MISS));
}

TEST_CASE("EventCategory — all 14 category values are unique",
          "[observability][events][uniqueness]") {
    std::vector<uint16_t> ids = {
        static_cast<uint16_t>(EventCategory::DECODE_SUCCESS),
        static_cast<uint16_t>(EventCategory::DECODE_FAILURE),
        static_cast<uint16_t>(EventCategory::CACHE_HIT),
        static_cast<uint16_t>(EventCategory::CACHE_MISS),
        static_cast<uint16_t>(EventCategory::CACHE_EVICT),
        static_cast<uint16_t>(EventCategory::PROBE_SUCCESS),
        static_cast<uint16_t>(EventCategory::PROBE_FAILURE),
        static_cast<uint16_t>(EventCategory::FORMAT_DETECT),
        static_cast<uint16_t>(EventCategory::PERF_BUDGET_HIT),
        static_cast<uint16_t>(EventCategory::PERF_BUDGET_MISS),
        static_cast<uint16_t>(EventCategory::COM_REGISTER),
        static_cast<uint16_t>(EventCategory::COM_UNREGISTER),
        static_cast<uint16_t>(EventCategory::STARTUP),
        static_cast<uint16_t>(EventCategory::SHUTDOWN),
    };
    for (size_t i = 0; i < ids.size(); ++i) {
        for (size_t j = i + 1; j < ids.size(); ++j) {
            INFO("Duplicate at indices " << i << " and " << j
                 << " (value " << ids[i] << ")");
            CHECK(ids[i] != ids[j]);
        }
    }
}

TEST_CASE("EventCategory — startup/shutdown are in the 0x0200 group",
          "[observability][events][grouping]") {
    REQUIRE((static_cast<uint16_t>(EventCategory::STARTUP)  & 0xFF00u) == 0x0200u);
    REQUIRE((static_cast<uint16_t>(EventCategory::SHUTDOWN) & 0xFF00u) == 0x0200u);
}

TEST_CASE("EventCategory — COM events are in the 0x0100 group",
          "[observability][events][grouping]") {
    REQUIRE((static_cast<uint16_t>(EventCategory::COM_REGISTER)   & 0xFF00u) == 0x0100u);
    REQUIRE((static_cast<uint16_t>(EventCategory::COM_UNREGISTER)  & 0xFF00u) == 0x0100u);
}

TEST_CASE("EventCategory — cache events are in the 0x0010 group",
          "[observability][events][grouping]") {
    REQUIRE((static_cast<uint16_t>(EventCategory::CACHE_HIT)   & 0xFFF0u) == 0x0010u);
    REQUIRE((static_cast<uint16_t>(EventCategory::CACHE_MISS)  & 0xFFF0u) == 0x0010u);
    REQUIRE((static_cast<uint16_t>(EventCategory::CACHE_EVICT) & 0xFFF0u) == 0x0010u);
}

// ===========================================================================
// Structured log field names
// ===========================================================================

TEST_CASE("LogFields — all 10 standard field names follow snake_case",
          "[observability][fields][naming]") {
    std::vector<std::string_view> fields = {
        FIELD_FILE_PATH,  FIELD_FORMAT_ID,  FIELD_DECODE_MS,
        FIELD_THUMB_SIZE, FIELD_DECODER_ID, FIELD_CACHE_KEY,
        FIELD_ERROR_CODE, FIELD_SESSION_ID, FIELD_THREAD_ID,
        FIELD_TIMESTAMP_US
    };
    for (auto f : fields) {
        INFO("Field: " << f);
        CHECK(IsValidFieldName(f));
    }
}

TEST_CASE("LogFields — all 10 field names are unique",
          "[observability][fields][uniqueness]") {
    std::vector<std::string_view> fields = {
        FIELD_FILE_PATH,  FIELD_FORMAT_ID,  FIELD_DECODE_MS,
        FIELD_THUMB_SIZE, FIELD_DECODER_ID, FIELD_CACHE_KEY,
        FIELD_ERROR_CODE, FIELD_SESSION_ID, FIELD_THREAD_ID,
        FIELD_TIMESTAMP_US
    };
    for (size_t i = 0; i < fields.size(); ++i) {
        for (size_t j = i + 1; j < fields.size(); ++j) {
            CHECK(fields[i] != fields[j]);
        }
    }
}

TEST_CASE("LogFields — IsValidFieldName rejects uppercase and spaces",
          "[observability][fields][validation]") {
    CHECK_FALSE(IsValidFieldName("FilePathBad"));   // uppercase
    CHECK_FALSE(IsValidFieldName("file path"));     // space
    CHECK_FALSE(IsValidFieldName("1_bad_start"));   // starts with digit
    CHECK_FALSE(IsValidFieldName(""));              // empty
    CHECK_FALSE(IsValidFieldName("file-path"));     // hyphen not allowed
}

TEST_CASE("LogFields — IsValidFieldName accepts valid snake_case names",
          "[observability][fields][validation]") {
    CHECK(IsValidFieldName("file_path"));
    CHECK(IsValidFieldName("decode_ms"));
    CHECK(IsValidFieldName("a"));
    CHECK(IsValidFieldName("a1_b2_c3"));
    CHECK(IsValidFieldName("timestamp_us"));
}

// ===========================================================================
// GUID format validation
// ===========================================================================

TEST_CASE("IsValidGuidFormat — canonical GUID format accepted",
          "[observability][guid][format]") {
    REQUIRE(IsValidGuidFormat("{00000000-0000-0000-0000-000000000000}"));
    REQUIRE(IsValidGuidFormat("{FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF}"));
    REQUIRE(IsValidGuidFormat("{abcdef12-3456-7890-ABCD-EF1234567890}"));
}

TEST_CASE("IsValidGuidFormat — malformed GUIDs rejected",
          "[observability][guid][format]") {
    CHECK_FALSE(IsValidGuidFormat(""));
    CHECK_FALSE(IsValidGuidFormat("{00000000-0000-0000-0000-0000000000}"));   // too short
    CHECK_FALSE(IsValidGuidFormat("00000000-0000-0000-0000-000000000000"));   // no braces
    CHECK_FALSE(IsValidGuidFormat("{GGGGGGGG-0000-0000-0000-000000000000}")); // non-hex
    CHECK_FALSE(IsValidGuidFormat("{00000000X0000-0000-0000-000000000000}")); // bad separator
}
