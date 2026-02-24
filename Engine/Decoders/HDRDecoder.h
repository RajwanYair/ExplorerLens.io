// HDRDecoder.h - Radiance RGBE HDR Image Decoder
// ExplorerLens Engine v6.1.0+
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: Radiance HDR (.hdr) - High Dynamic Range images
// Features:
// - RGBE format decoding
// - Run-length encoded (RLE) scanline decompression
// - HDR-to-SDR tone mapping (Reinhard operator)
// - Gamma correction for display
// - No external dependencies (custom implementation)

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>
#include <memory>

namespace ExplorerLens {
namespace Engine {

class HDRDecoder : public IThumbnailDecoder {
public:
    HDRDecoder();
    ~HDRDecoder() override;

    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"HDRDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

private:
    struct RGBE { uint8_t r, g, b, e; };

    HRESULT DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap);
    HRESULT ParseHDR(const uint8_t* data, size_t size,
                     float** ppPixels, uint32_t* pWidth, uint32_t* pHeight);
    HRESULT DecodeScanlineRLE(const uint8_t*& p, const uint8_t* end,
                              RGBE* scanline, uint32_t width);
    void RGBEToFloat(const RGBE& rgbe, float& r, float& g, float& b);
    void ToneMapReinhard(float* pixels, uint32_t pixelCount);
    void ToneMapReinhard_SSE(float* pixels, uint32_t pixelCount);
    HBITMAP CreateBitmapFromFloat(const float* pixels, uint32_t width, uint32_t height);
    bool IsHDRFormat(const wchar_t* path);
    std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* path, size_t& fileSize);

    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens

