/******************************************************************************
 * avif_decoder.cpp
 * AVIF/HEIF Image Decoder Implementation for DarkThumbs
 * Uses Windows Imaging Component (WIC) for native AVIF support
 * Requires Windows 10 1809+ and AV1 Video Extension from Microsoft Store
 ******************************************************************************/

#include "StdAfx.h"
#include "avif_decoder.h"
#include <wincodec.h>
#include <atlbase.h>
#include <Shlwapi.h>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shlwapi.lib")

namespace DarkThumbs {

bool AVIFDecoder::IsAVIFFormat(const BYTE* data, size_t size) {
    if (!data || size < 12) {
        return false;
    }
    
    // AVIF/HEIF files are ISO Base Media File Format (ISOBMFF) containers
    // Look for "ftyp" box (file type box) which contains brand information
    for (size_t i = 0; i < min(size - 8, 36); i++) {
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

bool AVIFDecoder::GetDimensions(const BYTE* data, size_t size, int* width, int* height) {
    if (!IsAVIFFormat(data, size) || !width || !height) {
        return false;
    }
    
    CComPtr<IWICImagingFactory> pFactory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory)
    );
    
    if (FAILED(hr)) return false;
    
    // Create stream from memory
    CComPtr<IWICStream> pStream;
    hr = pFactory->CreateStream(&pStream);
    if (FAILED(hr)) return false;
    
    hr = pStream->InitializeFromMemory(const_cast<BYTE*>(data), static_cast<DWORD>(size));
    if (FAILED(hr)) return false;
    
    // Create decoder
    CComPtr<IWICBitmapDecoder> pDecoder;
    hr = pFactory->CreateDecoderFromStream(
        pStream,
        NULL,
        WICDecodeMetadataCacheOnDemand,
        &pDecoder
    );
    
    if (FAILED(hr)) return false;
    
    // Get first frame
    CComPtr<IWICBitmapFrameDecode> pFrame;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) return false;
    
    UINT w, h;
    hr = pFrame->GetSize(&w, &h);
    if (SUCCEEDED(hr)) {
        *width = w;
        *height = h;
        return true;
    }
    
    return false;
}

HRESULT AVIFDecoder::DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap) {
    if (!data || size == 0 || !phBitmap) {
        return E_INVALIDARG;
    }
    
    *phBitmap = nullptr;
    
    // Verify format
    if (!IsAVIFFormat(data, size)) {
        return E_FAIL;
    }
    
    CComPtr<IWICImagingFactory> pFactory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory)
    );
    
    if (FAILED(hr)) return hr;
    
    // Create stream from memory
    CComPtr<IWICStream> pStream;
    hr = pFactory->CreateStream(&pStream);
    if (FAILED(hr)) return hr;
    
    hr = pStream->InitializeFromMemory(const_cast<BYTE*>(data), static_cast<DWORD>(size));
    if (FAILED(hr)) return hr;
    
    // Create decoder
    CComPtr<IWICBitmapDecoder> pDecoder;
    hr = pFactory->CreateDecoderFromStream(
        pStream,
        NULL,
        WICDecodeMetadataCacheOnDemand,
        &pDecoder
    );
    
    if (FAILED(hr)) return hr;
    
    // Get first frame
    CComPtr<IWICBitmapFrameDecode> pFrame;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) return hr;
    
    // Convert to 32bppBGRA format for Windows compatibility
    CComPtr<IWICFormatConverter> pConverter;
    hr = pFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) return hr;
    
    hr = pConverter->Initialize(
        pFrame,
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        0.0,
        WICBitmapPaletteTypeMedianCut
    );
    
    if (FAILED(hr)) return hr;
    
    // Get dimensions
    UINT width, height;
    hr = pConverter->GetSize(&width, &height);
    if (FAILED(hr)) return hr;
    
    // Create DIB section
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(LONG)height;  // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* pBits = NULL;
    HDC hdcScreen = GetDC(NULL);
    HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    ReleaseDC(NULL, hdcScreen);
    
    if (!hBitmap) return E_FAIL;
    
    // Copy pixels
    UINT stride = width * 4;
    UINT bufferSize = stride * height;
    
    hr = pConverter->CopyPixels(
        NULL,
        stride,
        bufferSize,
        static_cast<BYTE*>(pBits)
    );
    
    if (FAILED(hr)) {
        DeleteObject(hBitmap);
        return hr;
    }
    
    *phBitmap = hBitmap;
    return S_OK;
}

} // namespace DarkThumbs
