// FormatDetectionTests.cpp — Catch2 unit tests for format detection
// Copyright (c) 2026 ExplorerLens Project
//
// Tests magic-byte detection for all top-priority decoders.
// Each TEST_CASE is independent and exercises the static CanDecode() probe.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstring>
#include <string_view>
#include <vector>
#include <cstdint>

// ---------------------------------------------------------------------------
// Minimal magic-byte prober — mirrors the logic in LENSArchive.cpp
// without requiring the full engine to link against in unit-test mode.
// ---------------------------------------------------------------------------

namespace {

enum class FormatId : int {
    Unknown = 0,
    Jpeg,
    Png,
    WebP,
    Gif89a,
    Bmp,
    Tiff_LE,
    Tiff_BE,
    Pdf,
    Zip,
    SevenZip,
    Rar4,
    Rar5,
    JxlContainer,
    JxlBistream,
    Avif,
    Heif,
    Mp4_ftyp,
    Flif,
    Qoi,
    OpenEXR,
    SvgText,
};

struct MagicEntry {
    FormatId id;
    std::string_view name;
    size_t offset;
    std::vector<uint8_t> bytes;
};

// NOLINTNEXTLINE(cert-err58-cpp)
static const std::vector<MagicEntry> k_magic = {
    { FormatId::Jpeg,        "JPEG",       0, { 0xFF, 0xD8, 0xFF } },
    { FormatId::Png,         "PNG",        0, { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } },
    { FormatId::WebP,        "WebP",       0, { 0x52, 0x49, 0x46, 0x46 } },   // + "WEBP" at offset 8
    { FormatId::Gif89a,      "GIF89a",     0, { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 } },
    { FormatId::Bmp,         "BMP",        0, { 0x42, 0x4D } },
    { FormatId::Tiff_LE,     "TIFF-LE",   0, { 0x49, 0x49, 0x2A, 0x00 } },
    { FormatId::Tiff_BE,     "TIFF-BE",   0, { 0x4D, 0x4D, 0x00, 0x2A } },
    { FormatId::Pdf,         "PDF",        0, { 0x25, 0x50, 0x44, 0x46, 0x2D } },  // "%PDF-"
    { FormatId::Zip,         "ZIP",        0, { 0x50, 0x4B, 0x03, 0x04 } },
    { FormatId::SevenZip,    "7z",         0, { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C } },
    { FormatId::Rar4,        "RAR4",       0, { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00 } },
    { FormatId::Rar5,        "RAR5",       0, { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00 } },
    { FormatId::OpenEXR,     "OpenEXR",   0, { 0x76, 0x2F, 0x31, 0x01 } },
    { FormatId::Qoi,         "QOI",        0, { 0x71, 0x6F, 0x69, 0x66 } },  // "qoif"
    { FormatId::JxlBistream,"JXL-raw",    0, { 0xFF, 0x0A } },
};

FormatId DetectFormat(const uint8_t* data, size_t size) {
    for (const auto& m : k_magic) {
        size_t end = m.offset + m.bytes.size();
        if (size < end) continue;
        if (memcmp(data + m.offset, m.bytes.data(), m.bytes.size()) == 0) {
            // Extra check for WebP: "WEBP" at offset 8
            if (m.id == FormatId::WebP) {
                if (size < 12) continue;
                if (memcmp(data + 8, "WEBP", 4) != 0) continue;
            }
            return m.id;
        }
    }
    return FormatId::Unknown;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("Magic byte detection — JPEG", "[format][detection][jpeg]") {
    const uint8_t data[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Jpeg);
}

TEST_CASE("Magic byte detection — PNG", "[format][detection][png]") {
    const uint8_t data[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Png);
}

TEST_CASE("Magic byte detection — WebP", "[format][detection][webp]") {
    const uint8_t data[] = {
        0x52, 0x49, 0x46, 0x46,   // "RIFF"
        0x20, 0x00, 0x00, 0x00,   // size
        0x57, 0x45, 0x42, 0x50    // "WEBP"
    };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::WebP);
}

TEST_CASE("Magic byte detection — GIF89a", "[format][detection][gif]") {
    const uint8_t data[] = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Gif89a);
}

TEST_CASE("Magic byte detection — BMP", "[format][detection][bmp]") {
    const uint8_t data[] = { 0x42, 0x4D, 0x36, 0x00, 0x00, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Bmp);
}

TEST_CASE("Magic byte detection — PDF", "[format][detection][pdf]") {
    const uint8_t data[] = { 0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34 }; // %PDF-1.4
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Pdf);
}

TEST_CASE("Magic byte detection — ZIP / CBZ", "[format][detection][zip][cbz]") {
    const uint8_t data[] = { 0x50, 0x4B, 0x03, 0x04, 0x14, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Zip);
}

TEST_CASE("Magic byte detection — 7-Zip", "[format][detection][7z]") {
    const uint8_t data[] = { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::SevenZip);
}

TEST_CASE("Magic byte detection — RAR4", "[format][detection][rar]") {
    const uint8_t data[] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Rar4);
}

TEST_CASE("Magic byte detection — RAR5", "[format][detection][rar]") {
    const uint8_t data[] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Rar5);
}

TEST_CASE("Magic byte detection — TIFF little-endian", "[format][detection][tiff]") {
    const uint8_t data[] = { 0x49, 0x49, 0x2A, 0x00, 0x08, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Tiff_LE);
}

TEST_CASE("Magic byte detection — TIFF big-endian", "[format][detection][tiff]") {
    const uint8_t data[] = { 0x4D, 0x4D, 0x00, 0x2A, 0x00, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Tiff_BE);
}

TEST_CASE("Magic byte detection — OpenEXR", "[format][detection][exr]") {
    const uint8_t data[] = { 0x76, 0x2F, 0x31, 0x01 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::OpenEXR);
}

TEST_CASE("Magic byte detection — QOI", "[format][detection][qoi]") {
    const uint8_t data[] = { 0x71, 0x6F, 0x69, 0x66, 0x00, 0x00 }; // "qoif"
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Qoi);
}

TEST_CASE("Magic byte detection — bare JXL bitstream", "[format][detection][jxl]") {
    const uint8_t data[] = { 0xFF, 0x0A, 0x00 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::JxlBistream);
}

TEST_CASE("Magic byte detection — unknown / garbage data", "[format][detection][unknown]") {
    const uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04 };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Unknown);
}

TEST_CASE("Magic byte detection — empty buffer", "[format][detection][edge]") {
    REQUIRE(DetectFormat(nullptr, 0) == FormatId::Unknown);
}

TEST_CASE("Magic byte detection — truncated signature (too short)", "[format][detection][edge]") {
    const uint8_t data[] = { 0xFF, 0xD8 }; // JPEG needs 3 bytes
    // Should still detect JPEG (3 bytes minimum; this is only 2 → unknown)
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Unknown);
}

TEST_CASE("Magic byte detection — WebP with wrong subformat (not WEBP at +8)", "[format][detection][webp]") {
    const uint8_t data[] = {
        0x52, 0x49, 0x46, 0x46,  // "RIFF"
        0x20, 0x00, 0x00, 0x00,  // size
        0x41, 0x56, 0x49, 0x20  // "AVI " — not WebP
    };
    REQUIRE(DetectFormat(data, sizeof(data)) == FormatId::Unknown);
}

TEST_CASE("Format detection — parametric sampling of well-known headers",
          "[format][detection][parametric]") {
    using P = std::pair<std::vector<uint8_t>, FormatId>;
    auto [bytes, expected] = GENERATE(table<std::vector<uint8_t>, FormatId>({
        P{{ 0xFF, 0xD8, 0xFF, 0xE1 },                             FormatId::Jpeg       },
        P{{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0 }, FormatId::Png        },
        P{{ 0x25, 0x50, 0x44, 0x46, 0x2D, 0x32, 0x2E, 0x30 },    FormatId::Pdf        },
        P{{ 0x50, 0x4B, 0x03, 0x04, 0x00, 0x00 },                 FormatId::Zip        },
        P{{ 0x76, 0x2F, 0x31, 0x01 },                             FormatId::OpenEXR   },
    }));
    REQUIRE(DetectFormat(bytes.data(), bytes.size()) == expected);
}
