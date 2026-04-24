// ============================================================================
// LibPngDecoderStub.h -- S257 / ROADMAP v6.0 L10 replacement
//
// Phase 2 libpng-backed PNG decoder contract.  Replaces the stb_image PNG
// path for full progressive loading + 16-bit pipeline support (H24).
// Header-only until the vcpkg libpng port is wired.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "PixelPipeline16Bit.h"

namespace ExplorerLens::Engine {

enum class LibPngColorType : uint8_t
{
    GRAY              = 0,
    RGB               = 2,
    PALETTE           = 3,
    GRAY_ALPHA        = 4,
    RGBA              = 6,
    UNKNOWN           = 255,
};

enum class LibPngInterlace : uint8_t
{
    NONE              = 0,
    ADAM7             = 1,
};

enum class PngDecoderStatus : uint8_t
{
    OK                = 0,
    IO_ERROR          = 1,
    CRC_MISMATCH      = 2,
    UNSUPPORTED_DEPTH = 3,
    UNSUPPORTED_CSP   = 4,
    TRUNCATED         = 5,
    OUT_OF_BUDGET     = 6,
    CANCELLED         = 7,
};

struct LibPngDecodeOptions
{
    uint32_t         targetWidth         = 0;
    uint32_t         targetHeight        = 0;
    PixelDepth       interiorDepth       = PixelDepth::SIXTEEN;
    bool             expandGrayToRgba    = true;
    bool             expandPaletteToRgba = true;
    bool             stripSixteenToEight = false;   // override 16->8 quantise
    bool             honorGamma          = true;
    bool             honorIccProfile     = true;
    bool             progressive         = false;   // if true, use progressive API
};

struct LibPngProbeResult
{
    PngDecoderStatus status         = PngDecoderStatus::OK;
    uint32_t         width          = 0;
    uint32_t         height         = 0;
    uint8_t          bitDepth       = 0;
    LibPngColorType  colorType      = LibPngColorType::UNKNOWN;
    LibPngInterlace  interlace      = LibPngInterlace::NONE;
    bool             hasTransparency= false;
    bool             hasIccProfile  = false;
    bool             hasGama        = false;
};

struct LibPngVersionInfo
{
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
};

inline constexpr uint64_t kLibPngMaxFileBytes   = 256ull * 1024 * 1024;   // 256 MiB
inline constexpr uint32_t kLibPngMaxSide        = 16384;
inline constexpr uint32_t kLibPngMinSide        = 1;

static_assert(std::is_trivially_copyable_v<LibPngDecodeOptions>,
              "LibPngDecodeOptions must be trivially copyable");
static_assert(std::is_trivially_copyable_v<LibPngProbeResult>,
              "LibPngProbeResult must be trivially copyable");
static_assert(static_cast<uint8_t>(LibPngColorType::RGBA) == 6,
              "PNG color type RGBA must be libpng value 6");

} // namespace ExplorerLens::Engine
