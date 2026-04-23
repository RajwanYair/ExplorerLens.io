// SafeDimensionsTests.cpp — Catch2 tests for Engine/Core/SafeDimensions.h
// Copyright (c) 2026 ExplorerLens Project
//
// Verifies all invariants of SafeDimensions, SafeDim, and the safe arithmetic
// helpers.  These tests are critical for OWASP A4 (integer overflow) and §15.1
// (P0 security hardening Phase 1).
//
// ROADMAP: §10.4 (Catch2 migration), §15.1 (security hardening), D-new (SafeDimensions)
// Test count: ~50 TEST_CASE / SECTION assertions
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

// The header under test — no other Engine includes needed.
#include "Core/SafeDimensions.h"

using namespace ExplorerLens::Core;

// ============================================================================
// Constants sanity-check
// ============================================================================

TEST_CASE("SafeDimensions constants are consistent", "[SafeDimensions][constants]")
{
    REQUIRE(kMaxThumbDimension == 32768u);
    REQUIRE(kMaxThumbPixels    == static_cast<uint64_t>(32768u) * 32768u);
    REQUIRE(kMaxThumbBytes     == kMaxThumbPixels * 4u);
    REQUIRE(kMinThumbDimension == 1u);

    // Byte budget must be representable as uint64
    REQUIRE(kMaxThumbBytes > kMaxThumbPixels);  // no uint64 wrap
}

// ============================================================================
// SafeMul2
// ============================================================================

TEST_CASE("SafeMul2 basic arithmetic", "[SafeDimensions][SafeMul2]")
{
    SECTION("typical multiply")
    {
        auto r = SafeMul2(1920u, 1080u);
        REQUIRE(r.has_value());
        REQUIRE(*r == 1920ull * 1080ull);
    }

    SECTION("multiplying by zero returns 0")
    {
        auto r = SafeMul2(0u, 1080u);
        REQUIRE(r.has_value());
        REQUIRE(*r == 0ull);
    }

    SECTION("max dimension squared fits in uint64")
    {
        auto r = SafeMul2(kMaxThumbDimension, kMaxThumbDimension);
        REQUIRE(r.has_value());
        REQUIRE(*r == kMaxThumbPixels);
    }

    SECTION("result exceeding ceiling returns nullopt")
    {
        // Ask for a product that is 1 beyond the ceiling
        auto r = SafeMul2(kMaxThumbDimension, kMaxThumbDimension, kMaxThumbPixels - 1u);
        REQUIRE_FALSE(r.has_value());
    }

    SECTION("result at exactly the ceiling is accepted")
    {
        auto r = SafeMul2(kMaxThumbDimension, kMaxThumbDimension, kMaxThumbPixels);
        REQUIRE(r.has_value());
    }
}

// ============================================================================
// SafeMul3
// ============================================================================

TEST_CASE("SafeMul3 three-operand multiply", "[SafeDimensions][SafeMul3]")
{
    SECTION("typical BGRA32 buffer size")
    {
        auto r = SafeMul3(1920u, 1080u, 4u);
        REQUIRE(r.has_value());
        REQUIRE(*r == 1920ull * 1080ull * 4ull);
    }

    SECTION("max pixel count × 4 bytes = kMaxThumbBytes")
    {
        auto r = SafeMul3(kMaxThumbDimension, kMaxThumbDimension, 4u, kMaxThumbBytes);
        REQUIRE(r.has_value());
        REQUIRE(*r == kMaxThumbBytes);
    }

    SECTION("overflow beyond ceiling returns nullopt")
    {
        // max dims × 4, ceiling just below
        auto r = SafeMul3(kMaxThumbDimension, kMaxThumbDimension, 4u, kMaxThumbBytes - 1u);
        REQUIRE_FALSE(r.has_value());
    }

    SECTION("c=0 always returns 0")
    {
        auto r = SafeMul3(32768u, 32768u, 0u);
        REQUIRE(r.has_value());
        REQUIRE(*r == 0ull);
    }
}

// ============================================================================
// SafeAdd
// ============================================================================

TEST_CASE("SafeAdd overflow-safe addition", "[SafeDimensions][SafeAdd]")
{
    SECTION("normal addition")
    {
        auto r = SafeAdd(100ull, 200ull);
        REQUIRE(r.has_value());
        REQUIRE(*r == 300ull);
    }

    SECTION("adding at ceiling boundary is accepted")
    {
        auto r = SafeAdd(500ull, 500ull, 1000ull);
        REQUIRE(r.has_value());
        REQUIRE(*r == 1000ull);
    }

    SECTION("exceeding ceiling returns nullopt")
    {
        auto r = SafeAdd(500ull, 501ull, 1000ull);
        REQUIRE_FALSE(r.has_value());
    }
}

// ============================================================================
// SafeDim — single dimension value
// ============================================================================

TEST_CASE("SafeDim valid construction", "[SafeDimensions][SafeDim]")
{
    SECTION("minimum valid value (1)")
    {
        auto d = SafeDim::Make(1u);
        REQUIRE(d.has_value());
        REQUIRE(d->Value() == 1u);
    }

    SECTION("mid-range value")
    {
        auto d = SafeDim::Make(1920u);
        REQUIRE(d.has_value());
        REQUIRE(static_cast<uint32_t>(*d) == 1920u);
    }

    SECTION("maximum valid value (kMaxThumbDimension)")
    {
        auto d = SafeDim::Make(kMaxThumbDimension);
        REQUIRE(d.has_value());
        REQUIRE(d->Value() == kMaxThumbDimension);
    }
}

TEST_CASE("SafeDim rejects invalid values", "[SafeDimensions][SafeDim]")
{
    SECTION("zero is rejected")
    {
        REQUIRE_FALSE(SafeDim::Make(0u).has_value());
    }

    SECTION("one beyond max is rejected")
    {
        REQUIRE_FALSE(SafeDim::Make(kMaxThumbDimension + 1u).has_value());
    }

    SECTION("UINT32_MAX is rejected")
    {
        REQUIRE_FALSE(SafeDim::Make(0xFFFF'FFFFu).has_value());
    }
}

TEST_CASE("SafeDim equality", "[SafeDimensions][SafeDim]")
{
    auto a = SafeDim::Make(256u);
    auto b = SafeDim::Make(256u);
    auto c = SafeDim::Make(512u);
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ============================================================================
// SafeDimensions — validated (width, height) pair
// ============================================================================

TEST_CASE("SafeDimensions valid construction", "[SafeDimensions]")
{
    SECTION("1×1 minimum")
    {
        auto d = SafeDimensions::Make(1u, 1u);
        REQUIRE(d.has_value());
        REQUIRE(d->Width()  == 1u);
        REQUIRE(d->Height() == 1u);
    }

    SECTION("common thumbnail sizes")
    {
        for (uint32_t sz : {32u, 64u, 96u, 128u, 256u, 512u, 1024u})
        {
            auto d = SafeDimensions::Make(sz, sz);
            REQUIRE(d.has_value());
            REQUIRE(d->Width()  == sz);
            REQUIRE(d->Height() == sz);
        }
    }

    SECTION("maximum safe dimensions (kMaxThumbDimension × kMaxThumbDimension)")
    {
        auto d = SafeDimensions::Make(kMaxThumbDimension, kMaxThumbDimension);
        REQUIRE(d.has_value());
        REQUIRE(d->PixelCount() == kMaxThumbPixels);
    }

    SECTION("non-square wide image")
    {
        auto d = SafeDimensions::Make(3840u, 2160u);
        REQUIRE(d.has_value());
        REQUIRE(d->PixelCount() == 3840ull * 2160ull);
    }
}

TEST_CASE("SafeDimensions rejects invalid inputs", "[SafeDimensions]")
{
    SECTION("zero width")
    {
        REQUIRE_FALSE(SafeDimensions::Make(0u, 1080u).has_value());
    }

    SECTION("zero height")
    {
        REQUIRE_FALSE(SafeDimensions::Make(1920u, 0u).has_value());
    }

    SECTION("width > kMaxThumbDimension")
    {
        REQUIRE_FALSE(SafeDimensions::Make(kMaxThumbDimension + 1u, 1u).has_value());
    }

    SECTION("height > kMaxThumbDimension")
    {
        REQUIRE_FALSE(SafeDimensions::Make(1u, kMaxThumbDimension + 1u).has_value());
    }

    SECTION("pixel-budget overflow: narrow × very-tall")
    {
        // width = 1, height = kMaxThumbDimension: PixelCount = kMaxThumbDimension
        // which is under kMaxThumbPixels, so this is valid.
        REQUIRE(SafeDimensions::Make(1u, kMaxThumbDimension).has_value());
        // But (kMaxThumbDimension, kMaxThumbDimension) = kMaxThumbPixels — valid.
        REQUIRE(SafeDimensions::Make(kMaxThumbDimension, kMaxThumbDimension).has_value());
    }
}

// ============================================================================
// SafeDimensions — PixelCount / ByteCount
// ============================================================================

TEST_CASE("SafeDimensions pixel and byte counts", "[SafeDimensions][counts]")
{
    auto d = SafeDimensions::Make(256u, 256u);
    REQUIRE(d.has_value());

    SECTION("PixelCount == width * height")
    {
        REQUIRE(d->PixelCount() == 256ull * 256ull);
    }

    SECTION("ByteCount (default BGRA32) == PixelCount * 4")
    {
        REQUIRE(d->ByteCount() == d->PixelCount() * 4u);
    }

    SECTION("ByteCount(1) — grayscale")
    {
        auto b1 = d->ByteCount(1u);
        REQUIRE(b1.has_value());
        REQUIRE(*b1 == d->PixelCount());
    }

    SECTION("ByteCount(4) — BGRA32")
    {
        auto b4 = d->ByteCount(4u);
        REQUIRE(b4.has_value());
        REQUIRE(*b4 == d->ByteCount());
    }
}

// ============================================================================
// SafeDimensions — RowStride
// ============================================================================

TEST_CASE("SafeDimensions RowStride", "[SafeDimensions][stride]")
{
    auto d = SafeDimensions::Make(1920u, 1080u);
    REQUIRE(d.has_value());

    SECTION("stride for 4 bytes/px = width * 4")
    {
        auto s = d->RowStride(4u);
        REQUIRE(s.has_value());
        REQUIRE(*s == 1920u * 4u);
    }

    SECTION("stride for 3 bytes/px (RGB24)")
    {
        auto s = d->RowStride(3u);
        REQUIRE(s.has_value());
        REQUIRE(*s == 1920u * 3u);
    }

    SECTION("stride for 1 byte/px (grayscale)")
    {
        auto s = d->RowStride(1u);
        REQUIRE(s.has_value());
        REQUIRE(*s == 1920u);
    }
}

// ============================================================================
// SafeDimensions — FitsIn
// ============================================================================

TEST_CASE("SafeDimensions FitsIn", "[SafeDimensions][fitsIn]")
{
    auto small  = SafeDimensions::Make(256u, 256u);
    auto medium = SafeDimensions::Make(512u, 512u);
    auto wide   = SafeDimensions::Make(512u, 256u);

    REQUIRE(small->FitsIn(*medium));
    REQUIRE_FALSE(medium->FitsIn(*small));

    // Same size fits in itself
    REQUIRE(small->FitsIn(*small));

    // wide (512×256) does not fit in medium (512×512) height-wise? Actually 256 <= 512.
    // wide.w=512 <= medium.w=512, wide.h=256 <= medium.h=512 → fits
    REQUIRE(wide->FitsIn(*medium));
}

// ============================================================================
// SafeDimensions — ScaleToFit
// ============================================================================

TEST_CASE("SafeDimensions ScaleToFit", "[SafeDimensions][scale]")
{
    SECTION("square image that already fits")
    {
        auto d = SafeDimensions::Make(256u, 256u);
        auto scaled = d->ScaleToFit(512u);
        REQUIRE(scaled.has_value());
        REQUIRE(scaled->Width()  == 256u);
        REQUIRE(scaled->Height() == 256u);
    }

    SECTION("square image scaled down")
    {
        auto d = SafeDimensions::Make(1024u, 1024u);
        auto scaled = d->ScaleToFit(256u);
        REQUIRE(scaled.has_value());
        REQUIRE(scaled->Width()  == 256u);
        REQUIRE(scaled->Height() == 256u);
    }

    SECTION("landscape image scaled to fit 256")
    {
        auto d = SafeDimensions::Make(1920u, 1080u);
        auto scaled = d->ScaleToFit(256u);
        REQUIRE(scaled.has_value());
        REQUIRE(scaled->Width()  == 256u);
        // height = 256 * 1080 / 1920 = 144
        REQUIRE(scaled->Height() == 144u);
    }

    SECTION("portrait image: height is the longest edge")
    {
        auto d = SafeDimensions::Make(640u, 1280u);
        auto scaled = d->ScaleToFit(128u);
        REQUIRE(scaled.has_value());
        REQUIRE(scaled->Height() == 128u);
        REQUIRE(scaled->Width()  == 64u);
    }

    SECTION("maxEdge == 0 returns nullopt")
    {
        auto d = SafeDimensions::Make(256u, 256u);
        REQUIRE_FALSE(d->ScaleToFit(0u).has_value());
    }
}

// ============================================================================
// SafeDimensions — equality
// ============================================================================

TEST_CASE("SafeDimensions equality operator", "[SafeDimensions][equality]")
{
    auto a = SafeDimensions::Make(256u, 256u);
    auto b = SafeDimensions::Make(256u, 256u);
    auto c = SafeDimensions::Make(512u, 256u);

    REQUIRE(a == b);
    REQUIRE(a != c);
}
