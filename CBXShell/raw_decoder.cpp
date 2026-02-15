/******************************************************************************
 * raw_decoder.cpp
 * Camera RAW Image Decoder Implementation for DarkThumbs
 * 
 * Professional Formats Support (v5.3+) - WIC-based
 * Supports 50+ camera RAW formats via Windows Imaging Component
 * Requires: Windows Camera Codec Pack or manufacturer RAW codecs
 ******************************************************************************/

#include "StdAfx.h"
#include "raw_decoder.h"

// WIC is already included via StdAfx.h
// No external libraries needed - uses Windows built-in codecs

namespace DarkThumbs {

/******************************************************************************
 * GetDimensions - Fast dimension extraction from RAW
 * Uses existing RawDecoder namespace implementation
 ******************************************************************************/
bool RAWDecoder::GetDimensions(const BYTE* data, size_t size, int* width, int* height)
{
    if (!data || size == 0 || !width || !height) {
        return false;
    }

    // Create stream from memory
    CComPtr<IStream> pStream;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    if (FAILED(hr)) {
        return false;
    }

    // Write data to stream
    ULONG written = 0;
    hr = pStream->Write(data, static_cast<ULONG>(size), &written);
    if (FAILED(hr) || written != size) {
        return false;
    }

    // Reset stream position
    LARGE_INTEGER zero = {};
    hr = pStream->Seek(zero, STREAM_SEEK_SET, NULL);
    if (FAILED(hr)) {
        return false;
    }

    // Use WIC to get dimensions
    CComPtr<IWICImagingFactory> pFactory;
    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory));
    if (FAILED(hr)) {
        return false;
    }

    CComPtr<IWICBitmapDecoder> pDecoder;
    hr = pFactory->CreateDecoderFromStream(pStream, NULL,
        WICDecodeMetadataCacheOnDemand, &pDecoder);
    if (FAILED(hr)) {
        return false;
    }

    CComPtr<IWICBitmapFrameDecode> pFrame;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) {
        return false;
    }

    UINT w, h;
    hr = pFrame->GetSize(&w, &h);
    if (FAILED(hr)) {
        return false;
    }

    *width = static_cast<int>(w);
    *height = static_cast<int>(h);
    return true;
}

/******************************************************************************
 * DecodeToHBITMAP - Main RAW decode to Windows bitmap
 * Delegates to existing WIC-based implementation in RawDecoder namespace
 ******************************************************************************/
HRESULT RAWDecoder::DecodeToHBITMAP(
    const BYTE* data,
    size_t size,
    HBITMAP* phBitmap,
    int maxWidth,
    int maxHeight)
{
    if (!data || size == 0 || !phBitmap) {
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    // Create stream from memory
    CComPtr<IStream> pStream;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    if (FAILED(hr)) {
        return hr;
    }

    // Write data to stream
    ULONG written = 0;
    hr = pStream->Write(data, static_cast<ULONG>(size), &written);
    if (FAILED(hr) || written != size) {
        return E_FAIL;
    }

    // Reset stream position
    LARGE_INTEGER zero = {};
    hr = pStream->Seek(zero, STREAM_SEEK_SET, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    // Use existing WIC-based decoder from RawDecoder namespace
    UINT thumbnailSize = static_cast<ULONG>(max(maxWidth, maxHeight));
    HBITMAP hBitmap = RawDecoder::DecodeToHBITMAP(pStream, thumbnailSize);
    
    if (hBitmap) {
        *phBitmap = hBitmap;
        return S_OK;
    }

    return E_FAIL;
}

/******************************************************************************
 * GetCameraInfo - Extract camera make/model from EXIF
 ******************************************************************************/
bool RAWDecoder::GetCameraInfo(
    const BYTE* data,
    size_t size,
    std::wstring& cameraMake,
    std::wstring& cameraModel)
{
    if (!data || size == 0) {
        return false;
    }

    // Create stream from memory
    CComPtr<IStream> pStream;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    if (FAILED(hr)) {
        return false;
    }

    // Write data to stream
    ULONG written = 0;
    hr = pStream->Write(data, static_cast<ULONG>(size), &written);
    if (FAILED(hr) || written != size) {
        return false;
    }

    // Reset stream position
    LARGE_INTEGER zero = {};
    hr = pStream->Seek(zero, STREAM_SEEK_SET, NULL);
    if (FAILED(hr)) {
        return false;
    }

    // Create WIC factory
    CComPtr<IWICImagingFactory> pFactory;
    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory));
    if (FAILED(hr)) {
        return false;
    }

    // Create decoder
    CComPtr<IWICBitmapDecoder> pDecoder;
    hr = pFactory->CreateDecoderFromStream(pStream, NULL,
        WICDecodeMetadataCacheOnDemand, &pDecoder);
    if (FAILED(hr)) {
        return false;
    }

    // Get first frame
    CComPtr<IWICBitmapFrameDecode> pFrame;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) {
        return false;
    }

    // Get metadata query reader
    CComPtr<IWICMetadataQueryReader> pMetadata;
    hr = pFrame->GetMetadataQueryReader(&pMetadata);
    if (FAILED(hr)) {
        return false;
    }

    // Try to read camera make and model
    PROPVARIANT value;
    PropVariantInit(&value);
    
    // Camera Make
    hr = pMetadata->GetMetadataByName(L"/ifd/exif/{ushort=271}", &value);
    if (SUCCEEDED(hr) && value.vt == VT_LPWSTR) {
        cameraMake = value.pwszVal;
    }
    PropVariantClear(&value);
    
    // Camera Model
    hr = pMetadata->GetMetadataByName(L"/ifd/exif/{ushort=272}", &value);
    if (SUCCEEDED(hr) && value.vt == VT_LPWSTR) {
        cameraModel = value.pwszVal;
    }
    PropVariantClear(&value);
    
    return true;
}

} // namespace DarkThumbs
