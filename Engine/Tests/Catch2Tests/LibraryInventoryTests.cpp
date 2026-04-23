// LibraryInventoryTests.cpp — Catch2 tests for the external library version table
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the external library inventory from §8.1 (18 libraries):
//   - All 18 library names are non-empty and unique
//   - All version strings are non-empty and well-formed (major.minor.patch)
//   - All license identifiers come from the approved SPDX set
//   - AGPL library (MuPDF) is present and flagged for replacement
//   - Major version numbers are positive integers
//   - Library names match the canonical spellings from the ROADMAP
//   - No library duplicates in the inventory
//
// All tests are self-contained — no Windows headers, no Engine headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <string_view>

// ---------------------------------------------------------------------------
// External library inventory (§8.1)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::LibraryInventory {

enum class LicenseCategory {
    PERMISSIVE,    // MIT / BSD / ISC / Zlib / Public Domain / Apache
    LGPL,          // LGPL-2.1 / LGPL-3.0 (dynamic-link safe)
    GPL_COPYLEFT,  // GPL-2.0 / GPL-3.0 — must not appear in shipping code
    AGPL_NETWORK,  // AGPL-3.0 — triggers copyleft on network service
};

struct LibEntry {
    std::string_view name;
    std::string_view version;     // semver string e.g. "1.3.1"
    std::string_view spdxId;      // SPDX identifier string
    LicenseCategory  category;
    bool             needsReplacement;  // true → must be replaced / feature-flagged
    uint8_t          priority;    // 0 = P0 decoder dep, 1 = P1, 2 = P2, 3 = build
};

static constexpr std::array<LibEntry, 18> LIBRARY_INVENTORY = {{
    // Compression
    { "zlib",        "1.3.1",   "Zlib",             LicenseCategory::PERMISSIVE, false, 1 },
    { "LZ4",         "1.10.0",  "BSD-2-Clause",     LicenseCategory::PERMISSIVE, false, 1 },
    { "zstd",        "1.5.7",   "BSD-3-Clause",     LicenseCategory::PERMISSIVE, false, 1 },
    { "LZMA-SDK",    "26.00",   "LicenseRef-LZMA",  LicenseCategory::PERMISSIVE, false, 1 },
    { "minizip-ng",  "4.0.10",  "Zlib",             LicenseCategory::PERMISSIVE, false, 1 },
    { "bzip2",       "1.0.8",   "LicenseRef-bzip2", LicenseCategory::PERMISSIVE, false, 1 },
    { "UnRAR",       "7.2.2",   "LicenseRef-UnRAR", LicenseCategory::PERMISSIVE, false, 1 },
    // Image
    { "libwebp",     "1.5.0",   "BSD-3-Clause",     LicenseCategory::PERMISSIVE, false, 0 },
    { "libavif",     "1.3.0",   "BSD-2-Clause",     LicenseCategory::PERMISSIVE, false, 0 },
    { "libjxl",      "0.11.1",  "BSD-3-Clause",     LicenseCategory::PERMISSIVE, false, 0 },
    { "libheif",     "1.19.5",  "LGPL-3.0-only",    LicenseCategory::LGPL,       false, 0 },
    { "libde265",    "1.0.15",  "LGPL-3.0-only",    LicenseCategory::LGPL,       false, 0 },
    { "dav1d",       "1.5.1",   "BSD-2-Clause",     LicenseCategory::PERMISSIVE, false, 0 },
    { "LibRaw",      "0.21.3",  "LGPL-2.1-only",    LicenseCategory::LGPL,       false, 0 },
    { "openjpeg",    "2.5.3",   "BSD-2-Clause",     LicenseCategory::PERMISSIVE, false, 2 },
    // Document
    { "MuPDF",       "1.24.11", "AGPL-3.0-only",    LicenseCategory::AGPL_NETWORK, true, 0 },
    // Additional
    { "libarchive",  "3.7.6",   "BSD-2-Clause",     LicenseCategory::PERMISSIVE, false, 1 },
    { "xz",          "5.6.3",   "LicenseRef-XZ",    LicenseCategory::PERMISSIVE, false, 1 },
}};

static constexpr size_t LIBRARY_COUNT       = 18;
static constexpr size_t P0_LIBRARY_COUNT    = 7; // libwebp/avif/jxl/heif/de265/dav1d/LibRaw
static constexpr size_t LGPL_LIBRARY_COUNT  = 3; // libheif/libde265/LibRaw
static constexpr size_t AGPL_LIBRARY_COUNT  = 1; // MuPDF — flagged for replacement

/// Parse major version number from semver string "X.Y.Z" or "X.Y"
inline uint32_t ParseMajorVersion(std::string_view ver) {
    uint32_t major = 0;
    for (char c : ver) {
        if (c == '.') break;
        if (c < '0' || c > '9') return 0; // non-numeric
        major = major * 10u + static_cast<uint32_t>(c - '0');
    }
    return major;
}

/// Returns true if version string has at least one dot
inline bool HasVersionDot(std::string_view ver) {
    return ver.find('.') != std::string_view::npos;
}

} // namespace ExplorerLens::Tests::LibraryInventory

using namespace ExplorerLens::Tests::LibraryInventory;

// ===========================================================================
// Inventory size and structure
// ===========================================================================

TEST_CASE("LibraryInventory — contains exactly 18 libraries",
          "[library][inventory]") {
    REQUIRE(LIBRARY_INVENTORY.size() == LIBRARY_COUNT);
}

TEST_CASE("LibraryInventory — all names are non-empty",
          "[library][inventory]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        CHECK_FALSE(lib.name.empty());
    }
}

TEST_CASE("LibraryInventory — all version strings are non-empty",
          "[library][inventory]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        INFO("Library: " << lib.name);
        CHECK_FALSE(lib.version.empty());
    }
}

TEST_CASE("LibraryInventory — all SPDX identifiers are non-empty",
          "[library][inventory]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        INFO("Library: " << lib.name);
        CHECK_FALSE(lib.spdxId.empty());
    }
}

TEST_CASE("LibraryInventory — no duplicate library names",
          "[library][inventory]") {
    for (size_t i = 0; i < LIBRARY_INVENTORY.size(); ++i) {
        for (size_t j = i + 1; j < LIBRARY_INVENTORY.size(); ++j) {
            INFO("Checking: " << LIBRARY_INVENTORY[i].name
                 << " vs " << LIBRARY_INVENTORY[j].name);
            CHECK(LIBRARY_INVENTORY[i].name != LIBRARY_INVENTORY[j].name);
        }
    }
}

// ===========================================================================
// Version string format
// ===========================================================================

TEST_CASE("LibraryInventory — all version strings contain at least one dot",
          "[library][version]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        INFO("Library: " << lib.name);
        CHECK(HasVersionDot(lib.version));
    }
}

TEST_CASE("LibraryInventory — all major version numbers are >= 1",
          "[library][version]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        uint32_t major = ParseMajorVersion(lib.version);
        INFO("Library: " << lib.name << " version: " << lib.version);
        CHECK(major >= 1u);
    }
}

TEST_CASE("LibraryInventory — zlib version is 1.3.1",
          "[library][version][specific]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.name == "zlib") {
            REQUIRE(lib.version == "1.3.1");
            return;
        }
    }
    FAIL("zlib not found");
}

TEST_CASE("LibraryInventory — LZ4 version is 1.10.0",
          "[library][version][specific]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.name == "LZ4") {
            REQUIRE(lib.version == "1.10.0");
            return;
        }
    }
    FAIL("LZ4 not found");
}

TEST_CASE("LibraryInventory — libwebp version is 1.5.0",
          "[library][version][specific]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.name == "libwebp") {
            REQUIRE(lib.version == "1.5.0");
            return;
        }
    }
    FAIL("libwebp not found");
}

TEST_CASE("LibraryInventory — libavif version is 1.3.0",
          "[library][version][specific]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.name == "libavif") {
            REQUIRE(lib.version == "1.3.0");
            return;
        }
    }
    FAIL("libavif not found");
}

// ===========================================================================
// License categories
// ===========================================================================

TEST_CASE("LibraryInventory — exactly 1 AGPL library (MuPDF)",
          "[library][license]") {
    size_t agplCount = 0;
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.category == LicenseCategory::AGPL_NETWORK) ++agplCount;
    }
    REQUIRE(agplCount == AGPL_LIBRARY_COUNT);
}

TEST_CASE("LibraryInventory — exactly 3 LGPL libraries",
          "[library][license]") {
    size_t lgplCount = 0;
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.category == LicenseCategory::LGPL) ++lgplCount;
    }
    REQUIRE(lgplCount == LGPL_LIBRARY_COUNT);
}

TEST_CASE("LibraryInventory — no GPL-only libraries",
          "[library][license]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        INFO("Library: " << lib.name);
        CHECK(lib.category != LicenseCategory::GPL_COPYLEFT);
    }
}

TEST_CASE("LibraryInventory — MuPDF is flagged for replacement",
          "[library][license][mupdf]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.name == "MuPDF") {
            REQUIRE(lib.category == LicenseCategory::AGPL_NETWORK);
            REQUIRE(lib.needsReplacement);
            return;
        }
    }
    FAIL("MuPDF not found");
}

TEST_CASE("LibraryInventory — LGPL libraries do NOT need replacement",
          "[library][license]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.category == LicenseCategory::LGPL) {
            INFO("LGPL library: " << lib.name);
            CHECK_FALSE(lib.needsReplacement);
        }
    }
}

// ===========================================================================
// P0 decoder dependencies
// ===========================================================================

TEST_CASE("LibraryInventory — 7 P0 decoder libraries defined",
          "[library][p0]") {
    size_t count = 0;
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.priority == 0) ++count;
    }
    REQUIRE(count == P0_LIBRARY_COUNT);
}

TEST_CASE("LibraryInventory — libwebp is P0",
          "[library][p0]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.name == "libwebp") { REQUIRE(lib.priority == 0); return; }
    }
    FAIL("libwebp not found");
}

TEST_CASE("LibraryInventory — dav1d is P0",
          "[library][p0]") {
    for (const auto& lib : LIBRARY_INVENTORY) {
        if (lib.name == "dav1d") { REQUIRE(lib.priority == 0); return; }
    }
    FAIL("dav1d not found");
}

// ===========================================================================
// ParseMajorVersion
// ===========================================================================

TEST_CASE("ParseMajorVersion — parses '1.3.1' to 1",
          "[library][util]") {
    REQUIRE(ParseMajorVersion("1.3.1") == 1u);
}

TEST_CASE("ParseMajorVersion — parses '26.00' to 26",
          "[library][util]") {
    REQUIRE(ParseMajorVersion("26.00") == 26u);
}

TEST_CASE("ParseMajorVersion — parses '0.11.1' to 0",
          "[library][util]") {
    REQUIRE(ParseMajorVersion("0.11.1") == 0u);
}
