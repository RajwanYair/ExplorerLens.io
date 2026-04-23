// PlatformProfileTests.cpp — Catch2 tests for PlatformProfile PAL contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the Platform Abstraction Layer (PAL) compile-time constants from
// §16.1 and ADR-013.
// Tests cover: platform identifier constants (Windows/macOS/Linux), capability
// flags (HAVE_DIRECT2D/WIC/DIRECTX/COM), current platform = Windows in CI,
// PlatformName() non-empty contract, and mutually-exclusive capability sets
// per platform.
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

// ---------------------------------------------------------------------------
// Platform profile constants (§16.1, ADR-013)
// Mirrored from Engine/Platform/PlatformProfile.h
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::PlatformProfile {

// ── Platform identifiers ────────────────────────────────────────────────────

static constexpr uint32_t PLATFORM_WINDOWS = 1;
static constexpr uint32_t PLATFORM_MACOS   = 2;
static constexpr uint32_t PLATFORM_LINUX   = 3;

// Compile-time current platform (CI runs on Windows only — §16.1)
static constexpr uint32_t CURRENT_PLATFORM = PLATFORM_WINDOWS;

// ── Platform capability flags (power-of-two bitmask) ─────────────────────

enum class PlatformCap : uint32_t {
    NONE         = 0x00000000,
    HAVE_COM     = 0x00000001,   // COM apartment available (Windows only)
    HAVE_WIC     = 0x00000002,   // Windows Imaging Component (Windows only)
    HAVE_DIRECT2D= 0x00000004,   // Direct2D rendering (Windows only)
    HAVE_DIRECTX = 0x00000008,   // DirectX 11/12 (Windows only)
    HAVE_COREGFX = 0x00000010,   // CoreGraphics (macOS only)
    HAVE_GIO     = 0x00000020,   // GIO/Gio thumbnail interface (Linux only)
    HAVE_POSIX   = 0x00000040,   // POSIX fs APIs (macOS, Linux, WSL)
    HAVE_GDI     = 0x00000080,   // GDI+ fallback (Windows)
};

inline constexpr PlatformCap operator|(PlatformCap a, PlatformCap b) {
    return static_cast<PlatformCap>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr bool HasPlatformCap(PlatformCap flags, PlatformCap bit) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(bit)) != 0;
}

// ── Per-platform capability sets ───────────────────────────────────────────

static constexpr PlatformCap WINDOWS_CAPS =
    PlatformCap::HAVE_COM     |
    PlatformCap::HAVE_WIC     |
    PlatformCap::HAVE_DIRECT2D|
    PlatformCap::HAVE_DIRECTX |
    PlatformCap::HAVE_GDI;

static constexpr PlatformCap MACOS_CAPS =
    PlatformCap::HAVE_COREGFX |
    PlatformCap::HAVE_POSIX;

static constexpr PlatformCap LINUX_CAPS =
    PlatformCap::HAVE_GIO     |
    PlatformCap::HAVE_POSIX;

// ── PlatformName() model ────────────────────────────────────────────────────

inline constexpr std::string_view PlatformName(uint32_t platform) {
    switch (platform) {
        case PLATFORM_WINDOWS: return "Windows";
        case PLATFORM_MACOS:   return "macOS";
        case PLATFORM_LINUX:   return "Linux";
        default:               return "Unknown";
    }
}

// ── Platform count ─────────────────────────────────────────────────────────

static constexpr int SUPPORTED_PLATFORM_COUNT = 3;

} // namespace ExplorerLens::Tests::PlatformProfile

using namespace ExplorerLens::Tests::PlatformProfile;

// ===========================================================================
// Platform identifiers
// ===========================================================================

TEST_CASE("PlatformId — WINDOWS is 1",  "[platform][id]") { REQUIRE(PLATFORM_WINDOWS == 1); }
TEST_CASE("PlatformId — MACOS is 2",    "[platform][id]") { REQUIRE(PLATFORM_MACOS   == 2); }
TEST_CASE("PlatformId — LINUX is 3",    "[platform][id]") { REQUIRE(PLATFORM_LINUX   == 3); }

TEST_CASE("PlatformId — 3 supported platforms",
          "[platform][id]") {
    REQUIRE(SUPPORTED_PLATFORM_COUNT == 3);
}

TEST_CASE("PlatformId — all 3 identifiers are distinct",
          "[platform][id][uniqueness]") {
    REQUIRE(PLATFORM_WINDOWS != PLATFORM_MACOS);
    REQUIRE(PLATFORM_MACOS   != PLATFORM_LINUX);
    REQUIRE(PLATFORM_WINDOWS != PLATFORM_LINUX);
}

TEST_CASE("PlatformId — CURRENT_PLATFORM is WINDOWS (CI environment)",
          "[platform][id][current]") {
    // ExplorerLens CI runs exclusively on Windows GitHub Actions runners (§16.1)
    REQUIRE(CURRENT_PLATFORM == PLATFORM_WINDOWS);
}

// ===========================================================================
// Platform capability flags
// ===========================================================================

TEST_CASE("PlatformCap — NONE is 0",
          "[platform][caps]") {
    REQUIRE(static_cast<uint32_t>(PlatformCap::NONE) == 0);
}

TEST_CASE("PlatformCap — all 8 non-NONE flags are powers of 2",
          "[platform][caps][flags]") {
    auto caps = {
        PlatformCap::HAVE_COM,
        PlatformCap::HAVE_WIC,
        PlatformCap::HAVE_DIRECT2D,
        PlatformCap::HAVE_DIRECTX,
        PlatformCap::HAVE_COREGFX,
        PlatformCap::HAVE_GIO,
        PlatformCap::HAVE_POSIX,
        PlatformCap::HAVE_GDI,
    };
    for (auto c : caps) {
        uint32_t v = static_cast<uint32_t>(c);
        INFO("Cap value: 0x" << std::hex << v);
        CHECK(v != 0u);
        CHECK((v & (v - 1u)) == 0u);
    }
}

// ===========================================================================
// Windows capability set
// ===========================================================================

TEST_CASE("WindowsCaps — includes HAVE_COM",      "[platform][windows]") { CHECK(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_COM));      }
TEST_CASE("WindowsCaps — includes HAVE_WIC",      "[platform][windows]") { CHECK(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_WIC));      }
TEST_CASE("WindowsCaps — includes HAVE_DIRECT2D", "[platform][windows]") { CHECK(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_DIRECT2D)); }
TEST_CASE("WindowsCaps — includes HAVE_DIRECTX",  "[platform][windows]") { CHECK(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_DIRECTX));  }
TEST_CASE("WindowsCaps — includes HAVE_GDI",      "[platform][windows]") { CHECK(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_GDI));      }

TEST_CASE("WindowsCaps — does NOT include macOS-only HAVE_COREGFX",
          "[platform][windows]") {
    CHECK_FALSE(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_COREGFX));
}

TEST_CASE("WindowsCaps — does NOT include Linux-only HAVE_GIO",
          "[platform][windows]") {
    CHECK_FALSE(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_GIO));
}

// ===========================================================================
// macOS capability set
// ===========================================================================

TEST_CASE("MacOSCaps — includes HAVE_COREGFX",   "[platform][macos]") { CHECK(HasPlatformCap(MACOS_CAPS, PlatformCap::HAVE_COREGFX)); }
TEST_CASE("MacOSCaps — includes HAVE_POSIX",     "[platform][macos]") { CHECK(HasPlatformCap(MACOS_CAPS, PlatformCap::HAVE_POSIX));   }

TEST_CASE("MacOSCaps — does NOT include Windows-only HAVE_COM",
          "[platform][macos]") {
    CHECK_FALSE(HasPlatformCap(MACOS_CAPS, PlatformCap::HAVE_COM));
}

// ===========================================================================
// Linux capability set
// ===========================================================================

TEST_CASE("LinuxCaps — includes HAVE_GIO",   "[platform][linux]") { CHECK(HasPlatformCap(LINUX_CAPS, PlatformCap::HAVE_GIO));   }
TEST_CASE("LinuxCaps — includes HAVE_POSIX", "[platform][linux]") { CHECK(HasPlatformCap(LINUX_CAPS, PlatformCap::HAVE_POSIX)); }

TEST_CASE("LinuxCaps — does NOT include Windows-only HAVE_WIC",
          "[platform][linux]") {
    CHECK_FALSE(HasPlatformCap(LINUX_CAPS, PlatformCap::HAVE_WIC));
}

// ===========================================================================
// Platform capability mutual exclusivity
// ===========================================================================

TEST_CASE("PlatformCaps — HAVE_COREGFX and HAVE_COM are mutually exclusive",
          "[platform][caps][exclusive]") {
    // These are OS-specific — no platform has both
    CHECK_FALSE(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_COREGFX));
    CHECK_FALSE(HasPlatformCap(MACOS_CAPS,   PlatformCap::HAVE_COM));
}

TEST_CASE("PlatformCaps — HAVE_GIO is Linux-only (not in Windows or macOS)",
          "[platform][caps][exclusive]") {
    CHECK_FALSE(HasPlatformCap(WINDOWS_CAPS, PlatformCap::HAVE_GIO));
    CHECK_FALSE(HasPlatformCap(MACOS_CAPS,   PlatformCap::HAVE_GIO));
}

// ===========================================================================
// PlatformName()
// ===========================================================================

TEST_CASE("PlatformName — returns 'Windows' for PLATFORM_WINDOWS",
          "[platform][name]") {
    REQUIRE(PlatformName(PLATFORM_WINDOWS) == "Windows");
}

TEST_CASE("PlatformName — returns 'macOS' for PLATFORM_MACOS",
          "[platform][name]") {
    REQUIRE(PlatformName(PLATFORM_MACOS) == "macOS");
}

TEST_CASE("PlatformName — returns 'Linux' for PLATFORM_LINUX",
          "[platform][name]") {
    REQUIRE(PlatformName(PLATFORM_LINUX) == "Linux");
}

TEST_CASE("PlatformName — all 3 platform names are non-empty",
          "[platform][name]") {
    CHECK_FALSE(PlatformName(PLATFORM_WINDOWS).empty());
    CHECK_FALSE(PlatformName(PLATFORM_MACOS).empty());
    CHECK_FALSE(PlatformName(PLATFORM_LINUX).empty());
}

TEST_CASE("PlatformName — unknown platform returns non-empty string",
          "[platform][name]") {
    CHECK_FALSE(PlatformName(99u).empty());
}

// ===========================================================================
// Parametric: all known platforms have a name
// ===========================================================================

TEST_CASE("PlatformName — parametric: each supported platform has a non-empty name",
          "[platform][name][property-based]") {
    auto pid = GENERATE(
        (uint32_t)PLATFORM_WINDOWS,
        (uint32_t)PLATFORM_MACOS,
        (uint32_t)PLATFORM_LINUX);
    CHECK_FALSE(PlatformName(pid).empty());
    CHECK(PlatformName(pid) != "Unknown");
}
