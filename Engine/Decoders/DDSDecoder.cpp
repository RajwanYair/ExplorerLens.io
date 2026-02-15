// DDSDecoder.cpp - DirectDraw Surface Texture Decoder Implementation
// DarkThumbs Engine v6.1.0+
// Uses WIC (Windows Imaging Component) which has native DDS support on Win10+

#include "DDSDecoder.h"

namespace DarkThumbs {
namespace Engine {

const wchar_t* DDSDecoder::m_extensions[] = { L".dds" };
const uint32_t DDSDecoder::m_extensionCount = 1;

std::mutex DDSDecoder::s_factoryMutex;

DDSDecoder::DDSDecoder() {}
DDSDecoder::~DDSDecoder() {}

Microsoft::WRL::ComPtr<IWICImagingFactory> DDSDecoder::GetWICFactory() {
    static Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
    std::lock_guard<std::mutex> lock(s_factoryMutex);
    if (!factory) {
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&factory));
    }
    return factory;
}

bool DDSDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath || filePath[0] == L'\0') return false;
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) return false;
    return _wcsicmp(ext, L".dds") == 0 && IsDDSFormat(filePath);
}

bool DDSDecoder::IsDDSFormat(const wchar_t* path) {
    // DDS magic: "DDS " (0x20534444)
    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    uint32_t magic = 0;
    DWORD bytesRead = 0;
    ReadFile(hFile, &magic, 4, &bytesRead, nullptr);
    CloseHandle(hFile);
    return bytesRead == 4 && magic == 0x20534444; // "DDS "
}

HRESULT DDSDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    if (!request.filePath) return E_INVALIDARG;
    HRESULT hr = DecodeFromFile(request.filePath, request.width,
                                 request.height, &result.hBitmap);
    if (SUCCEEDED(hr) && result.hBitmap) {
        BITMAP bm;
        if (GetObject(result.hBitmap, sizeof(bm), &bm)) {
            result.width = bm.bmWidth;
            result.height = bm.bmHeight;
        }
    }
    return hr;
}

DecoderInfo DDSDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"DDSDecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount = m_extensionCount;
    info.supportsGPU = true;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** DDSDecoder::GetSupportedExtensions() const {
    return m_extensions;
}

HRESULT DDSDecoder::DecodeFromFile(const wchar_t* path, UINT targetWidth,
                                    UINT targetHeight, HBITMAP* phBitmap) {
    if (!phBitmap) return E_POINTER;
    *phBitmap = nullptr;

    auto factory = GetWICFactory();
    if (!factory) return E_FAIL;

    // WIC has a built-in DDS decoder on Windows 10+
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = factory->CreateDecoderFromFilename(
        path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) return hr;

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame); // Frame 0 = largest mip
    if (FAILED(hr)) return hr;

    UINT width = 0, height = 0;
    frame->GetSize(&width, &height);

    // Convert to 32bppBGRA
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return hr;

    hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppBGRA,
                                WICBitmapDitherTypeNone, nullptr, 0.0,
                                WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return hr;

    // Scale if needed
    Microsoft::WRL::ComPtr<IWICBitmapScaler> scaler;
    IWICBitmapSource* source = converter.Get();

    UINT outW = width, outH = height;
    if (targetWidth > 0 && targetHeight > 0 &&
        (width > targetWidth || height > targetHeight)) {
        float scale = (std::min)((float)targetWidth / width, (float)targetHeight / height);
        outW = (UINT)(width * scale);
        outH = (UINT)(height * scale);
        if (outW == 0) outW = 1;
        if (outH == 0) outH = 1;

        hr = factory->CreateBitmapScaler(&scaler);
        if (SUCCEEDED(hr)) {
            hr = scaler->Initialize(converter.Get(), outW, outH,
                                    WICBitmapInterpolationModeHighQualityCubic);
            if (SUCCEEDED(hr)) source = scaler.Get();
        }
    }

    // Create HBITMAP
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = outW;
    bmi.bmiHeader.biHeight = -(LONG)outH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    *phBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!*phBitmap || !pBits) return E_FAIL;

    UINT stride = outW * 4;
    hr = source->CopyPixels(nullptr, stride, stride * outH,
                            static_cast<BYTE*>(pBits));
    if (FAILED(hr)) {
        DeleteObject(*phBitmap);
        *phBitmap = nullptr;
        return hr;
    }

    return S_OK;
}

} // namespace Engine
} // namespace DarkThumbs
