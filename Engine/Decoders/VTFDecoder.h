#pragma once
//==============================================================================
// VTF (Valve Texture Format) Decoder
// Game Texture Format Support
// Supports VTF versions 7.0-7.5 (Source Engine textures)
// Handles DXT1/DXT5/BGRA8888/RGB888 image formats
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

    //==========================================================================
    // VTF image formats (subset relevant for thumbnail extraction)
    //==========================================================================
    enum class VTFImageFormat : int32_t {
        RGBA8888      = 0,
        ABGR8888      = 1,
        RGB888        = 2,
        BGR888         = 3,
        RGB565        = 4,
        I8            = 5,   // Luminance
        IA88          = 6,   // Luminance + Alpha
        A8            = 8,
        RGB888_BLUESCREEN = 9,
        BGR888_BLUESCREEN = 10,
        ARGB8888      = 11,
        BGRA8888      = 12,
        DXT1          = 13,
        DXT3          = 14,
        DXT5          = 15,
        BGRX8888      = 16,
        BGR565        = 17,
        BGRX5551      = 18,
        BGRA4444      = 19,
        DXT1_ONEBITALPHA = 20,
        BGRA5551      = 21,
        UV88          = 22,
        UVWQ8888      = 23,
        RGBA16161616F = 24,
        RGBA16161616  = 25,
        UVLX8888      = 26,
        NONE          = -1
    };

    //==========================================================================
    // VTF file header (version 7.x)
    //==========================================================================
    struct VTFHeader {
        char     signature[4];     // "VTF\0"
        uint32_t versionMajor;     // 7
        uint32_t versionMinor;     // 0-5
        uint32_t headerSize;
        uint16_t width;
        uint16_t height;
        uint32_t flags;
        uint16_t frames;
        uint16_t firstFrame;
        uint8_t  padding0[4];
        float    reflectivity[3];
        uint8_t  padding1[4];
        float    bumpmapScale;
        int32_t  highResImageFormat; // VTFImageFormat
        uint8_t  mipmapCount;
        int32_t  lowResImageFormat;  // VTFImageFormat (thumbnail)
        uint8_t  lowResImageWidth;
        uint8_t  lowResImageHeight;
    };

    //==========================================================================
    // VTF Decoder
    //==========================================================================
    class VTFDecoder {
    public:
        VTFDecoder() = default;

        struct DecodeResult {
            bool     success = false;
            uint32_t width = 0;
            uint32_t height = 0;
            std::vector<uint8_t> pixelData; // BGRA32
            std::string error;
        };

        struct TextureInfo {
            uint32_t       width = 0;
            uint32_t       height = 0;
            uint32_t       versionMajor = 0;
            uint32_t       versionMinor = 0;
            uint8_t        mipmapCount = 0;
            uint16_t       frames = 0;
            VTFImageFormat format = VTFImageFormat::NONE;
            bool           hasLowResImage = false;
            uint8_t        lowResWidth = 0;
            uint8_t        lowResHeight = 0;

            bool IsValid() const { return width > 0 && height > 0 && versionMajor == 7; }
        };

        /// Read VTF header info without decoding.
        TextureInfo ReadInfo(const std::string& filePath) const;

        /// Decode VTF texture to BGRA32 pixels.
        DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const;

        /// Check if a file extension is a VTF format.
        static bool IsVTFExtension(const std::string& ext);

        /// Supported extensions
        static constexpr const char* EXTENSIONS[] = { ".vtf", nullptr };

    private:
        /// Compute the byte size of a single mip level for a given format.
        static size_t ComputeImageSize(uint32_t width, uint32_t height, VTFImageFormat format);

        /// Decode DXT1-compressed data to BGRA32.
        static void DecompressDXT1(const uint8_t* src, uint8_t* dst,
                                    uint32_t width, uint32_t height);

        /// Decode DXT5-compressed data to BGRA32.
        static void DecompressDXT5(const uint8_t* src, uint8_t* dst,
                                    uint32_t width, uint32_t height);

        /// Decode a single BC1 block.
        static void DecompressBC1Block(const uint8_t* block, uint8_t* output,
                                       uint32_t outputStride);
    };

} // namespace ExplorerLens::Decoders

