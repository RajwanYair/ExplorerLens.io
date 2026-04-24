// ============================================================================
// LibTiffDecoderStub.h -- S258 / ROADMAP v6.0 L10 replacement, H10
//
// Phase 2 libtiff-backed TIFF decoder contract.  Pairs with TiledTiffReaderStub
// (S255) for pyramid / tile reads on large TIFFs.  Header-only until vcpkg
// libtiff port is wired.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "PixelPipeline16Bit.h"

namespace ExplorerLens::Engine {

enum class LibTiffCompression : uint16_t
{
    NONE               = 1,
    CCITT_RLE          = 2,
    CCITT_FAX3         = 3,
    CCITT_FAX4         = 4,
    LZW                = 5,
    OLD_JPEG           = 6,
    JPEG               = 7,
    DEFLATE            = 8,
    PACKBITS           = 32773,
    LZMA               = 34925,
    ZSTD               = 50000,
    WEBP               = 50001,
    JPEG2000           = 34712,
    UNKNOWN            = 65535,
};

enum class LibTiffPlanarConfig : uint8_t
{
    CONTIGUOUS         = 1,     // RGBRGB...
    SEPARATE           = 2,     // RRR...GGG...BBB
};

enum class LibTiffStatus : uint8_t
{
    OK                 = 0,
    IO_ERROR           = 1,
    UNSUPPORTED_COMPRESSION = 2,
    UNSUPPORTED_PLANAR = 3,
    TRUNCATED          = 4,
    OUT_OF_BUDGET      = 5,
    CANCELLED          = 6,
    BIGTIFF_UNSUPPORTED= 7,
};

struct LibTiffPageSelector
{
    uint32_t preferredPage   = 0;     // IFD index
    uint32_t maxPagesToScan  = 16;    // cap per-file IFD walk
    bool     pickLargestPage = true;  // for multi-page TIFF, pick biggest thumb
    bool     pickFirstOnly   = false;
};

struct LibTiffDecodeOptions
{
    uint32_t             targetWidth        = 0;
    uint32_t             targetHeight       = 0;
    PixelDepth           interiorDepth      = PixelDepth::SIXTEEN;
    LibTiffPageSelector  pageSelector       = {};
    bool                 honorIccProfile    = true;
    bool                 honorOrientation   = true;
    bool                 allowTiled         = true;   // pairs with S255
    bool                 allowPyramid       = true;
};

struct LibTiffProbeResult
{
    LibTiffStatus        status             = LibTiffStatus::OK;
    uint32_t             width              = 0;
    uint32_t             height             = 0;
    uint16_t             samplesPerPixel    = 0;
    uint16_t             bitsPerSample      = 0;
    LibTiffCompression   compression        = LibTiffCompression::UNKNOWN;
    LibTiffPlanarConfig  planar             = LibTiffPlanarConfig::CONTIGUOUS;
    uint32_t             pageCount          = 0;
    bool                 isTiled            = false;
    bool                 isBigTiff          = false;
};

inline constexpr uint64_t kLibTiffMaxFileBytes  = 2ull * 1024 * 1024 * 1024;  // 2 GiB
inline constexpr uint32_t kLibTiffMaxSide       = 32768;
inline constexpr uint32_t kLibTiffMaxPages      = 1024;

static_assert(std::is_trivially_copyable_v<LibTiffDecodeOptions>,
              "LibTiffDecodeOptions must be trivially copyable");
static_assert(std::is_trivially_copyable_v<LibTiffProbeResult>,
              "LibTiffProbeResult must be trivially copyable");
static_assert(static_cast<uint16_t>(LibTiffCompression::JPEG) == 7,
              "TIFF compression JPEG must be libtiff value 7");
static_assert(static_cast<uint16_t>(LibTiffCompression::DEFLATE) == 8,
              "TIFF compression DEFLATE must be libtiff value 8");

} // namespace ExplorerLens::Engine
