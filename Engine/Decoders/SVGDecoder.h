// SVGDecoder.h - SVG/SVGZ Vector Image Decoder
// ExplorerLens Engine v6.2.0+
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: SVG (.svg), SVGZ (.svgz - gzip-compressed SVG)
// Features:
// - Renders SVG to bitmap using GDI+ for basic shapes/text
// - SVGZ decompression via zlib inflate
// - Fallback placeholder rendering for complex SVGs
// - No external library dependency (uses Windows built-in APIs)
// - Thread-safe, no global state

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class SVGDecoder : public IThumbnailDecoder {
public:
 SVGDecoder();
 ~SVGDecoder() override;

 // IThumbnailDecoder interface
 bool CanDecode(const wchar_t* filePath) override;
 HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
 DecoderInfo GetInfo() const override;
 const wchar_t* GetName() const override { return L"SVGDecoder"; }
 const wchar_t** GetSupportedExtensions() const override;
 uint32_t GetExtensionCount() const override { return m_extensionCount; }
 bool SupportsGPU() const override { return false; }
 bool IsArchiveDecoder() const override { return false; }

private:
 // SVG rendering
 HRESULT RenderSVGToHBITMAP(const wchar_t* filePath, uint32_t width,
 uint32_t height, HBITMAP* phBitmap);
 
 // SVGZ decompression
 bool DecompressSVGZ(const std::vector<uint8_t>& compressed, std::string& svgContent);
 
 // Read file into buffer
 std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* path, size_t& fileSize);

 // Quick SVG dimension extraction from viewBox/width/height attrs
 bool ExtractSVGDimensions(const std::string& svgContent, 
 uint32_t& outWidth, uint32_t& outHeight);
 
 // Render placeholder with SVG icon representation
 HBITMAP CreateSVGPlaceholder(uint32_t width, uint32_t height,
 const std::string& svgContent);
 
 // Format detection
 bool IsSVGFormat(const wchar_t* path);
 bool IsSVGZFormat(const wchar_t* path);
 
 // Extension list
 static const wchar_t* m_extensions[];
 static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens

