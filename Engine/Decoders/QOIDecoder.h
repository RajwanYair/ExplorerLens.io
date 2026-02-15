// QOIDecoder.h - Quite OK Image Format Decoder
// DarkThumbs Engine v5.3.0+
// Sprint 15 - Modern Format Support
//
// Supports: QOI - The "Quite OK Image Format"
// Features:
// - Lossless compression
// - Extremely fast decode (simpler than PNG)
// - RGB and RGBA support
// - No external dependencies (reference implementation)
// - Typical compression: 50-70% file size vs PNG

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>
#include <memory>

namespace DarkThumbs {
namespace Engine {

class QOIDecoder : public IThumbnailDecoder {
public:
    QOIDecoder();
    ~QOIDecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"QOIDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; } // CPU decode
    bool IsArchiveDecoder() const override { return false; }

private:
    // QOI header (14 bytes)
    #pragma pack(push, 1)
    struct QOIHeader {
        char     magic[4];      // "qoif"
        uint32_t width;         // Big-endian
        uint32_t height;        // Big-endian
        uint8_t  channels;      // 3 = RGB, 4 = RGBA
        uint8_t  colorspace;    // 0 = sRGB, 1 = linear
    };
    #pragma pack(pop)

    // QOI opcodes
    enum QOIOpcode {
        QOI_OP_RGB   = 0xFE,
        QOI_OP_RGBA  = 0xFF,
        QOI_OP_INDEX = 0x00,  // bits: 00xxxxxx
        QOI_OP_DIFF  = 0x40,  // bits: 01xxxxxx
        QOI_OP_LUMA  = 0x80,  // bits: 10xxxxxx
        QOI_OP_RUN   = 0xC0   // bits: 11xxxxxx
    };

    // Decoding
    HRESULT DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap);
    HRESULT DecodeQOI(const uint8_t* data, size_t size, uint8_t** ppPixels,
                      uint32_t* pWidth, uint32_t* pHeight, uint8_t* pChannels);
    
    // Format validation
    bool IsQOIFormat(const wchar_t* path);
    bool IsQOIFormat(const uint8_t* data, size_t size);
    
    // Bitmap creation
    HBITMAP CreateBitmapFromRGBA(const uint8_t* pixels, uint32_t width,
                                 uint32_t height, uint8_t channels);
    
    // Utility
    uint32_t ReadBE32(const uint8_t* data);
    std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* path, size_t& fileSize);
    
    // Extension list
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace DarkThumbs
