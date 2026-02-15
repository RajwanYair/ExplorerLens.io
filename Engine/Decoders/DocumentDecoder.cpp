// DocumentDecoder.cpp - Document Thumbnail Decoder Implementation
// DarkThumbs Engine v6.2.0+
// Copyright (c) 2026 DarkThumbs Project

#include "DocumentDecoder.h"
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

const wchar_t* DocumentDecoder::m_extensions[] = {
    L".epub", L".mobi", L".azw", L".azw3", L".fb2",
    L".docx", L".doc", L".xlsx", L".xls", L".pptx", L".ppt",
    L".xps", L".oxps", L".djvu", L".djv",
    L".rtf", L".odt", L".ods", L".odp",
    nullptr
};
const uint32_t DocumentDecoder::m_extensionCount = 19;

DocumentDecoder::DocumentDecoder() = default;
DocumentDecoder::~DocumentDecoder() = default;

bool DocumentDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath) return false;
    return IsDocumentFormat(filePath);
}

HRESULT DocumentDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    PROFILE_SCOPE(ProfileComponent::DECODE_DOCUMENT);

    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    if (!request.filePath) return E_INVALIDARG;

    // Try Shell thumbnail first (works for Office docs, XPS, etc.)
    HRESULT hr = ExtractThumbnailShell(request.filePath, request.width,
                                        request.height, &result.hBitmap);

    // Fallback: document placeholder
    if (FAILED(hr) || !result.hBitmap) {
        const wchar_t* ext = PathFindExtensionW(request.filePath);
        result.hBitmap = CreateDocumentPlaceholder(request.width, request.height, ext);
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

DecoderInfo DocumentDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"Document Decoder";
    info.version = L"1.0.0";
    info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** DocumentDecoder::GetSupportedExtensions() const {
    return const_cast<const wchar_t**>(m_extensions);
}

// ============================================================================
// Shell Thumbnail Extraction
// ============================================================================

HRESULT DocumentDecoder::ExtractThumbnailShell(const wchar_t* filePath, uint32_t width,
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
// Document Placeholder
// ============================================================================

HBITMAP DocumentDecoder::CreateDocumentPlaceholder(uint32_t width, uint32_t height,
                                                    const wchar_t* ext) {
    Gdiplus::GdiplusStartupInput gdipInput;
    ULONG_PTR gdipToken = 0;
    if (Gdiplus::GdiplusStartup(&gdipToken, &gdipInput, nullptr) != Gdiplus::Ok)
        return nullptr;

    auto bmp = std::make_unique<Gdiplus::Bitmap>(width, height, PixelFormat32bppARGB);
    Gdiplus::Graphics g(bmp.get());
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    g.Clear(Gdiplus::Color(255, 255, 255, 255));

    float margin = width * 0.08f;
    float innerW = width - margin * 2;
    float innerH = height - margin * 2;

    // Page shadow
    Gdiplus::SolidBrush shadowBrush(Gdiplus::Color(40, 0, 0, 0));
    g.FillRectangle(&shadowBrush, margin + 3, margin + 3, innerW, innerH);

    // White page
    Gdiplus::SolidBrush pageBrush(Gdiplus::Color(255, 255, 255, 255));
    g.FillRectangle(&pageBrush, margin, margin, innerW, innerH);
    Gdiplus::Pen borderPen(Gdiplus::Color(200, 180, 180, 180), 1.0f);
    g.DrawRectangle(&borderPen, margin, margin, innerW, innerH);

    // Determine badge color and label based on extension
    Gdiplus::Color badgeColor(255, 50, 120, 200); // Default: blue
    std::wstring label = L"DOC";

    if (ext) {
        std::wstring e(ext);
        for (auto& c : e) c = towupper(c);

        if (e == L".EPUB") { badgeColor = Gdiplus::Color(255, 140, 90, 180); label = L"EPUB"; }
        else if (e == L".MOBI" || e == L".AZW" || e == L".AZW3") { badgeColor = Gdiplus::Color(255, 255, 153, 0); label = L"MOBI"; }
        else if (e == L".FB2") { badgeColor = Gdiplus::Color(255, 0, 150, 136); label = L"FB2"; }
        else if (e == L".DOCX" || e == L".DOC" || e == L".RTF" || e == L".ODT") { badgeColor = Gdiplus::Color(255, 40, 90, 180); label = L"DOC"; }
        else if (e == L".XLSX" || e == L".XLS" || e == L".ODS") { badgeColor = Gdiplus::Color(255, 33, 115, 70); label = L"XLS"; }
        else if (e == L".PPTX" || e == L".PPT" || e == L".ODP") { badgeColor = Gdiplus::Color(255, 210, 71, 38); label = L"PPT"; }
        else if (e == L".XPS" || e == L".OXPS") { badgeColor = Gdiplus::Color(255, 0, 120, 215); label = L"XPS"; }
        else if (e == L".DJVU" || e == L".DJV") { badgeColor = Gdiplus::Color(255, 60, 60, 60); label = L"DJVU"; }
        else { label = e.substr(1); }  // Remove dot
    }

    // Badge
    float badgeW = innerW * 0.55f;
    float badgeH = innerH * 0.18f;
    float badgeX = margin + (innerW - badgeW) / 2;
    float badgeY = margin + innerH * 0.25f;
    Gdiplus::SolidBrush badgeBrush(badgeColor);
    g.FillRectangle(&badgeBrush, badgeX, badgeY, badgeW, badgeH);

    // Badge text
    Gdiplus::FontFamily fontFamily(L"Arial");
    float fontSize = badgeH * 0.60f;
    if (fontSize < 8) fontSize = 8;
    Gdiplus::Font badgeFont(&fontFamily, fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::StringFormat centerFmt;
    centerFmt.SetAlignment(Gdiplus::StringAlignmentCenter);
    centerFmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    Gdiplus::RectF badgeRect(badgeX, badgeY, badgeW, badgeH);
    g.DrawString(label.c_str(), -1, &badgeFont, badgeRect, &centerFmt, &whiteBrush);

    // Simulated text lines
    Gdiplus::SolidBrush lineBrush(Gdiplus::Color(50, 100, 100, 100));
    float lineY = badgeY + badgeH + innerH * 0.08f;
    for (int i = 0; i < 5 && lineY + 2 < margin + innerH - 10; i++) {
        float lineW = innerW * (0.5f + (i % 3) * 0.12f);
        g.FillRectangle(&lineBrush, margin + 10, lineY, lineW, 2.0f);
        lineY += innerH * 0.05f;
    }

    HBITMAP hBitmap = nullptr;
    bmp->GetHBITMAP(Gdiplus::Color(255, 255, 255), &hBitmap);
    Gdiplus::GdiplusShutdown(gdipToken);
    return hBitmap;
}

// ============================================================================
// Format Detection
// ============================================================================

bool DocumentDecoder::IsDocumentFormat(const wchar_t* path) {
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
