// AVIFDecoder.cpp - WIC-based AVIF/HEIF Decoder Implementation
// DarkThumbs Engine v1.0.0
// Copyright (c) 2025 DarkThumbs Project

#include "AVIFDecoder.h"
#include <shlwapi.h>
#include <fstream>
#include <vector>
#include <algorithm>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shlwapi.lib")

namespace DarkThumbs {
namespace Engine {

using Microsoft::WRL::ComPtr;

// Static members
std::mutex AVIFDecoder::s_factoryMutex;
ComPtr<IWICImagingFactory> AVIFDecoder::s_wicFactory;

// Extension list (null-terminated)
const wchar_t* AVIFDecoder::m_extensions[] = {
    L".avif",
    L".heif",
    L".heic",
    nullptr  // Null terminator
};

const uint32_t AVIFDecoder::m_extensionCount = 3;

//=============================================================================
// Constructor
//=============================================================================

AVIFDecoder::AVIFDecoder() {
    // Nothing to initialize
}

//=============================================================================
// WIC Factory Singleton
//=============================================================================

ComPtr<IWICImagingFactory> AVIFDecoder::GetWICFactory() {
    std::lock_guard<std::mutex> lock(s_factoryMutex);
    
    if (!s_wicFactory) {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&s_wicFactory)
        );
        
        if (FAILED(hr)) {
            return nullptr;
        }
    }
    
    return s_wicFactory;
}

//=============================================================================
// IThumbnailDecoder Implementation
//=============================================================================

bool AVIFDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath || !*filePath) {
        return false;
    }

    // Check extension
    const wchar_t* ext = PathFindExtensionW(filePath);
    if (!ext || !*ext) {
        return false;
    }

    // Check against supported extensions
    for (uint32_t i = 0; i < m_extensionCount; i++) {
        if (_wcsicmp(ext, m_extensions[i]) == 0) {
            return true;
        }
    }

    return false;
}

HRESULT AVIFDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
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

DecoderInfo AVIFDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"AVIFDecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount = m_extensionCount;
    info.supportsGPU = true;
    info.isArchiveDecoder = false;
    return info;
}

//=============================================================================
// Decoding Implementation
//=============================================================================

HRESULT AVIFDecoder::DecodeFromFile(const wchar_t* path, UINT targetWidth,
                                    UINT targetHeight, HBITMAP* phBitmap) {
    if (!path || !*path || !phBitmap) {
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    ComPtr<IWICImagingFactory> factory = GetWICFactory();
    if (!factory) {
        return E_FAIL;
    }

    // Create decoder from file
    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = factory->CreateDecoderFromFilename(
        path,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );

    if (FAILED(hr)) {
        return hr;
    }

    // Get first frame
    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        return hr;
    }

    // Convert to 32bppBGRA format for Windows compatibility
    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        return hr;
    }

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeMedianCut
    );

    if (FAILED(hr)) {
        return hr;
    }

    // Get source dimensions
    UINT sourceWidth = 0, sourceHeight = 0;
    hr = converter->GetSize(&sourceWidth, &sourceHeight);
    if (FAILED(hr)) {
        return hr;
    }

    // Scale if needed
    ComPtr<IWICBitmapSource> finalSource = converter;
    
    if (targetWidth > 0 && targetHeight > 0 && 
        (sourceWidth != targetWidth || sourceHeight != targetHeight)) {
        
        ComPtr<IWICBitmapScaler> scaler;
        hr = factory->CreateBitmapScaler(&scaler);
        if (FAILED(hr)) {
            return hr;
        }

        hr = scaler->Initialize(
            converter.Get(),
            targetWidth,
            targetHeight,
            WICBitmapInterpolationModeFant
        );
        
        if (FAILED(hr)) {
            return hr;
        }

        finalSource = scaler;
    }

    // Get final dimensions
    UINT width = 0, height = 0;
    hr = finalSource->GetSize(&width, &height);
    if (FAILED(hr)) {
        return hr;
    }

    // Create DIB section
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -static_cast<LONG>(height); // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!hBitmap) {
        return E_OUTOFMEMORY;
    }

    // Copy pixels
    UINT stride = width * 4;
    UINT bufferSize = stride * height;

    hr = finalSource->CopyPixels(nullptr, stride, bufferSize, static_cast<BYTE*>(pBits));

    if (FAILED(hr)) {
        DeleteObject(hBitmap);
        return hr;
    }

    *phBitmap = hBitmap;
    return S_OK;
}

HRESULT AVIFDecoder::DecodeFromMemory(const BYTE* data, size_t size, UINT targetWidth,
                                      UINT targetHeight, HBITMAP* phBitmap) {
    if (!data || size == 0 || !phBitmap) {
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    // Verify AVIF format
    if (!IsAVIFFormat(data, size)) {
        return E_FAIL;
    }

    ComPtr<IWICImagingFactory> factory = GetWICFactory();
    if (!factory) {
        return E_FAIL;
    }

    // Create stream from memory
    ComPtr<IWICStream> stream;
    HRESULT hr = factory->CreateStream(&stream);
    if (FAILED(hr)) {
        return hr;
    }

    hr = stream->InitializeFromMemory(const_cast<BYTE*>(data), static_cast<DWORD>(size));
    if (FAILED(hr)) {
        return hr;
    }

    // Create decoder
    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromStream(
        stream.Get(),
        nullptr,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );

    if (FAILED(hr)) {
        return hr;
    }

    // Get first frame
    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        return hr;
    }

    // Convert to 32bppBGRA
    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        return hr;
    }

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeMedianCut
    );

    if (FAILED(hr)) {
        return hr;
    }

    // Scale if needed (similar to file decode)
    // For now, skip scaling in memory decode
    (void)targetWidth;
    (void)targetHeight;

    UINT width = 0, height = 0;
    hr = converter->GetSize(&width, &height);
    if (FAILED(hr)) {
        return hr;
    }

    // Create DIB section and copy pixels
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -static_cast<LONG>(height);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!hBitmap) {
        return E_OUTOFMEMORY;
    }

    UINT stride = width * 4;
    UINT bufferSize = stride * height;
    hr = converter->CopyPixels(nullptr, stride, bufferSize, static_cast<BYTE*>(pBits));

    if (FAILED(hr)) {
        DeleteObject(hBitmap);
        return hr;
    }

    *phBitmap = hBitmap;
    return S_OK;
}

//=============================================================================
// Helper Methods
//=============================================================================

bool AVIFDecoder::IsAVIFFormat(const BYTE* data, size_t size) {
    if (!data || size < 12) {
        return false;
    }
    
    // AVIF/HEIF files are ISO Base Media File Format (ISOBMFF) containers
    // Look for "ftyp" box (file type box) which contains brand information
    for (size_t i = 0; i < std::min(size - 8, static_cast<size_t>(36)); i++) {
        if (memcmp(data + i, "ftyp", 4) == 0) {
            // Check major brand (4 bytes after "ftyp")
            const char* brand = reinterpret_cast<const char*>(data + i + 4);
            
            // Common AVIF brands
            if (memcmp(brand, "avif", 4) == 0 ||  // AVIF
                memcmp(brand, "avis", 4) == 0) {  // AVIF sequence
                return true;
            }
        }
    }
    
    return false;
}

} // namespace Engine
} // namespace DarkThumbs
