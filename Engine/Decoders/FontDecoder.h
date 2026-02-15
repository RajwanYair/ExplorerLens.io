// FontDecoder.h - Font Preview Thumbnail Decoder
// DarkThumbs Engine v6.2.0+
// Copyright (c) 2026 DarkThumbs Project
//
// Supports: TTF, OTF, WOFF, WOFF2, TTC, FON
// Features:
// - Renders sample text using DirectWrite
// - Shows font name and style in preview
// - Falls back to Shell thumbnail provider

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>

namespace DarkThumbs {
namespace Engine {

class FontDecoder : public IThumbnailDecoder {
public:
    FontDecoder();
    ~FontDecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"FontDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

private:
    // Shell-based font thumbnail
    HRESULT ExtractFontPreviewShell(const wchar_t* filePath, uint32_t width,
                                     uint32_t height, HBITMAP* phBitmap);

    // Placeholder with font-style rendering
    HBITMAP CreateFontPlaceholder(uint32_t width, uint32_t height,
                                  const wchar_t* filePath);

    bool IsFontFormat(const wchar_t* path);

    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace DarkThumbs
