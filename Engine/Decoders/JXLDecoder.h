// JXLDecoder.h - JPEG XL (JXL) format decoder
// Part of DarkThumbs Engine v5.3.0+
// Modern high-efficiency image format with superior compression

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <string>
#include <memory>

namespace DarkThumbs {
namespace Engine {

    /// <summary>
    /// Decoder for JPEG XL (.jxl) images
    /// 
    /// JPEG XL is a modern royalty-free image format that provides:
    /// - Superior compression (30-60% smaller than JPEG at same quality)
    /// - Lossless and lossy compression
    /// - HDR and wide color gamut support
    /// - Progressive decoding
    /// - Animation support
    /// - Alpha channel support
    /// </summary>
    class JXLDecoder : public IThumbnailDecoder {
    public:
        JXLDecoder();
        virtual ~JXLDecoder();

        // IThumbnailDecoder implementation
        bool CanDecode(const wchar_t* filePath) override;
        HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
        DecoderInfo GetInfo() const override;
        const wchar_t* GetName() const override { return L"JXLDecoder"; }
        const wchar_t** GetSupportedExtensions() const override;
        uint32_t GetExtensionCount() const override { return 1; }
        bool SupportsGPU() const override { return false; }
        bool IsArchiveDecoder() const override { return false; }

    private:
        /// <summary>
        /// Verifies JXL file signature
        /// JXL signature: 0xFF 0x0A (for codestream) or "JXL " for container
        /// </summary>
        bool VerifyJXLSignature(const uint8_t* data, size_t size) const;

        /// <summary>
        /// Decode JXL image to RGB(A) bitmap
        /// </summary>
        /// <param name="fileData">Raw JXL file data</param>
        /// <param name="dataSize">Size of file data in bytes</param>
        /// <param name="targetWidth">Desired thumbnail width</param>
        /// <param name="targetHeight">Desired thumbnail height</param>
        /// <param name="outWidth">Output: actual decoded width</param>
        /// <param name="outHeight">Output: actual decoded height</param>
        /// <param name="outChannels">Output: number of channels (3=RGB, 4=RGBA)</param>
        /// <returns>Decoded RGBA pixel data (caller must free with delete[])</returns>
        uint8_t* DecodeJXLImage(
            const uint8_t* fileData,
            size_t dataSize,
            uint32_t targetWidth,
            uint32_t targetHeight,
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
        bool m_useMultithreading;  // Enable parallel decoding
        uint32_t m_maxThreads;     // Maximum decode threads
        
        // Supported extensions
        static constexpr const wchar_t* s_extensions[2] = { L".jxl", nullptr };
    };

} // namespace Engine
} // namespace DarkThumbs
