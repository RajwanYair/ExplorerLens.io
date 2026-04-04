#pragma once
//==============================================================================
// SGI Image Decoder (.sgi/.rgb/.rgba/.bw/.int/.inta)
// SGI/RGB & Legacy Format Support
// Handles SGI image format used in IRIX/workstation graphics.
// Supports RLE and verbatim (uncompressed) encoding, 1-4 channels.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

//==========================================================================
// SGI Image storage types
//==========================================================================
enum class SGIStorageType : uint8_t {
    Verbatim = 0,  // Uncompressed
    RLE = 1        // Run-length encoded
};

//==========================================================================
// SGI Decoder
//==========================================================================
class SGIDecoder
{
  public:
    SGIDecoder() = default;

    struct DecodeResult
    {
        bool success = false;
        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<uint8_t> pixelData;  // BGRA32
        std::string error;
    };

    struct ImageInfo
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint16_t channels = 0;        // 1=grayscale, 2=gray+alpha, 3=RGB, 4=RGBA
        uint8_t bytesPerChannel = 1;  // 1 or 2
        SGIStorageType storage = SGIStorageType::Verbatim;
        std::string imageName;  // 80-char name field

        bool IsValid() const
        {
            return width > 0 && height > 0 && channels >= 1 && channels <= 4;
        }
    };

    /// Read SGI header metadata.
    ImageInfo ReadInfo(const std::string& filePath) const;

    /// Decode SGI image to BGRA32 pixels.
    DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const;

    /// Check if extension is an SGI image format.
    static bool IsSGIExtension(const std::string& ext);

    /// Supported extensions
    static constexpr const char* EXTENSIONS[] = {".sgi", ".rgb", ".rgba", ".bw", ".int", ".inta", nullptr};

  private:
    /// Decode RLE-compressed scanline.
    static size_t DecodeRLEScanline(const uint8_t* src, size_t srcLen, uint8_t* dst, size_t dstLen);
};

}  // namespace ExplorerLens::Decoders
