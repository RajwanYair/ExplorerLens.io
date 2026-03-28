// FLIFDecoderV2.h — Free Lossless Image Format (FLIF) v2 Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Second-generation FLIF decoder supporting FLIF16 (v1.3+) bitstream with
// optional interlaced and non-interlaced modes, alpha channel, and animation
// frame extraction. Provides progressive early-exit for fast thumbnail decode.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

enum class FLIFColorSpace : uint8_t {
    Grayscale = 1,
    RGB       = 3,
    RGBA      = 4,
};

enum class FLIFInterlace : uint8_t {
    NonInterlaced = 0,
    Interlaced    = 1,
};

class FLIFDecoderV2 {
public:
    FLIFDecoderV2() = default;

    struct DecodeResult {
        bool        success     = false;
        uint32_t    width       = 0;
        uint32_t    height      = 0;
        uint32_t    frameCount  = 1;
        FLIFColorSpace colorSpace = FLIFColorSpace::RGBA;
        std::vector<uint8_t> pixelData;  // BGRA32 of first/best frame
        std::string error;
    };

    struct FLIFInfo {
        uint32_t       width       = 0;
        uint32_t       height      = 0;
        uint32_t       frameCount  = 1;
        uint16_t       bitDepth    = 8;
        FLIFColorSpace colorSpace  = FLIFColorSpace::RGBA;
        FLIFInterlace  interlace   = FLIFInterlace::Interlaced;

        bool IsValid() const { return width > 0 && height > 0; }
        bool IsAnimated() const { return frameCount > 1; }
    };

    FLIFInfo ReadInfo(const std::string& filePath) const;

    /// Decode with optional quality level (0=fastest thumbnail, 100=full).
    DecodeResult Decode(const std::string& filePath,
                        uint32_t targetWidth = 256,
                        int      quality     = 50) const;

    static bool IsFLIFExtension(const std::string& ext);
    static constexpr const char* EXTENSIONS[] = { ".flif", nullptr };

private:
    static bool VerifyFLIFHeader(const uint8_t* header, size_t len);
};

}  // namespace ExplorerLens::Decoders
