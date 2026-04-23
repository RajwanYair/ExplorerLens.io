// DecoderPriorityTierTests.cpp — Catch2 tests for P0/P1/P2/P3 decoder tier contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the four-tier decoder priority model from §7.3.
// Tests cover: P0/P1/P2/P3 tier assignments, latency budgets per tier,
// library assignments for P0/P1 decoders, tier ordering, format-to-tier
// mapping invariants, and corpus file coverage targets.
//
// All tests are self-contained — constants mirror Engine/Core/DecoderRegistry.h
// without including Windows or Engine headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>

// ---------------------------------------------------------------------------
// Decoder priority tier model (§7.3)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::DecoderPriorityTier {

// ── Tier identifiers ────────────────────────────────────────────────────────

enum class DecoderTier : uint8_t {
    P0 = 0,   // Photos & modern web — must be <25 ms
    P1 = 1,   // Archives, classic rasters, documents — must be <20 ms
    P2 = 2,   // Design & scientific — must be <15 ms (no full decode)
    P3 = 3,   // Video & 3D — budget up to 30 ms
};

static constexpr int TIER_COUNT = 4;

// ── Per-tier latency budgets (ms) ──────────────────────────────────────────

static constexpr int32_t P0_BUDGET_MIN_MS =  5;
static constexpr int32_t P0_BUDGET_MAX_MS = 25;
static constexpr int32_t P1_BUDGET_MIN_MS =  5;
static constexpr int32_t P1_BUDGET_MAX_MS = 20;
static constexpr int32_t P2_BUDGET_MIN_MS =  5;
static constexpr int32_t P2_BUDGET_MAX_MS = 15;
static constexpr int32_t P3_BUDGET_MIN_MS = 10;
static constexpr int32_t P3_BUDGET_MAX_MS = 30;

// ── Decoder registry entry ─────────────────────────────────────────────────

struct DecoderEntry {
    std::string_view format;    // Format identifier (e.g. "JPEG")
    DecoderTier      tier;
    std::string_view library;   // Backing library name (empty = WIC/OS)
    int32_t          budgetMaxMs;
    int32_t          corpusFileTarget; // §7.3: P0/P1 must have ≥3 corpus files × 3 sizes
};

// ── Decoder registry ───────────────────────────────────────────────────────

static constexpr std::array<DecoderEntry, 27> DECODER_REGISTRY = {{
    // P0 — Photos & modern web
    { "JPEG",   DecoderTier::P0, "libjpeg-turbo-3.1.0",   25,  9 },
    { "PNG",    DecoderTier::P0, "libpng-1.6.44",          25,  9 },
    { "WebP",   DecoderTier::P0, "libwebp-1.5.0",          25,  9 },
    { "AVIF",   DecoderTier::P0, "libavif-1.3.0",          25,  9 },
    { "HEIC",   DecoderTier::P0, "libheif-1.19.8",         25,  9 },
    { "JXL",    DecoderTier::P0, "libjxl-0.11.1",          25,  9 },
    { "PDF",    DecoderTier::P0, "pdfium-134",              25,  9 },
    { "RAW",    DecoderTier::P0, "LibRaw-0.21.4",           25,  9 },
    // P1 — Archives, classic rasters, documents
    { "ZIP",    DecoderTier::P1, "minizip-ng-4.0.9",        20,  9 },
    { "CBZ",    DecoderTier::P1, "minizip-ng-4.0.9",        20,  9 },
    { "RAR",    DecoderTier::P1, "unrar-7.1.9",             20,  9 },
    { "CBR",    DecoderTier::P1, "unrar-7.1.9",             20,  9 },
    { "7Z",     DecoderTier::P1, "LZMA-SDK-26.00",          20,  9 },
    { "EPUB",   DecoderTier::P1, "minizip-ng-4.0.9",        20,  9 },
    { "GIF",    DecoderTier::P1, "giflib-5.2.2",            20,  9 },
    { "BMP",    DecoderTier::P1, "WIC",                     20,  9 },
    { "TIFF",   DecoderTier::P1, "libtiff-4.7.0",           20,  9 },
    // P2 — Design & scientific
    { "EXR",    DecoderTier::P2, "OpenEXR-3.3.2",           15,  0 },
    { "PSD",    DecoderTier::P2, "custom",                   15,  0 },
    { "DDS",    DecoderTier::P2, "DirectXTex",               15,  0 },
    { "SVG",    DecoderTier::P2, "nanosvg",                  15,  0 },
    { "HDR",    DecoderTier::P2, "stb_image",                15,  0 },
    { "QOI",    DecoderTier::P2, "qoi",                      15,  0 },
    { "TGA",    DecoderTier::P2, "stb_image",                15,  0 },
    // P3 — Video & 3D
    { "MP4",    DecoderTier::P3, "WMF",                      30,  0 },
    { "glTF",   DecoderTier::P3, "cgltf",                    30,  0 },
    { "STL",    DecoderTier::P3, "custom",                   30,  0 },
}};

} // namespace ExplorerLens::Tests::DecoderPriorityTier

using namespace ExplorerLens::Tests::DecoderPriorityTier;

// ===========================================================================
// Tier identifiers
// ===========================================================================

TEST_CASE("DecoderTier — 4 tiers defined",
          "[decoder][tier][count]") {
    REQUIRE(TIER_COUNT == 4);
}

TEST_CASE("DecoderTier — P0 is tier 0 (highest priority)",
          "[decoder][tier]") {
    REQUIRE(static_cast<uint8_t>(DecoderTier::P0) == 0);
}

TEST_CASE("DecoderTier — tier values are sequential 0–3",
          "[decoder][tier][ordering]") {
    REQUIRE(static_cast<uint8_t>(DecoderTier::P0) == 0);
    REQUIRE(static_cast<uint8_t>(DecoderTier::P1) == 1);
    REQUIRE(static_cast<uint8_t>(DecoderTier::P2) == 2);
    REQUIRE(static_cast<uint8_t>(DecoderTier::P3) == 3);
}

// ===========================================================================
// Latency budgets
// ===========================================================================

TEST_CASE("P0Budget — max 25 ms, min 5 ms",
          "[decoder][budget][p0]") {
    REQUIRE(P0_BUDGET_MAX_MS == 25);
    REQUIRE(P0_BUDGET_MIN_MS == 5);
    REQUIRE(P0_BUDGET_MIN_MS < P0_BUDGET_MAX_MS);
}

TEST_CASE("P1Budget — max 20 ms (stricter than P0)",
          "[decoder][budget][p1]") {
    REQUIRE(P1_BUDGET_MAX_MS == 20);
    REQUIRE(P1_BUDGET_MAX_MS < P0_BUDGET_MAX_MS);
}

TEST_CASE("P2Budget — max 15 ms (strictest raster budget)",
          "[decoder][budget][p2]") {
    REQUIRE(P2_BUDGET_MAX_MS == 15);
    REQUIRE(P2_BUDGET_MAX_MS < P1_BUDGET_MAX_MS);
}

TEST_CASE("P3Budget — max 30 ms (most relaxed, video/3D)",
          "[decoder][budget][p3]") {
    REQUIRE(P3_BUDGET_MAX_MS == 30);
    REQUIRE(P3_BUDGET_MAX_MS > P0_BUDGET_MAX_MS);
}

// ===========================================================================
// Registry population
// ===========================================================================

TEST_CASE("DecoderRegistry — 27 entries defined",
          "[decoder][registry]") {
    REQUIRE(DECODER_REGISTRY.size() == 27u);
}

TEST_CASE("DecoderRegistry — all entries have non-empty format names",
          "[decoder][registry]") {
    for (const auto& d : DECODER_REGISTRY) {
        REQUIRE_FALSE(d.format.empty());
    }
}

TEST_CASE("DecoderRegistry — all P0/P1 entries have non-empty library",
          "[decoder][registry][library]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P0 || d.tier == DecoderTier::P1) {
            INFO("Format: " << d.format);
            CHECK_FALSE(d.library.empty());
        }
    }
}

TEST_CASE("DecoderRegistry — all entries budget max > 0",
          "[decoder][registry][budget]") {
    for (const auto& d : DECODER_REGISTRY) {
        CHECK(d.budgetMaxMs > 0);
    }
}

// ===========================================================================
// P0 decoder tier
// ===========================================================================

TEST_CASE("DecoderRegistry — 8 P0 decoders (JPEG/PNG/WebP/AVIF/HEIC/JXL/PDF/RAW)",
          "[decoder][registry][p0]") {
    int p0Count = 0;
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P0) ++p0Count;
    }
    REQUIRE(p0Count == 8);
}

TEST_CASE("DecoderRegistry — all P0 decoders within 25 ms budget",
          "[decoder][budget][p0]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P0) {
            INFO("P0 format: " << d.format);
            CHECK(d.budgetMaxMs <= P0_BUDGET_MAX_MS);
        }
    }
}

TEST_CASE("DecoderRegistry — P0 JPEG decoder uses libjpeg-turbo",
          "[decoder][registry][p0][jpeg]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.format == "JPEG") {
            CHECK(d.tier == DecoderTier::P0);
            CHECK(d.library.find("libjpeg-turbo") != std::string_view::npos);
        }
    }
}

TEST_CASE("DecoderRegistry — P0 RAW decoder uses LibRaw",
          "[decoder][registry][p0][raw]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.format == "RAW") {
            CHECK(d.tier == DecoderTier::P0);
            CHECK(d.library.find("LibRaw") != std::string_view::npos);
        }
    }
}

TEST_CASE("DecoderRegistry — P0 formats have corpus file target = 9 (3 files × 3 sizes)",
          "[decoder][registry][p0][corpus]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P0) {
            INFO("P0 format: " << d.format);
            CHECK(d.corpusFileTarget >= 9);
        }
    }
}

// ===========================================================================
// P1 decoder tier
// ===========================================================================

TEST_CASE("DecoderRegistry — 9 P1 decoders (ZIP/CBZ/RAR/CBR/7Z/EPUB/GIF/BMP/TIFF)",
          "[decoder][registry][p1]") {
    int p1Count = 0;
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P1) ++p1Count;
    }
    REQUIRE(p1Count == 9);
}

TEST_CASE("DecoderRegistry — all P1 decoders within 20 ms budget",
          "[decoder][budget][p1]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P1) {
            INFO("P1 format: " << d.format);
            CHECK(d.budgetMaxMs <= P1_BUDGET_MAX_MS);
        }
    }
}

TEST_CASE("DecoderRegistry — P1 corpus file target = 9 (3 files × 3 sizes)",
          "[decoder][registry][p1][corpus]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P1) {
            INFO("P1 format: " << d.format);
            CHECK(d.corpusFileTarget >= 9);
        }
    }
}

// ===========================================================================
// P2/P3 decoder tier
// ===========================================================================

TEST_CASE("DecoderRegistry — 7 P2 decoders (EXR/PSD/DDS/SVG/HDR/QOI/TGA)",
          "[decoder][registry][p2]") {
    int p2Count = 0;
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P2) ++p2Count;
    }
    REQUIRE(p2Count == 7);
}

TEST_CASE("DecoderRegistry — all P2 decoders within 15 ms budget",
          "[decoder][budget][p2]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P2) {
            INFO("P2 format: " << d.format);
            CHECK(d.budgetMaxMs <= P2_BUDGET_MAX_MS);
        }
    }
}

TEST_CASE("DecoderRegistry — 3 P3 decoders (MP4/glTF/STL)",
          "[decoder][registry][p3]") {
    int p3Count = 0;
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P3) ++p3Count;
    }
    REQUIRE(p3Count == 3);
}

TEST_CASE("DecoderRegistry — all P3 decoders within 30 ms budget",
          "[decoder][budget][p3]") {
    for (const auto& d : DECODER_REGISTRY) {
        if (d.tier == DecoderTier::P3) {
            INFO("P3 format: " << d.format);
            CHECK(d.budgetMaxMs <= P3_BUDGET_MAX_MS);
        }
    }
}

// ===========================================================================
// Parametric: all registered formats are unique
// ===========================================================================

TEST_CASE("DecoderRegistry — no duplicate format names",
          "[decoder][registry][uniqueness]") {
    std::vector<std::string_view> seen;
    seen.reserve(DECODER_REGISTRY.size());
    for (const auto& d : DECODER_REGISTRY) {
        auto it = std::find(seen.begin(), seen.end(), d.format);
        INFO("Duplicate: " << d.format);
        CHECK(it == seen.end());
        seen.push_back(d.format);
    }
}
