// WebPDecoder.cpp - libwebp Decoder Implementation
// DarkThumbs Engine v1.0.0
// Copyright (c) 2025 DarkThumbs Project

#include "WebPDecoder.h"
#include <shlwapi.h>
#include <fstream>
#include <vector>

// libwebp headers
#include <webp/decode.h>

#pragma comment(lib, "shlwapi.lib")
// Note: libwebp.lib will be added to CMakeLists.txt

namespace DarkThumbs {
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

    // Decode to RGBA (libwebp allocates the buffer)
    int decodedWidth = 0, decodedHeight = 0;
    BYTE* rgba = WebPDecodeRGBA(data, size, &decodedWidth, &decodedHeight);

    if (!rgba) {
        return E_FAIL;
    }

    // For now, use the full decoded image
    // TODO: Add scaling support using WebPDecodeRGBA with config for target size
    // targetWidth and targetHeight will be used in future scaling implementation
    (void)targetWidth;
    (void)targetHeight;
    
    HBITMAP hBitmap = CreateBitmapFromRGBA(rgba, decodedWidth, decodedHeight);

    // Free libwebp buffer
    WebPFree(rgba);

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

} // namespace Engine
} // namespace DarkThumbs
