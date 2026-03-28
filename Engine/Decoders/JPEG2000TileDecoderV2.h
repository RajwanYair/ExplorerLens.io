// JPEG2000TileDecoderV2.h — JPEG 2000 Tiled Decode v2 (Sub-resolution)
// Copyright (c) 2026 ExplorerLens Project
//
// Second-generation JPEG 2000 tile decoder that exploits the built-in
// resolution hierarchy (DWT sub-bands) to decode only the smallest
// sufficient tile resolution. Avoids full-image decode for thumbnails.
// Builds on top of the openjpeg library.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

/// Sub-resolution factor used when decoding a JPEG 2000 tile.
enum class J2KResductionFactor : uint8_t {
    Full      = 0,   // 1:1 — full resolution
    Half      = 1,   // 1:2
    Quarter   = 2,   // 1:4
    Eighth    = 3,   // 1:8
    Sixteenth = 4,   // 1:16
};

class JPEG2000TileDecoderV2 {
public:
    JPEG2000TileDecoderV2() = default;

    struct DecodeResult {
        bool     success       = false;
        uint32_t width         = 0;
        uint32_t height        = 0;
        uint32_t tileWidth     = 0;
        uint32_t tileHeight    = 0;
        uint32_t components    = 0;
        std::vector<uint8_t> pixelData;  // BGRA32
        std::string error;
    };

    struct TileInfo {
        uint32_t imageWidth    = 0;
        uint32_t imageHeight   = 0;
        uint32_t tileWidth     = 0;
        uint32_t tileHeight    = 0;
        uint32_t tilesX        = 0;
        uint32_t tilesY        = 0;
        uint32_t resolutionLevels = 0;
        uint32_t components    = 0;

        bool IsValid() const { return imageWidth > 0 && imageHeight > 0 && tileWidth > 0; }
        uint32_t TotalTiles() const { return tilesX * tilesY; }
    };

    TileInfo  ReadInfo(const std::string& filePath) const;

    /// Decode at the smallest sub-resolution sufficient for targetWidth×targetHeight.
    DecodeResult DecodeThumbnail(const std::string& filePath,
                                  uint32_t targetWidth  = 256,
                                  uint32_t targetHeight = 256) const;

    static bool IsJ2KExtension(const std::string& ext);
    static constexpr const char* EXTENSIONS[] = { ".jp2", ".j2k", ".jpx", ".jpc", nullptr };

private:
    static J2KResductionFactor ChooseReductionFactor(uint32_t srcW, uint32_t srcH,
                                                      uint32_t dstW, uint32_t dstH,
                                                      uint32_t maxLevels);
};

}  // namespace ExplorerLens::Decoders
