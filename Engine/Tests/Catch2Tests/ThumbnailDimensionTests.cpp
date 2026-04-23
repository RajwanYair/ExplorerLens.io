// ThumbnailDimensionTests.cpp — Catch2 tests for thumbnail output dimension contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the IThumbnailProvider output-dimension rules (§6.1, §7.2):
//   - Explorer size classes (16, 32, 48, 96, 256, 768, 1024)
//   - Valid output range [16, 4096]
//   - Aspect-ratio preservation via letterbox/pillarbox model
//   - Square-thumbnail guarantee for equal-dimension requests
//   - Output pixel format: BGRA32 (DXGI_FORMAT_B8G8R8A8_UNORM)
//   - Sub-16 requests → placeholder icon, not error
//   - Over-4096 requests → clamped to 4096
//
// All tests are self-contained — no Windows headers, no Engine headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <cstdint>

// ---------------------------------------------------------------------------
// Thumbnail dimension model (§6.1, §7.2)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::ThumbnailDimension {

/// Minimum and maximum pixel dimensions for a valid thumbnail
static constexpr uint32_t THUMB_MIN_PX   = 16;
static constexpr uint32_t THUMB_MAX_PX   = 4096;

/// Default Explorer target size (Large Icons view)
static constexpr uint32_t THUMB_DEFAULT  = 256;

/// Hardware texture size limit (WDDM minimum caps)
static constexpr uint32_t TEXTURE_MAX    = 4096;

/// Standard Windows Explorer icon-size classes
/// (SHIL_SMALL=16, SHIL_SYSSMALL=32, SHIL_LARGE=32, custom 48,
///  SHIL_EXTRALARGE=96, SHIL_JUMBO=256, extra-large 768, max 1024)
enum class ExplorerSize : uint32_t {
    SMALL         =  16,
    MEDIUM_16     =  16,
    MEDIUM_32     =  32,
    MEDIUM_48     =  48,
    MEDIUM_96     =  96,
    LARGE         = 256,
    EXTRA_LARGE   = 768,
    JUMBO         = 1024,
};

static constexpr uint32_t STANDARD_SIZES[] = {16, 32, 48, 96, 256, 768, 1024};

/// Result of a dimension negotiation
struct ThumbnailDimensions {
    uint32_t outputW = 0;
    uint32_t outputH = 0;
    bool     isLetterboxed  = false; // padding on top/bottom
    bool     isPillarboxed  = false; // padding on left/right
    bool     isPlaceholder  = false; // request was below minimum — return icon
};

/// Pixel format for thumbnail output (matches IThumbnailProvider contract)
enum class PixelFormat : uint32_t {
    BGRA32  = 0,  // ← Required by COM IThumbnailProvider (DXGI B8G8R8A8_UNORM)
    RGBA32  = 1,
    BGR24   = 2,
    GRAY8   = 3,
};

static constexpr PixelFormat COM_THUMBNAIL_FORMAT = PixelFormat::BGRA32;

/// Clamp a requested target size to [THUMB_MIN_PX, THUMB_MAX_PX]
inline uint32_t ClampTarget(uint32_t requested) {
    return (std::max)(THUMB_MIN_PX, (std::min)(requested, THUMB_MAX_PX));
}

/// Compute output dimensions preserving aspect ratio of (srcW × srcH) for a
/// square target of `target` pixels.  Result fits within target×target.
inline ThumbnailDimensions ComputeScaledDims(
    uint32_t srcW, uint32_t srcH, uint32_t target)
{
    ThumbnailDimensions d{};
    if (target < THUMB_MIN_PX) {
        d.isPlaceholder = true;
        d.outputW = d.outputH = THUMB_MIN_PX;
        return d;
    }
    target = (std::min)(target, THUMB_MAX_PX);

    if (srcW == 0 || srcH == 0) {
        d.outputW = d.outputH = target;
        return d;
    }

    // Fit source into target×target preserving aspect
    // scale = min(target/srcW, target/srcH)
    // Use integer math: multiply out to avoid float
    uint64_t scaleW = (uint64_t)target * 1000 / srcW;
    uint64_t scaleH = (uint64_t)target * 1000 / srcH;
    uint64_t scale  = (scaleW < scaleH) ? scaleW : scaleH;

    d.outputW = static_cast<uint32_t>((uint64_t)srcW * scale / 1000);
    d.outputH = static_cast<uint32_t>((uint64_t)srcH * scale / 1000);

    // Clamp to at least 1 pixel
    if (d.outputW < 1) d.outputW = 1;
    if (d.outputH < 1) d.outputH = 1;

    d.isLetterboxed = (d.outputH < target);
    d.isPillarboxed = (d.outputW < target);
    return d;
}

/// Returns true if the request needs a placeholder (below minimum)
inline bool NeedsPlaceholder(uint32_t requestedTarget) {
    return requestedTarget < THUMB_MIN_PX;
}

} // namespace ExplorerLens::Tests::ThumbnailDimension

using namespace ExplorerLens::Tests::ThumbnailDimension;

// ===========================================================================
// Constants
// ===========================================================================

TEST_CASE("ThumbnailDimension — minimum pixel size is 16",
          "[dimension][constants]") {
    REQUIRE(THUMB_MIN_PX == 16u);
}

TEST_CASE("ThumbnailDimension — maximum pixel size is 4096",
          "[dimension][constants]") {
    REQUIRE(THUMB_MAX_PX == 4096u);
}

TEST_CASE("ThumbnailDimension — default thumbnail size is 256",
          "[dimension][constants]") {
    REQUIRE(THUMB_DEFAULT == 256u);
}

TEST_CASE("ThumbnailDimension — texture max matches hardware limit",
          "[dimension][constants]") {
    REQUIRE(TEXTURE_MAX == 4096u);
    REQUIRE(TEXTURE_MAX == THUMB_MAX_PX);
}

// ===========================================================================
// Explorer standard sizes
// ===========================================================================

TEST_CASE("ThumbnailDimension — 7 standard Explorer size classes defined",
          "[dimension][explorer-sizes]") {
    REQUIRE(std::size(STANDARD_SIZES) == 7u);
}

TEST_CASE("ThumbnailDimension — all standard sizes within [16, 1024]",
          "[dimension][explorer-sizes]") {
    for (uint32_t sz : STANDARD_SIZES) {
        CHECK(sz >= 16u);
        CHECK(sz <= 1024u);
    }
}

TEST_CASE("ThumbnailDimension — standard sizes are in ascending order",
          "[dimension][explorer-sizes]") {
    for (size_t i = 1; i < std::size(STANDARD_SIZES); ++i) {
        CHECK(STANDARD_SIZES[i] > STANDARD_SIZES[i - 1]);
    }
}

TEST_CASE("ThumbnailDimension — default 256 is in the standard size list",
          "[dimension][explorer-sizes]") {
    bool found = false;
    for (uint32_t sz : STANDARD_SIZES) {
        if (sz == THUMB_DEFAULT) { found = true; break; }
    }
    REQUIRE(found);
}

TEST_CASE("ThumbnailDimension — JUMBO (1024) is the largest standard size",
          "[dimension][explorer-sizes]") {
    REQUIRE(static_cast<uint32_t>(ExplorerSize::JUMBO) == 1024u);
}

// ===========================================================================
// Target clamping
// ===========================================================================

TEST_CASE("ClampTarget — requests below 16 are raised to 16",
          "[dimension][clamp]") {
    CHECK(ClampTarget(0)  == 16u);
    CHECK(ClampTarget(1)  == 16u);
    CHECK(ClampTarget(15) == 16u);
}

TEST_CASE("ClampTarget — requests within [16, 4096] are unchanged",
          "[dimension][clamp]") {
    auto target = GENERATE(16u, 32u, 48u, 96u, 256u, 768u, 1024u, 4096u);
    REQUIRE(ClampTarget(target) == target);
}

TEST_CASE("ClampTarget — requests above 4096 are clamped to 4096",
          "[dimension][clamp]") {
    CHECK(ClampTarget(4097) == 4096u);
    CHECK(ClampTarget(8192) == 4096u);
    CHECK(ClampTarget(UINT32_MAX) == 4096u);
}

// ===========================================================================
// COM pixel format
// ===========================================================================

TEST_CASE("PixelFormat — COM thumbnail format is BGRA32",
          "[dimension][pixelformat]") {
    REQUIRE(COM_THUMBNAIL_FORMAT == PixelFormat::BGRA32);
    REQUIRE(static_cast<uint32_t>(PixelFormat::BGRA32) == 0u);
}

// ===========================================================================
// Aspect ratio preservation
// ===========================================================================

TEST_CASE("ComputeScaledDims — square source produces square output",
          "[dimension][aspect]") {
    auto d = ComputeScaledDims(1000, 1000, 256);
    CHECK(d.outputW == d.outputH);
    CHECK(d.outputW <= 256u);
    CHECK_FALSE(d.isLetterboxed);
    CHECK_FALSE(d.isPillarboxed);
}

TEST_CASE("ComputeScaledDims — landscape source fits within target width",
          "[dimension][aspect][landscape]") {
    // 1920×1080 landscape, target 256
    auto d = ComputeScaledDims(1920, 1080, 256);
    CHECK(d.outputW <= 256u);
    CHECK(d.outputH <= 256u);
    // Width should fill the target for landscape
    CHECK(d.outputW == 256u);
    CHECK(d.outputH < 256u);
    CHECK(d.isLetterboxed);
    CHECK_FALSE(d.isPillarboxed);
}

TEST_CASE("ComputeScaledDims — portrait source fits within target height",
          "[dimension][aspect][portrait]") {
    // 1080×1920 portrait, target 256
    auto d = ComputeScaledDims(1080, 1920, 256);
    CHECK(d.outputW <= 256u);
    CHECK(d.outputH <= 256u);
    CHECK(d.outputH == 256u);
    CHECK(d.outputW < 256u);
    CHECK(d.isPillarboxed);
    CHECK_FALSE(d.isLetterboxed);
}

TEST_CASE("ComputeScaledDims — output never exceeds target on either axis",
          "[dimension][aspect]") {
    struct Case { uint32_t sw, sh, t; };
    auto [sw, sh, t] = GENERATE(
        Case{100, 100, 256}, Case{3840, 2160, 256}, Case{256, 256, 256},
        Case{1, 1000, 96}, Case{4096, 4096, 16}
    );
    auto d = ComputeScaledDims(sw, sh, t);
    CHECK(d.outputW <= t);
    CHECK(d.outputH <= t);
}

TEST_CASE("ComputeScaledDims — output dimensions are at least 1 pixel",
          "[dimension][aspect]") {
    auto d = ComputeScaledDims(100, 100, 16);
    CHECK(d.outputW >= 1u);
    CHECK(d.outputH >= 1u);
}

// ===========================================================================
// Placeholder path
// ===========================================================================

TEST_CASE("NeedsPlaceholder — requests below 16 need placeholder",
          "[dimension][placeholder]") {
    CHECK(NeedsPlaceholder(0));
    CHECK(NeedsPlaceholder(1));
    CHECK(NeedsPlaceholder(15));
}

TEST_CASE("NeedsPlaceholder — requests at or above 16 do not need placeholder",
          "[dimension][placeholder]") {
    auto target = GENERATE(16u, 32u, 64u, 256u, 1024u);
    REQUIRE_FALSE(NeedsPlaceholder(target));
}

TEST_CASE("ComputeScaledDims — sub-16 target sets isPlaceholder flag",
          "[dimension][placeholder]") {
    auto d = ComputeScaledDims(1920, 1080, 8);
    CHECK(d.isPlaceholder);
}

// ===========================================================================
// Extreme / edge cases
// ===========================================================================

TEST_CASE("ComputeScaledDims — 1×1 source at target 256 returns non-zero dimensions",
          "[dimension][edge]") {
    auto d = ComputeScaledDims(1, 1, 256);
    CHECK(d.outputW >= 1u);
    CHECK(d.outputH >= 1u);
}

TEST_CASE("ComputeScaledDims — zero source dimensions treated gracefully",
          "[dimension][edge]") {
    auto d = ComputeScaledDims(0, 0, 256);
    // Should not crash; returns target×target as fallback
    CHECK(d.outputW == 256u);
    CHECK(d.outputH == 256u);
}

TEST_CASE("ComputeScaledDims — 4K source downscaled to 16 fits in 16×16",
          "[dimension][edge]") {
    auto d = ComputeScaledDims(3840, 2160, 16);
    CHECK(d.outputW <= 16u);
    CHECK(d.outputH <= 16u);
}
