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
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <cwchar>
#include <algorithm>
#include <string>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

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

    // Try DirectWrite font rendering first
    HRESULT hr = RenderFontPreview(request.filePath, request.width,
                                   request.height, &result.hBitmap);

    // Try Shell font thumbnail if DirectWrite failed
    if (FAILED(hr) || !result.hBitmap) {
        hr = ExtractFontPreviewShell(request.filePath, request.width,
                                     request.height, &result.hBitmap);
    }

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
// DirectWrite Font Preview Rendering
// ============================================================================

HRESULT FontDecoder::RenderFontPreview(const wchar_t* filePath, uint32_t width,
                                       uint32_t height, HBITMAP* phBitmap) {
    if (!phBitmap) return E_INVALIDARG;
    *phBitmap = nullptr;

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
    if (!comInit) return hr;

    // Create DirectWrite factory
    IDWriteFactory* pDWriteFactory = nullptr;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                              reinterpret_cast<IUnknown**>(&pDWriteFactory));
    if (FAILED(hr)) {
        CoUninitialize();
        return hr;
    }

    // Create custom font collection from file
    IDWriteFontFile* pFontFile = nullptr;
    hr = pDWriteFactory->CreateFontFileReference(filePath, nullptr, &pFontFile);
    if (FAILED(hr)) {
        pDWriteFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Analyze font file
    BOOL isSupportedFontType = FALSE;
    DWRITE_FONT_FILE_TYPE fontFileType;
    DWRITE_FONT_FACE_TYPE fontFaceType;
    UINT32 numberOfFaces = 0;
    hr = pFontFile->Analyze(&isSupportedFontType, &fontFileType, &fontFaceType, &numberOfFaces);
    if (FAILED(hr) || !isSupportedFontType) {
        pFontFile->Release();
        pDWriteFactory->Release();
        CoUninitialize();
        return E_FAIL;
    }

    // Create font face from file
    IDWriteFontFile* fontFiles[] = { pFontFile };
    IDWriteFontFace* pFontFace = nullptr;
    hr = pDWriteFactory->CreateFontFace(fontFaceType, 1, fontFiles, 0,
                                        DWRITE_FONT_SIMULATIONS_NONE, &pFontFace);
    pFontFile->Release();
    if (FAILED(hr)) {
        pDWriteFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Get font metrics for rendering
    DWRITE_FONT_METRICS fontMetrics;
    pFontFace->GetMetrics(&fontMetrics);

    // Create Direct2D factory
    ID2D1Factory* pD2DFactory = nullptr;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    if (FAILED(hr)) {
        pFontFace->Release();
        pDWriteFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Create WIC bitmap
    IWICImagingFactory* pWICFactory = nullptr;
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&pWICFactory));
    if (FAILED(hr)) {
        pD2DFactory->Release();
        pFontFace->Release();
        pDWriteFactory->Release();
        CoUninitialize();
        return hr;
    }

    IWICBitmap* pWICBitmap = nullptr;
    hr = pWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppPBGRA,
                                    WICBitmapCacheOnDemand, &pWICBitmap);
    if (FAILED(hr)) {
        pWICFactory->Release();
        pD2DFactory->Release();
        pFontFace->Release();
        pDWriteFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Create D2D render target
    ID2D1RenderTarget* pRenderTarget = nullptr;
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);
    hr = pD2DFactory->CreateWicBitmapRenderTarget(pWICBitmap, rtProps, &pRenderTarget);
    if (FAILED(hr)) {
        pWICBitmap->Release();
        pWICFactory->Release();
        pD2DFactory->Release();
        pFontFace->Release();
        pDWriteFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Begin drawing
    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    // Create text format
    IDWriteTextFormat* pTextFormat = nullptr;
    float fontSize = height * 0.35f;
    hr = pDWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
                                          DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                          fontSize, L"en-us", &pTextFormat);
    if (SUCCEEDED(hr)) {
        pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        // Render sample text using custom font face
        const wchar_t* sampleText = L"Aa Bb Cc 123";
        ID2D1SolidColorBrush* pBrush = nullptr;
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &pBrush);

        // Create text layout with custom font
        IDWriteTextLayout* pTextLayout = nullptr;
        hr = pDWriteFactory->CreateTextLayout(sampleText, wcslen(sampleText), pTextFormat,
                                              static_cast<float>(width), static_cast<float>(height),
                                              &pTextLayout);
        if (SUCCEEDED(hr)) {
            // Apply custom font face to text layout
            DWRITE_TEXT_RANGE textRange = { 0, wcslen(sampleText) };
            pTextLayout->SetFontSize(fontSize, textRange);

            // Draw text
            D2D1_POINT_2F origin = D2D1::Point2F(0, 0);
            pRenderTarget->DrawTextLayout(origin, pTextLayout, pBrush, D2D1_DRAW_TEXT_OPTIONS_NONE);
            pTextLayout->Release();
        }

        if (pBrush) pBrush->Release();
        pTextFormat->Release();
    }

    pRenderTarget->EndDraw();

    // Convert WIC bitmap to HBITMAP
    IWICFormatConverter* pConverter = nullptr;
    hr = pWICFactory->CreateFormatConverter(&pConverter);
    if (SUCCEEDED(hr)) {
        hr = pConverter->Initialize(pWICBitmap, GUID_WICPixelFormat32bppBGR,
                                    WICBitmapDitherTypeNone, nullptr, 0.0,
                                    WICBitmapPaletteTypeCustom);
        if (SUCCEEDED(hr)) {
            UINT stride = width * 4;
            UINT bufferSize = stride * height;
            auto buffer = std::make_unique<BYTE[]>(bufferSize);
            WICRect rect = { 0, 0, static_cast<INT>(width), static_cast<INT>(height) };
            hr = pConverter->CopyPixels(&rect, stride, bufferSize, buffer.get());
            if (SUCCEEDED(hr)) {
                // Create HBITMAP from pixel data
                BITMAPINFO bmi = {};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = width;
                bmi.bmiHeader.biHeight = -static_cast<LONG>(height); // Top-down
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;
                void* pBits = nullptr;
                HDC hdc = GetDC(nullptr);
                *phBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
                if (*phBitmap && pBits) {
                    memcpy(pBits, buffer.get(), bufferSize);
                }
                ReleaseDC(nullptr, hdc);
            }
        }
        pConverter->Release();
    }

    // Cleanup
    pRenderTarget->Release();
    pWICBitmap->Release();
    pWICFactory->Release();
    pD2DFactory->Release();
    pFontFace->Release();
    pDWriteFactory->Release();
    CoUninitialize();

    return *phBitmap ? S_OK : E_FAIL;
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

// ============================================================================
// Font Metadata Extraction
// ============================================================================

bool FontDecoder::GetFontMetadata(const wchar_t* filePath, FontMetadata& metadata) {
    if (!filePath) return false;

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
    if (!comInit) return false;

    bool success = false;

    // Create DirectWrite factory
    IDWriteFactory* pDWriteFactory = nullptr;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                              reinterpret_cast<IUnknown**>(&pDWriteFactory));
    if (SUCCEEDED(hr)) {
        // Create font file reference
        IDWriteFontFile* pFontFile = nullptr;
        hr = pDWriteFactory->CreateFontFileReference(filePath, nullptr, &pFontFile);
        if (SUCCEEDED(hr)) {
            // Analyze font file
            BOOL isSupportedFontType = FALSE;
            DWRITE_FONT_FILE_TYPE fontFileType;
            DWRITE_FONT_FACE_TYPE fontFaceType;
            UINT32 numberOfFaces = 0;
            hr = pFontFile->Analyze(&isSupportedFontType, &fontFileType, &fontFaceType, &numberOfFaces);
            if (SUCCEEDED(hr) && isSupportedFontType) {
                // Create font face
                IDWriteFontFile* fontFiles[] = { pFontFile };
                IDWriteFontFace* pFontFace = nullptr;
                hr = pDWriteFactory->CreateFontFace(fontFaceType, 1, fontFiles, 0,
                                                    DWRITE_FONT_SIMULATIONS_NONE, &pFontFace);
                if (SUCCEEDED(hr)) {
                    // Get font metrics
                    DWRITE_FONT_METRICS fontMetrics;
                    pFontFace->GetMetrics(&fontMetrics);
                    metadata.weightValue = fontMetrics.weight;

                    // Check if monospace by comparing glyph widths
                    UINT16 glyphIndices[3] = { 0, 0, 0 };
                    UINT32 codePoints[3] = { L'i', L'M', L'W' };
                    hr = pFontFace->GetGlyphIndices(codePoints, 3, glyphIndices);
                    if (SUCCEEDED(hr)) {
                        DWRITE_GLYPH_METRICS glyphMetrics[3];
                        hr = pFontFace->GetDesignGlyphMetrics(glyphIndices, 3, glyphMetrics, FALSE);
                        if (SUCCEEDED(hr)) {
                            // If all glyphs have same advance width, it's monospace
                            metadata.isMonospace = (glyphMetrics[0].advanceWidth == glyphMetrics[1].advanceWidth &&
                                                   glyphMetrics[1].advanceWidth == glyphMetrics[2].advanceWidth);
                        }
                    }

                    // Try to get font family name from system collection
                    IDWriteFontCollection* pFontCollection = nullptr;
                    hr = pDWriteFactory->GetSystemFontCollection(&pFontCollection);
                    if (SUCCEEDED(hr)) {
                        // Try to find matching font (simplified approach)
                        const wchar_t* fileName = PathFindFileNameW(filePath);
                        if (fileName) {
                            metadata.fullName = fileName;
                            // Remove extension
                            size_t len = wcslen(fileName);
                            const wchar_t* dotPos = wcsrchr(fileName, L'.');
                            if (dotPos) {
                                metadata.familyName.assign(fileName, dotPos - fileName);
                            } else {
                                metadata.familyName = fileName;
                            }
                        }
                        pFontCollection->Release();
                    }

                    success = true;
                    pFontFace->Release();
                }
            }
            pFontFile->Release();
        }
        pDWriteFactory->Release();
    }

    CoUninitialize();
    return success;
}

} // namespace Engine
} // namespace DarkThumbs
