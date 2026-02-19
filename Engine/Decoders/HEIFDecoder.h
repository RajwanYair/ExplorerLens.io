// HEIFDecoder.h - HEIF/HEIC format decoder
// Part of DarkThumbs Engine v5.3.0+
// High Efficiency Image Format - Apple's default photo format

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <string>
#include <memory>

namespace DarkThumbs {
namespace Engine {

    /// <summary>
    /// Decoder for HEIF/HEIC (.heif, .heic) images
    /// 
    /// HEIF (High Efficiency Image Format) is a container format based on HEVC:
    /// - Used by Apple (iPhone photos since iOS 11)
    /// - Superior compression vs JPEG (50% smaller at same quality)
    /// - Supports HDR, 16-bit depth, wide color
    /// - Can store multiple images (bursts, depth maps)
    /// - Supports alpha channel and animations
    /// </summary>
    class HEIFDecoder : public IThumbnailDecoder {
    public:
        HEIFDecoder();
        virtual ~HEIFDecoder();

        // IThumbnailDecoder implementation
        bool CanDecode(const wchar_t* filePath) override;
        HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
        DecoderInfo GetInfo() const override;
        const wchar_t* GetName() const override { return L"HEIFDecoder"; }
        const wchar_t** GetSupportedExtensions() const override;
        uint32_t GetExtensionCount() const override;
        bool SupportsGPU() const override { return false; }
        bool IsArchiveDecoder() const override { return false; }

    private:
        /// <summary>
        /// Verifies HEIF file signature
        /// HEIF signature: "ftyp" box with brands: heic, heix, hevc, heim, heis, mif1
        /// </summary>
        bool VerifyHEIFSignature(const uint8_t* data, size_t size) const;

        /// <summary>
        /// Decode HEIF image to RGB(A) bitmap
        /// </summary>
        /// <param name="fileData">Raw HEIF file data</param>
        /// <param name="dataSize">Size of file data in bytes</param>
        /// <param name="targetWidth">Desired thumbnail width</param>
        /// <param name="targetHeight">Desired thumbnail height</param>
        /// <param name="outWidth">Output: actual decoded width</param>
        /// <param name="outHeight">Output: actual decoded height</param>
        /// <param name="outChannels">Output: number of channels (3=RGB, 4=RGBA)</param>
        /// <returns>Decoded RGBA pixel data (caller must free with delete[])</returns>
        uint8_t* DecodeHEIFImage(
            const uint8_t* fileData,
            size_t dataSize,
            uint32_t targetWidth,
            uint32_t targetHeight,
            uint32_t& outWidth,
            uint32_t& outHeight,
            uint32_t& outChannels
        );

        /// <summary>
        /// Extract embedded thumbnail from HEIF (fast path)
        /// Many HEIF files contain pre-generated thumbnails
        /// </summary>
        uint8_t* ExtractEmbeddedThumbnail(
            const uint8_t* fileData,
            size_t dataSize,
            uint32_t& outWidth,
            uint32_t& outHeight,
            uint32_t& outChannels
        );

        /// <summary>
        /// Read entire file into memory
        /// </summary>
        std::unique_ptr<uint8_t[]> ReadFileData(const std::wstring& filePath, size_t& outSize);

        /// <summary>
        /// Create HBITMAP from RGBA pixel data
        /// </summary>
        HBITMAP CreateHBITMAPFromRGBA(
            const uint8_t* pixels,
            uint32_t width,
            uint32_t height,
            uint32_t channels
        );

        // Configuration
    [[maybe_unused]] bool m_preferEmbeddedThumbnail;  // Use embedded thumbnail if available
        
        // Supported extensions (HEIF, HEIC, Apple variants)
        // Sprint 175: Fixed overlap — HEIFDecoder handles HEIF container formats (HEVC codec)
        // AVIF (.avif/.avifs) is handled by AVIFDecoder (AV1 codec)
        static constexpr const wchar_t* s_extensions[10] = {
            L".heif", L".heic", L".hif", L".heifs", L".heics",
            L".avci", L".avcs", L".heif-sequence", L".heic-sequence", nullptr
        };
    };

} // namespace Engine
} // namespace DarkThumbs
