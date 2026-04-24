// ============================================================================
// PixelPipeline16Bit.h -- S252 / ROADMAP v6.0 H24, A11
//
// Phase 2 16-bit pixel pipeline policy.  Header-only; the actual 16-bit decode
// lives in per-decoder source once libpng / libtiff / libraw wiring lands.
//
// Goal: eliminate premature 8-bit clamping in the decode -> color-correct ->
// resize path (H24 from RawTherapee).  Downstream consumers (GDI+ HBITMAP
// emitter, IPreviewHandler) still request 8-bit BGRA -- 16-bit is an interior
// pipeline representation, not a wire format.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class PixelDepth : uint8_t
{
    EIGHT       = 0,   // BGRA8 / RGBA8          (wire format to GDI+)
    SIXTEEN     = 1,   // RGBA16                 (interior depth for PNG, TIFF, RAW)
    HALF_FLOAT  = 2,   // RGBA16F                (interior depth for EXR)
    FLOAT       = 3,   // RGBA32F                (interior depth for HDR tone-map)
};

enum class Pixel16Path : uint8_t
{
    PRESERVE    = 0,   // Keep 16-bit through color-correct + resize
    QUANTISE_8  = 1,   // Quantise to 8-bit immediately after decode (legacy)
    DITHER_8    = 2,   // Dither to 8-bit (for noisy RAW thumbs)
};

struct Pixel16Policy
{
    PixelDepth   interiorDepth   = PixelDepth::SIXTEEN;
    Pixel16Path  emitPath        = Pixel16Path::PRESERVE;
    bool         linearPremul    = true;           // premultiply alpha in linear space
    bool         srgbFinal       = true;           // final encode must be sRGB gamma
};

// Pixel size in bytes for a given depth.  Used by PixelSpan2D stride checks.
constexpr size_t PixelBytesPerSample(PixelDepth d) noexcept
{
    switch (d) {
        case PixelDepth::EIGHT:      return 1;
        case PixelDepth::SIXTEEN:    return 2;
        case PixelDepth::HALF_FLOAT: return 2;
        case PixelDepth::FLOAT:      return 4;
    }
    return 1;
}

constexpr size_t PixelBytesPerBGRAPixel(PixelDepth d) noexcept
{
    return PixelBytesPerSample(d) * 4;
}

// Decision helper: do we preserve 16-bit for this input depth + user setting?
constexpr bool ShouldPreserve16Bit(PixelDepth decodedDepth,
                                   Pixel16Path emitPath) noexcept
{
    if (decodedDepth == PixelDepth::EIGHT) return false;
    return emitPath == Pixel16Path::PRESERVE;
}

static_assert(std::is_trivially_copyable_v<Pixel16Policy>,
              "Pixel16Policy must be trivially copyable");
static_assert(PixelBytesPerBGRAPixel(PixelDepth::EIGHT)   ==  4, "BGRA8  sanity");
static_assert(PixelBytesPerBGRAPixel(PixelDepth::SIXTEEN) ==  8, "BGRA16 sanity");
static_assert(PixelBytesPerBGRAPixel(PixelDepth::FLOAT)   == 16, "BGRA32F sanity");

} // namespace ExplorerLens::Engine
