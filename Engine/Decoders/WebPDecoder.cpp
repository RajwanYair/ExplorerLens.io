// WebPDecoder.cpp - libwebp Decoder Implementation
// ExplorerLens Engine v1.0.0
// Copyright (c) 2025 ExplorerLens Project

#include "WebPDecoder.h"
#include "../Utils/PerformanceProfiler.h"
#include <shlwapi.h>
#include <fstream>
#include <vector>

// libwebp headers
#include <webp/decode.h>

#pragma comment(lib, "shlwapi.lib")
// Note: libwebp.lib will be added to CMakeLists.txt

namespace ExplorerLens {
namespace Engine {

// Extension list (null-terminated)
const wchar_t* WebPDecoder::m_extensions[] = {
    L".webp",
    nullptr  // Null terminator
};

const uint32_t WebPDecoder::m_extensionCount = 1;

//=============================================================================
// Constructor
//=============================================================================

WebPDecoder::WebPDecoder() {
    // Nothing to initialize
}

//=============================================================================
// IThumbnailDecoder Implementation
//=============================================================================

bool WebPDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath || !*filePath) {
        return false;
    }

    // Check extension
    const wchar_t* ext = PathFindExtensionW(filePath);
    if (!ext || !*ext) {
        return false;
    }

    return _wcsicmp(ext, L".webp") == 0;
}

HRESULT WebPDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    PROFILE_SCOPE(ProfileComponent::DECODE_WEBP);
    
    if (!CanDecode(request.filePath)) {
        return E_INVALIDARG;
    }

    // Decode the image
    HBITMAP hBitmap = nullptr;
    HRESULT hr = DecodeFromFile(request.filePath, request.width,
                                 request.height, &hBitmap);

    if (FAILED(hr)) {
        result.status = hr;
        return hr;
    }

    // Success
    result.hBitmap = hBitmap;
    result.width = request.width;
    result.height = request.height;
    result.status = S_OK;

    return S_OK;
}

DecoderInfo WebPDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"WebPDecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

//=============================================================================
// Decoding Implementation
//=============================================================================

HRESULT WebPDecoder::DecodeFromFile(const wchar_t* path, UINT targetWidth,
                                    UINT targetHeight, HBITMAP* phBitmap) {
    if (!path || !*path || !phBitmap) {
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    // Read file into memory
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    std::streamsize fileSize = file.tellg();
    if (fileSize <= 0 || fileSize > 100 * 1024 * 1024) { // 100MB limit
        return E_FAIL;
    }

    file.seekg(0, std::ios::beg);
    std::vector<BYTE> buffer(static_cast<size_t>(fileSize));
    
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        return E_FAIL;
    }

    return DecodeFromMemory(buffer.data(), buffer.size(), targetWidth, targetHeight, phBitmap);
}

HRESULT WebPDecoder::DecodeFromMemory(const BYTE* data, size_t size, UINT targetWidth,
                                      UINT targetHeight, HBITMAP* phBitmap) {
    if (!data || size == 0 || !phBitmap) {
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    // Verify WebP format
    if (!IsWebPFormat(data, size)) {
        return E_FAIL;
    }

    // Get original dimensions
    int originalWidth = 0, originalHeight = 0;
    if (!WebPGetInfo(data, size, &originalWidth, &originalHeight)) {
        return E_FAIL;
    }

    //Use WebPDecoderConfig for native scaling support (performance optimization)
    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config)) {
        return E_FAIL;
    }

    // Enable native scaling to target size
    config.options.use_scaling = 1;
    config.options.scaled_width = targetWidth;
    config.options.scaled_height = targetHeight;
    
    // Enable multi-threaded decoding for better performance
    config.options.use_threads = 1;
    
    // Output format: RGBA for easier GDI+ integration
    config.output.colorspace = MODE_RGBA;

    // Decode with native scaling
    VP8StatusCode status = WebPDecode(data, size, &config);
    if (status != VP8_STATUS_OK) {
        WebPFreeDecBuffer(&config.output);
        return E_FAIL;
    }

    // Create bitmap from scaled output
    HBITMAP hBitmap = CreateBitmapFromRGBA(
        config.output.u.RGBA.rgba,
        config.output.width,
        config.output.height
    );

    // Free WebP decoder buffer
    WebPFreeDecBuffer(&config.output);

    if (!hBitmap) {
        return E_FAIL;
    }

    *phBitmap = hBitmap;
    return S_OK;
}

//=============================================================================
// Helper Methods
//=============================================================================

bool WebPDecoder::IsWebPFormat(const BYTE* data, size_t size) {
    if (!data || size < 12) {
        return false;
    }

    // Check RIFF header: "RIFF"
    if (memcmp(data, "RIFF", 4) != 0) {
        return false;
    }

    // Check WEBP signature at offset 8: "WEBP"
    if (memcmp(data + 8, "WEBP", 4) != 0) {
        return false;
    }

    return true;
}

bool WebPDecoder::IsAnimatedWebP(const BYTE* data, size_t size) {
    if (!IsWebPFormat(data, size) || size < 20) {
        return false;
    }

    // Check for ANIM chunk (animated WebP)
    // Search for "ANIM" chunk in the file
    for (size_t i = 12; i < size - 4; i++) {
        if (memcmp(data + i, "ANIM", 4) == 0) {
            return true;
        }
        // Also check for VP8X with animation flag
        if (i < size - 10 && memcmp(data + i, "VP8X", 4) == 0) {
            // VP8X flags are at offset i+8, bit 1 indicates animation
            if (i + 9 < size) {
                uint8_t flags = data[i + 8];
                if (flags & 0x02) {  // Animation flag
                    return true;
                }
            }
        }
    }

    return false;
}

HBITMAP WebPDecoder::CreateBitmapFromRGBA(const BYTE* rgba, int width, int height) {
    if (!rgba || width <= 0 || height <= 0) {
        return nullptr;
    }

    // Setup DIB header (Windows bitmap format)
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Negative = top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // 32-bit ARGB
    bmi.bmiHeader.biCompression = BI_RGB;

    // Create DIB section
    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!hBitmap || !pBits) {
        return nullptr;
    }

    // Convert RGBA to BGRA (Windows expects BGR byte order)
    const BYTE* src = rgba;
    BYTE* dst = static_cast<BYTE*>(pBits);

    for (int i = 0; i < width * height; i++) {
        dst[0] = src[2]; // B
        dst[1] = src[1]; // G
        dst[2] = src[0]; // R
        dst[3] = src[3]; // A
        src += 4;
        dst += 4;
    }

    return hBitmap;
}

void WebPDecoder::AddAnimationBadge(HBITMAP hBitmap, int width, int height) {
    if (!hBitmap || width <= 0 || height <= 0) {
        return;
    }

    // Create DC for drawing
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Draw animation indicator in bottom-right corner
    // Use a small semi-transparent badge with "GIF-style" icon
    const int badgeSize = std::min(width / 6, height / 6);
    const int badgeSize2 = std::max(20, badgeSize); // Minimum 20 pixels
    const int margin = 4;
    const int x = width - badgeSize2 - margin;
    const int y = height - badgeSize2 - margin;

    // Draw semi-transparent rounded rectangle
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);
    
    // Draw background circle
    Ellipse(hdcMem, x, y, x + badgeSize2, y + badgeSize2);

    // Draw play symbol (triangle) for animation
    POINT pts[3] = {
        { x + badgeSize2 / 3, y + badgeSize2 / 4 },
        { x + badgeSize2 / 3, y + 3 * badgeSize2 / 4 },
        { x + 2 * badgeSize2 / 3, y + badgeSize2 / 2 }
    };
    
    SelectObject(hdcMem, GetStockObject(WHITE_BRUSH));
    Polygon(hdcMem, pts, 3);

    // Cleanup
    SelectObject(hdcMem, hOldBrush);
    SelectObject(hdcMem, hOldPen);
    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hBrush);
    DeleteObject(hPen);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

} // namespace Engine
} // namespace ExplorerLens

