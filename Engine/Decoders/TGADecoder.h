// TGADecoder.h - Targa Image Format Decoder
// ExplorerLens Engine v5.3.0+
// HDR/Professional Format Support
//
// Supports: Truevision TGA/TARGA (.tga, .tpic)
// Features:
// - Uncompressed RGB/RGBA images
// - RLE-compressed images
// - 16-bit, 24-bit, 32-bit color depths
// - Alpha channel support
// - Top-down and bottom-up orientations

#pragma once

#include <cstdint>
#include <memory>
#include "../Core/IThumbnailDecoder.h"

namespace ExplorerLens {
namespace Engine {

class TGADecoder : public IThumbnailDecoder
{
  public:
    TGADecoder();
    ~TGADecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override
    {
        return L"TGADecoder";
    }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override
    {
        return m_extensionCount;
    }
    bool SupportsGPU() const override
    {
        return false;
    }  // CPU decode
    bool IsArchiveDecoder() const override
    {
        return false;
    }

  private:
// TGA header structure (18 bytes)
#pragma pack(push, 1)
    struct TGAHeader
    {
        uint8_t idLength;
        uint8_t colorMapType;
        uint8_t imageType;
        uint16_t colorMapStart;
        uint16_t colorMapLength;
        uint8_t colorMapDepth;
        uint16_t xOrigin;
        uint16_t yOrigin;
        uint16_t width;
        uint16_t height;
        uint8_t bitsPerPixel;
        uint8_t imageDescriptor;
    };
#pragma pack(pop)

    // Image types
    enum TGAImageType {
        TGA_NO_IMAGE = 0,
        TGA_INDEXED = 1,
        TGA_RGB = 2,
        TGA_GRAYSCALE = 3,
        TGA_RLE_INDEXED = 9,
        TGA_RLE_RGB = 10,
        TGA_RLE_GRAYSCALE = 11
    };

    // Decoding functions
    HRESULT DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap);
    HRESULT DecodeUncompressed(const uint8_t* data, const TGAHeader& header, uint8_t** ppPixels, uint32_t* pStride);
    HRESULT DecodeRLE(const uint8_t* data, const TGAHeader& header, uint8_t** ppPixels, uint32_t* pStride);

    // Format validation
    bool IsTGAFormat(const wchar_t* path);
    bool IsTGAFormat(const uint8_t* data, size_t size);

    // Bitmap creation
    HBITMAP CreateBitmapFromRGB(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t bpp, bool flipY);

    // Read file data
    std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* path, size_t& fileSize);

    // Extension list
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

}  // namespace Engine
}  // namespace ExplorerLens
