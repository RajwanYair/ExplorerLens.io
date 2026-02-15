// EXRDecoder.cpp - OpenEXR Image Decoder Implementation
// DarkThumbs Engine v6.1.0+
// Uses WIC with Windows built-in or third-party OpenEXR codec
// Falls back gracefully if no EXR codec is installed

#include "EXRDecoder.h"
#include <fstream>
#include <cstring>
#include <algorithm>

namespace DarkThumbs {
namespace Engine {

const wchar_t* EXRDecoder::m_extensions[] = { L".exr" };
const uint32_t EXRDecoder::m_extensionCount = 1;

Microsoft::WRL::ComPtr<IWICImagingFactory> EXRDecoder::s_wicFactory;
std::mutex EXRDecoder::s_factoryMutex;

EXRDecoder::EXRDecoder() {}
EXRDecoder::~EXRDecoder() {}

bool EXRDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath) return false;
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) return false;
    if (_wcsicmp(ext, L".exr") != 0) return false;

    // Verify magic bytes: 0x76, 0x2F, 0x31, 0x01
    size_t fileSize = 0;
    auto data = ReadFileData(filePath, fileSize);
    if (!data || fileSize < 4) return false;
    return data[0] == 0x76 && data[1] == 0x2F && data[2] == 0x31 && data[3] == 0x01;
}

HRESULT EXRDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    if (!request.filePath) return E_INVALIDARG;

    uint32_t maxSize = (request.width > 0) ? request.width : 256;
    HRESULT hr = DecodeFromFile(request.filePath, maxSize, &result.hBitmap);
    if (SUCCEEDED(hr) && result.hBitmap) {
        BITMAP bm;
        if (GetObject(result.hBitmap, sizeof(bm), &bm)) {
            result.width = bm.bmWidth;
            result.height = bm.bmHeight;
        }
    }
    return hr;
}

DecoderInfo EXRDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"EXRDecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** EXRDecoder::GetSupportedExtensions() const { return m_extensions; }

HRESULT EXRDecoder::EnsureWICFactory() {
    std::lock_guard<std::mutex> lock(s_factoryMutex);
    if (!s_wicFactory) {
        return CoCreateInstance(
            CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&s_wicFactory));
    }
    return S_OK;
}

HRESULT EXRDecoder::DecodeFromFile(const wchar_t* path, uint32_t maxSize, HBITMAP* phBitmap) {
    if (!phBitmap) return E_POINTER;
    *phBitmap = nullptr;
    return DecodeWithWIC(path, maxSize, phBitmap);
}

HRESULT EXRDecoder::DecodeWithWIC(const wchar_t* path, uint32_t maxSize, HBITMAP* phBitmap) {
    HRESULT hr = EnsureWICFactory();
    if (FAILED(hr)) return hr;

    // Attempt to create a decoder from file. This requires a WIC codec that supports EXR.
    // On Windows, this may be installed via Microsoft Store (Raw Image Extension) or third-party.
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    hr = s_wicFactory->CreateDecoderFromFilename(
        path, nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return hr; // No EXR codec available

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return hr;

    UINT width = 0, height = 0;
    frame->GetSize(&width, &height);
    if (width == 0 || height == 0) return E_FAIL;

    // Convert to 32bppBGRA
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = s_wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return hr;

    hr = converter->Initialize(
        frame.Get(), GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0f,
        WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return hr;

    // Scale if needed
    IWICBitmapSource* source = converter.Get();
    Microsoft::WRL::ComPtr<IWICBitmapScaler> scaler;

    if (width > maxSize || height > maxSize) {
        float scale = (std::min)((float)maxSize / width, (float)maxSize / height);
        UINT newW = (UINT)(width * scale);
        UINT newH = (UINT)(height * scale);
        if (newW == 0) newW = 1;
        if (newH == 0) newH = 1;

        hr = s_wicFactory->CreateBitmapScaler(&scaler);
        if (SUCCEEDED(hr)) {
            hr = scaler->Initialize(converter.Get(), newW, newH, WICBitmapInterpolationModeHighQualityCubic);
            if (SUCCEEDED(hr)) {
                source = scaler.Get();
                width = newW;
                height = newH;
            }
        }
    }

    // Create DIB section
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(LONG)height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!hBmp || !pBits) return E_FAIL;

    UINT stride = width * 4;
    UINT bufSize = stride * height;
    hr = source->CopyPixels(nullptr, stride, bufSize, static_cast<BYTE*>(pBits));
    if (FAILED(hr)) {
        DeleteObject(hBmp);
        return hr;
    }

    *phBitmap = hBmp;
    return S_OK;
}

std::unique_ptr<uint8_t[]> EXRDecoder::ReadFileData(const wchar_t* path, size_t& fileSize) {
    fileSize = 0;
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return nullptr;
    fileSize = static_cast<size_t>(file.tellg());
    // Only read header for magic check
    size_t readSize = (std::min)(fileSize, (size_t)64);
    file.seekg(0, std::ios::beg);
    auto data = std::make_unique<uint8_t[]>(readSize);
    file.read(reinterpret_cast<char*>(data.get()), readSize);
    fileSize = readSize;
    return data;
}

} // namespace Engine
} // namespace DarkThumbs
