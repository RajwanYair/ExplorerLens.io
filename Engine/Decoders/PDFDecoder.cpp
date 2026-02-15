// PDFDecoder.cpp - PDF Document Thumbnail Decoder Implementation
// DarkThumbs Engine v6.2.0+
// Copyright (c) 2026 DarkThumbs Project

#include "PDFDecoder.h"
#include "../Utils/PerformanceProfiler.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <thumbcache.h>
#include <cwchar>
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

namespace DarkThumbs {
namespace Engine {

const wchar_t* PDFDecoder::m_extensions[] = { L".pdf", nullptr };
const uint32_t PDFDecoder::m_extensionCount = 1;

PDFDecoder::PDFDecoder() = default;
PDFDecoder::~PDFDecoder() = default;

bool PDFDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath) return false;
    return IsPDFFormat(filePath);
}

HRESULT PDFDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    PROFILE_SCOPE(ProfileComponent::DECODE_PDF);

    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    if (!request.filePath) return E_INVALIDARG;

    // Try Shell thumbnail provider (works if Edge/Acrobat/Foxit installed)
    HRESULT hr = ExtractThumbnailShell(request.filePath, request.width,
                                        request.height, &result.hBitmap);

    // Fallback: placeholder
    if (FAILED(hr) || !result.hBitmap) {
        result.hBitmap = CreatePDFPlaceholder(request.width, request.height, request.filePath);
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

DecoderInfo PDFDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"PDF Decoder";
    info.version = L"1.0.0";
    info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** PDFDecoder::GetSupportedExtensions() const {
    return const_cast<const wchar_t**>(m_extensions);
}

// ============================================================================
// Shell Thumbnail Extraction
// ============================================================================

HRESULT PDFDecoder::ExtractThumbnailShell(const wchar_t* filePath, uint32_t width,
                                            uint32_t height, HBITMAP* phBitmap) {
    if (!phBitmap) return E_INVALIDARG;
    *phBitmap = nullptr;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
    if (!comInit) return hr;

    // Use IShellItemImageFactory for high-quality extraction
    IShellItem* pItem = nullptr;
    hr = SHCreateItemFromParsingName(filePath, nullptr, IID_PPV_ARGS(&pItem));
    if (FAILED(hr)) {
        CoUninitialize();
        return hr;
    }

    IShellItemImageFactory* pFactory = nullptr;
    hr = pItem->QueryInterface(IID_PPV_ARGS(&pFactory));
    if (SUCCEEDED(hr) && pFactory) {
        SIZE sz = { static_cast<LONG>(width), static_cast<LONG>(height) };
        hr = pFactory->GetImage(sz, SIIGBF_THUMBNAILONLY | SIIGBF_BIGGERSIZEOK, phBitmap);

        // If thumbnail-only fails, try with icon fallback
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
// PDF Placeholder
// ============================================================================

HBITMAP PDFDecoder::CreatePDFPlaceholder(uint32_t width, uint32_t height,
                                          const wchar_t* filePath) {
    (void)filePath; // Reserved for future filename display on placeholder
    Gdiplus::GdiplusStartupInput gdipInput;
    ULONG_PTR gdipToken = 0;
    if (Gdiplus::GdiplusStartup(&gdipToken, &gdipInput, nullptr) != Gdiplus::Ok)
        return nullptr;

    auto bmp = std::make_unique<Gdiplus::Bitmap>(width, height, PixelFormat32bppARGB);
    Gdiplus::Graphics g(bmp.get());
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    // White page background
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

    // Page border
    Gdiplus::Pen borderPen(Gdiplus::Color(200, 180, 180, 180), 1.0f);
    g.DrawRectangle(&borderPen, margin, margin, innerW, innerH);

    // PDF icon area (red badge)
    float badgeW = innerW * 0.5f;
    float badgeH = innerH * 0.18f;
    float badgeX = margin + (innerW - badgeW) / 2;
    float badgeY = margin + innerH * 0.25f;
    Gdiplus::SolidBrush redBrush(Gdiplus::Color(255, 220, 50, 50));
    g.FillRectangle(&redBrush, badgeX, badgeY, badgeW, badgeH);

    // "PDF" text on badge
    Gdiplus::FontFamily fontFamily(L"Arial");
    float fontSize = badgeH * 0.65f;
    if (fontSize < 8) fontSize = 8;
    Gdiplus::Font pdfFont(&fontFamily, fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::StringFormat centerFmt;
    centerFmt.SetAlignment(Gdiplus::StringAlignmentCenter);
    centerFmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    Gdiplus::RectF badgeRect(badgeX, badgeY, badgeW, badgeH);
    g.DrawString(L"PDF", 3, &pdfFont, badgeRect, &centerFmt, &whiteBrush);

    // Simulated text lines
    Gdiplus::SolidBrush lineBrush(Gdiplus::Color(60, 100, 100, 100));
    float lineY = badgeY + badgeH + innerH * 0.08f;
    float lineH = 2.0f;
    for (int i = 0; i < 6 && lineY + lineH < margin + innerH - 10; i++) {
        float lineW = innerW * (0.6f + (i % 3) * 0.1f);
        g.FillRectangle(&lineBrush, margin + 10, lineY, lineW, lineH);
        lineY += lineH + innerH * 0.04f;
    }

    HBITMAP hBitmap = nullptr;
    bmp->GetHBITMAP(Gdiplus::Color(255, 255, 255), &hBitmap);
    Gdiplus::GdiplusShutdown(gdipToken);
    return hBitmap;
}

// ============================================================================
// Format Detection
// ============================================================================

bool PDFDecoder::IsPDFFormat(const wchar_t* path) {
    if (!path) return false;
    const wchar_t* ext = PathFindExtensionW(path);
    if (ext && _wcsicmp(ext, L".pdf") == 0) return true;

    // Also check signature
    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ,
                                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    uint8_t sig[5] = {};
    DWORD bytesRead = 0;
    ReadFile(hFile, sig, 5, &bytesRead, nullptr);
    CloseHandle(hFile);
    // %PDF-
    return bytesRead >= 5 && sig[0] == 0x25 && sig[1] == 0x50 &&
           sig[2] == 0x44 && sig[3] == 0x46 && sig[4] == 0x2D;
}

} // namespace Engine
} // namespace DarkThumbs
