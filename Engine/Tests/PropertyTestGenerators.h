// ============================================================================
// PropertyTestGenerators.h -- S256 / ROADMAP v6.0 T9
//
// Phase 2 property-based pixel-math test generator scaffolding.  Header-only.
// Emits deterministic `GeneratedPixelCase` seeds for tests that exercise
// stride, clamp, premultiply, and color-convert invariants across a wide
// input range without requiring RapidCheck.
//
// The actual TEST() bodies that consume these land alongside the 16-bit
// pipeline (S252) and D3D11 resize (S249) once their C++ code is written.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class PropertyTestFamily : uint8_t
{
    STRIDE_INVARIANT      = 0,   // output unchanged under stride padding
    CLAMP_IDEMPOTENT      = 1,   // clamp(clamp(x)) == clamp(x)
    PREMUL_ROUNDTRIP      = 2,   // unpremul(premul(x,a),a) ~= x
    COLOR_ROUNDTRIP       = 3,   // sRGB->linear->sRGB ~= identity
    RESIZE_BOX_CONSERVES  = 4,   // box-resize preserves total luminance (+/- eps)
};

struct PixelPropertySeed
{
    uint32_t           seed           = 0;      // 0..kPropertyTestIterations-1
    PropertyTestFamily family         = PropertyTestFamily::STRIDE_INVARIANT;
    uint32_t           width          = 0;
    uint32_t           height         = 0;
    uint8_t            bytesPerPixel  = 4;
    bool               linear         = false;
};

struct GeneratedPixelCase
{
    PixelPropertySeed seed          = {};
    uint32_t          strideBytes   = 0;
    uint64_t          bufferBytes   = 0;
    uint32_t          expectedHash  = 0;   // FNV-1a of canonical result (optional)
};

struct PropertyTestPolicy
{
    uint32_t iterations              = 0;
    uint32_t widthRangeLo            = 1;
    uint32_t widthRangeHi            = 2048;
    uint32_t heightRangeLo           = 1;
    uint32_t heightRangeHi           = 2048;
    bool     allowZeroSizedBuffers   = false;
    bool     allowUnalignedStride    = true;
};

inline constexpr uint32_t kPropertyTestIterations      = 256;
inline constexpr uint32_t kPropertyTestMaxDim          = 4096;
inline constexpr uint32_t kPropertyTestSmokeIterations = 16;

// Deterministic xorshift RNG so tests are reproducible across platforms.
constexpr uint32_t PropertyRng(uint32_t state) noexcept
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

constexpr PixelPropertySeed MakePropertySeed(uint32_t iteration,
                                             PropertyTestFamily fam) noexcept
{
    const uint32_t r = PropertyRng(iteration + 1);
    PixelPropertySeed s{};
    s.seed          = iteration;
    s.family        = fam;
    s.width         = 1 + (r         % kPropertyTestMaxDim);
    s.height        = 1 + ((r >> 8)  % kPropertyTestMaxDim);
    s.bytesPerPixel = ((r >> 16) & 1) ? 4 : 8;      // BGRA8 or BGRA16
    s.linear        = ((r >> 17) & 1) != 0;
    return s;
}

static_assert(std::is_trivially_copyable_v<PixelPropertySeed>,
              "PixelPropertySeed must be trivially copyable");
static_assert(std::is_trivially_copyable_v<GeneratedPixelCase>,
              "GeneratedPixelCase must be trivially copyable");
static_assert(kPropertyTestSmokeIterations < kPropertyTestIterations,
              "Smoke count must be below full count");
static_assert(MakePropertySeed(0, PropertyTestFamily::CLAMP_IDEMPOTENT).width >= 1,
              "PropertyRng must yield width >= 1");

} // namespace ExplorerLens::Engine
