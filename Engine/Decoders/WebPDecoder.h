// WebPDecoder.h - libwebp Decoder for WebP Format
// Copyright (c) 2025 ExplorerLens Project
//
// Supports: WebP (lossy and lossless) using libwebp 1.5.0
// Features:
// - Fast dimension extraction without full decode
// - RGBA to BGRA conversion for Windows
// - Both VP8 (lossy) and VP8L (lossless) formats
// - Animation support (first frame only)
// - Alpha channel preservation

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <mutex>

namespace ExplorerLens {
namespace Engine {

class WebPDecoder : public IThumbnailDecoder {
public:
 WebPDecoder();
 ~WebPDecoder() override = default;

 // IThumbnailDecoder interface
 bool CanDecode(const wchar_t* filePath) override;
 HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
 DecoderInfo GetInfo() const override;
 const wchar_t* GetName() const override { return L"WebPDecoder"; }
 const wchar_t** GetSupportedExtensions() const override { return m_extensions; }
 uint32_t GetExtensionCount() const override { return m_extensionCount; }
 bool SupportsGPU() const override { return false; } // CPU-based libwebp
 bool IsArchiveDecoder() const override { return false; }

private:
 // Decode helpers
 HRESULT DecodeFromFile(const wchar_t* path, UINT targetWidth,
 UINT targetHeight, HBITMAP* phBitmap);
 HRESULT DecodeFromMemory(const BYTE* data, size_t size, UINT targetWidth,
 UINT targetHeight, HBITMAP* phBitmap);
 
 // Check if data is WebP format (RIFF...WEBP signature)
 bool IsWebPFormat(const BYTE* data, size_t size);
 
 // Check if WebP is animated (has ANIM chunk or VP8X animation flag)
 bool IsAnimatedWebP(const BYTE* data, size_t size);
 
 // Add animation badge overlay to bitmap
 void AddAnimationBadge(HBITMAP hBitmap, int width, int height);
 
 // Convert RGBA to Windows BGRA DIB section
 HBITMAP CreateBitmapFromRGBA(const BYTE* rgba, int width, int height);
 
 // Extension list (must be static for lifetime guarantee)
 static const wchar_t* m_extensions[];
 static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens

