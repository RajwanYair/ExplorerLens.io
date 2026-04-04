// DocumentDecoder.cpp - Document Thumbnail Decoder Implementation
// ExplorerLens Engine v6.2.0+
// Copyright (c) 2026 ExplorerLens Project

#include "DocumentDecoder.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <propkey.h>
#include <propsys.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <thumbcache.h>
#include <algorithm>
#include <cwchar>
#include <fstream>
#include <string>
#include <vector>
#include "../Utils/PerformanceProfiler.h"

// Minizip-NG for EPUB extraction (EPUBs are ZIP files)
#include "mz.h"
#include "mz_strm.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

namespace ExplorerLens {
namespace Engine {

const wchar_t* DocumentDecoder::m_extensions[] = {L".epub", L".mobi", L".azw",  L".azw3", L".fb2", L".docx", L".doc",
                                                  L".xlsx", L".xls",  L".pptx", L".ppt",  L".xps", L".oxps", L".djvu",
                                                  L".djv",  L".rtf",  L".odt",  L".ods",  L".odp", nullptr};
const uint32_t DocumentDecoder::m_extensionCount = 19;

DocumentDecoder::DocumentDecoder() = default;
DocumentDecoder::~DocumentDecoder() = default;

bool DocumentDecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath)
        return false;
    return IsDocumentFormat(filePath);
}

HRESULT DocumentDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    PROFILE_SCOPE(ProfileComponent::DECODE_DOCUMENT);

    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    if (!request.filePath)
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;

    // Try format-specific extraction first
    if (IsEpubFormat(request.filePath)) {
        hr = ExtractEPUBCover(request.filePath, request.width, request.height, &result.hBitmap);
    } else if (IsMobiFormat(request.filePath)) {
        hr = ExtractMOBICover(request.filePath, request.width, request.height, &result.hBitmap);
    }

    // Try Shell thumbnail if format-specific failed
    if (FAILED(hr) || !result.hBitmap) {
        hr = ExtractThumbnailShell(request.filePath, request.width, request.height, &result.hBitmap);
    }

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

DecoderInfo DocumentDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name = L"Document Decoder";
    info.version = L"1.0.0";
    info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** DocumentDecoder::GetSupportedExtensions() const
{
    return const_cast<const wchar_t**>(m_extensions);
}

// ============================================================================
// Shell Thumbnail Extraction
// ============================================================================

HRESULT DocumentDecoder::ExtractThumbnailShell(const wchar_t* filePath, uint32_t width, uint32_t height,
                                               HBITMAP* phBitmap)
{
    if (!phBitmap)
        return E_INVALIDARG;
    *phBitmap = nullptr;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
    if (!comInit)
        return hr;

    IShellItem* pItem = nullptr;
    hr = SHCreateItemFromParsingName(filePath, nullptr, IID_PPV_ARGS(&pItem));
    if (FAILED(hr)) {
        CoUninitialize();
        return hr;
    }

    IShellItemImageFactory* pFactory = nullptr;
    hr = pItem->QueryInterface(IID_PPV_ARGS(&pFactory));
    if (SUCCEEDED(hr) && pFactory) {
        SIZE sz = {static_cast<LONG>(width), static_cast<LONG>(height)};
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
// EPUB Cover Extraction (EPUBs are ZIP files with cover images)
// ============================================================================

HRESULT DocumentDecoder::ExtractEPUBCover(const wchar_t* filePath, uint32_t width, uint32_t height, HBITMAP* phBitmap)
{
    if (!phBitmap)
        return E_INVALIDARG;
    *phBitmap = nullptr;

    // Convert wchar_t* to char* for minizip
    char filePathA[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, filePath, -1, filePathA, MAX_PATH, nullptr, nullptr);

    // Open ZIP file
    void* zip_reader = mz_zip_reader_create();
    if (!zip_reader)
        return E_FAIL;

    int32_t err = mz_zip_reader_open_file(zip_reader, filePathA);
    if (err != MZ_OK) {
        mz_zip_reader_delete(&zip_reader);
        return E_FAIL;
    }

    // Search for cover image (common locations in EPUB)
    const char* coverNames[] = {"cover.jpg",
                                "cover.jpeg",
                                "cover.png",
                                "OEBPS/cover.jpg",
                                "OEBPS/cover.jpeg",
                                "OEBPS/cover.png",
                                "OEBPS/Images/cover.jpg",
                                "OEBPS/Images/cover.png",
                                "META-INF/cover.jpg",
                                "META-INF/cover.png",
                                nullptr};

    std::vector<uint8_t> imageData;
    HRESULT hr = E_FAIL;

    // Try each possible cover location
    for (int i = 0; coverNames[i] != nullptr; i++) {
        err = mz_zip_reader_locate_entry(zip_reader, coverNames[i], 1);  // 1 = case-insensitive
        if (err == MZ_OK) {
            err = mz_zip_reader_entry_open(zip_reader);
            if (err == MZ_OK) {
                // Get file size
                mz_zip_file* file_info = nullptr;
                mz_zip_reader_entry_get_info(zip_reader, &file_info);
                if (file_info && file_info->uncompressed_size > 0 && file_info->uncompressed_size < 50 * 1024 * 1024) {
                    imageData.resize(file_info->uncompressed_size);
                    int32_t bytesRead =
                        mz_zip_reader_entry_read(zip_reader, imageData.data(), static_cast<int32_t>(imageData.size()));
                    if (bytesRead == static_cast<int32_t>(imageData.size())) {
                        mz_zip_reader_entry_close(zip_reader);
                        // Successfully read cover image
                        hr = S_OK;
                        break;
                    }
                }
                mz_zip_reader_entry_close(zip_reader);
            }
        }
    }

    mz_zip_reader_close(zip_reader);
    mz_zip_reader_delete(&zip_reader);

    // Decode image data to HBITMAP using GDI+
    if (SUCCEEDED(hr) && !imageData.empty()) {
        Gdiplus::GdiplusStartupInput gdipInput;
        ULONG_PTR gdipToken = 0;
        if (Gdiplus::GdiplusStartup(&gdipToken, &gdipInput, nullptr) == Gdiplus::Ok) {
            IStream* stream = SHCreateMemStream(imageData.data(), static_cast<UINT>(imageData.size()));
            if (stream) {
                Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(stream);
                if (bmp && bmp->GetLastStatus() == Gdiplus::Ok) {
                    // Resize if needed
                    Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
                    Gdiplus::Graphics g(resized);
                    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
                    g.DrawImage(bmp, 0, 0, width, height);
                    resized->GetHBITMAP(Gdiplus::Color(255, 255, 255), phBitmap);
                    delete resized;
                    delete bmp;
                } else {
                    delete bmp;
                }
                stream->Release();
            }
            Gdiplus::GdiplusShutdown(gdipToken);
        }
    }

    return *phBitmap ? S_OK : E_FAIL;
}

// ============================================================================
// MOBI Cover Extraction (MOBI files have embedded covers in EXTH records)
// ============================================================================

HRESULT DocumentDecoder::ExtractMOBICover(const wchar_t* filePath, uint32_t width, uint32_t height, HBITMAP* phBitmap)
{
    if (!phBitmap)
        return E_INVALIDARG;
    *phBitmap = nullptr;

    // Read file data
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
        return E_FAIL;

    // Read first 1 MB (cover is usually at the beginning)
    std::vector<uint8_t> data(1024 * 1024);
    file.read(reinterpret_cast<char*>(data.data()), data.size());
    size_t bytesRead = file.gcount();
    if (bytesRead < 100)
        return E_FAIL;
    data.resize(bytesRead);

    // MOBI format: Look for JPEG signature (0xFFD8FFE0 or 0xFFD8FFE1)
    // MOBI files often have embedded JPEG cover art
    size_t jpegStart = std::string::npos;
    for (size_t i = 0; i < data.size() - 4; i++) {
        if (data[i] == 0xFF && data[i + 1] == 0xD8 && data[i + 2] == 0xFF) {
            // Found JPEG start marker
            jpegStart = i;
            break;
        }
    }

    if (jpegStart == std::string::npos)
        return E_FAIL;

    // Find JPEG end marker (0xFFD9)
    size_t jpegEnd = std::string::npos;
    for (size_t i = jpegStart + 10; i < data.size() - 1; i++) {
        if (data[i] == 0xFF && data[i + 1] == 0xD9) {
            jpegEnd = i + 2;
            break;
        }
    }

    if (jpegEnd == std::string::npos || jpegEnd <= jpegStart)
        return E_FAIL;

    // Extract JPEG data
    std::vector<uint8_t> jpegData(data.begin() + jpegStart, data.begin() + jpegEnd);

    // Decode JPEG to HBITMAP using GDI+
    Gdiplus::GdiplusStartupInput gdipInput;
    ULONG_PTR gdipToken = 0;
    if (Gdiplus::GdiplusStartup(&gdipToken, &gdipInput, nullptr) == Gdiplus::Ok) {
        IStream* stream = SHCreateMemStream(jpegData.data(), static_cast<UINT>(jpegData.size()));
        if (stream) {
            Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(stream);
            if (bmp && bmp->GetLastStatus() == Gdiplus::Ok) {
                // Resize to requested dimensions
                Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
                Gdiplus::Graphics g(resized);
                g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
                g.DrawImage(bmp, 0, 0, width, height);
                resized->GetHBITMAP(Gdiplus::Color(255, 255, 255), phBitmap);
                delete resized;
                delete bmp;
            } else {
                delete bmp;
            }
            stream->Release();
        }
        Gdiplus::GdiplusShutdown(gdipToken);
    }

    return *phBitmap ? S_OK : E_FAIL;
}

// ============================================================================
// Document Placeholder
// ============================================================================

HBITMAP DocumentDecoder::CreateDocumentPlaceholder(uint32_t width, uint32_t height, const wchar_t* ext)
{
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
    Gdiplus::Color badgeColor(255, 50, 120, 200);  // Default: blue
    std::wstring label = L"DOC";

    if (ext) {
        std::wstring e(ext);
        for (auto& c : e)
            c = towupper(c);

        if (e == L".EPUB") {
            badgeColor = Gdiplus::Color(255, 140, 90, 180);
            label = L"EPUB";
        } else if (e == L".MOBI" || e == L".AZW" || e == L".AZW3") {
            badgeColor = Gdiplus::Color(255, 255, 153, 0);
            label = L"MOBI";
        } else if (e == L".FB2") {
            badgeColor = Gdiplus::Color(255, 0, 150, 136);
            label = L"FB2";
        } else if (e == L".DOCX" || e == L".DOC" || e == L".RTF" || e == L".ODT") {
            badgeColor = Gdiplus::Color(255, 40, 90, 180);
            label = L"DOC";
        } else if (e == L".XLSX" || e == L".XLS" || e == L".ODS") {
            badgeColor = Gdiplus::Color(255, 33, 115, 70);
            label = L"XLS";
        } else if (e == L".PPTX" || e == L".PPT" || e == L".ODP") {
            badgeColor = Gdiplus::Color(255, 210, 71, 38);
            label = L"PPT";
        } else if (e == L".XPS" || e == L".OXPS") {
            badgeColor = Gdiplus::Color(255, 0, 120, 215);
            label = L"XPS";
        } else if (e == L".DJVU" || e == L".DJV") {
            badgeColor = Gdiplus::Color(255, 60, 60, 60);
            label = L"DJVU";
        } else {
            label = e.substr(1);
        }  // Remove dot
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
    if (fontSize < 8)
        fontSize = 8;
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

bool DocumentDecoder::IsDocumentFormat(const wchar_t* path)
{
    if (!path)
        return false;
    const wchar_t* ext = PathFindExtensionW(path);
    if (!ext || ext[0] == L'\0')
        return false;
    for (int i = 0; m_extensions[i] != nullptr; i++) {
        if (_wcsicmp(ext, m_extensions[i]) == 0)
            return true;
    }
    return false;
}

bool DocumentDecoder::IsEpubFormat(const wchar_t* path)
{
    if (!path)
        return false;
    const wchar_t* ext = PathFindExtensionW(path);
    return ext && _wcsicmp(ext, L".epub") == 0;
}

bool DocumentDecoder::IsMobiFormat(const wchar_t* path)
{
    if (!path)
        return false;
    const wchar_t* ext = PathFindExtensionW(path);
    return ext && (_wcsicmp(ext, L".mobi") == 0 || _wcsicmp(ext, L".azw") == 0 || _wcsicmp(ext, L".azw3") == 0);
}

// ============================================================================
// Document Metadata Extraction
// ============================================================================

bool DocumentDecoder::GetDocumentMetadata(const wchar_t* filePath, DocumentMetadata& metadata)
{
    if (!filePath)
        return false;

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
    if (!comInit)
        return false;

    bool success = false;

    // Use Windows Property System to extract metadata
    IPropertyStore* pProps = nullptr;
    hr = SHGetPropertyStoreFromParsingName(filePath, nullptr, GPS_DEFAULT, IID_PPV_ARGS(&pProps));
    if (SUCCEEDED(hr) && pProps) {
        PROPVARIANT propVar;
        PropVariantInit(&propVar);

        // Extract title
        if (SUCCEEDED(pProps->GetValue(PKEY_Title, &propVar))) {
            if (propVar.vt == VT_LPWSTR && propVar.pwszVal) {
                metadata.title = propVar.pwszVal;
            }
            PropVariantClear(&propVar);
        }

        // Extract author
        if (SUCCEEDED(pProps->GetValue(PKEY_Author, &propVar))) {
            if (propVar.vt == VT_LPWSTR && propVar.pwszVal) {
                metadata.author = propVar.pwszVal;
            } else if (propVar.vt == (VT_VECTOR | VT_LPWSTR)) {
                // Author can be a vector
                if (propVar.calpwstr.cElems > 0 && propVar.calpwstr.pElems[0]) {
                    metadata.author = propVar.calpwstr.pElems[0];
                }
            }
            PropVariantClear(&propVar);
        }

        // Extract subject
        if (SUCCEEDED(pProps->GetValue(PKEY_Subject, &propVar))) {
            if (propVar.vt == VT_LPWSTR && propVar.pwszVal) {
                metadata.subject = propVar.pwszVal;
            }
            PropVariantClear(&propVar);
        }

        // Extract page count (for Office documents)
        if (SUCCEEDED(pProps->GetValue(PKEY_Document_PageCount, &propVar))) {
            if (propVar.vt == VT_I4) {
                metadata.pageCount = static_cast<uint32_t>(propVar.lVal);
            }
            PropVariantClear(&propVar);
        }

        success = !metadata.title.empty() || !metadata.author.empty();
        pProps->Release();
    }

    // Check for cover image (EPUB/MOBI)
    if (IsEpubFormat(filePath) || IsMobiFormat(filePath)) {
        metadata.hasCoverImage = true;
    }

    CoUninitialize();
    return success;
}

}  // namespace Engine
}  // namespace ExplorerLens
