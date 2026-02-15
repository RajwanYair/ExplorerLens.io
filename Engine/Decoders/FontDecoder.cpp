// FontDecoder.cpp - Font Preview Thumbnail Decoder Implementation
// DarkThumbs Engine v6.2.0+
// Copyright (c) 2026 DarkThumbs Project

#include "FontDecoder.h"
#include "../Utils/PerformanceProfiler.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <thumbcache.h>
#include <cwchar>
#include <algorithm>
#include <string>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

namespace DarkThumbs {
namespace Engine {

const wchar_t* FontDecoder::m_extensions[] = {
    L".ttf", L".otf", L".woff", L".woff2",
    L".ttc", L".fon", L".fnt",
    nullptr
};
const uint32_t FontDecoder::m_extensionCount = 7;

FontDecoder::FontDecoder() = default;
FontDecoder::~FontDecoder() = default;

bool FontDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath) return false;
    return IsFontFormat(filePath);
}

HRESULT FontDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    PROFILE_SCOPE(ProfileComponent::DECODE_FONT);

    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    if (!request.filePath) return E_INVALIDARG;

    // Try Shell font thumbnail (Windows has built-in font previewer)
    HRESULT hr = ExtractFontPreviewShell(request.filePath, request.width,
                                          request.height, &result.hBitmap);

    // Fallback: placeholder
    if (FAILED(hr) || !result.hBitmap) {
        result.hBitmap = CreateFontPlaceholder(request.width, request.height, request.filePath);
        hr = result.hBitmap ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr) && result.hBitmap) {
        BITMAP bm;
        if (GetObject(result.hBitmap, sizeof(bm), &bm)) {
            result.width = bm.bmWidth;
            result.height = bm.bmHeight;
        }
    }
    return hr;
}

DecoderInfo FontDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"Font Decoder";
    info.version = L"1.0.0";
    info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** FontDecoder::GetSupportedExtensions() const {
    return const_cast<const wchar_t**>(m_extensions);
}

// ============================================================================
// Shell Font Preview
// ============================================================================

HRESULT FontDecoder::ExtractFontPreviewShell(const wchar_t* filePath, uint32_t width,
                                              uint32_t height, HBITMAP* phBitmap) {
    if (!phBitmap) return E_INVALIDARG;
    *phBitmap = nullptr;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
    if (!comInit) return hr;

    IShellItem* pItem = nullptr;
    hr = SHCreateItemFromParsingName(filePath, nullptr, IID_PPV_ARGS(&pItem));
    if (FAILED(hr)) { CoUninitialize(); return hr; }

    IShellItemImageFactory* pFactory = nullptr;
    hr = pItem->QueryInterface(IID_PPV_ARGS(&pFactory));
    if (SUCCEEDED(hr) && pFactory) {
        SIZE sz = { static_cast<LONG>(width), static_cast<LONG>(height) };
        hr = pFactory->GetImage(sz, SIIGBF_THUMBNAILONLY | SIIGBF_BIGGERSIZEOK, phBitmap);
        if (FAILED(hr)) {
            hr = pFactory->GetImage(sz, SIIGBF_BIGGERSIZEOK, phBitmap);
        }
        pFactory->Release();
    }

    pItem->Release();
    CoUninitialize();
    return hr;
}

// ============================================================================
// Font Placeholder
// ============================================================================

HBITMAP FontDecoder::CreateFontPlaceholder(uint32_t width, uint32_t height,
                                            const wchar_t* filePath) {
    Gdiplus::GdiplusStartupInput gdipInput;
    ULONG_PTR gdipToken = 0;
    if (Gdiplus::GdiplusStartup(&gdipToken, &gdipInput, nullptr) != Gdiplus::Ok)
        return nullptr;

    auto bmp = std::make_unique<Gdiplus::Bitmap>(width, height, PixelFormat32bppARGB);
    Gdiplus::Graphics g(bmp.get());
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    // Light background
    g.Clear(Gdiplus::Color(255, 248, 248, 250));

    float margin = width * 0.06f;

    // Extract filename for display
    std::wstring fileName;
    if (filePath) {
        const wchar_t* name = PathFindFileNameW(filePath);
        if (name) fileName = name;
    }

    // Extension for badge
    const wchar_t* ext = filePath ? PathFindExtensionW(filePath) : L".ttf";
    std::wstring extUpper;
    if (ext && ext[0] == L'.') {
        extUpper = ext + 1;
        for (auto& c : extUpper) c = towupper(c);
    }

    // Font badge in top-left
    float badgeH = height * 0.12f;
    float badgeW = badgeH * 2.5f;
    Gdiplus::SolidBrush badgeBrush(Gdiplus::Color(255, 90, 90, 90));
    g.FillRectangle(&badgeBrush, margin, margin, badgeW, badgeH);

    Gdiplus::FontFamily fontFamily(L"Arial");
    float smallFontSize = badgeH * 0.6f;
    if (smallFontSize < 7) smallFontSize = 7;
    Gdiplus::Font smallFont(&fontFamily, smallFontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::StringFormat centerFmt;
    centerFmt.SetAlignment(Gdiplus::StringAlignmentCenter);
    centerFmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    Gdiplus::RectF badgeRect(margin, margin, badgeW, badgeH);
    g.DrawString(extUpper.c_str(), -1, &smallFont, badgeRect, &centerFmt, &whiteBrush);

    // Large sample text "Aa" in center
    float sampleSize = height * 0.4f;
    if (sampleSize < 20) sampleSize = 20;
    Gdiplus::Font sampleFont(&fontFamily, sampleSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 50, 50, 50));
    Gdiplus::RectF sampleRect(0, height * 0.2f, static_cast<float>(width), sampleSize * 1.3f);
    centerFmt.SetAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(L"Aa", 2, &sampleFont, sampleRect, &centerFmt, &textBrush);

    // Alphabet preview below
    float alphaSize = height * 0.08f;
    if (alphaSize < 6) alphaSize = 6;
    Gdiplus::Font alphaFont(&fontFamily, alphaSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush grayBrush(Gdiplus::Color(140, 80, 80, 80));
    Gdiplus::RectF alphaRect(margin, height * 0.65f, width - margin * 2, alphaSize * 2);
    Gdiplus::StringFormat leftFmt;
    leftFmt.SetAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(L"ABCDEFGHIJKLM", -1, &alphaFont, alphaRect, &leftFmt, &grayBrush);

    Gdiplus::RectF alphaRect2(margin, height * 0.65f + alphaSize * 1.4f, width - margin * 2, alphaSize * 2);
    g.DrawString(L"NOPQRSTUVWXYZ", -1, &alphaFont, alphaRect2, &leftFmt, &grayBrush);

    // File name at bottom
    if (!fileName.empty()) {
        float nameSize = height * 0.055f;
        if (nameSize < 6) nameSize = 6;
        Gdiplus::Font nameFont(&fontFamily, nameSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush nameBrush(Gdiplus::Color(120, 100, 100, 100));
        Gdiplus::RectF nameRect(margin, height * 0.88f, width - margin * 2, nameSize * 1.5f);
        g.DrawString(fileName.c_str(), -1, &nameFont, nameRect, &leftFmt, &nameBrush);
    }

    HBITMAP hBitmap = nullptr;
    bmp->GetHBITMAP(Gdiplus::Color(255, 248, 248, 250), &hBitmap);
    Gdiplus::GdiplusShutdown(gdipToken);
    return hBitmap;
}

// ============================================================================
// Format Detection
// ============================================================================

bool FontDecoder::IsFontFormat(const wchar_t* path) {
    if (!path) return false;
    const wchar_t* ext = PathFindExtensionW(path);
    if (!ext || ext[0] == L'\0') return false;
    for (int i = 0; m_extensions[i] != nullptr; i++) {
        if (_wcsicmp(ext, m_extensions[i]) == 0) return true;
    }
    return false;
}

} // namespace Engine
} // namespace DarkThumbs
