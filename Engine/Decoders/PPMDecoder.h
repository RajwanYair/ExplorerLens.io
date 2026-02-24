// PPMDecoder.h - Netpbm Format Decoder
// ExplorerLens Engine v6.1.0+
// Supports: PPM (P3/P6), PGM (P2/P5), PBM (P1/P4), PNM, PAM, PFM

#pragma once
#include "../Core/IThumbnailDecoder.h"
#include <memory>

namespace ExplorerLens {
namespace Engine {

class PPMDecoder : public IThumbnailDecoder {
public:
    PPMDecoder();
    ~PPMDecoder();

    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"PPMDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

private:
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;

    HRESULT DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap);
    HRESULT DecodePPM(const uint8_t* data, size_t size, HBITMAP* phBitmap); // P3/P6
    HRESULT DecodePGM(const uint8_t* data, size_t size, HBITMAP* phBitmap); // P2/P5
    HRESULT DecodePBM(const uint8_t* data, size_t size, HBITMAP* phBitmap); // P1/P4
    HRESULT DecodePFM(const uint8_t* data, size_t size, HBITMAP* phBitmap); // PFM float

    void SkipWhitespaceAndComments(const char*& p, const char* end);
    int ReadASCIIInt(const char*& p, const char* end);
    HBITMAP CreateBitmap32(uint32_t width, uint32_t height, const uint8_t* bgra);
    std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* path, size_t& fileSize);
};

} // namespace Engine
} // namespace ExplorerLens

