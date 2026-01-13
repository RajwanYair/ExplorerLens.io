//==============================================================================
// ExampleDecoder.h - Reference Implementation Template
// DarkThumbs Engine v1.0.0
// Copyright (c) 2026 - DarkThumbs Project
//
// This is a complete reference implementation showing how to create a custom
// thumbnail decoder plugin for the DarkThumbs Engine.
//
// USAGE:
// 1. Copy this file and ExampleDecoder.cpp to your project
// 2. Rename class and files to match your format (e.g., TGADecoder)
// 3. Implement the format-specific decoding logic in DecodeImageData()
// 4. Update extension list and metadata
// 5. Register decoder in DecoderRegistry.cpp
//
// See docs/PLUGIN_API.md for complete API documentation.
//==============================================================================

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <vector>
#include <mutex>

namespace DarkThumbs {
namespace Engine {

//==============================================================================
/// Example decoder demonstrating best practices for implementing IThumbnailDecoder
/// 
/// This decoder shows:
/// - Thread-safe implementation with proper synchronization
/// - Efficient format detection (extension + magic bytes)
/// - Proper error handling with HRESULT codes
/// - Memory management and resource cleanup
/// - Aspect ratio preservation
/// - RGBA to BGRA conversion for Windows
/// - Diagnostic logging for debugging
/// 
/// Replace the format-specific logic with your own decoder implementation.
//==============================================================================
class ExampleDecoder : public IThumbnailDecoder {
public:
    ExampleDecoder();
    ~ExampleDecoder() override = default;

    //==========================================================================
    // IThumbnailDecoder Interface Implementation
    //==========================================================================
    
    /// Fast format check (< 1ms required)
    bool CanDecode(const wchar_t* filePath) override;
    
    /// Decode file and generate thumbnail
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    
    /// Get decoder metadata
    DecoderInfo GetInfo() const override;
    
    /// Get decoder display name
    const wchar_t* GetName() const override { return L"ExampleDecoder"; }
    
    /// Get supported file extensions (null-terminated array)
    const wchar_t** GetSupportedExtensions() const override { return m_extensions; }
    
    /// Get extension count (not including nullptr terminator)
    uint32_t GetExtensionCount() const override { return 1; }
    
    /// Check if GPU acceleration is supported
    bool SupportsGPU() const override { return false; } // Change to true if GPU-accelerated
    
    /// Check if this handles archive formats
    bool IsArchiveDecoder() const override { return false; }

private:
    //==========================================================================
    // Format-Specific Implementation (Replace with your decoder logic)
    //==========================================================================
    
    /// Check if file has valid format signature (magic bytes)
    /// Example: TGA files start with specific header bytes
    bool HasValidSignature(const uint8_t* data, size_t size) const;
    
    /// Decode raw image data into RGBA pixels
    /// @param fileData Raw file bytes
    /// @param fileSize File size in bytes
    /// @param outWidth Output: decoded image width
    /// @param outHeight Output: decoded image height
    /// @return RGBA pixel data (width * height * 4 bytes), or empty on error
    std::vector<uint8_t> DecodeImageData(
        const uint8_t* fileData, 
        size_t fileSize,
        int* outWidth, 
        int* outHeight) const;
    
    //==========================================================================
    // Helper Methods (Generic - reusable across decoders)
    //==========================================================================
    
    /// Read entire file into memory buffer
    HRESULT ReadFileToBuffer(const wchar_t* filePath, std::vector<uint8_t>& buffer) const;
    
    /// Scale RGBA image using bilinear interpolation
    std::vector<uint8_t> ScaleRGBA(
        const uint8_t* srcPixels, 
        int srcWidth, 
        int srcHeight,
        int dstWidth, 
        int dstHeight) const;
    
    /// Convert RGBA pixels to BGRA (Windows bitmap format)
    void ConvertRGBAtoBGRA(uint8_t* pixels, size_t pixelCount) const;
    
    /// Create Windows HBITMAP from BGRA pixel data
    HBITMAP CreateBitmapFromBGRA(
        const uint8_t* pixels, 
        uint32_t width, 
        uint32_t height) const;
    
    /// Calculate aspect-preserving dimensions
    void CalculateAspectDimensions(
        int sourceWidth, 
        int sourceHeight,
        uint32_t maxWidth, 
        uint32_t maxHeight,
        uint32_t& outWidth, 
        uint32_t& outHeight) const;
    
    /// Log diagnostic message to debug output
    void LogDebug(const wchar_t* format, ...) const;
    
    //==========================================================================
    // Member Data
    //==========================================================================
    
    /// Supported file extensions (must be static for lifetime guarantee)
    static const wchar_t* m_extensions[];
    
    /// Mutex for thread-safe operations (if needed for shared state)
    /// Note: If decoder is stateless, mutex may not be needed
    mutable std::mutex m_mutex;
};

} // namespace Engine
} // namespace DarkThumbs
