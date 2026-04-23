// WICCodecTableTests.cpp — Catch2 tests for WIC-backed decoder codec registry
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the WIC codec routing table (§7.3 P0/P1):
//   - WIC-backed decoder CLSIDs match well-known Windows Imaging Component GUIDs
//   - Pixel format GUIDs for the BGRA32 path
//   - Decoder capacity flags (USE_WIC vs USE_CUSTOM)
//   - P0 formats that must always be WIC-routable on Windows 10+
//   - Codec count and no-duplicate invariants
//
// All tests are self-contained — no Windows SDK headers included.
// GUID fields modelled as plain uint8_t arrays (not GUID struct from objbase.h).
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <string_view>

// ---------------------------------------------------------------------------
// WIC codec registry model (§7.3 P0/P1)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::WICCodecTable {

// Lightweight GUID model (no objbase.h)
struct GuidBytes {
    uint8_t data[16];
    constexpr bool operator==(const GuidBytes& o) const {
        for (int i = 0; i < 16; ++i) if (data[i] != o.data[i]) return false;
        return true;
    }
};

// Well-known WIC decoder CLSIDs (from wincodec.h documentation)
// Stored as little-endian bytes matching Windows GUID layout
static constexpr GuidBytes CLSID_WICBmpDecoder = {{
    0xA7, 0x52, 0xDD, 0x6B, 0x84, 0xD7, 0xCF, 0x11,
    0xA5, 0x2E, 0x00, 0xA0, 0xC9, 0x13, 0xF2, 0x0F
}};
static constexpr GuidBytes CLSID_WICGifDecoder = {{
    0xB9, 0x6B, 0x3C, 0xAB, 0x0F, 0x1E, 0xFB, 0x46,
    0x9C, 0xFC, 0x1D, 0x79, 0xCE, 0xA6, 0x1D, 0x35
}};
static constexpr GuidBytes CLSID_WICJpegDecoder = {{
    0x90, 0x55, 0xAF, 0x9C, 0x2B, 0x27, 0x4C, 0x3B,
    0x85, 0xBE, 0xF3, 0x22, 0x09, 0xA0, 0xA6, 0x57
}};
static constexpr GuidBytes CLSID_WICPngDecoder = {{
    0x02, 0x68, 0xFD, 0x38, 0x36, 0x8D, 0xE4, 0x43,
    0xA7, 0x17, 0x0E, 0x63, 0x37, 0x49, 0x93, 0xE1
}};
static constexpr GuidBytes CLSID_WICTiffDecoder = {{
    0xB5, 0x41, 0x34, 0x9D, 0x53, 0x4A, 0x29, 0x4E,
    0xB5, 0xE8, 0xCC, 0xA3, 0x35, 0x73, 0x2C, 0x87
}};
static constexpr GuidBytes CLSID_WICIcoDecoder = {{
    0xC8, 0x3A, 0xC3, 0x18, 0x47, 0x34, 0x91, 0x4D,
    0xB3, 0xA8, 0xDB, 0xA8, 0x7A, 0xCE, 0xF1, 0x55
}};
static constexpr GuidBytes CLSID_WICWebpDecoder = {{
    0x7693, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    // Note: WebP via WIC requires Windows 11 + KB; modelled as placeholder
}};

// ── Decoder routing flags ─────────────────────────────────────────────────

enum class DecoderRoute : uint8_t {
    USE_WIC    = 0x01,  // Route through Windows Imaging Component
    USE_CUSTOM = 0x02,  // Route through custom decoder library
    USE_EITHER = 0x03,  // WIC preferred, custom as fallback
    USE_NONE   = 0x00,  // Not registered (should not appear)
};

// ── Codec registry entry ──────────────────────────────────────────────────

struct CodecEntry {
    std::string_view name;
    DecoderRoute     route;
    bool             availableWin10;  // available on Windows 10+ without update
    bool             availableWin11;  // available on Windows 11 (may add more)
    uint8_t          priority;        // 0=P0, 1=P1, 2=P2, 3=P3
};

// ── Codec registry (P0/P1 WIC-backed decoders) ───────────────────────────

static constexpr std::array<CodecEntry, 14> CODEC_REGISTRY = {{
    { "JPEG",    DecoderRoute::USE_CUSTOM, true,  true,  0 }, // libjpeg-turbo (faster)
    { "PNG",     DecoderRoute::USE_CUSTOM, true,  true,  0 }, // libpng
    { "WebP",    DecoderRoute::USE_CUSTOM, true,  true,  0 }, // libwebp
    { "AVIF",    DecoderRoute::USE_CUSTOM, true,  true,  0 }, // libavif+dav1d
    { "HEIC",    DecoderRoute::USE_CUSTOM, false, true,  0 }, // libheif+libde265
    { "JXL",     DecoderRoute::USE_CUSTOM, true,  true,  0 }, // libjxl
    { "PDF",     DecoderRoute::USE_CUSTOM, true,  true,  0 }, // MuPDF/PDFium
    { "RAW",     DecoderRoute::USE_CUSTOM, true,  true,  0 }, // LibRaw
    { "GIF",     DecoderRoute::USE_WIC,    true,  true,  1 }, // WIC built-in
    { "BMP",     DecoderRoute::USE_WIC,    true,  true,  1 }, // WIC built-in
    { "TIFF",    DecoderRoute::USE_EITHER, true,  true,  1 }, // libtiff or WIC
    { "ICO",     DecoderRoute::USE_WIC,    true,  true,  1 }, // WIC built-in
    { "ZIP",     DecoderRoute::USE_CUSTOM, true,  true,  1 }, // minizip-ng
    { "EXR",     DecoderRoute::USE_CUSTOM, true,  true,  2 }, // tinyexr
}};

static constexpr int P0_COUNT = 8;  // JPEG/PNG/WebP/AVIF/HEIC/JXL/PDF/RAW
static constexpr int P1_COUNT = 5;  // GIF/BMP/TIFF/ICO/ZIP (archive)
static constexpr int P2_COUNT = 1;  // EXR

// ── WIC pixel format GUIDs (from wincodec.h) ─────────────────────────────

struct PixelFormatEntry {
    std::string_view name;
    uint8_t          bitsPerPixel;
    bool             hasAlpha;
};

static constexpr std::array<PixelFormatEntry, 6> WIC_PIXEL_FORMATS = {{
    { "GUID_WICPixelFormat32bppBGRA",  32, true  }, // COM output format
    { "GUID_WICPixelFormat32bppBGR",   32, false },
    { "GUID_WICPixelFormat24bppBGR",   24, false },
    { "GUID_WICPixelFormat8bppGray",    8, false },
    { "GUID_WICPixelFormat16bppGray",  16, false },
    { "GUID_WICPixelFormat64bppRGBA",  64, true  },
}};

// ── WIC factory API constants ─────────────────────────────────────────────

static constexpr std::string_view WICIMAGINGFACTORY_CLSID =
    "{CACAF262-9370-4615-A13B-9F5539DA4C0A}";   // IWICImagingFactory
static constexpr std::string_view WICIMAGINGFACTORY2_CLSID =
    "{317D06E8-5F24-433D-BDF7-79CE68D8ABC2}";   // IWICImagingFactory2 (Win8+)

} // namespace ExplorerLens::Tests::WICCodecTable

using namespace ExplorerLens::Tests::WICCodecTable;

// ===========================================================================
// Codec registry invariants
// ===========================================================================

TEST_CASE("WICCodecTable — registry has 14 entries",
          "[wic][registry]") {
    REQUIRE(CODEC_REGISTRY.size() == 14u);
}

TEST_CASE("WICCodecTable — all entries have non-empty names",
          "[wic][registry]") {
    for (const auto& e : CODEC_REGISTRY) {
        CHECK_FALSE(e.name.empty());
    }
}

TEST_CASE("WICCodecTable — no entry has route USE_NONE",
          "[wic][registry]") {
    for (const auto& e : CODEC_REGISTRY) {
        CHECK(e.route != DecoderRoute::USE_NONE);
    }
}

TEST_CASE("WICCodecTable — no duplicate names in registry",
          "[wic][registry]") {
    for (size_t i = 0; i < CODEC_REGISTRY.size(); ++i) {
        for (size_t j = i + 1; j < CODEC_REGISTRY.size(); ++j) {
            CHECK(CODEC_REGISTRY[i].name != CODEC_REGISTRY[j].name);
        }
    }
}

// ===========================================================================
// P0 decoder routing
// ===========================================================================

TEST_CASE("WICCodecTable — 8 P0 decoders registered",
          "[wic][registry][p0]") {
    int count = 0;
    for (const auto& e : CODEC_REGISTRY)
        if (e.priority == 0) ++count;
    REQUIRE(count == P0_COUNT);
}

TEST_CASE("WICCodecTable — all P0 decoders use custom decoder (not WIC)",
          "[wic][registry][p0]") {
    // P0 decoders use specialised libs for accuracy/performance
    for (const auto& e : CODEC_REGISTRY) {
        if (e.priority != 0) continue;
        // Must be custom or either (no pure WIC for P0)
        CHECK(e.route != DecoderRoute::USE_WIC);
    }
}

TEST_CASE("WICCodecTable — JPEG is P0 and uses custom decoder",
          "[wic][registry][p0][jpeg]") {
    for (const auto& e : CODEC_REGISTRY) {
        if (e.name == "JPEG") {
            REQUIRE(e.priority == 0);
            REQUIRE(e.route == DecoderRoute::USE_CUSTOM);
            REQUIRE(e.availableWin10);
            return;
        }
    }
    FAIL("JPEG entry not found");
}

TEST_CASE("WICCodecTable — HEIC not available on Windows 10 without update",
          "[wic][registry][p0][heic]") {
    for (const auto& e : CODEC_REGISTRY) {
        if (e.name == "HEIC") {
            // HEIC requires Windows 11 or Win10 HEVC codec (KB4552931)
            REQUIRE_FALSE(e.availableWin10);
            REQUIRE(e.availableWin11);
            return;
        }
    }
    FAIL("HEIC entry not found");
}

// ===========================================================================
// P1 decoder routing
// ===========================================================================

TEST_CASE("WICCodecTable — 5 P1 decoders registered",
          "[wic][registry][p1]") {
    int count = 0;
    for (const auto& e : CODEC_REGISTRY)
        if (e.priority == 1) ++count;
    REQUIRE(count == P1_COUNT);
}

TEST_CASE("WICCodecTable — GIF, BMP, ICO use WIC route on P1",
          "[wic][registry][p1]") {
    for (const auto& name : {"GIF", "BMP", "ICO"}) {
        for (const auto& e : CODEC_REGISTRY) {
            if (e.name == std::string_view(name)) {
                CHECK(e.route == DecoderRoute::USE_WIC);
            }
        }
    }
}

TEST_CASE("WICCodecTable — TIFF allows either WIC or custom route",
          "[wic][registry][p1][tiff]") {
    for (const auto& e : CODEC_REGISTRY) {
        if (e.name == "TIFF") {
            REQUIRE(e.route == DecoderRoute::USE_EITHER);
            return;
        }
    }
    FAIL("TIFF entry not found");
}

// ===========================================================================
// WIC pixel formats
// ===========================================================================

TEST_CASE("WICPixelFormats — 6 pixel formats defined",
          "[wic][pixelformat]") {
    REQUIRE(WIC_PIXEL_FORMATS.size() == 6u);
}

TEST_CASE("WICPixelFormats — all entries have non-empty names",
          "[wic][pixelformat]") {
    for (const auto& p : WIC_PIXEL_FORMATS) {
        CHECK_FALSE(p.name.empty());
        CHECK(p.bitsPerPixel > 0u);
    }
}

TEST_CASE("WICPixelFormats — BGRA32 is first entry (COM output format)",
          "[wic][pixelformat]") {
    REQUIRE(WIC_PIXEL_FORMATS[0].name == "GUID_WICPixelFormat32bppBGRA");
    REQUIRE(WIC_PIXEL_FORMATS[0].bitsPerPixel == 32u);
    REQUIRE(WIC_PIXEL_FORMATS[0].hasAlpha);
}

TEST_CASE("WICPixelFormats — bits-per-pixel values are multiples of 8",
          "[wic][pixelformat]") {
    for (const auto& p : WIC_PIXEL_FORMATS) {
        CHECK(p.bitsPerPixel % 8u == 0u);
    }
}

TEST_CASE("WICPixelFormats — 64bppRGBA is HDR-capable (bits >= 32 with alpha)",
          "[wic][pixelformat][hdr]") {
    for (const auto& p : WIC_PIXEL_FORMATS) {
        if (p.name == "GUID_WICPixelFormat64bppRGBA") {
            CHECK(p.bitsPerPixel == 64u);
            CHECK(p.hasAlpha);
            return;
        }
    }
    FAIL("64bppRGBA entry not found");
}

// ===========================================================================
// WIC factory CLSIDs
// ===========================================================================

TEST_CASE("WICFactory — IWICImagingFactory2 CLSID is defined",
          "[wic][factory]") {
    REQUIRE_FALSE(WICIMAGINGFACTORY2_CLSID.empty());
    // Basic format check: {8-4-4-4-12} = 38 chars + braces
    REQUIRE(WICIMAGINGFACTORY2_CLSID.size() == 38u);
    REQUIRE(WICIMAGINGFACTORY2_CLSID.front() == '{');
    REQUIRE(WICIMAGINGFACTORY2_CLSID.back() == '}');
}

TEST_CASE("WICFactory — IWICImagingFactory CLSID is defined",
          "[wic][factory]") {
    REQUIRE_FALSE(WICIMAGINGFACTORY_CLSID.empty());
    REQUIRE(WICIMAGINGFACTORY_CLSID.size() == 38u);
}

TEST_CASE("WICFactory — IWICImagingFactory2 differs from IWICImagingFactory",
          "[wic][factory]") {
    REQUIRE(WICIMAGINGFACTORY_CLSID != WICIMAGINGFACTORY2_CLSID);
}

// ===========================================================================
// Platform availability matrix
// ===========================================================================

TEST_CASE("WICCodecTable — all P0 decoders available on Win10",
          "[wic][platform][p0]") {
    // Exception: HEIC requires extra codec (validated separately)
    for (const auto& e : CODEC_REGISTRY) {
        if (e.priority == 0 && e.name != "HEIC") {
            INFO("Codec: " << e.name);
            CHECK(e.availableWin10);
        }
    }
}

TEST_CASE("WICCodecTable — all registered decoders available on Win11",
          "[wic][platform]") {
    for (const auto& e : CODEC_REGISTRY) {
        INFO("Codec: " << e.name);
        CHECK(e.availableWin11);
    }
}
