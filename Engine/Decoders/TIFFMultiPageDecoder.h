// TIFFMultiPageDecoder.h — Multi-Page TIFF Thumbnail Selector
// Copyright (c) 2026 ExplorerLens Project
//
// Enumerates TIFF IFD chains, selects the best page for thumbnail extraction
// (thumbnail sub-IFD > reduced-resolution IFD > largest full-size page),
// and decodes it to BGRA with support for CMYK, LAB, and bilevel inputs.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- IFD / Page Metadata ----------------------------------------------------

enum class TIFFCompression : uint16_t {
    None          = 1,
    CCITT_RLE     = 2,
    CCITT_T4      = 3,
    CCITT_T6      = 4,
    LZW           = 5,
    OJPEG         = 6,
    JPEG          = 7,
    AdobeDeflate  = 8,
    PackBits      = 32773,
    JBIG          = 34661,
    LERC          = 34887,
    WebP          = 50001,
    ZStd          = 50000,
    Unknown       = 0xFFFF,
};

enum class TIFFColorSpace : uint8_t {
    Bilevel     = 0,   // 1 bpp black/white
    Grayscale   = 1,
    Palette     = 2,   // Indexed color
    RGB         = 3,
    CMYK        = 4,
    YCbCr       = 5,
    CIELab      = 6,
    ICCLab      = 7,
    Unknown     = 0xFF,
};

struct TIFFPageInfo {
    uint32_t         ifdIndex      = 0;     // 0-based IFD chain position
    uint32_t         width         = 0;
    uint32_t         height        = 0;
    uint16_t         bitsPerSample = 8;
    TIFFColorSpace   colorSpace    = TIFFColorSpace::Unknown;
    TIFFCompression  compression   = TIFFCompression::None;
    bool             isThumbIFD    = false; // Found via SubIFD tag (279) or EXIF thumb
    bool             isReducedRes  = false; // NewSubfileType bit 0 = 1
    bool             isMask        = false; // NewSubfileType transparency mask
    bool             hasTiling     = false; // Tiled vs strip organization
};

struct TIFFDocumentInfo {
    uint32_t                  pageCount    = 0;
    bool                      isBigTIFF    = false;  // 64-bit TIFF (BigTIFF v2)
    bool                      isLittleEndian = true;
    std::vector<TIFFPageInfo> pages;
};

// ---- Options & Result -------------------------------------------------------

struct TIFFDecodeOptions {
    bool     autoSelectBestPage    = true;   // Use heuristic (thumb > reduced > largest)
    uint32_t preferredPageIndex    = 0;      // Override if autoSelectBestPage = false
    bool     cmykToSRGB            = true;   // Convert CMYK pages to sRGB
    bool     bilevelToGray         = true;   // Expand 1bpp to 8bpp grayscale
    uint32_t maxDimension          = 4096;   // Clamp to this before BGRA conversion
};

enum class TIFFDecodeStatus {
    Success             = 0,
    InvalidData         = 1,
    UnsupportedCodec    = 2,   // Compression not handled (e.g., LERC)
    NoPagesFound        = 3,
    PageOutOfRange      = 4,
    LibraryMissing      = 5,
    OutOfMemory         = 6,
    InternalError       = 99,
};

struct TIFFDecodeResult {
    TIFFDecodeStatus     status      = TIFFDecodeStatus::InternalError;
    std::vector<uint8_t> pixels;     // BGRA, row-major, stride = width * 4
    uint32_t             width       = 0;
    uint32_t             height      = 0;
    uint32_t             decodedPage = 0;
    bool                 colorConverted = false;   // CMYK/LAB -> sRGB applied
};

// ---- TIFFMultiPageDecoder ---------------------------------------------------

class TIFFMultiPageDecoder {
public:
    TIFFMultiPageDecoder();
    ~TIFFMultiPageDecoder();

    // Scan IFD chain and populate TIFFDocumentInfo without decoding pixels.
    bool ParseInfo(const uint8_t* data, size_t size, TIFFDocumentInfo& outInfo) const;

    // Select and decode the best page using thumbnail-selection heuristic.
    TIFFDecodeResult DecodeBestPage(
        const uint8_t*         data,
        size_t                 size,
        const TIFFDecodeOptions& opts = {}) const;

    // Decode a specific page by 0-based IFD index.
    TIFFDecodeResult DecodePage(
        const uint8_t*         data,
        size_t                 size,
        uint32_t               pageIndex,
        const TIFFDecodeOptions& opts = {}) const;

    // Magic probe: "II" (0x4949) little-endian or "MM" (0x4D4D) big-endian
    // followed by 42 (Classic) or 43 (BigTIFF) magic word.
    static bool LooksLikeTIFF(const uint8_t* data, size_t size);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // Heuristic: prefer SubIFD thumbnail → reduced-res IFD → page with smallest
    // dimension above 256px → fallback to IFD at index 0.
    uint32_t SelectBestPageIndex(const TIFFDocumentInfo& info) const;

    TIFFDecodeResult DecodeInternal(
        const uint8_t*          data,
        size_t                  size,
        uint32_t                ifdIndex,
        const TIFFDecodeOptions& opts) const;

    static void CMYKtoSRGB(
        std::vector<uint8_t>& pixels,
        uint32_t width,
        uint32_t height);
};

} // namespace Engine
} // namespace ExplorerLens
