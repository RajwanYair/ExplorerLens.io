//==============================================================================
// ExampleDecoder.cpp - Reference Implementation Template
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "ExampleDecoder.h"
#include <fstream>
#include <cmath>
#include <cstdarg>

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// Static Member Initialization
//==============================================================================

// Extension list - MUST include leading dot and nullptr terminator
// Example for TGA format: { L".tga", L".tpic", nullptr }
const wchar_t* ExampleDecoder::m_extensions[] = { 
    L".example",  // Replace with your format's extension
    nullptr       // REQUIRED: null terminator
};

//==============================================================================
// Constructor
//==============================================================================

ExampleDecoder::ExampleDecoder() {
    // Initialize decoder resources here
    // - Load dependent libraries (e.g., LoadLibrary for codec DLLs)
    // - Allocate decoder state
    // - Initialize lookup tables
    
    LogDebug(L"ExampleDecoder initialized");
}

//==============================================================================
// IThumbnailDecoder Interface Implementation
//==============================================================================

bool ExampleDecoder::CanDecode(const wchar_t* filePath) {
    // PERFORMANCE CRITICAL: Must be < 1ms
    // This method is called for EVERY thumbnail request before trying decoders
    
    if (!filePath) {
        return false;
    }
    
    // Step 1: Check file extension (fastest check)
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) {
        return false; // No extension
    }
    
    // Case-insensitive extension comparison
    if (_wcsicmp(ext, L".example") != 0) {
        return false; // Not our format
    }
    
    // Step 2 (Optional): Validate magic bytes for extra confidence
    // Only do this if extension check passes and you need to distinguish
    // between formats with same extension (e.g., .img could be many formats)
    //
    // NOTE: Reading file here adds latency (~1-2ms). Only do if necessary!
    /*
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    uint8_t header[16] = {0};
    file.read((char*)header, sizeof(header));
    file.close();
    
    if (!HasValidSignature(header, sizeof(header))) {
        return false;
    }
    */
    
    return true;
}

HRESULT ExampleDecoder::Decode(
    const ThumbnailRequest& request, 
    ThumbnailResult& result) 
{
    // Initialize result structure
    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    result.status = E_FAIL;
    result.usedGPU = false;
    
    LogDebug(L"ExampleDecoder::Decode() - File: %s, Target: %ux%u", 
            request.filePath, request.width, request.height);
    
    //==========================================================================
    // Step 1: Validate Input
    //==========================================================================
    
    if (!request.filePath) {
        LogDebug(L"ExampleDecoder::Decode() - ERROR: filePath is null");
        result.status = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    if (request.width == 0 || request.height == 0) {
        LogDebug(L"ExampleDecoder::Decode() - ERROR: Invalid dimensions");
        result.status = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    //==========================================================================
    // Step 2: Read File Into Memory
    //==========================================================================
    
    std::vector<uint8_t> fileData;
    HRESULT hr = ReadFileToBuffer(request.filePath, fileData);
    if (FAILED(hr)) {
        LogDebug(L"ExampleDecoder::Decode() - ERROR: Failed to read file, HRESULT=0x%08X", hr);
        result.status = hr;
        return hr;
    }
    
    LogDebug(L"ExampleDecoder::Decode() - Read %zu bytes from file", fileData.size());
    
    //==========================================================================
    // Step 3: Decode Format-Specific Image Data
    //==========================================================================
    
    int imageWidth = 0, imageHeight = 0;
    std::vector<uint8_t> rgbaPixels = DecodeImageData(
        fileData.data(), 
        fileData.size(), 
        &imageWidth, 
        &imageHeight);
    
    if (rgbaPixels.empty() || imageWidth <= 0 || imageHeight <= 0) {
        LogDebug(L"ExampleDecoder::Decode() - ERROR: Failed to decode image data");
        result.status = E_FAIL;
        return E_FAIL;
    }
    
    LogDebug(L"ExampleDecoder::Decode() - Decoded %dx%d image (%zu bytes)", 
            imageWidth, imageHeight, rgbaPixels.size());
    
    //==========================================================================
    // Step 4: Calculate Target Dimensions (Aspect Ratio Preservation)
    //==========================================================================
    
    uint32_t targetWidth = request.width;
    uint32_t targetHeight = request.height;
    
    if (request.flags & ThumbnailFlags::PreserveAspect) {
        CalculateAspectDimensions(
            imageWidth, imageHeight,
            request.width, request.height,
            targetWidth, targetHeight);
        
        LogDebug(L"ExampleDecoder::Decode() - Aspect-preserving scale: %dx%d -> %ux%u",
                imageWidth, imageHeight, targetWidth, targetHeight);
    }
    
    //==========================================================================
    // Step 5: Scale Image to Target Size
    //==========================================================================
    
    std::vector<uint8_t> scaledPixels;
    
    if (imageWidth == (int)targetWidth && imageHeight == (int)targetHeight) {
        // No scaling needed - use original pixels
        scaledPixels = std::move(rgbaPixels);
        LogDebug(L"ExampleDecoder::Decode() - No scaling needed");
    }
    else {
        // Scale using bilinear interpolation
        scaledPixels = ScaleRGBA(
            rgbaPixels.data(), imageWidth, imageHeight,
            targetWidth, targetHeight);
        
        LogDebug(L"ExampleDecoder::Decode() - Scaled image to %ux%u", 
                targetWidth, targetHeight);
    }
    
    if (scaledPixels.empty()) {
        LogDebug(L"ExampleDecoder::Decode() - ERROR: Scaling failed");
        result.status = E_FAIL;
        return E_FAIL;
    }
    
    //==========================================================================
    // Step 6: Convert RGBA to BGRA (Windows Bitmap Format)
    //==========================================================================
    
    size_t pixelCount = (size_t)targetWidth * targetHeight;
    ConvertRGBAtoBGRA(scaledPixels.data(), pixelCount);
    
    //==========================================================================
    // Step 7: Create HBITMAP
    //==========================================================================
    
    HBITMAP hBitmap = CreateBitmapFromBGRA(
        scaledPixels.data(), 
        targetWidth, 
        targetHeight);
    
    if (!hBitmap) {
        LogDebug(L"ExampleDecoder::Decode() - ERROR: Failed to create HBITMAP");
        result.status = E_OUTOFMEMORY;
        return E_OUTOFMEMORY;
    }
    
    //==========================================================================
    // Step 8: Fill Result Structure
    //==========================================================================
    
    result.hBitmap = hBitmap;
    result.width = targetWidth;
    result.height = targetHeight;
    result.status = S_OK;
    result.usedGPU = false; // Set to true if you used GPU acceleration
    
    LogDebug(L"ExampleDecoder::Decode() - SUCCESS: Created %ux%u thumbnail", 
            targetWidth, targetHeight);
    
    return S_OK;
}

DecoderInfo ExampleDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"Example Decoder";
    info.version = L"1.0.0";
    info.description = L"Example decoder implementation template";
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

//==============================================================================
// Format-Specific Implementation (REPLACE THIS WITH YOUR DECODER)
//==============================================================================

bool ExampleDecoder::HasValidSignature(const uint8_t* data, size_t size) const {
    // Example: Check for format-specific magic bytes
    // TGA format doesn't have a strong signature, but many formats do
    
    if (!data || size < 4) {
        return false;
    }
    
    // Example signatures:
    // PNG:  0x89 0x50 0x4E 0x47
    // JPEG: 0xFF 0xD8 0xFF
    // GIF:  "GIF8"
    // BMP:  "BM"
    
    // Replace this with your format's signature check
    // For this example, we'll accept anything (not recommended for real decoder)
    return true;
}

std::vector<uint8_t> ExampleDecoder::DecodeImageData(
    const uint8_t* fileData, 
    size_t fileSize,
    int* outWidth, 
    int* outHeight) const 
{
    //==========================================================================
    // REPLACE THIS ENTIRE FUNCTION WITH YOUR DECODER LOGIC
    //==========================================================================
    
    // This is where you implement the format-specific decoding
    // Examples:
    // - Call libpng functions for PNG
    // - Call libjpeg functions for JPEG
    // - Call libwebp functions for WebP
    // - Implement custom parser for proprietary formats
    
    // For demonstration, we'll create a simple test pattern
    // (Remove this and implement real decoding)
    
    *outWidth = 512;
    *outHeight = 512;
    
    std::vector<uint8_t> pixels(*outWidth * *outHeight * 4);
    
    // Create gradient test pattern
    for (int y = 0; y < *outHeight; y++) {
        for (int x = 0; x < *outWidth; x++) {
            size_t idx = (y * *outWidth + x) * 4;
            pixels[idx + 0] = (x * 255) / *outWidth;      // R
            pixels[idx + 1] = (y * 255) / *outHeight;     // G
            pixels[idx + 2] = 128;                        // B
            pixels[idx + 3] = 255;                        // A (opaque)
        }
    }
    
    return pixels;
    
    //==========================================================================
    // REAL IMPLEMENTATION EXAMPLE (pseudo-code for reference):
    //==========================================================================
    /*
    // Parse header to get dimensions
    if (fileSize < 18) return {};  // Too small for TGA header
    
    *outWidth = fileData[12] | (fileData[13] << 8);
    *outHeight = fileData[14] | (fileData[15] << 8);
    int bpp = fileData[16];
    
    if (bpp != 24 && bpp != 32) return {};  // Unsupported bit depth
    
    // Decode pixel data
    size_t headerSize = 18;
    size_t bytesPerPixel = bpp / 8;
    size_t expectedSize = *outWidth * *outHeight * bytesPerPixel;
    
    if (fileSize < headerSize + expectedSize) return {};
    
    std::vector<uint8_t> rgba(*outWidth * *outHeight * 4);
    const uint8_t* srcPixels = fileData + headerSize;
    
    for (int i = 0; i < *outWidth * *outHeight; i++) {
        rgba[i*4 + 0] = srcPixels[i*bytesPerPixel + 2];  // R
        rgba[i*4 + 1] = srcPixels[i*bytesPerPixel + 1];  // G
        rgba[i*4 + 2] = srcPixels[i*bytesPerPixel + 0];  // B
        rgba[i*4 + 3] = (bpp == 32) ? srcPixels[i*bytesPerPixel + 3] : 255;  // A
    }
    
    return rgba;
    */
}

//==============================================================================
// Helper Methods (Generic - reusable across decoders)
//==============================================================================

HRESULT ExampleDecoder::ReadFileToBuffer(
    const wchar_t* filePath, 
    std::vector<uint8_t>& buffer) const 
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    
    std::streamsize size = file.tellg();
    if (size <= 0) {
        return E_FAIL;
    }
    
    file.seekg(0, std::ios::beg);
    buffer.resize(size);
    
    if (!file.read((char*)buffer.data(), size)) {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }
    
    return S_OK;
}

std::vector<uint8_t> ExampleDecoder::ScaleRGBA(
    const uint8_t* srcPixels, 
    int srcWidth, 
    int srcHeight,
    int dstWidth, 
    int dstHeight) const 
{
    // Simple bilinear interpolation
    // For production, consider using stb_image_resize or similar library
    
    std::vector<uint8_t> dst(dstWidth * dstHeight * 4);
    
    float xRatio = (float)srcWidth / dstWidth;
    float yRatio = (float)srcHeight / dstHeight;
    
    for (int y = 0; y < dstHeight; y++) {
        for (int x = 0; x < dstWidth; x++) {
            float srcX = x * xRatio;
            float srcY = y * yRatio;
            
            int x1 = (int)srcX;
            int y1 = (int)srcY;
            int x2 = std::min(x1 + 1, srcWidth - 1);
            int y2 = std::min(y1 + 1, srcHeight - 1);
            
            float xFrac = srcX - x1;
            float yFrac = srcY - y1;
            
            for (int c = 0; c < 4; c++) {
                float p1 = srcPixels[(y1 * srcWidth + x1) * 4 + c];
                float p2 = srcPixels[(y1 * srcWidth + x2) * 4 + c];
                float p3 = srcPixels[(y2 * srcWidth + x1) * 4 + c];
                float p4 = srcPixels[(y2 * srcWidth + x2) * 4 + c];
                
                float top = p1 * (1 - xFrac) + p2 * xFrac;
                float bottom = p3 * (1 - xFrac) + p4 * xFrac;
                float value = top * (1 - yFrac) + bottom * yFrac;
                
                dst[(y * dstWidth + x) * 4 + c] = (uint8_t)value;
            }
        }
    }
    
    return dst;
}

void ExampleDecoder::ConvertRGBAtoBGRA(uint8_t* pixels, size_t pixelCount) const {
    // Swap R and B channels (RGBA → BGRA)
    for (size_t i = 0; i < pixelCount; i++) {
        uint8_t temp = pixels[i * 4 + 0];      // R
        pixels[i * 4 + 0] = pixels[i * 4 + 2]; // B → R
        pixels[i * 4 + 2] = temp;              // R → B
        // G and A stay in place
    }
}

HBITMAP ExampleDecoder::CreateBitmapFromBGRA(
    const uint8_t* pixels, 
    uint32_t width, 
    uint32_t height) const 
{
    // Create 32-bit BGRA DIB section (Windows native format)
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(int)height;  // Negative = top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = width * height * 4;
    
    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(
        nullptr,            // Device context (null = screen DC)
        &bmi,               // Bitmap info
        DIB_RGB_COLORS,     // Color usage
        &pBits,             // Output: pointer to bitmap bits
        nullptr,            // File mapping object (null = not file-backed)
        0                   // Offset into file mapping
    );
    
    if (!hBitmap) {
        return nullptr;
    }
    
    // Copy pixel data into bitmap
    memcpy(pBits, pixels, width * height * 4);
    
    return hBitmap;
}

void ExampleDecoder::CalculateAspectDimensions(
    int sourceWidth, 
    int sourceHeight,
    uint32_t maxWidth, 
    uint32_t maxHeight,
    uint32_t& outWidth, 
    uint32_t& outHeight) const 
{
    float aspectRatio = (float)sourceWidth / sourceHeight;
    
    // Start with max width
    outWidth = maxWidth;
    outHeight = (uint32_t)(maxWidth / aspectRatio);
    
    // If height exceeds max, scale by height instead
    if (outHeight > maxHeight) {
        outHeight = maxHeight;
        outWidth = (uint32_t)(maxHeight * aspectRatio);
    }
    
    // Ensure non-zero dimensions
    if (outWidth == 0) outWidth = 1;
    if (outHeight == 0) outHeight = 1;
}

void ExampleDecoder::LogDebug(const wchar_t* format, ...) const {
    // Format and output debug message
    wchar_t buffer[1024];
    
    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, format, args);
    va_end(args);
    
    // Output to Windows debug console (visible in DebugView or Visual Studio)
    OutputDebugStringW(L"[ExampleDecoder] ");
    OutputDebugStringW(buffer);
    OutputDebugStringW(L"\n");
}

} // namespace Engine
} // namespace ExplorerLens

