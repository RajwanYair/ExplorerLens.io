// ImageDecoder.cpp - WIC-based Image Decoder Implementation
// DarkThumbs Engine v1.0.0
// Copyright (c) 2025 DarkThumbs Project

#include "ImageDecoder.h"
#include "../Utils/PerformanceProfiler.h"
#include <shlwapi.h>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shlwapi.lib")

namespace DarkThumbs {
namespace Engine {

using Microsoft::WRL::ComPtr;

// Static members
std::mutex ImageDecoder::s_factoryMutex;
ComPtr<IWICImagingFactory> ImageDecoder::s_wicFactory;

// Extension list (null-terminated)
const wchar_t* ImageDecoder::m_extensions[] = {
    L".jpg",
    L".jpeg", 
    L".jpe",
    L".jfif",
    L".png",
    L".bmp",
    L".dib",
    L".gif",
    L".tif",
    L".tiff",
    nullptr  // Null terminator
};

const uint32_t ImageDecoder::m_extensionCount = 10;

//=============================================================================
// Constructor
//=============================================================================

ImageDecoder::ImageDecoder() {
    // Nothing to initialize - using static arrays
}

//=============================================================================
// WIC Factory Singleton
//=============================================================================

ComPtr<IWICImagingFactory> ImageDecoder::GetWICFactory() {
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

bool ImageDecoder::CanDecode(const wchar_t* filePath) {
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

HRESULT ImageDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    PROFILE_SCOPE(ProfileComponent::DECODE_IMAGE);
    
    OutputDebugStringW(L"[ImageDecoder] Decode() called\n");
    
    if (!CanDecode(request.filePath)) {
        OutputDebugStringW(L"[ImageDecoder] ERROR: CanDecode() returned false\n");
        return E_INVALIDARG;
    }

    // Decode the image
    HBITMAP hBitmap = nullptr;
    HRESULT hr = DecodeFromFile(request.filePath, request.width,
                                 request.height, &hBitmap);

    if (FAILED(hr)) {
        wchar_t errLog[256];
        swprintf_s(errLog, L"[ImageDecoder] DecodeFromFile failed: HRESULT=0x%08X\n", hr);
        OutputDebugStringW(errLog);
        result.status = hr;
        return hr;
    }

    OutputDebugStringW(L"[ImageDecoder] DecodeFromFile succeeded, HBITMAP created\n");

    // Success
    result.hBitmap = hBitmap;
    result.width = request.width;
    result.height = request.height;
    result.status = S_OK;

    return S_OK;
}

DecoderInfo ImageDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"ImageDecoder";
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

HRESULT ImageDecoder::DecodeFromFile(const wchar_t* path, UINT targetWidth,
                                     UINT targetHeight, HBITMAP* phBitmap) {
    if (!path || !*path || !phBitmap) {
        OutputDebugStringW(L"[ImageDecoder] DecodeFromFile: Invalid arguments\n");
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    ComPtr<IWICImagingFactory> factory = GetWICFactory();
    if (!factory) {
        OutputDebugStringW(L"[ImageDecoder] ERROR: WIC Factory creation failed\n");
        return E_FAIL;
    }
    
    OutputDebugStringW(L"[ImageDecoder] WIC Factory obtained\n");

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
        wchar_t errLog[256];
        swprintf_s(errLog, L"[ImageDecoder] CreateDecoderFromFilename failed: HRESULT=0x%08X\n", hr);
        OutputDebugStringW(errLog);
        return hr;
    }
    
    OutputDebugStringW(L"[ImageDecoder] WIC decoder created from file\n");

    // Get first frame
    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        return hr;
    }

    // Apply EXIF orientation if present
    ComPtr<IWICBitmapSource> orientedSource;
    hr = ApplyEXIFOrientation(frame.Get(), &orientedSource);
    if (FAILED(hr)) {
        // If orientation fails, use original frame
        orientedSource = frame;
        hr = S_OK;
    }

    // Get source dimensions
    UINT sourceWidth = 0, sourceHeight = 0;
    hr = orientedSource->GetSize(&sourceWidth, &sourceHeight);
    if (FAILED(hr)) {
        return hr;
    }

    // Scale if needed
    ComPtr<IWICBitmapSource> finalSource = orientedSource;
    
    if (targetWidth > 0 && targetHeight > 0 && 
        (sourceWidth != targetWidth || sourceHeight != targetHeight)) {
        
        ComPtr<IWICBitmapScaler> scaler;
        hr = factory->CreateBitmapScaler(&scaler);
        if (FAILED(hr)) {
            return hr;
        }

        // Use high-quality Fant interpolation
        hr = scaler->Initialize(
            orientedSource.Get(),
            targetWidth,
            targetHeight,
            WICBitmapInterpolationModeFant
        );
        
        if (FAILED(hr)) {
            return hr;
        }

        finalSource = scaler;
    }

    // Convert to HBITMAP
    return CreateHBITMAPFromWIC(finalSource.Get(), phBitmap);
}

HRESULT ImageDecoder::DecodeFromStream(IStream* pStream, UINT targetWidth,
                                       UINT targetHeight, HBITMAP* phBitmap) {
    if (!pStream || !phBitmap) {
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    ComPtr<IWICImagingFactory> factory = GetWICFactory();
    if (!factory) {
        return E_FAIL;
    }

    // Create decoder from stream
    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = factory->CreateDecoderFromStream(
        pStream,
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

    // Apply EXIF orientation
    ComPtr<IWICBitmapSource> orientedSource;
    hr = ApplyEXIFOrientation(frame.Get(), &orientedSource);
    if (FAILED(hr)) {
        orientedSource = frame;
        hr = S_OK;
    }

    // Scale if needed
    ComPtr<IWICBitmapSource> finalSource = orientedSource;
    
    if (targetWidth > 0 && targetHeight > 0) {
        UINT sourceWidth = 0, sourceHeight = 0;
        hr = orientedSource->GetSize(&sourceWidth, &sourceHeight);
        if (SUCCEEDED(hr) && (sourceWidth != targetWidth || sourceHeight != targetHeight)) {
            ComPtr<IWICBitmapScaler> scaler;
            hr = factory->CreateBitmapScaler(&scaler);
            if (SUCCEEDED(hr)) {
                hr = scaler->Initialize(orientedSource.Get(), targetWidth, targetHeight,
                                       WICBitmapInterpolationModeFant);
                if (SUCCEEDED(hr)) {
                    finalSource = scaler;
                }
            }
        }
    }

    // Convert to HBITMAP
    return CreateHBITMAPFromWIC(finalSource.Get(), phBitmap);
}

//=============================================================================
// Helper Methods
//=============================================================================

HRESULT ImageDecoder::CreateHBITMAPFromWIC(IWICBitmapSource* pSource, HBITMAP* phBitmap) {
    if (!pSource || !phBitmap) {
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    ComPtr<IWICImagingFactory> factory = GetWICFactory();
    if (!factory) {
        return E_FAIL;
    }

    // Get dimensions
    UINT width = 0, height = 0;
    HRESULT hr = pSource->GetSize(&width, &height);
    if (FAILED(hr)) {
        return hr;
    }

    // Convert to 32bpp BGRA format (required for HBITMAP)
    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        return hr;
    }

    hr = converter->Initialize(
        pSource,
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

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

    hr = converter->CopyPixels(nullptr, stride, bufferSize, static_cast<BYTE*>(pBits));

    if (FAILED(hr)) {
        DeleteObject(hBitmap);
        return hr;
    }

    *phBitmap = hBitmap;
    return S_OK;
}

HRESULT ImageDecoder::ApplyEXIFOrientation(IWICBitmapFrameDecode* pFrame,
                                           IWICBitmapSource** ppCorrected) {
    if (!pFrame || !ppCorrected) {
        return E_INVALIDARG;
    }

    *ppCorrected = nullptr;

    ComPtr<IWICImagingFactory> factory = GetWICFactory();
    if (!factory) {
        return E_FAIL;
    }

    // Try to get metadata reader
    ComPtr<IWICMetadataQueryReader> metadataReader;
    HRESULT hr = pFrame->GetMetadataQueryReader(&metadataReader);
    if (FAILED(hr)) {
        // No metadata, return original frame
        *ppCorrected = pFrame;
        pFrame->AddRef();
        return S_OK;
    }

    // Query EXIF orientation tag
    PROPVARIANT value;
    PropVariantInit(&value);
    
    hr = metadataReader->GetMetadataByName(L"/app1/ifd/{ushort=274}", &value);
    if (FAILED(hr) || value.vt != VT_UI2) {
        // No orientation tag
        PropVariantClear(&value);
        *ppCorrected = pFrame;
        pFrame->AddRef();
        return S_OK;
    }

    USHORT orientation = value.uiVal;
    PropVariantClear(&value);

    // Orientation values:
    // 1 = Normal (no rotation)
    // 2 = Flip horizontal
    // 3 = Rotate 180
    // 4 = Flip vertical
    // 5 = Rotate 90 CW + flip horizontal
    // 6 = Rotate 90 CW
    // 7 = Rotate 270 CW + flip horizontal
    // 8 = Rotate 270 CW

    if (orientation == 1) {
        // No transformation needed
        *ppCorrected = pFrame;
        pFrame->AddRef();
        return S_OK;
    }

    // Determine transformation
    WICBitmapTransformOptions transformOptions = WICBitmapTransformRotate0;
    
    switch (orientation) {
        case 2:
            transformOptions = WICBitmapTransformFlipHorizontal;
            break;
        case 3:
            transformOptions = WICBitmapTransformRotate180;
            break;
        case 4:
            transformOptions = WICBitmapTransformFlipVertical;
            break;
        case 5:
            transformOptions = static_cast<WICBitmapTransformOptions>(
                WICBitmapTransformRotate90 | WICBitmapTransformFlipHorizontal);
            break;
        case 6:
            transformOptions = WICBitmapTransformRotate90;
            break;
        case 7:
            transformOptions = static_cast<WICBitmapTransformOptions>(
                WICBitmapTransformRotate270 | WICBitmapTransformFlipHorizontal);
            break;
        case 8:
            transformOptions = WICBitmapTransformRotate270;
            break;
        default:
            // Unknown orientation, use original
            *ppCorrected = pFrame;
            pFrame->AddRef();
            return S_OK;
    }

    // Create flipper/rotator
    ComPtr<IWICBitmapFlipRotator> flipRotator;
    hr = factory->CreateBitmapFlipRotator(&flipRotator);
    if (FAILED(hr)) {
        return hr;
    }

    hr = flipRotator->Initialize(pFrame, transformOptions);
    if (FAILED(hr)) {
        return hr;
    }

    *ppCorrected = flipRotator.Detach();
    return S_OK;
}

} // namespace Engine
} // namespace DarkThumbs
