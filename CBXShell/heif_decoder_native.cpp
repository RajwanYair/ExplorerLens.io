/******************************************************************************
 * heif_decoder_native.cpp
 * Native Windows HEIF/HEIC Decoder using Windows Imaging Component (WIC)
 * For DarkThumbs v5.0 - Sprint 2 Alternative Implementation
 ******************************************************************************/

#include "StdAfx.h"
#include "heif_decoder_native.h"
#include <wincodec.h>

#pragma comment(lib, "windowscodecs.lib")

namespace DarkThumbs {

bool HEIFDecoderNative::IsHEIFFormat(const BYTE* data, size_t size) {
    if (!data || size < 12) {
        return false;
    }
    
    // HEIF/HEIC files are ISO Base Media File Format (ISOBMFF) containers
    // Look for "ftyp" box with HEIF brands
    for (size_t i = 0; i < min(size - 8, 36); i++) {
        if (memcmp(data + i, "ftyp", 4) == 0) {
            const char* brand = reinterpret_cast<const char*>(data + i + 4);
            
            // HEIF/HEIC brands (iPhone photos, etc.)
            if (memcmp(brand, "heic", 4) == 0 ||  // HEIC (iPhone)
                memcmp(brand, "heix", 4) == 0 ||  // HEIC extended range
                memcmp(brand, "hevc", 4) == 0 ||  // HEVC-based
                memcmp(brand, "hevx", 4) == 0 ||  // HEVC extended
                memcmp(brand, "mif1", 4) == 0) {  // Generic HEIF
                return true;
            }
        }
    }
    
    return false;
}

HRESULT HEIFDecoderNative::DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap, bool isDarkMode) {
    if (!data || size == 0 || !phBitmap) {
        return E_INVALIDARG;
    }
    
    *phBitmap = nullptr;
    
    // Verify format
    if (!IsHEIFFormat(data, size)) {
        return E_FAIL;
    }
    
    // Initialize COM for WIC
    CoInitialize(nullptr);
    
    // Create WIC factory
    IWICImagingFactory* pFactory = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory)
    );
    
    if (FAILED(hr)) {
        CoUninitialize();
        return hr;
    }
    
    // Create stream from memory
    IWICStream* pStream = nullptr;
    hr = pFactory->CreateStream(&pStream);
    
    if (SUCCEEDED(hr)) {
        hr = pStream->InitializeFromMemory(const_cast<BYTE*>(data), static_cast<DWORD>(size));
    }
    
    // Create decoder - Windows 10/11 has built-in HEIF codec
    IWICBitmapDecoder* pDecoder = nullptr;
    if (SUCCEEDED(hr)) {
        hr = pFactory->CreateDecoderFromStream(
            pStream,
            nullptr,
            WICDecodeMetadataCacheOnDemand,
            &pDecoder
        );
    }
    
    // Get first frame
    IWICBitmapFrameDecode* pFrame = nullptr;
    if (SUCCEEDED(hr)) {
        hr = pDecoder->GetFrame(0, &pFrame);
    }
    
    // Convert to 32bpp BGRA
    IWICFormatConverter* pConverter = nullptr;
    if (SUCCEEDED(hr)) {
        hr = pFactory->CreateFormatConverter(&pConverter);
    }
    
    if (SUCCEEDED(hr)) {
        hr = pConverter->Initialize(
            pFrame,
            GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom
        );
    }
    
    // Get image dimensions
    UINT width = 0, height = 0;
    if (SUCCEEDED(hr)) {
        hr = pConverter->GetSize(&width, &height);
    }
    
    // Create DIB section for HBITMAP
    if (SUCCEEDED(hr)) {
        HDC hdcScreen = GetDC(nullptr);
        
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -(LONG)height;  // Top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        
        void* pBits = nullptr;
        HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
        
        if (hBitmap && pBits) {
            // Copy pixel data
            UINT stride = width * 4;
            hr = pConverter->CopyPixels(nullptr, stride, stride * height, static_cast<BYTE*>(pBits));
            
            if (SUCCEEDED(hr)) {
                // Apply dark mode background if needed
                if (isDarkMode) {
                    ApplyDarkModeBackground(static_cast<BYTE*>(pBits), width, height);
                }
                
                *phBitmap = hBitmap;
            }
            else {
                DeleteObject(hBitmap);
            }
        }
        else {
            hr = E_FAIL;
        }
        
        ReleaseDC(nullptr, hdcScreen);
    }
    
    // Cleanup
    if (pConverter) pConverter->Release();
    if (pFrame) pFrame->Release();
    if (pDecoder) pDecoder->Release();
    if (pStream) pStream->Release();
    if (pFactory) pFactory->Release();
    
    CoUninitialize();
    
    return hr;
}

void HEIFDecoderNative::ApplyDarkModeBackground(BYTE* pPixels, UINT width, UINT height) {
    // Dark mode background color (from Windows 11 dark theme)
    const BYTE darkR = 32, darkG = 32, darkB = 32;
    
    UINT pixelCount = width * height;
    for (UINT i = 0; i < pixelCount; i++) {
        BYTE* pixel = pPixels + (i * 4);
        BYTE b = pixel[0];
        BYTE g = pixel[1];
        BYTE r = pixel[2];
        BYTE a = pixel[3];
        
        if (a < 255) {
            // Alpha blend with dark background
            float alpha = a / 255.0f;
            float invAlpha = 1.0f - alpha;
            
            pixel[0] = static_cast<BYTE>(b * alpha + darkB * invAlpha);  // B
            pixel[1] = static_cast<BYTE>(g * alpha + darkG * invAlpha);  // G
            pixel[2] = static_cast<BYTE>(r * alpha + darkR * invAlpha);  // R
            pixel[3] = 255;  // Make opaque
        }
    }
}

} // namespace DarkThumbs
