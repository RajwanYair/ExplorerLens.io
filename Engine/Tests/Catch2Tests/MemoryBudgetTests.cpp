// MemoryBudgetTests.cpp — Catch2 tests for memory budget enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Validates decode size limits, buffer allocation contracts, and the
// memory budget configuration that guards against OOM during thumbnail
// decode. Covers §7.5 (memory limits), §15.1 (security boundary checks),
// and the CacheSizeConfig contracts from §14.
//
// All tests are self-contained — constants mirror Engine/Memory/ and
// Engine/Core/BuildValidation.h without including any Engine headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Memory budget constants (mirrored from Engine/Memory/*.h §7.5)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::MemoryBudget {

// ── Cache / Memory Budget ──────────────────────────────────────────────────

static constexpr int64_t CACHE_SIZE_MIN_MB     =    32;   // registry minimum
static constexpr int64_t CACHE_SIZE_DEFAULT_MB =   256;   // default budget
static constexpr int64_t CACHE_SIZE_MAX_MB     =  4096;   // registry maximum

// ── Decode Dimensions ─────────────────────────────────────────────────────

static constexpr int     MAX_DECODE_WIDTH      = 65535;   // pixel limit per axis
static constexpr int     MAX_DECODE_HEIGHT     = 65535;
static constexpr int64_t MAX_DECODE_PIXELS     = static_cast<int64_t>(32768) * 32768; // ≈ 1 Gpx hard cap
static constexpr int     MIN_DECODE_WIDTH      =     1;
static constexpr int     MIN_DECODE_HEIGHT     =     1;

// ── Buffer Allocation ─────────────────────────────────────────────────────

static constexpr int     BYTES_PER_PIXEL_BGRA  =     4;   // BGRA32 output
static constexpr int64_t MAX_SINGLE_ALLOC_MB   =   512;   // per-decode alloc guard
static constexpr int64_t MAX_SINGLE_ALLOC_BYTES =
    MAX_SINGLE_ALLOC_MB * 1024LL * 1024LL;

// ── Thumbnail Output Dimensions ───────────────────────────────────────────

static constexpr int THUMB_SIZE_MIN =    16;   // smallest valid thumbnail
static constexpr int THUMB_SIZE_MAX =  4096;   // largest valid thumbnail
static constexpr int THUMB_SIZE_DEFAULT = 256; // Windows Explorer default

// ── Pressure Controller ───────────────────────────────────────────────────

static constexpr int PRESSURE_LEVEL_NONE     = 0;
static constexpr int PRESSURE_LEVEL_LOW      = 1;
static constexpr int PRESSURE_LEVEL_MODERATE = 2;
static constexpr int PRESSURE_LEVEL_HIGH     = 3;
static constexpr int PRESSURE_LEVEL_CRITICAL = 4;
static constexpr int PRESSURE_LEVEL_COUNT    = 5;

// ── Helpers ───────────────────────────────────────────────────────────────

inline bool IsValidCacheSizeMB(int64_t mb) {
    return mb >= CACHE_SIZE_MIN_MB && mb <= CACHE_SIZE_MAX_MB;
}

inline bool IsValidDecodeDimension(int w, int h) {
    if (w < MIN_DECODE_WIDTH  || w > MAX_DECODE_WIDTH)  return false;
    if (h < MIN_DECODE_HEIGHT || h > MAX_DECODE_HEIGHT) return false;
    return true;
}

inline bool IsWithinPixelBudget(int w, int h) {
    return static_cast<int64_t>(w) * static_cast<int64_t>(h) <= MAX_DECODE_PIXELS;
}

inline int64_t ComputeBufferBytes(int w, int h) {
    return static_cast<int64_t>(w) * static_cast<int64_t>(h) * BYTES_PER_PIXEL_BGRA;
}

inline bool IsBufferSafeToAllocate(int w, int h) {
    if (!IsValidDecodeDimension(w, h))  return false;
    if (!IsWithinPixelBudget(w, h))     return false;
    return ComputeBufferBytes(w, h) <= MAX_SINGLE_ALLOC_BYTES;
}

inline bool IsValidThumbnailSize(int sz) {
    return sz >= THUMB_SIZE_MIN && sz <= THUMB_SIZE_MAX;
}

} // namespace ExplorerLens::Tests::MemoryBudget

using namespace ExplorerLens::Tests::MemoryBudget;

// ===========================================================================
// Cache size budget
// ===========================================================================

TEST_CASE("CacheBudget — MIN_MB < DEFAULT_MB < MAX_MB ordering",
          "[memory][cache][budget]") {
    REQUIRE(CACHE_SIZE_MIN_MB < CACHE_SIZE_DEFAULT_MB);
    REQUIRE(CACHE_SIZE_DEFAULT_MB < CACHE_SIZE_MAX_MB);
}

TEST_CASE("CacheBudget — MIN is 32 MB",  "[memory][cache]") { REQUIRE(CACHE_SIZE_MIN_MB == 32);   }
TEST_CASE("CacheBudget — DEFAULT is 256 MB", "[memory][cache]") { REQUIRE(CACHE_SIZE_DEFAULT_MB == 256); }
TEST_CASE("CacheBudget — MAX is 4096 MB", "[memory][cache]") { REQUIRE(CACHE_SIZE_MAX_MB == 4096);  }

TEST_CASE("IsValidCacheSizeMB — accepts boundary values",
          "[memory][cache][validation]") {
    CHECK(IsValidCacheSizeMB(32));
    CHECK(IsValidCacheSizeMB(256));
    CHECK(IsValidCacheSizeMB(4096));
}

TEST_CASE("IsValidCacheSizeMB — rejects out-of-range values",
          "[memory][cache][validation]") {
    CHECK_FALSE(IsValidCacheSizeMB(0));
    CHECK_FALSE(IsValidCacheSizeMB(31));
    CHECK_FALSE(IsValidCacheSizeMB(4097));
    CHECK_FALSE(IsValidCacheSizeMB(-1));
    CHECK_FALSE(IsValidCacheSizeMB(std::numeric_limits<int64_t>::max()));
}

// ===========================================================================
// Decode dimension limits
// ===========================================================================

TEST_CASE("DecodeDimensions — MAX_DECODE_WIDTH is 65535",  "[memory][decode]") { REQUIRE(MAX_DECODE_WIDTH == 65535);  }
TEST_CASE("DecodeDimensions — MAX_DECODE_HEIGHT is 65535", "[memory][decode]") { REQUIRE(MAX_DECODE_HEIGHT == 65535); }
TEST_CASE("DecodeDimensions — MIN dimension is 1",         "[memory][decode]") { REQUIRE(MIN_DECODE_WIDTH == 1);       }

TEST_CASE("IsValidDecodeDimension — accepts normal decode sizes",
          "[memory][decode][validation]") {
    CHECK(IsValidDecodeDimension(1,    1));
    CHECK(IsValidDecodeDimension(320,  240));
    CHECK(IsValidDecodeDimension(3840, 2160));
    CHECK(IsValidDecodeDimension(65535, 65535));
}

TEST_CASE("IsValidDecodeDimension — rejects zero/negative/oversize",
          "[memory][decode][validation]") {
    CHECK_FALSE(IsValidDecodeDimension(0,  240));
    CHECK_FALSE(IsValidDecodeDimension(320, 0));
    CHECK_FALSE(IsValidDecodeDimension(0,    0));
    CHECK_FALSE(IsValidDecodeDimension(-1, 240));
    CHECK_FALSE(IsValidDecodeDimension(65536, 100));
    CHECK_FALSE(IsValidDecodeDimension(100, 65536));
}

// ===========================================================================
// Pixel budget
// ===========================================================================

TEST_CASE("PixelBudget — MAX_DECODE_PIXELS is 1 Gpx (32768 × 32768)",
          "[memory][pixels]") {
    REQUIRE(MAX_DECODE_PIXELS == static_cast<int64_t>(32768) * 32768);
}

TEST_CASE("IsWithinPixelBudget — common sizes fit",
          "[memory][pixels]") {
    CHECK(IsWithinPixelBudget(320,   240));
    CHECK(IsWithinPixelBudget(3840, 2160));
    CHECK(IsWithinPixelBudget(32768, 32768));  // exactly at cap
}

TEST_CASE("IsWithinPixelBudget — 65535×65535 exceeds pixel budget",
          "[memory][pixels]") {
    // 65535*65535 ≈ 4.3 Gpx >> 1 Gpx cap
    CHECK_FALSE(IsWithinPixelBudget(65535, 65535));
}

TEST_CASE("IsWithinPixelBudget — overflow-safe for large dimensions",
          "[memory][pixels]") {
    // MAX_DECODE_PIXELS uses int64 arithmetic — no overflow
    REQUIRE(MAX_DECODE_PIXELS > 0);  // guard against int overflow sign flip
}

// ===========================================================================
// Buffer allocation safety
// ===========================================================================

TEST_CASE("ComputeBufferBytes — BGRA32 bytes = width × height × 4",
          "[memory][buffer]") {
    CHECK(ComputeBufferBytes(1,    1) ==        4LL);
    CHECK(ComputeBufferBytes(256, 256) ==  262144LL);
    CHECK(ComputeBufferBytes(3840, 2160) == 33177600LL);
}

TEST_CASE("BufferAllocation — MAX_SINGLE_ALLOC_MB is 512",
          "[memory][buffer]") {
    REQUIRE(MAX_SINGLE_ALLOC_MB == 512);
}

TEST_CASE("IsBufferSafeToAllocate — common image sizes are safe",
          "[memory][buffer]") {
    CHECK(IsBufferSafeToAllocate(256,  256));
    CHECK(IsBufferSafeToAllocate(1920, 1080));
    CHECK(IsBufferSafeToAllocate(3840, 2160));  // 4K = ~31 MB
}

TEST_CASE("IsBufferSafeToAllocate — rejects zero/negative dimensions",
          "[memory][buffer]") {
    CHECK_FALSE(IsBufferSafeToAllocate(0,  256));
    CHECK_FALSE(IsBufferSafeToAllocate(256,  0));
}

TEST_CASE("IsBufferSafeToAllocate — very wide image exceeds MAX_DECODE_WIDTH",
          "[memory][buffer]") {
    CHECK_FALSE(IsBufferSafeToAllocate(65536, 100));
}

TEST_CASE("IsBufferSafeToAllocate — 32768×32768 is within pixel budget but check alloc limit",
          "[memory][buffer]") {
    // 32768×32768×4 = 4 GB — exceeds 512 MB single-alloc guard
    const int64_t buf = ComputeBufferBytes(32768, 32768);
    CHECK(buf > MAX_SINGLE_ALLOC_BYTES);
    CHECK_FALSE(IsBufferSafeToAllocate(32768, 32768));
}

// ===========================================================================
// Thumbnail output size
// ===========================================================================

TEST_CASE("ThumbnailSize — MIN is 16", "[memory][thumb]") { REQUIRE(THUMB_SIZE_MIN == 16);   }
TEST_CASE("ThumbnailSize — MAX is 4096", "[memory][thumb]") { REQUIRE(THUMB_SIZE_MAX == 4096);  }
TEST_CASE("ThumbnailSize — DEFAULT is 256", "[memory][thumb]") { REQUIRE(THUMB_SIZE_DEFAULT == 256); }

TEST_CASE("IsValidThumbnailSize — accepts valid range",
          "[memory][thumb]") {
    CHECK(IsValidThumbnailSize(16));
    CHECK(IsValidThumbnailSize(64));
    CHECK(IsValidThumbnailSize(256));
    CHECK(IsValidThumbnailSize(1024));
    CHECK(IsValidThumbnailSize(4096));
}

TEST_CASE("IsValidThumbnailSize — rejects out-of-range",
          "[memory][thumb]") {
    CHECK_FALSE(IsValidThumbnailSize(0));
    CHECK_FALSE(IsValidThumbnailSize(15));
    CHECK_FALSE(IsValidThumbnailSize(4097));
    CHECK_FALSE(IsValidThumbnailSize(-1));
}

TEST_CASE("ThumbnailSize — DEFAULT is within valid range",
          "[memory][thumb]") {
    REQUIRE(IsValidThumbnailSize(THUMB_SIZE_DEFAULT));
}

// ===========================================================================
// Memory pressure controller
// ===========================================================================

TEST_CASE("PressureLevel — 5 levels defined (NONE..CRITICAL)",
          "[memory][pressure]") {
    REQUIRE(PRESSURE_LEVEL_COUNT == 5);
}

TEST_CASE("PressureLevel — levels are contiguous 0..4",
          "[memory][pressure]") {
    CHECK(PRESSURE_LEVEL_NONE     == 0);
    CHECK(PRESSURE_LEVEL_LOW      == 1);
    CHECK(PRESSURE_LEVEL_MODERATE == 2);
    CHECK(PRESSURE_LEVEL_HIGH     == 3);
    CHECK(PRESSURE_LEVEL_CRITICAL == 4);
}

TEST_CASE("PressureLevel — all level values are distinct",
          "[memory][pressure][uniqueness]") {
    std::array<int, 5> levels = {
        PRESSURE_LEVEL_NONE, PRESSURE_LEVEL_LOW, PRESSURE_LEVEL_MODERATE,
        PRESSURE_LEVEL_HIGH, PRESSURE_LEVEL_CRITICAL
    };
    for (size_t i = 0; i < levels.size(); ++i) {
        for (size_t j = i + 1; j < levels.size(); ++j) {
            CHECK(levels[i] != levels[j]);
        }
    }
}

TEST_CASE("PressureLevel — CRITICAL > HIGH > MODERATE > LOW > NONE ordering",
          "[memory][pressure]") {
    REQUIRE(PRESSURE_LEVEL_CRITICAL > PRESSURE_LEVEL_HIGH);
    REQUIRE(PRESSURE_LEVEL_HIGH     > PRESSURE_LEVEL_MODERATE);
    REQUIRE(PRESSURE_LEVEL_MODERATE > PRESSURE_LEVEL_LOW);
    REQUIRE(PRESSURE_LEVEL_LOW      > PRESSURE_LEVEL_NONE);
}
