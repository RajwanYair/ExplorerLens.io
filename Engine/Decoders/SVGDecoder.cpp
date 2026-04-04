// SVGDecoder.cpp - SVG/SVGZ Vector Image Decoder Implementation
// ExplorerLens Engine v6.2.0+
// Copyright (c) 2026 ExplorerLens Project

#include "SVGDecoder.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <algorithm>
#include <cstring>
#include <cwchar>
#include <fstream>
#include <regex>
#include "../Utils/PerformanceProfiler.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

namespace ExplorerLens {
namespace Engine {

// Supported extensions
const wchar_t* SVGDecoder::m_extensions[] = {L".svg", L".svgz", nullptr};
const uint32_t SVGDecoder::m_extensionCount = 2;

SVGDecoder::SVGDecoder() = default;
SVGDecoder::~SVGDecoder() = default;

bool SVGDecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath)
        return false;
    return IsSVGFormat(filePath) || IsSVGZFormat(filePath);
}

HRESULT SVGDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    PROFILE_SCOPE(ProfileComponent::DECODE_SVG);

    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    if (!request.filePath)
        return E_INVALIDARG;

    HRESULT hr = RenderSVGToHBITMAP(request.filePath, request.width, request.height, &result.hBitmap);
    if (SUCCEEDED(hr) && result.hBitmap) {
        BITMAP bm;
        if (GetObject(result.hBitmap, sizeof(bm), &bm)) {
            result.width = bm.bmWidth;
            result.height = bm.bmHeight;
        }
    }
    return hr;
}

DecoderInfo SVGDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name = L"SVG Decoder";
    info.version = L"1.0.0";
    info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** SVGDecoder::GetSupportedExtensions() const
{
    return const_cast<const wchar_t**>(m_extensions);
}

// ============================================================================
// Private Implementation
// ============================================================================

HRESULT SVGDecoder::RenderSVGToHBITMAP(const wchar_t* filePath, uint32_t width, uint32_t height, HBITMAP* phBitmap)
{
    if (!filePath || !phBitmap)
        return E_INVALIDARG;
    *phBitmap = nullptr;

    // Read file
    size_t fileSize = 0;
    auto fileData = ReadFileData(filePath, fileSize);
    if (!fileData || fileSize == 0)
        return E_FAIL;

    std::string svgContent;

    // Check if SVGZ (gzip compressed)
    if (IsSVGZFormat(filePath) || (fileSize >= 2 && fileData[0] == 0x1F && fileData[1] == 0x8B)) {
        std::vector<uint8_t> compressed(fileData.get(), fileData.get() + fileSize);
        if (!DecompressSVGZ(compressed, svgContent)) {
            return E_FAIL;
        }
    } else {
        svgContent.assign(reinterpret_cast<char*>(fileData.get()), fileSize);
    }

    // Validate it's actually SVG
    if (svgContent.find("<svg") == std::string::npos) {
        return DT_E_UNSUPPORTED_FORMAT;
    }

    // Create placeholder thumbnail with SVG representation
    *phBitmap = CreateSVGPlaceholder(width, height, svgContent);
    if (!*phBitmap)
        return E_FAIL;

    return S_OK;
}

bool SVGDecoder::DecompressSVGZ(const std::vector<uint8_t>& compressed, std::string& svgContent)
{
    (void)svgContent;  // Output parameter, set on success
    // Simple gzip decompression using Windows Compression API (Win8+)
    // or manual inflate. For now use a buffer-based approach.

    // Check gzip magic bytes
    if (compressed.size() < 10 || compressed[0] != 0x1F || compressed[1] != 0x8B) {
        return false;
    }

    // Use Windows built-in decompression (RtlDecompressBuffer)
    // Or fall back to manual deflate stream parsing
    // For robust implementation, we can use the zlib already linked

    // Try RtlDecompressBuffer (NT API, available on all modern Windows)
    typedef LONG(WINAPI * RtlDecompressBufferFn)(USHORT CompressionFormat, PUCHAR UncompressedBuffer,
                                                 ULONG UncompressedBufferSize, PUCHAR CompressedBuffer,
                                                 ULONG CompressedBufferSize, PULONG FinalUncompressedSize);

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll)
        return false;

    auto RtlDecompressBuffer = reinterpret_cast<RtlDecompressBufferFn>(GetProcAddress(hNtdll, "RtlDecompressBuffer"));
    if (!RtlDecompressBuffer)
        return false;

    // Skip gzip header to get to raw deflate data
    size_t offset = 10;  // Fixed gzip header size
    uint8_t flags = compressed[3];
    if (flags & 0x04) {  // FEXTRA
        if (offset + 2 > compressed.size())
            return false;
        uint16_t xlen = compressed[offset] | (compressed[offset + 1] << 8);
        offset += 2 + xlen;
    }
    if (flags & 0x08) {  // FNAME - skip null-terminated string
        while (offset < compressed.size() && compressed[offset] != 0)
            offset++;
        offset++;
    }
    if (flags & 0x10) {  // FCOMMENT - skip null-terminated string
        while (offset < compressed.size() && compressed[offset] != 0)
            offset++;
        offset++;
    }
    if (flags & 0x02) {  // FHCRC
        offset += 2;
    }

    if (offset >= compressed.size())
        return false;

    // Allocate output buffer - SVGs typically decompress to 5-20x
    const size_t maxDecompressed = compressed.size() * 30;
    std::vector<uint8_t> output(maxDecompressed);

    // Use COMPRESSION_FORMAT_LZNT1 isn't right for gzip (deflate)
    // RtlDecompressBuffer doesn't support gzip/deflate natively.
    // Fall back to reading uncompressed SVG content after gzip header
    // by trying a simpler approach - write temp file and use WinAPI

    // Alternative: Use the raw deflate data with a basic inflate
    // For now, attempt to read the original uncompressed size from gzip footer
    if (compressed.size() >= 4) {
        uint32_t uncompressedSize = compressed[compressed.size() - 4] | (compressed[compressed.size() - 3] << 8)
                                    | (compressed[compressed.size() - 2] << 16)
                                    | (compressed[compressed.size() - 1] << 24);

        // Sanity check - don't allocate more than 100MB
        if (uncompressedSize > 0 && uncompressedSize < 100 * 1024 * 1024) {
            // We'll need zlib for proper decompression
            // For now, return failure to signal SVGZ needs zlib
            // The raw deflate data starts at offset
        }
    }

    // Without zlib linked, we can't decompress SVGZ
    // Return false and let the caller handle it
    // When zlib is available, use inflate() here
    return false;
}

bool SVGDecoder::ExtractSVGDimensions(const std::string& svgContent, uint32_t& outWidth, uint32_t& outHeight)
{
    outWidth = 0;
    outHeight = 0;

    // Try viewBox first: viewBox="0 0 width height"
    auto vbPos = svgContent.find("viewBox");
    if (vbPos != std::string::npos) {
        auto quote = svgContent.find('"', vbPos);
        if (quote != std::string::npos) {
            auto endQuote = svgContent.find('"', quote + 1);
            if (endQuote != std::string::npos) {
                std::string vb = svgContent.substr(quote + 1, endQuote - quote - 1);
                float x, y, w, h;
                if (sscanf_s(vb.c_str(), "%f %f %f %f", &x, &y, &w, &h) == 4) {
                    outWidth = static_cast<uint32_t>(w);
                    outHeight = static_cast<uint32_t>(h);
                    return true;
                }
            }
        }
    }

    // Try width/height attributes
    auto parseAttr = [&](const std::string& name) -> uint32_t {
        auto pos = svgContent.find(name + "=");
        if (pos != std::string::npos) {
            auto q = svgContent.find('"', pos);
            if (q != std::string::npos) {
                return static_cast<uint32_t>(atof(svgContent.c_str() + q + 1));
            }
        }
        return 0;
    };

    outWidth = parseAttr("width");
    outHeight = parseAttr("height");
    return outWidth > 0 && outHeight > 0;
}

HBITMAP SVGDecoder::CreateSVGPlaceholder(uint32_t width, uint32_t height, const std::string& svgContent)
{
    // Initialize GDI+ for rendering
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken = 0;
    if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Gdiplus::Ok) {
        return nullptr;
    }

    // Create a GDI+ bitmap
    auto bitmap = std::make_unique<Gdiplus::Bitmap>(width, height, PixelFormat32bppARGB);
    Gdiplus::Graphics graphics(bitmap.get());
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    // White background
    graphics.Clear(Gdiplus::Color(255, 255, 255, 255));

    // Extract dimensions for aspect ratio
    uint32_t svgW = 0, svgH = 0;
    ExtractSVGDimensions(svgContent, svgW, svgH);

    // Count some basic SVG elements for visual complexity indicator
    int rectCount = 0, circleCount = 0, pathCount = 0, textCount = 0;
    {
        std::string lower = svgContent;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        size_t pos = 0;
        while ((pos = lower.find("<rect", pos)) != std::string::npos) {
            rectCount++;
            pos++;
        }
        pos = 0;
        while ((pos = lower.find("<circle", pos)) != std::string::npos) {
            circleCount++;
            pos++;
        }
        pos = 0;
        while ((pos = lower.find("<path", pos)) != std::string::npos) {
            pathCount++;
            pos++;
        }
        pos = 0;
        while ((pos = lower.find("<text", pos)) != std::string::npos) {
            textCount++;
            pos++;
        }
    }

    // Draw a visual SVG representation
    float margin = width * 0.08f;
    float innerW = width - margin * 2;
    float innerH = height - margin * 2;

    // Draw document border with shadow
    Gdiplus::SolidBrush shadowBrush(Gdiplus::Color(40, 0, 0, 0));
    graphics.FillRectangle(&shadowBrush, margin + 3.0f, margin + 3.0f, innerW, innerH);

    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 252, 252, 252));
    graphics.FillRectangle(&whiteBrush, margin, margin, innerW, innerH);

    Gdiplus::Pen borderPen(Gdiplus::Color(180, 100, 100, 100), 1.5f);
    graphics.DrawRectangle(&borderPen, margin, margin, innerW, innerH);

    // SVG icon in top-left corner
    float iconSize = (std::min)(innerW, innerH) * 0.2f;
    Gdiplus::SolidBrush svgBrush(Gdiplus::Color(255, 255, 152, 0));  // Orange
    Gdiplus::RectF iconRect(margin + 4, margin + 4, iconSize, iconSize * 0.7f);
    graphics.FillRectangle(&svgBrush, iconRect);

    // "SVG" label
    Gdiplus::FontFamily fontFamily(L"Consolas");
    float fontSize = iconSize * 0.35f;
    if (fontSize < 6)
        fontSize = 6;
    Gdiplus::Font font(&fontFamily, fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::StringFormat centerFormat;
    centerFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    centerFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    graphics.DrawString(L"SVG", 3, &font, iconRect, &centerFormat, &textBrush);

    // Draw abstract representation of SVG content
    float contentY = margin + iconSize * 0.7f + 8;
    float contentH = innerH - (contentY - margin) - 8;
    float contentX = margin + 8;
    float contentW = innerW - 16;

    // Colored shapes representing SVG complexity
    Gdiplus::Color shapeColors[] = {
        Gdiplus::Color(180, 66, 133, 244),  // Blue
        Gdiplus::Color(180, 52, 168, 83),   // Green
        Gdiplus::Color(180, 234, 67, 53),   // Red
        Gdiplus::Color(180, 251, 188, 4),   // Yellow
        Gdiplus::Color(180, 136, 57, 239),  // Purple
    };
    int totalElements = rectCount + circleCount + pathCount + textCount;
    if (totalElements == 0)
        totalElements = 3;  // Default

    int colorIdx = 0;

    // Draw some representative shapes
    if (circleCount > 0 || totalElements > 2) {
        Gdiplus::SolidBrush circleBrush(shapeColors[colorIdx++ % 5]);
        float cx = contentX + contentW * 0.3f;
        float cy = contentY + contentH * 0.4f;
        float r = (std::min)(contentW, contentH) * 0.15f;
        graphics.FillEllipse(&circleBrush, cx - r, cy - r, r * 2, r * 2);
    }

    if (rectCount > 0 || totalElements > 1) {
        Gdiplus::SolidBrush rectBrush(shapeColors[colorIdx++ % 5]);
        graphics.FillRectangle(&rectBrush, contentX + contentW * 0.5f, contentY + contentH * 0.2f, contentW * 0.35f,
                               contentH * 0.3f);
    }

    if (pathCount > 0 || totalElements > 0) {
        Gdiplus::Pen pathPen(shapeColors[colorIdx++ % 5], 2.0f);
        // Simple curve representing <path> elements
        Gdiplus::PointF pts[] = {{contentX + contentW * 0.1f, contentY + contentH * 0.7f},
                                 {contentX + contentW * 0.3f, contentY + contentH * 0.5f},
                                 {contentX + contentW * 0.6f, contentY + contentH * 0.8f},
                                 {contentX + contentW * 0.9f, contentY + contentH * 0.6f}};
        graphics.DrawCurve(&pathPen, pts, 4);
    }

    // Dimension info at bottom
    if (svgW > 0 && svgH > 0) {
        wchar_t dimText[64];
        swprintf_s(dimText, L"%ux%u", svgW, svgH);
        Gdiplus::Font smallFont(&fontFamily, fontSize * 0.7f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush dimBrush(Gdiplus::Color(160, 80, 80, 80));
        Gdiplus::RectF dimRect(margin, margin + innerH - fontSize, innerW, fontSize * 1.5f);
        Gdiplus::StringFormat rightFormat;
        rightFormat.SetAlignment(Gdiplus::StringAlignmentFar);
        rightFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        graphics.DrawString(dimText, -1, &smallFont, dimRect, &rightFormat, &dimBrush);
    }

    // Convert to HBITMAP
    HBITMAP hBitmap = nullptr;
    bitmap->GetHBITMAP(Gdiplus::Color(255, 255, 255), &hBitmap);

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return hBitmap;
}

bool SVGDecoder::IsSVGFormat(const wchar_t* path)
{
    if (!path)
        return false;
    const wchar_t* ext = PathFindExtensionW(path);
    return ext && _wcsicmp(ext, L".svg") == 0;
}

bool SVGDecoder::IsSVGZFormat(const wchar_t* path)
{
    if (!path)
        return false;
    const wchar_t* ext = PathFindExtensionW(path);
    return ext && _wcsicmp(ext, L".svgz") == 0;
}

std::unique_ptr<uint8_t[]> SVGDecoder::ReadFileData(const wchar_t* path, size_t& fileSize)
{
    fileSize = 0;
    HANDLE hFile =
        CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return nullptr;

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size) || size.QuadPart > 50 * 1024 * 1024) {  // 50MB limit
        CloseHandle(hFile);
        return nullptr;
    }

    fileSize = static_cast<size_t>(size.QuadPart);
    auto buffer = std::make_unique<uint8_t[]>(fileSize);
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer.get(), static_cast<DWORD>(fileSize), &bytesRead, nullptr) || bytesRead != fileSize) {
        CloseHandle(hFile);
        return nullptr;
    }

    CloseHandle(hFile);
    return buffer;
}

}  // namespace Engine
}  // namespace ExplorerLens
