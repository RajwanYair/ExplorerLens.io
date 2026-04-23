// PluginSDKContractTests.cpp — Catch2 tests for Plugin C-ABI SDK contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the Plugin SDK C-ABI contracts from §17.4 (Phase 3 preparation).
// Tests cover: API version macros, trust level enumeration, capability flags
// (power-of-two distinct values), exported symbol naming convention
// (lens_plugin_ prefix), plugin_info_t struct field presence model, and
// capability composition.
//
// All tests are self-contained — no plugin SDK headers included.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Plugin SDK contract model (§17.4, Phase 3 prep)
// Mirrored from SDK/plugin_api.h
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::PluginSDK {

// ── API version ────────────────────────────────────────────────────────────

static constexpr uint32_t PLUGIN_API_VERSION_MAJOR = 1;
static constexpr uint32_t PLUGIN_API_VERSION_MINOR = 0;
// Combined version for ABI compatibility check
static constexpr uint32_t PLUGIN_API_VERSION =
    (PLUGIN_API_VERSION_MAJOR << 16) | PLUGIN_API_VERSION_MINOR;

// ── Trust levels ────────────────────────────────────────────────────────────

enum class PluginTrustLevel : uint32_t {
    UNTRUSTED   = 0,   // Not verified — restricted sandbox
    COMMUNITY   = 1,   // Community-built, unreviewed
    VERIFIED    = 2,   // ExplorerLens team reviewed
    SIGNED      = 3,   // Code-signed by trusted publisher
};

static constexpr int TRUST_LEVEL_COUNT = 4;

// ── Plugin capability flags (power-of-two) ─────────────────────────────────

enum class PluginCapability : uint32_t {
    NONE                = 0x00000000,
    DECODE              = 0x00000001,   // Provides decoder(s)
    ENCODE              = 0x00000002,   // Provides encoder(s) (future)
    FILTER              = 0x00000004,   // Image filter / post-processor
    METADATA            = 0x00000008,   // Metadata reader / writer
    THEME               = 0x00000010,   // UI theme (LENSManager integration)
    PROVIDER            = 0x00000020,   // Custom thumbnail provider delegate
};

static constexpr int PLUGIN_CAPABILITY_COUNT = 6; // non-NONE flags

inline constexpr PluginCapability operator|(PluginCapability a, PluginCapability b) {
    return static_cast<PluginCapability>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr bool HasPluginCap(PluginCapability flags, PluginCapability bit) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(bit)) != 0;
}

// ── plugin_info_t field model ──────────────────────────────────────────────

// Enumerates required fields in plugin_info_t (checked by name/index)
enum class PluginInfoField : uint8_t {
    API_VERSION     = 0,   // uint32_t api_version — must match PLUGIN_API_VERSION
    NAME            = 1,   // const char* name — short identifier
    AUTHOR          = 2,   // const char* author
    DESCRIPTION     = 3,   // const char* description
    CAPABILITIES    = 4,   // uint32_t capabilities — PluginCapability bitmask
};

static constexpr int PLUGIN_INFO_FIELD_COUNT = 5;

// ── ABI symbol naming convention ────────────────────────────────────────────
// All exported symbols must begin with "lens_plugin_"

static constexpr std::string_view PLUGIN_SYMBOL_PREFIX = "lens_plugin_";

// Required exported symbols
static constexpr std::array<std::string_view, 5> REQUIRED_PLUGIN_SYMBOLS = {{
    "lens_plugin_get_info",       // Returns pointer to plugin_info_t
    "lens_plugin_create",         // Factory: creates plugin instance
    "lens_plugin_destroy",        // Destroys plugin instance (no-throw)
    "lens_plugin_api_version",    // Returns PLUGIN_API_VERSION as uint32_t
    "lens_plugin_on_load",        // Called once after successful trust check
}};

inline bool HasPluginPrefix(std::string_view sym) {
    return sym.size() > PLUGIN_SYMBOL_PREFIX.size() &&
           sym.substr(0, PLUGIN_SYMBOL_PREFIX.size()) == PLUGIN_SYMBOL_PREFIX;
}

} // namespace ExplorerLens::Tests::PluginSDK

using namespace ExplorerLens::Tests::PluginSDK;

// ===========================================================================
// API version
// ===========================================================================

TEST_CASE("PluginAPIVersion — MAJOR is 1",   "[plugin][version]") { REQUIRE(PLUGIN_API_VERSION_MAJOR == 1); }
TEST_CASE("PluginAPIVersion — MINOR is 0",   "[plugin][version]") { REQUIRE(PLUGIN_API_VERSION_MINOR == 0); }

TEST_CASE("PluginAPIVersion — combined version encodes MAJOR in high 16 bits",
          "[plugin][version]") {
    REQUIRE((PLUGIN_API_VERSION >> 16) == PLUGIN_API_VERSION_MAJOR);
}

TEST_CASE("PluginAPIVersion — combined version encodes MINOR in low 16 bits",
          "[plugin][version]") {
    REQUIRE((PLUGIN_API_VERSION & 0xFFFF) == PLUGIN_API_VERSION_MINOR);
}

// ===========================================================================
// Trust levels
// ===========================================================================

TEST_CASE("PluginTrustLevel — 4 levels defined", "[plugin][trust]") { REQUIRE(TRUST_LEVEL_COUNT == 4); }

TEST_CASE("PluginTrustLevel — UNTRUSTED is 0 (lowest trust)",
          "[plugin][trust]") {
    REQUIRE(static_cast<uint32_t>(PluginTrustLevel::UNTRUSTED) == 0);
}

TEST_CASE("PluginTrustLevel — SIGNED is 3 (highest trust)",
          "[plugin][trust]") {
    REQUIRE(static_cast<uint32_t>(PluginTrustLevel::SIGNED) == 3);
}

TEST_CASE("PluginTrustLevel — trust ordering: UNTRUSTED < COMMUNITY < VERIFIED < SIGNED",
          "[plugin][trust][ordering]") {
    REQUIRE(static_cast<uint32_t>(PluginTrustLevel::UNTRUSTED) <
            static_cast<uint32_t>(PluginTrustLevel::COMMUNITY));
    REQUIRE(static_cast<uint32_t>(PluginTrustLevel::COMMUNITY) <
            static_cast<uint32_t>(PluginTrustLevel::VERIFIED));
    REQUIRE(static_cast<uint32_t>(PluginTrustLevel::VERIFIED) <
            static_cast<uint32_t>(PluginTrustLevel::SIGNED));
}

TEST_CASE("PluginTrustLevel — all 4 values are distinct",
          "[plugin][trust][uniqueness]") {
    std::array<uint32_t, 4> vals = {
        static_cast<uint32_t>(PluginTrustLevel::UNTRUSTED),
        static_cast<uint32_t>(PluginTrustLevel::COMMUNITY),
        static_cast<uint32_t>(PluginTrustLevel::VERIFIED),
        static_cast<uint32_t>(PluginTrustLevel::SIGNED),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

// ===========================================================================
// Plugin capability flags
// ===========================================================================

TEST_CASE("PluginCapability — NONE is 0",
          "[plugin][capability]") {
    REQUIRE(static_cast<uint32_t>(PluginCapability::NONE) == 0);
}

TEST_CASE("PluginCapability — 6 non-NONE flags defined",
          "[plugin][capability]") {
    REQUIRE(PLUGIN_CAPABILITY_COUNT == 6);
}

TEST_CASE("PluginCapability — all 6 non-NONE flags are powers of 2",
          "[plugin][capability][flags]") {
    auto caps = {
        PluginCapability::DECODE,
        PluginCapability::ENCODE,
        PluginCapability::FILTER,
        PluginCapability::METADATA,
        PluginCapability::THEME,
        PluginCapability::PROVIDER,
    };
    for (auto c : caps) {
        uint32_t v = static_cast<uint32_t>(c);
        INFO("Capability value: 0x" << std::hex << v);
        CHECK(v != 0u);
        CHECK((v & (v - 1u)) == 0u);
    }
}

TEST_CASE("PluginCapability — DECODE and ENCODE are distinct flags",
          "[plugin][capability]") {
    auto combined = PluginCapability::DECODE | PluginCapability::ENCODE;
    CHECK(HasPluginCap(combined, PluginCapability::DECODE));
    CHECK(HasPluginCap(combined, PluginCapability::ENCODE));
    CHECK_FALSE(HasPluginCap(combined, PluginCapability::FILTER));
}

TEST_CASE("PluginCapability — combination of all flags has no collision",
          "[plugin][capability]") {
    auto all = PluginCapability::DECODE | PluginCapability::ENCODE |
               PluginCapability::FILTER | PluginCapability::METADATA |
               PluginCapability::THEME  | PluginCapability::PROVIDER;
    for (auto c : {PluginCapability::DECODE, PluginCapability::ENCODE,
                   PluginCapability::FILTER, PluginCapability::METADATA,
                   PluginCapability::THEME, PluginCapability::PROVIDER}) {
        CHECK(HasPluginCap(all, c));
    }
}

// ===========================================================================
// plugin_info_t field model
// ===========================================================================

TEST_CASE("PluginInfoField — 5 fields defined",
          "[plugin][info]") {
    REQUIRE(PLUGIN_INFO_FIELD_COUNT == 5);
}

TEST_CASE("PluginInfoField — API_VERSION is field 0",
          "[plugin][info]") {
    REQUIRE(static_cast<uint8_t>(PluginInfoField::API_VERSION) == 0);
}

TEST_CASE("PluginInfoField — all 5 field values are distinct",
          "[plugin][info][uniqueness]") {
    std::array<uint8_t, 5> vals = {
        static_cast<uint8_t>(PluginInfoField::API_VERSION),
        static_cast<uint8_t>(PluginInfoField::NAME),
        static_cast<uint8_t>(PluginInfoField::AUTHOR),
        static_cast<uint8_t>(PluginInfoField::DESCRIPTION),
        static_cast<uint8_t>(PluginInfoField::CAPABILITIES),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

// ===========================================================================
// ABI symbol naming convention
// ===========================================================================

TEST_CASE("PluginSymbols — prefix is 'lens_plugin_'",
          "[plugin][symbols]") {
    REQUIRE(PLUGIN_SYMBOL_PREFIX == "lens_plugin_");
}

TEST_CASE("PluginSymbols — 5 required symbols defined",
          "[plugin][symbols]") {
    REQUIRE(REQUIRED_PLUGIN_SYMBOLS.size() == 5u);
}

TEST_CASE("PluginSymbols — all required symbols start with 'lens_plugin_'",
          "[plugin][symbols][naming]") {
    for (auto sym : REQUIRED_PLUGIN_SYMBOLS) {
        INFO("Symbol: " << sym);
        CHECK(HasPluginPrefix(sym));
    }
}

TEST_CASE("PluginSymbols — all required symbol names are unique",
          "[plugin][symbols][uniqueness]") {
    std::vector<std::string_view> seen;
    for (auto sym : REQUIRED_PLUGIN_SYMBOLS) {
        auto it = std::find(seen.begin(), seen.end(), sym);
        INFO("Duplicate: " << sym);
        CHECK(it == seen.end());
        seen.push_back(sym);
    }
}

TEST_CASE("HasPluginPrefix — accepts valid symbols",
          "[plugin][symbols][naming]") {
    CHECK(HasPluginPrefix("lens_plugin_get_info"));
    CHECK(HasPluginPrefix("lens_plugin_decode"));
    CHECK(HasPluginPrefix("lens_plugin_custom_ext"));
}

TEST_CASE("HasPluginPrefix — rejects symbols without prefix",
          "[plugin][symbols][naming]") {
    CHECK_FALSE(HasPluginPrefix("get_info"));
    CHECK_FALSE(HasPluginPrefix("lens_decode"));
    CHECK_FALSE(HasPluginPrefix("plugin_get_info"));
    CHECK_FALSE(HasPluginPrefix("lens_plugin_"));  // empty after prefix
}

// ===========================================================================
// Parametric: 5 required symbol names via GENERATE
// ===========================================================================

TEST_CASE("PluginSymbols — each required symbol is non-empty",
          "[plugin][symbols][property-based]") {
    auto sym = GENERATE("lens_plugin_get_info", "lens_plugin_create",
                        "lens_plugin_destroy", "lens_plugin_api_version",
                        "lens_plugin_on_load");
    CHECK_FALSE(std::string_view(sym).empty());
    CHECK(HasPluginPrefix(sym));
}
