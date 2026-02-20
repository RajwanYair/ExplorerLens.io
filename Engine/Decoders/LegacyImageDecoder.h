//==============================================================================
// DarkThumbs Engine — Sprint 284: FLIF/BPG Legacy Image Decoder
// Free Lossless Image Format and Better Portable Graphics decoders.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Legacy image format type
enum class LegacyImageFormat : uint8_t {
    FLIF,       // Free Lossless Image Format
    BPG,        // Better Portable Graphics (H.265 intra)
    JPEG2000,   // JPEG 2000 (already supported, ref here)
    PCX,        // PC Paintbrush
    TGA,        // Truevision TGA
    SGI,        // Silicon Graphics Image
    COUNT
};

/// Image color space
enum class LegacyColorSpace : uint8_t {
    Grayscale,
    RGB,
    RGBA,
    YCbCr,
    CMYK,
    Palette,
    COUNT
};

/// Legacy image metadata
struct LegacyImageInfo {
    LegacyImageFormat format    = LegacyImageFormat::FLIF;
    LegacyColorSpace  colorSpace = LegacyColorSpace::RGB;
    uint32_t    width           = 0;
    uint32_t    height          = 0;
    uint32_t    bitsPerPixel    = 0;
    uint32_t    frameCount      = 1;
    bool        interlaced      = false;
    bool        hasAlpha        = false;
};

/// Legacy image decoder config
struct LegacyImageConfig {
    uint32_t maxDimension   = 16384;
    bool     enableDithering = false;
    bool     convertToRGB   = true;
};

/// Legacy image decoder
class LegacyImageDecoder {
public:
    static const wchar_t* FormatName(LegacyImageFormat f) {
        switch (f) {
            case LegacyImageFormat::FLIF:     return L"FLIF";
            case LegacyImageFormat::BPG:      return L"BPG";
            case LegacyImageFormat::JPEG2000: return L"JPEG 2000";
            case LegacyImageFormat::PCX:      return L"PCX";
            case LegacyImageFormat::TGA:      return L"TGA";
            case LegacyImageFormat::SGI:      return L"SGI";
            default: return L"Unknown";
        }
    }

    static const wchar_t* ColorSpaceName(LegacyColorSpace cs) {
        switch (cs) {
            case LegacyColorSpace::Grayscale: return L"Grayscale";
            case LegacyColorSpace::RGB:       return L"RGB";
            case LegacyColorSpace::RGBA:      return L"RGBA";
            case LegacyColorSpace::YCbCr:     return L"YCbCr";
            case LegacyColorSpace::CMYK:      return L"CMYK";
            case LegacyColorSpace::Palette:   return L"Palette";
            default: return L"Unknown";
        }
    }

    /// FLIF magic: "FLIF"
    static bool CheckFLIFMagic(const uint8_t* data, size_t size) {
        if (size < 4) return false;
        return data[0] == 'F' && data[1] == 'L' && data[2] == 'I' && data[3] == 'F';
    }

    /// BPG magic: 0x42 0x50 0x47 0xFB
    static bool CheckBPGMagic(const uint8_t* data, size_t size) {
        if (size < 4) return false;
        return data[0] == 0x42 && data[1] == 0x50 && data[2] == 0x47 && data[3] == 0xFB;
    }

    static constexpr size_t FormatCount() { return static_cast<size_t>(LegacyImageFormat::COUNT); }
    static constexpr size_t ColorSpaceCount() { return static_cast<size_t>(LegacyColorSpace::COUNT); }
};

}} // namespace DarkThumbs::Engine
