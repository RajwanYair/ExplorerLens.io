// ============================================================================
// OpenEXR3DecoderStub.h -- S259 / ROADMAP v6.0 L11 evaluation
//
// Phase 2 OpenEXR 3.x evaluation contract -- replaces the tinyexr main path
// with the real OpenEXR library for full multi-part / multi-view / DWAA
// compressed scanline support.
//
// Header-only.  Actual decode lands once OpenEXR 3.3+ (Imath 3.1+) is wired
// into vcpkg manifest mode.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "../Core/PixelPipeline16Bit.h"

namespace ExplorerLens::Engine {

enum class OpenEXRChannelLayout : uint8_t
{
    RGB                = 0,
    RGBA               = 1,
    Y                  = 2,   // luminance only
    YA                 = 3,   // luminance + alpha
    YC                 = 4,   // luminance + chroma
    YCA                = 5,
    DEEP               = 6,   // deep scanline / tiled (Phase 3+)
    CUSTOM             = 7,
};

enum class OpenEXRCompression : uint8_t
{
    NONE               = 0,
    RLE                = 1,
    ZIPS               = 2,   // single-scanline deflate
    ZIP                = 3,   // 16-scanline deflate
    PIZ                = 4,
    PXR24              = 5,
    B44                = 6,
    B44A               = 7,
    DWAA               = 8,   // 32-scanline DCT
    DWAB               = 9,   // 256-scanline DCT
    HTJ2K              = 10,  // Phase 3+
    UNKNOWN            = 255,
};

enum class OpenEXRStatus : uint8_t
{
    OK                 = 0,
    IO_ERROR           = 1,
    UNSUPPORTED_COMP   = 2,
    UNSUPPORTED_LAYOUT = 3,
    UNSUPPORTED_DEPTH  = 4,
    TRUNCATED          = 5,
    OUT_OF_BUDGET      = 6,
    CANCELLED          = 7,
};

enum class OpenEXRPixelType : uint8_t
{
    UINT32             = 0,
    HALF               = 1,   // 16-bit IEEE 754 half
    FLOAT              = 2,   // 32-bit float
};

struct OpenEXRDecodeOptions
{
    uint32_t             targetWidth         = 0;
    uint32_t             targetHeight        = 0;
    PixelDepth           interiorDepth       = PixelDepth::HALF_FLOAT;
    OpenEXRChannelLayout preferredLayout     = OpenEXRChannelLayout::RGBA;
    const char*          partName            = nullptr;   // null -> first part
    bool                 honorChromaticities = true;
    bool                 applyToneMap        = true;      // Reinhard for 8-bit out
    bool                 allowDeep           = false;     // Phase 3+
};

struct OpenEXRProbeResult
{
    OpenEXRStatus        status         = OpenEXRStatus::OK;
    uint32_t             width          = 0;
    uint32_t             height         = 0;
    uint32_t             partCount      = 0;
    OpenEXRChannelLayout layout         = OpenEXRChannelLayout::RGB;
    OpenEXRCompression   compression    = OpenEXRCompression::UNKNOWN;
    OpenEXRPixelType     pixelType      = OpenEXRPixelType::HALF;
    bool                 isTiled        = false;
    bool                 isMultiView    = false;
    bool                 isDeep         = false;
};

inline constexpr uint64_t kOpenEXRMaxFileBytes = 2ull * 1024 * 1024 * 1024; // 2 GiB
inline constexpr uint32_t kOpenEXRMaxSide      = 16384;
inline constexpr uint32_t kOpenEXRMaxPartCount = 64;

static_assert(std::is_trivially_copyable_v<OpenEXRDecodeOptions>,
              "OpenEXRDecodeOptions must be trivially copyable");
static_assert(std::is_trivially_copyable_v<OpenEXRProbeResult>,
              "OpenEXRProbeResult must be trivially copyable");
static_assert(sizeof(OpenEXRProbeResult) <= 64,
              "OpenEXRProbeResult should stay small and cache-friendly");

} // namespace ExplorerLens::Engine
