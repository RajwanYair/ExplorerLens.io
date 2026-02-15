// PSDDecoder.h - Adobe Photoshop PSD/PSB Decoder
// DarkThumbs Engine v6.1.0+
// Copyright (c) 2026 DarkThumbs Project
//
// Supports: Adobe Photoshop (.psd), Large Document Format (.psb)
// Features:
// - Extracts embedded composite (merged) image
// - Supports 8-bit RGB/RGBA/Grayscale color modes
// - No external dependencies (direct PSD binary parsing)
// - Reads image dimensions from header for fast preview

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>
#include <memory>

namespace DarkThumbs {
namespace Engine {

class PSDDecoder : public IThumbnailDecoder {
public:
    PSDDecoder();
    ~PSDDecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"PSDDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

private:
    // PSD file header (26 bytes)
    #pragma pack(push, 1)
    struct PSDHeader {
        char     signature[4];  // "8BPS"
        uint16_t version;       // 1 = PSD, 2 = PSB
        uint8_t  reserved[6];
        uint16_t channels;      // 1-56
        uint32_t height;
        uint32_t width;
        uint16_t depth;         // bits per channel: 1,8,16,32
        uint16_t colorMode;     // 0=Bitmap,1=Grayscale,3=RGB,4=CMYK
    };
    #pragma pack(pop)

    // Image Resource Block for thumbnail (resource ID 1036)
    static constexpr uint16_t THUMBNAIL_RESOURCE_ID = 1036;

    // Decoding
    HRESULT DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap);
    HRESULT ExtractCompositeImage(const uint8_t* data, size_t size,
                                  const PSDHeader& header, HBITMAP* phBitmap);
    HRESULT ExtractThumbnailResource(const uint8_t* data, size_t size,
                                     HBITMAP* phBitmap);

    // Format validation
    bool IsPSDFormat(const wchar_t* path);

    // Bitmap creation
    HBITMAP CreateBitmapFromRGB(const uint8_t* pixels, uint32_t width,
                                uint32_t height, uint16_t channels);

    // Utility
    uint16_t ReadBE16(const uint8_t* data);
    uint32_t ReadBE32(const uint8_t* data);
    std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* path, size_t& fileSize);

    // Extension list
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace DarkThumbs
