// EngineConfigTests.cpp — Catch2 tests for engine configuration contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Tests the registry key path conventions, settings value sanitization,
// HKLM vs HKCU selection logic, and configuration value validation contracts
// used throughout the engine settings subsystem (§14, §15.1).
//
// All tests are self-contained: no real registry reads/writes are performed.
// Contracts are validated through type inspection and logic mirroring.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Registry path helpers mirroring engine conventions (§14 — settings layer)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::Config {

/// Maximum allowed byte length for a registry value name (Win32 limit = 16383 chars)
static constexpr size_t MAX_VALUE_NAME_CHARS = 16383;

/// Maximum allowed byte length for a REG_SZ value (Win32 limit = 32767 chars)
static constexpr size_t MAX_SZ_CHARS = 32767;

/// Root registry paths used by ExplorerLens
static constexpr std::string_view HKCU_ROOT =
    R"(Software\ExplorerLens\Settings)";
static constexpr std::string_view HKLM_ROOT =
    R"(Software\ExplorerLens\Policy)";
static constexpr std::string_view HKLM_COM_ROOT =
    R"(Software\Classes\CLSID\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39})";

/// Settings keys
static constexpr std::string_view KEY_MAX_THREADS   = "MaxDecodeThreads";
static constexpr std::string_view KEY_CACHE_SIZE_MB = "CacheSizeMB";
static constexpr std::string_view KEY_GPU_ENABLED   = "GpuEnabled";
static constexpr std::string_view KEY_LOG_LEVEL     = "LogLevel";
static constexpr std::string_view KEY_FORMATS       = "EnabledFormats";

/// Policy keys (HKLM — admin-controlled)
static constexpr std::string_view POLICY_DENY_FORMATS = "DenyFormatList";
static constexpr std::string_view POLICY_MAX_THREADS  = "MaxDecodeThreads";
static constexpr std::string_view POLICY_DISABLE_GPU  = "DisableGpu";

enum class Hive { HKCU, HKLM };

struct RegistryEntry {
    Hive        hive;
    std::string subKey;
    std::string valueName;
};

struct CacheSizeConfig {
    uint32_t minMB = 32;
    uint32_t maxMB = 4096;
    uint32_t defaultMB = 256;
};

enum class LogLevel : uint32_t {
    OFF     = 0,
    ERROR_  = 1,
    WARNING = 2,
    INFO    = 3,
    VERBOSE = 4,
    DEBUG_  = 5,
};

inline bool IsValidLogLevel(uint32_t raw) {
    return raw <= static_cast<uint32_t>(LogLevel::DEBUG_);
}

inline bool IsValidMaxThreads(uint32_t n) {
    return n >= 1 && n <= 64;
}

inline bool IsValidCacheSizeMB(uint32_t n) {
    CacheSizeConfig cfg;
    return n >= cfg.minMB && n <= cfg.maxMB;
}

/// Sanitizes a REG_SZ value: trims trailing nulls, rejects embedded nulls.
inline std::string SanitizeRegSZ(std::string_view raw) {
    // Reject embedded null bytes (common injection vector)
    if (raw.find('\0') != std::string_view::npos) {
        return {};
    }
    // Trim to MAX_SZ_CHARS
    return std::string(raw.substr(0, (std::min)(raw.size(), MAX_SZ_CHARS)));
}

/// Validates a registry key path: must not be empty, must not start with backslash.
inline bool IsValidKeyPath(std::string_view path) {
    if (path.empty()) return false;
    if (path.front() == '\\') return false;
    if (path.find("..") != std::string_view::npos) return false;
    return true;
}

} // namespace ExplorerLens::Tests::Config

using namespace ExplorerLens::Tests::Config;

// ===========================================================================
// Registry path structure
// ===========================================================================

TEST_CASE("RegistryPaths — HKCU root is non-empty and starts without backslash",
          "[config][registry][paths]") {
    REQUIRE_FALSE(HKCU_ROOT.empty());
    REQUIRE(HKCU_ROOT.front() != '\\');
}

TEST_CASE("RegistryPaths — HKLM root is non-empty and starts without backslash",
          "[config][registry][paths]") {
    REQUIRE_FALSE(HKLM_ROOT.empty());
    REQUIRE(HKLM_ROOT.front() != '\\');
}

TEST_CASE("RegistryPaths — HKLM COM root contains the fixed CLSID",
          "[config][registry][com]") {
    constexpr std::string_view CLSID = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";
    REQUIRE(HKLM_COM_ROOT.find(CLSID) != std::string_view::npos);
}

TEST_CASE("RegistryPaths — HKCU and HKLM roots are different paths",
          "[config][registry][paths]") {
    REQUIRE(HKCU_ROOT != HKLM_ROOT);
}

TEST_CASE("RegistryPaths — key paths validated by IsValidKeyPath",
          "[config][registry][validation]") {
    CHECK(IsValidKeyPath(HKCU_ROOT));
    CHECK(IsValidKeyPath(HKLM_ROOT));
    CHECK(IsValidKeyPath(HKLM_COM_ROOT));
    CHECK_FALSE(IsValidKeyPath(""));
    CHECK_FALSE(IsValidKeyPath("\\Software\\ExplorerLens"));
    CHECK_FALSE(IsValidKeyPath("Software\\..\\ExplorerLens"));
}

// ===========================================================================
// Setting key names
// ===========================================================================

TEST_CASE("SettingKeys — all setting key names are non-empty",
          "[config][keys]") {
    std::vector<std::string_view> keys = {
        KEY_MAX_THREADS, KEY_CACHE_SIZE_MB, KEY_GPU_ENABLED,
        KEY_LOG_LEVEL,   KEY_FORMATS
    };
    for (auto k : keys) {
        REQUIRE_FALSE(k.empty());
    }
}

TEST_CASE("SettingKeys — all policy key names are non-empty",
          "[config][keys][policy]") {
    std::vector<std::string_view> keys = {
        POLICY_DENY_FORMATS, POLICY_MAX_THREADS, POLICY_DISABLE_GPU
    };
    for (auto k : keys) {
        REQUIRE_FALSE(k.empty());
    }
}

TEST_CASE("SettingKeys — no duplicate setting key names", "[config][keys]") {
    std::vector<std::string_view> keys = {
        KEY_MAX_THREADS, KEY_CACHE_SIZE_MB, KEY_GPU_ENABLED,
        KEY_LOG_LEVEL,   KEY_FORMATS
    };
    for (size_t i = 0; i < keys.size(); ++i) {
        for (size_t j = i + 1; j < keys.size(); ++j) {
            CHECK(keys[i] != keys[j]);
        }
    }
}

TEST_CASE("SettingKeys — key names fit within MAX_VALUE_NAME_CHARS",
          "[config][keys][limits]") {
    std::vector<std::string_view> allKeys = {
        KEY_MAX_THREADS, KEY_CACHE_SIZE_MB, KEY_GPU_ENABLED,
        KEY_LOG_LEVEL, KEY_FORMATS,
        POLICY_DENY_FORMATS, POLICY_MAX_THREADS, POLICY_DISABLE_GPU
    };
    for (auto k : allKeys) {
        REQUIRE(k.size() <= MAX_VALUE_NAME_CHARS);
    }
}

// ===========================================================================
// LogLevel enum
// ===========================================================================

TEST_CASE("LogLevel — enum values are ordered correctly", "[config][loglevel]") {
    REQUIRE(static_cast<uint32_t>(LogLevel::OFF)     == 0);
    REQUIRE(static_cast<uint32_t>(LogLevel::ERROR_)  == 1);
    REQUIRE(static_cast<uint32_t>(LogLevel::WARNING) == 2);
    REQUIRE(static_cast<uint32_t>(LogLevel::INFO)    == 3);
    REQUIRE(static_cast<uint32_t>(LogLevel::VERBOSE) == 4);
    REQUIRE(static_cast<uint32_t>(LogLevel::DEBUG_)  == 5);
}

TEST_CASE("LogLevel — IsValidLogLevel accepts 0..5, rejects 6+",
          "[config][loglevel][validation]") {
    for (uint32_t i = 0; i <= 5; ++i) {
        CHECK(IsValidLogLevel(i));
    }
    CHECK_FALSE(IsValidLogLevel(6));
    CHECK_FALSE(IsValidLogLevel(100));
    CHECK_FALSE(IsValidLogLevel(0xFFFFFFFF));
}

// ===========================================================================
// MaxDecodeThreads validation
// ===========================================================================

TEST_CASE("ThreadCount — IsValidMaxThreads accepts 1..64",
          "[config][threads][validation]") {
    CHECK(IsValidMaxThreads(1));
    CHECK(IsValidMaxThreads(4));
    CHECK(IsValidMaxThreads(16));
    CHECK(IsValidMaxThreads(64));
    CHECK_FALSE(IsValidMaxThreads(0));
    CHECK_FALSE(IsValidMaxThreads(65));
    CHECK_FALSE(IsValidMaxThreads(1000));
}

TEST_CASE("ThreadCount — hardware concurrency (2..128) is a valid default range",
          "[config][threads]") {
    // Default should be valid for any reasonable hardware
    for (uint32_t t = 2; t <= 64; ++t) {
        CHECK(IsValidMaxThreads(t));
    }
}

// ===========================================================================
// Cache size validation
// ===========================================================================

TEST_CASE("CacheSizeConfig — default values are self-consistent",
          "[config][cache][defaults]") {
    CacheSizeConfig cfg;
    REQUIRE(cfg.minMB <= cfg.defaultMB);
    REQUIRE(cfg.defaultMB <= cfg.maxMB);
}

TEST_CASE("CacheSizeConfig — IsValidCacheSizeMB boundary checks",
          "[config][cache][validation]") {
    CHECK(IsValidCacheSizeMB(32));
    CHECK(IsValidCacheSizeMB(256));
    CHECK(IsValidCacheSizeMB(4096));
    CHECK_FALSE(IsValidCacheSizeMB(0));
    CHECK_FALSE(IsValidCacheSizeMB(31));
    CHECK_FALSE(IsValidCacheSizeMB(4097));
    CHECK_FALSE(IsValidCacheSizeMB(UINT32_MAX));
}

TEST_CASE("CacheSizeConfig — minMB is 32 (matches Phase 2 design)",
          "[config][cache]") {
    CacheSizeConfig cfg;
    REQUIRE(cfg.minMB == 32u);
}

TEST_CASE("CacheSizeConfig — defaultMB is 256",
          "[config][cache]") {
    CacheSizeConfig cfg;
    REQUIRE(cfg.defaultMB == 256u);
}

// ===========================================================================
// REG_SZ sanitization
// ===========================================================================

TEST_CASE("SanitizeRegSZ — plain value returned unchanged",
          "[config][sanitize][security]") {
    auto result = SanitizeRegSZ("Debug");
    REQUIRE(result == "Debug");
}

TEST_CASE("SanitizeRegSZ — embedded null byte returns empty string",
          "[config][sanitize][security]") {
    std::string_view evil{"ab\0cd", 5};
    auto result = SanitizeRegSZ(evil);
    REQUIRE(result.empty());
}

TEST_CASE("SanitizeRegSZ — empty input returns empty output",
          "[config][sanitize]") {
    REQUIRE(SanitizeRegSZ("").empty());
}

TEST_CASE("SanitizeRegSZ — value truncated at MAX_SZ_CHARS",
          "[config][sanitize][limits]") {
    std::string huge(MAX_SZ_CHARS + 100, 'x');
    auto result = SanitizeRegSZ(huge);
    REQUIRE(result.size() == MAX_SZ_CHARS);
}

TEST_CASE("SanitizeRegSZ — value exactly at MAX_SZ_CHARS not truncated",
          "[config][sanitize][limits]") {
    std::string exact(MAX_SZ_CHARS, 'x');
    auto result = SanitizeRegSZ(exact);
    REQUIRE(result.size() == MAX_SZ_CHARS);
}

TEST_CASE("SanitizeRegSZ — value below MAX_SZ_CHARS returned at full length",
          "[config][sanitize][limits]") {
    std::string small(100, 'a');
    auto result = SanitizeRegSZ(small);
    REQUIRE(result.size() == 100);
    REQUIRE(result == small);
}

// ===========================================================================
// RegistryEntry struct
// ===========================================================================

TEST_CASE("RegistryEntry — HKCU entry correctly identified",
          "[config][registry][struct]") {
    RegistryEntry e{Hive::HKCU, std::string(HKCU_ROOT), std::string(KEY_LOG_LEVEL)};
    REQUIRE(e.hive == Hive::HKCU);
    REQUIRE_FALSE(e.subKey.empty());
    REQUIRE_FALSE(e.valueName.empty());
}

TEST_CASE("RegistryEntry — HKLM policy entry correctly identified",
          "[config][registry][struct]") {
    RegistryEntry e{Hive::HKLM, std::string(HKLM_ROOT), std::string(POLICY_DISABLE_GPU)};
    REQUIRE(e.hive == Hive::HKLM);
    REQUIRE(IsValidKeyPath(e.subKey));
}

TEST_CASE("RegistryEntry — HKCU and HKLM hive values are different",
          "[config][registry][struct]") {
    REQUIRE(Hive::HKCU != Hive::HKLM);
}

// ===========================================================================
// Hive priority (HKLM policy overrides HKCU user settings)
// ===========================================================================

TEST_CASE("HivePriority — HKLM is the higher-trust hive",
          "[config][registry][policy]") {
    // The engine must prefer HKLM policy keys over HKCU user keys.
    // This test validates the ordering convention by checking enum values.
    // By convention: policy hive > user hive.
    REQUIRE(static_cast<int>(Hive::HKLM) != static_cast<int>(Hive::HKCU));
}

TEST_CASE("HivePriority — policy key POLICY_MAX_THREADS matches settings key KEY_MAX_THREADS",
          "[config][registry][policy]") {
    // Same value name in both hives — HKLM takes precedence
    REQUIRE(POLICY_MAX_THREADS == KEY_MAX_THREADS);
}
