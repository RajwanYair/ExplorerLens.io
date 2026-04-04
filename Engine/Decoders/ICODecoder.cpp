// ICODecoder.cpp - Windows Icon Decoder Implementation
// ExplorerLens Engine v5.3.0+

#include "ICODecoder.h"
#include <algorithm>
#include <cmath>

#pragma comment(lib, "windowscodecs.lib")

namespace ExplorerLens {
namespace Engine {

// Static members
const wchar_t* ICODecoder::m_extensions[] = {L".ico", L".cur"};

const uint32_t ICODecoder::m_extensionCount = static_cast<uint32_t>(sizeof(m_extensions) / sizeof(m_extensions[0]));

std::mutex ICODecoder::s_factoryMutex;

// Constructor/Destructor
ICODecoder::ICODecoder() {}

ICODecoder::~ICODecoder() {}

// Get WIC Factory (thread-safe singleton)
Microsoft::WRL::ComPtr<IWICImagingFactory> ICODecoder::GetWICFactory()
{
    static Microsoft::WRL::ComPtr<IWICImagingFactory> s_factory;

    std::lock_guard<std::mutex> lock(s_factoryMutex);

    if (!s_factory) {
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&s_factory));

        if (FAILED(hr)) {
            return nullptr;
        }
    }

    return s_factory;
}

bool ICODecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath || filePath[0] == L'\0') {
        return false;
    }

    // Check extension
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) {
        return false;
    }

    // Case-insensitive extension check
    for (uint32_t i = 0; i < m_extensionCount; ++i) {
        if (_wcsicmp(ext, m_extensions[i]) == 0) {
            return IsICOFormat(filePath);
        }
    }

    return false;
}

HRESULT ICODecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;

    if (!request.filePath || request.filePath[0] == L'\0') {
        return E_INVALIDARG;
    }

    HRESULT hr = DecodeFromFile(request.filePath, request.width, request.height, &result.hBitmap);

    if (SUCCEEDED(hr) && result.hBitmap) {
        // Get actual bitmap dimensions
        BITMAP bm;
        if (GetObject(result.hBitmap, sizeof(bm), &bm)) {
            result.width = bm.bmWidth;
            result.height = bm.bmHeight;
        }
    }

    return hr;
}

DecoderInfo ICODecoder::GetInfo() const
{
    DecoderInfo info;
    info.name = L"ICODecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount = m_extensionCount;
    info.supportsGPU = true;  // WIC can use GPU
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** ICODecoder::GetSupportedExtensions() const
{
    return m_extensions;
}

HRESULT ICODecoder::DecodeFromFile(const wchar_t* path, UINT targetWidth, UINT targetHeight, HBITMAP* phBitmap)
{
    if (!phBitmap) {
        return E_POINTER;
    }

    *phBitmap = nullptr;

    // Get WIC factory
    auto factory = GetWICFactory();
    if (!factory) {
        return E_FAIL;
    }

    // Create decoder from file
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr =
        factory->CreateDecoderFromFilename(path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);

    if (FAILED(hr)) {
        return hr;
    }

    // ICO files can have multiple frames (different resolutions)
    // Select the best frame for our target size
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    UINT maxDim = std::max(targetWidth, targetHeight);
    hr = SelectBestFrame(decoder.Get(), maxDim, &frame);

    if (FAILED(hr)) {
        return hr;
    }

    // Convert to 32bpp BGRA format
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);

    if (FAILED(hr)) {
        return hr;
    }

    hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0.0,
                               WICBitmapPaletteTypeCustom);

    if (FAILED(hr)) {
        return hr;
    }

    // Get dimensions
    UINT width, height;
    hr = converter->GetSize(&width, &height);

    if (FAILED(hr)) {
        return hr;
    }

    // Create DIB section for HBITMAP
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

    if (!hBitmap || !pBits) {
        ReleaseDC(nullptr, hdcScreen);
        return E_FAIL;
    }

    // Copy pixels
    UINT stride = width * 4;
    hr = converter->CopyPixels(nullptr, stride, stride * height, static_cast<BYTE*>(pBits));

    ReleaseDC(nullptr, hdcScreen);

    if (SUCCEEDED(hr)) {
        *phBitmap = hBitmap;
    } else {
        DeleteObject(hBitmap);
    }

    return hr;
}

HRESULT ICODecoder::SelectBestFrame(IWICBitmapDecoder* decoder, UINT targetSize, IWICBitmapFrameDecode** ppFrame)
{
    if (!decoder || !ppFrame) {
        return E_POINTER;
    }

    *ppFrame = nullptr;

    // Get frame count
    UINT frameCount = 0;
    HRESULT hr = decoder->GetFrameCount(&frameCount);

    if (FAILED(hr) || frameCount == 0) {
        return hr;
    }

    // If only one frame, use it
    if (frameCount == 1) {
        return decoder->GetFrame(0, ppFrame);
    }

    // Find best matching frame for target size
    UINT bestFrame = 0;
    UINT bestScore = UINT_MAX;

    for (UINT i = 0; i < frameCount; ++i) {
        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(i, &frame);

        if (FAILED(hr)) {
            continue;
        }

        UINT width, height;
        hr = frame->GetSize(&width, &height);

        if (FAILED(hr)) {
            continue;
        }

        // Calculate score (prefer size >= target, penalize oversized)
        UINT maxDim = std::max(width, height);
        UINT score;

        if (maxDim >= targetSize) {
            // Prefer smallest size that's >= target
            score = maxDim - targetSize;
        } else {
            // Penalize undersized (but still usable)
            score = (targetSize - maxDim) * 2;
        }

        if (score < bestScore) {
            bestScore = score;
            bestFrame = i;
        }
    }

    return decoder->GetFrame(bestFrame, ppFrame);
}

bool ICODecoder::IsICOFormat(const wchar_t* path)
{
    // Quick check: read first 4 bytes
    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    BYTE header[4];
    DWORD bytesRead = 0;
    BOOL success = ReadFile(hFile, header, 4, &bytesRead, nullptr);
    CloseHandle(hFile);

    if (!success || bytesRead < 4) {
        return false;
    }

    // ICO format: 00 00 01 00 (icon) or 00 00 02 00 (cursor)
    if (header[0] == 0x00 && header[1] == 0x00) {
        if ((header[2] == 0x01 && header[3] == 0x00) ||  // Icon
            (header[2] == 0x02 && header[3] == 0x00)) {  // Cursor
            return true;
        }
    }

    return false;
}

}  // namespace Engine
}  // namespace ExplorerLens
