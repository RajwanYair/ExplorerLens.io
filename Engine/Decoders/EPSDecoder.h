//==============================================================================
// EPS Decoder - Encapsulated PostScript Thumbnail Provider
// Vector Format Expansion
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>
#include <memory>

namespace ExplorerLens {
namespace Engine {

    /// <summary>
    /// Decoder for Encapsulated PostScript (EPS) and PostScript (PS) files.
    /// 
    /// Strategy:
    /// 1. Try to extract embedded TIFF/WMF preview from EPS binary header
    /// 2. Fall back to generating a text-based placeholder showing file info
    /// 
    /// EPS files often contain an embedded preview image in their binary header:
    ///   - DOS EPS Binary Header magic: 0xC5D0D3C6
    ///   - Contains offsets to TIFF and/or WMF preview images
    ///
    /// Supported: .eps, .epsf, .ps
    /// LENSTYPE: LENSTYPE_EPS (87)
    /// </summary>
    class EPSDecoder : public IThumbnailDecoder
    {
    public:
        EPSDecoder();
        ~EPSDecoder() override;

        // IThumbnailDecoder interface
        bool CanDecode(const wchar_t* filePath) override;
        HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
        const wchar_t** GetSupportedExtensions() const override;
        DecoderInfo GetInfo() const override;
        const wchar_t* GetName() const override;
        uint32_t GetExtensionCount() const override;
        bool SupportsGPU() const override;
        bool IsArchiveDecoder() const override;

    private:
        // EPS Binary Header (DOS EPS format)
        struct EPSBinaryHeader {
            uint32_t magic;         // 0xC5D0D3C6
            uint32_t psOffset;      // Offset to PostScript
            uint32_t psLength;      // Length of PostScript
            uint32_t wmfOffset;     // Offset to WMF preview
            uint32_t wmfLength;     // Length of WMF preview
            uint32_t tiffOffset;    // Offset to TIFF preview
            uint32_t tiffLength;    // Length of TIFF preview
            uint16_t checksum;      // Checksum (optional)
        };

        // Extract embedded TIFF preview from EPS binary header
        HBITMAP ExtractTIFFPreview(const uint8_t* data, size_t size,
                                   uint32_t tiffOffset, uint32_t tiffLength,
                                   uint32_t width, uint32_t height);

        // Extract embedded WMF preview from EPS binary header
        HBITMAP ExtractWMFPreview(const uint8_t* data, size_t size,
                                  uint32_t wmfOffset, uint32_t wmfLength,
                                  uint32_t width, uint32_t height);

        // Parse BoundingBox from PostScript text
        bool ParseBoundingBox(const char* psText, size_t length,
                              float& llx, float& lly, float& urx, float& ury);

        // Create placeholder thumbnail showing file info
        HBITMAP CreateEPSPlaceholder(uint32_t width, uint32_t height,
                                     const wchar_t* filePath,
                                     float bbWidth, float bbHeight);

        // Read file into memory
        std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* filePath, size_t& outSize);

        static const wchar_t* s_extensions[];
        static const uint32_t s_extensionCount;
    };

} // namespace Engine
} // namespace ExplorerLens

