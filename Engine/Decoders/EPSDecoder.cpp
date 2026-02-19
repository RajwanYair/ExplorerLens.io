//==============================================================================
// EPS Decoder - Encapsulated PostScript Thumbnail Provider Implementation
// Sprint 183: Vector Format Expansion
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include "EPSDecoder.h"
#include <windows.h>
#include <gdiplus.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cwchar>
#include <algorithm>
#include <string>

#pragma comment(lib, "gdiplus.lib")

namespace DarkThumbs {
namespace Engine {

    static const uint32_t EPS_BINARY_MAGIC = 0xC5D0D3C6;

    const wchar_t* EPSDecoder::s_extensions[] = {
        L".eps", L".epsf", L".ps", nullptr
    };
    const uint32_t EPSDecoder::s_extensionCount = 3;

    EPSDecoder::EPSDecoder() = default;
    EPSDecoder::~EPSDecoder() = default;

    bool EPSDecoder::CanDecode(const wchar_t* filePath)
    {
        if (!filePath) return false;
        const wchar_t* ext = wcsrchr(filePath, L'.');
        if (!ext) return false;

        return (_wcsicmp(ext, L".eps") == 0 ||
                _wcsicmp(ext, L".epsf") == 0 ||
                _wcsicmp(ext, L".ps") == 0);
    }

    const wchar_t** EPSDecoder::GetSupportedExtensions() const
    {
        return s_extensions;
    }

    DecoderInfo EPSDecoder::GetInfo() const
    {
        DecoderInfo info;
        info.name = L"EPSDecoder";
        info.version = L"1.0.0";
        info.supportedExtensions = s_extensions;
        info.extensionCount = s_extensionCount;
        info.supportsGPU = false;
        info.isArchiveDecoder = false;
        return info;
    }

    const wchar_t* EPSDecoder::GetName() const { return L"EPSDecoder"; }
    uint32_t EPSDecoder::GetExtensionCount() const { return s_extensionCount; }
    bool EPSDecoder::SupportsGPU() const { return false; }
    bool EPSDecoder::IsArchiveDecoder() const { return false; }

    HRESULT EPSDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
    {
        result.hBitmap = nullptr;
        result.width = 0;
        result.height = 0;
        result.status = E_FAIL;
        result.usedGPU = false;

        if (!request.filePath) return E_INVALIDARG;

        // Read file data
        size_t fileSize = 0;
        auto data = ReadFileData(request.filePath, fileSize);
        if (!data || fileSize < 30) return E_FAIL;

        float bbWidth = 0, bbHeight = 0;

        // Check for EPS binary header (DOS EPS format)
        if (fileSize >= sizeof(EPSBinaryHeader)) {
            const auto* header = reinterpret_cast<const EPSBinaryHeader*>(data.get());

            if (header->magic == EPS_BINARY_MAGIC) {
                // Try TIFF preview first (better quality)
                if (header->tiffOffset > 0 && header->tiffLength > 0) {
                    result.hBitmap = ExtractTIFFPreview(data.get(), fileSize,
                                                         header->tiffOffset, header->tiffLength,
                                                         request.width, request.height);
                }

                // Try WMF preview if TIFF not available
                if (!result.hBitmap && header->wmfOffset > 0 && header->wmfLength > 0) {
                    result.hBitmap = ExtractWMFPreview(data.get(), fileSize,
                                                        header->wmfOffset, header->wmfLength,
                                                        request.width, request.height);
                }

                // Parse BoundingBox from PS section
                if (header->psOffset > 0 && header->psLength > 0 &&
                    header->psOffset + header->psLength <= fileSize) {
                    ParseBoundingBox(reinterpret_cast<const char*>(data.get() + header->psOffset),
                                     header->psLength, bbWidth, bbHeight, bbWidth, bbHeight);
                    // bbWidth/bbHeight will be overwritten properly below
                    float llx, lly, urx, ury;
                    if (ParseBoundingBox(reinterpret_cast<const char*>(data.get() + header->psOffset),
                                          header->psLength, llx, lly, urx, ury)) {
                        bbWidth = urx - llx;
                        bbHeight = ury - lly;
                    }
                }
            }
        }

        // For ASCII EPS/PS: try to parse BoundingBox for placeholder
        if (!result.hBitmap) {
            if (fileSize >= 2 && data.get()[0] == '%' && data.get()[1] == '!') {
                // ASCII PostScript
                float llx, lly, urx, ury;
                if (ParseBoundingBox(reinterpret_cast<const char*>(data.get()),
                                      std::min(fileSize, static_cast<size_t>(4096)),
                                      llx, lly, urx, ury)) {
                    bbWidth = urx - llx;
                    bbHeight = ury - lly;
                }
            }
        }

        // Fallback: create informative placeholder
        if (!result.hBitmap) {
            result.hBitmap = CreateEPSPlaceholder(request.width, request.height,
                                                    request.filePath, bbWidth, bbHeight);
        }

        if (result.hBitmap) {
            result.status = S_OK;
            result.width = request.width;
            result.height = request.height;
            return S_OK;
        }

        return E_FAIL;
    }

    HBITMAP EPSDecoder::ExtractTIFFPreview(const uint8_t* data, size_t size,
                                            uint32_t tiffOffset, uint32_t tiffLength,
                                            uint32_t width, uint32_t height)
    {
        if (static_cast<size_t>(tiffOffset) + tiffLength > size) return nullptr;

        // Use GDI+ to decode the embedded TIFF
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, tiffLength);
        if (!hMem) return nullptr;

        void* pMem = GlobalLock(hMem);
        if (!pMem) { GlobalFree(hMem); return nullptr; }
        memcpy(pMem, data + tiffOffset, tiffLength);
        GlobalUnlock(hMem);

        IStream* pStream = nullptr;
        if (FAILED(CreateStreamOnHGlobal(hMem, TRUE, &pStream))) {
            GlobalFree(hMem);
            return nullptr;
        }

        Gdiplus::Bitmap* gpBitmap = Gdiplus::Bitmap::FromStream(pStream);
        HBITMAP hBitmap = nullptr;

        if (gpBitmap && gpBitmap->GetLastStatus() == Gdiplus::Ok) {
            // Scale to requested size
            Gdiplus::Bitmap scaled(width, height, PixelFormat32bppARGB);
            Gdiplus::Graphics g(&scaled);
            g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            g.Clear(Gdiplus::Color(255, 255, 255, 255));

            float scaleX = static_cast<float>(width) / gpBitmap->GetWidth();
            float scaleY = static_cast<float>(height) / gpBitmap->GetHeight();
            float scale = (std::min)(scaleX, scaleY);
            int dw = static_cast<int>(gpBitmap->GetWidth() * scale);
            int dh = static_cast<int>(gpBitmap->GetHeight() * scale);
            int dx = (width - dw) / 2;
            int dy = (height - dh) / 2;

            g.DrawImage(gpBitmap, dx, dy, dw, dh);
            scaled.GetHBITMAP(Gdiplus::Color(255, 255, 255, 255), &hBitmap);
        }

        delete gpBitmap;
        pStream->Release();

        return hBitmap;
    }

    HBITMAP EPSDecoder::ExtractWMFPreview(const uint8_t* data, size_t size,
                                           uint32_t wmfOffset, uint32_t wmfLength,
                                           uint32_t width, uint32_t height)
    {
        if (static_cast<size_t>(wmfOffset) + wmfLength > size) return nullptr;

        // Use GDI+ to render the embedded WMF
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, wmfLength);
        if (!hMem) return nullptr;

        void* pMem = GlobalLock(hMem);
        if (!pMem) { GlobalFree(hMem); return nullptr; }
        memcpy(pMem, data + wmfOffset, wmfLength);
        GlobalUnlock(hMem);

        IStream* pStream = nullptr;
        if (FAILED(CreateStreamOnHGlobal(hMem, TRUE, &pStream))) {
            GlobalFree(hMem);
            return nullptr;
        }

        Gdiplus::Metafile metafile(pStream);
        HBITMAP hBitmap = nullptr;

        if (metafile.GetLastStatus() == Gdiplus::Ok) {
            Gdiplus::Bitmap bitmap(width, height, PixelFormat32bppARGB);
            Gdiplus::Graphics g(&bitmap);
            g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            g.Clear(Gdiplus::Color(255, 255, 255, 255));

            float scaleX = static_cast<float>(width) / metafile.GetWidth();
            float scaleY = static_cast<float>(height) / metafile.GetHeight();
            float scale = (std::min)(scaleX, scaleY);
            int dw = static_cast<int>(metafile.GetWidth() * scale);
            int dh = static_cast<int>(metafile.GetHeight() * scale);
            int dx = (width - dw) / 2;
            int dy = (height - dh) / 2;

            g.DrawImage(&metafile, dx, dy, dw, dh);
            bitmap.GetHBITMAP(Gdiplus::Color(255, 255, 255, 255), &hBitmap);
        }

        pStream->Release();
        return hBitmap;
    }

    bool EPSDecoder::ParseBoundingBox(const char* psText, size_t length,
                                       float& llx, float& lly, float& urx, float& ury)
    {
        // Look for %%BoundingBox: llx lly urx ury
        std::string text(psText, std::min(length, static_cast<size_t>(8192)));
        size_t pos = text.find("%%BoundingBox:");
        if (pos == std::string::npos) return false;

        // Skip "(atend)" responses
        std::string bbLine = text.substr(pos + 14, 100);
        if (bbLine.find("(atend)") != std::string::npos) {
            // Look for HiResBoundingBox instead
            pos = text.find("%%HiResBoundingBox:");
            if (pos == std::string::npos) return false;
            bbLine = text.substr(pos + 19, 100);
        }

        std::istringstream iss(bbLine);
        iss >> llx >> lly >> urx >> ury;
        return !iss.fail() && urx > llx && ury > lly;
    }

    HBITMAP EPSDecoder::CreateEPSPlaceholder(uint32_t width, uint32_t height,
                                               const wchar_t* filePath,
                                               float bbWidth, float bbHeight)
    {
        HDC hdc = GetDC(NULL);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        // White background with light gray border
        RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        HBRUSH bgBrush = CreateSolidBrush(RGB(248, 248, 248));
        FillRect(memDC, &rect, bgBrush);
        DeleteObject(bgBrush);

        // Draw page-like border
        HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(180, 180, 180));
        HPEN oldPen = (HPEN)SelectObject(memDC, borderPen);

        int margin = static_cast<int>(width * 0.08f);
        RECT pageRect = { margin, margin,
                          static_cast<LONG>(width) - margin,
                          static_cast<LONG>(height) - margin };
        Rectangle(memDC, pageRect.left, pageRect.top, pageRect.right, pageRect.bottom);

        // Draw diagonal placeholder lines (indicating vector content)
        HPEN diagPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 220));
        SelectObject(memDC, diagPen);

        int lineSpacing = static_cast<int>(width * 0.12f);
        for (int i = 0; i < 8; ++i) {
            int offset = margin + lineSpacing + i * lineSpacing;
            MoveToEx(memDC, pageRect.left + 5, offset, NULL);
            LineTo(memDC, pageRect.right - 5, offset);
        }

        SelectObject(memDC, oldPen);
        DeleteObject(diagPen);
        DeleteObject(borderPen);

        // Draw "EPS" label and size info
        HFONT hFont = CreateFontW(
            static_cast<int>(height * 0.12f), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT oldFont = (HFONT)SelectObject(memDC, hFont);
        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, RGB(100, 100, 120));

        // Format label
        const wchar_t* ext = filePath ? wcsrchr(filePath, L'.') : nullptr;
        const wchar_t* label = L"EPS";
        if (ext && _wcsicmp(ext, L".ps") == 0) label = L"PS";

        RECT textRect = pageRect;
        textRect.top = pageRect.top + static_cast<int>(height * 0.15f);
        DrawTextW(memDC, label, -1, &textRect, DT_CENTER | DT_SINGLELINE);

        // Show dimensions if BoundingBox was found
        if (bbWidth > 0 && bbHeight > 0) {
            HFONT smallFont = CreateFontW(
                static_cast<int>(height * 0.07f), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            SelectObject(memDC, smallFont);
            SetTextColor(memDC, RGB(140, 140, 160));

            wchar_t sizeStr[64];
            swprintf_s(sizeStr, L"%.0f × %.0f pt", bbWidth, bbHeight);
            textRect.top = pageRect.bottom - static_cast<int>(height * 0.15f);
            DrawTextW(memDC, sizeStr, -1, &textRect, DT_CENTER | DT_SINGLELINE);

            DeleteObject(smallFont);
        }

        SelectObject(memDC, oldFont);
        DeleteObject(hFont);

        SelectObject(memDC, hOldBitmap);
        DeleteDC(memDC);
        ReleaseDC(NULL, hdc);

        return hBitmap;
    }

    std::unique_ptr<uint8_t[]> EPSDecoder::ReadFileData(const wchar_t* filePath, size_t& outSize)
    {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            outSize = 0;
            return nullptr;
        }

        outSize = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        auto data = std::make_unique<uint8_t[]>(outSize);
        file.read(reinterpret_cast<char*>(data.get()), outSize);

        return data;
    }

} // namespace Engine
} // namespace DarkThumbs
