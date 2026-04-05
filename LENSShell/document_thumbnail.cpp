#include "document_thumbnail.h"

#include <atlbase.h>
#include <gdiplus.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shlobj.h>
#include <wincodec.h>

#include <algorithm>

#include "StdAfx.h"

// Link required libraries
#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "gdiplus.lib")

namespace ExplorerLens {

// Main extraction function - tries multiple strategies
HBITMAP DocumentThumbnail::ExtractDocumentThumbnail(const std::wstring& filePath, int width, int height)
{
    if (filePath.empty() || !PathFileExists(filePath.c_str())) {
        return NULL;
    }

    DocumentType docType = GetDocumentType(filePath);

    // Strategy 1: Try IShellItemImageFactory (most reliable for Office files)
    HBITMAP hBitmap = ExtractUsingShellItem(filePath, width, height);
    if (hBitmap)
        return hBitmap;

    // Strategy 2: Try embedded thumbnail from Open XML files
    if (docType == DocumentType::Word || docType == DocumentType::Excel || docType == DocumentType::PowerPoint) {
        hBitmap = ExtractEmbeddedThumbnail(filePath);
        if (hBitmap)
            return hBitmap;
    }

    // Strategy 3: Generate generic document thumbnail
    hBitmap = GenerateGenericThumbnail(filePath, docType, width, height);

    return hBitmap;
}

// Strategy 2: Use IShellItemImageFactory (Windows Vista+)
HBITMAP DocumentThumbnail::ExtractUsingShellItem(const std::wstring& filePath, int width, int height)
{
    CComPtr<IShellItem> pShellItem;
    HRESULT hr = SHCreateItemFromParsingName(filePath.c_str(), NULL, IID_PPV_ARGS(&pShellItem));

    if (FAILED(hr))
        return NULL;

    CComPtr<IShellItemImageFactory> pImageFactory;
    hr = pShellItem->QueryInterface(IID_PPV_ARGS(&pImageFactory));
    if (FAILED(hr))
        return NULL;

    SIZE size = {width, height};
    HBITMAP hBitmap = NULL;

    // Try different quality options
    SIIGBF flags[] = {
        SIIGBF_THUMBNAILONLY,                     // Embedded thumbnail only
        SIIGBF_RESIZETOFIT,                       // Resize to fit
        SIIGBF_BIGGERSIZEOK | SIIGBF_RESIZETOFIT  // Allow bigger size
    };

    for (int i = 0; i < ARRAYSIZE(flags); i++) {
        hr = pImageFactory->GetImage(size, flags[i], &hBitmap);
        if (SUCCEEDED(hr) && hBitmap) {
            return hBitmap;
        }
    }

    return NULL;
}

// Strategy 3: Extract embedded thumbnail from Office Open XML
HBITMAP DocumentThumbnail::ExtractEmbeddedThumbnail(const std::wstring& filePath)
{
    // Office Open XML files are ZIP archives
    // Thumbnail is typically at: docProps/thumbnail.jpeg or thumbnail.png

    // FUTURE ENHANCEMENT: Implement ZIP extraction using minizip-ng 4.0.10
    // Library is built and linked, extraction logic needs implementation.
    // Path in archive: "docProps/thumbnail.jpeg"
    //
    // For now, return NULL and rely on other strategies (Property System works for most files)

    return NULL;
}

// Strategy 4: Generate generic document thumbnail
HBITMAP DocumentThumbnail::GenerateGenericThumbnail(const std::wstring& filePath, DocumentType type, int width,
                                                    int height)
{
    // Create DC and bitmap
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Create background
    RECT rect = {0, 0, width, height};

    // Dark theme background with gradient
    TRIVERTEX vertices[2];
    vertices[0].x = 0;
    vertices[0].y = 0;
    vertices[0].Red = 0x3000;  // RGB(48, 48, 48)
    vertices[0].Green = 0x3000;
    vertices[0].Blue = 0x3000;
    vertices[0].Alpha = 0xff00;

    vertices[1].x = width;
    vertices[1].y = height;
    vertices[1].Red = 0x2000;  // RGB(32, 32, 32)
    vertices[1].Green = 0x2000;
    vertices[1].Blue = 0x2000;
    vertices[1].Alpha = 0xff00;

    GRADIENT_RECT gRect = {0, 1};
    GradientFill(hdcMem, vertices, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

    // Draw document icon/representation
    // Different colors for different document types
    COLORREF accentColor = RGB(100, 150, 200);  // Default blue
    switch (type) {
        case DocumentType::Word:
            accentColor = RGB(43, 87, 154);  // Word blue
            break;
        case DocumentType::Excel:
            accentColor = RGB(33, 115, 70);  // Excel green
            break;
        case DocumentType::PowerPoint:
            accentColor = RGB(183, 71, 42);  // PowerPoint orange
            break;
        case DocumentType::PDF:
            accentColor = RGB(220, 60, 60);  // PDF red
            break;
        default:
            accentColor = RGB(100, 100, 100);  // Gray
            break;
    }

    // Draw simple document representation
    int margin = width / 6;
    RECT docRect = {margin, margin, width - margin, height - margin};

    // Document outline
    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
    HPEN hPen = CreatePen(PS_SOLID, 2, accentColor);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);

    RoundRect(hdcMem, docRect.left, docRect.top, docRect.right, docRect.bottom, 8, 8);

    SelectObject(hdcMem, hOldPen);
    SelectObject(hdcMem, hOldBrush);
    DeleteObject(hPen);
    DeleteObject(hBrush);

    // Draw lines representing text
    hPen = CreatePen(PS_SOLID, 2, RGB(180, 180, 180));
    SelectObject(hdcMem, hPen);

    int lineY = docRect.top + margin / 2;
    int lineSpacing = (docRect.bottom - docRect.top - margin) / 5;

    for (int i = 0; i < 4; i++) {
        int lineWidth =
            (i == 3) ? (docRect.right - docRect.left - margin) / 2 : (docRect.right - docRect.left - margin);
        MoveToEx(hdcMem, docRect.left + margin / 2, lineY, NULL);
        LineTo(hdcMem, docRect.left + margin / 2 + lineWidth, lineY);
        lineY += lineSpacing;
    }

    SelectObject(hdcMem, hOldPen);
    DeleteObject(hPen);

    // Draw file extension as text
    std::wstring ext = PathFindExtension(filePath.c_str());
    if (!ext.empty() && ext[0] == L'.') {
        ext = ext.substr(1);  // Remove dot
        _wcsupr_s(const_cast<wchar_t*>(ext.c_str()), ext.length() + 1);

        // Setup font
        HFONT hFont = CreateFont(height / 6,                   // Height
                                 0,                            // Width
                                 0,                            // Escapement
                                 0,                            // Orientation
                                 FW_BOLD,                      // Weight
                                 FALSE,                        // Italic
                                 FALSE,                        // Underline
                                 FALSE,                        // Strikeout
                                 DEFAULT_CHARSET,              // Charset
                                 OUT_DEFAULT_PRECIS,           // Output precision
                                 CLIP_DEFAULT_PRECIS,          // Clipping precision
                                 CLEARTYPE_QUALITY,            // Quality
                                 DEFAULT_PITCH | FF_DONTCARE,  // Pitch and family
                                 L"Segoe UI"                   // Font name
        );

        HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
        SetBkMode(hdcMem, TRANSPARENT);
        SetTextColor(hdcMem, accentColor);

        RECT textRect = {0, height - height / 4, width, height};
        DrawText(hdcMem, ext.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdcMem, hOldFont);
        DeleteObject(hFont);
    }

    // Cleanup
    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return hBitmap;
}

// Get document metadata
bool DocumentThumbnail::GetMetadata(const std::wstring& filePath, DocumentMetadata& metadata)
{
    ZeroMemory(&metadata, sizeof(DocumentMetadata));

    CComPtr<IPropertyStore> pPropertyStore;
    HRESULT hr = SHGetPropertyStoreFromParsingName(filePath.c_str(), NULL, GPS_DEFAULT, IID_PPV_ARGS(&pPropertyStore));

    if (FAILED(hr))
        return false;

    // Extract common document properties
    PROPVARIANT propVar;
    PropVariantInit(&propVar);

    // Title
    if (SUCCEEDED(pPropertyStore->GetValue(PKEY_Title, &propVar))) {
        if (propVar.vt == VT_LPWSTR) {
            metadata.title = propVar.pwszVal;
        }
        PropVariantClear(&propVar);
    }

    // Author
    if (SUCCEEDED(pPropertyStore->GetValue(PKEY_Author, &propVar))) {
        if (propVar.vt == VT_LPWSTR) {
            metadata.author = propVar.pwszVal;
        }
        PropVariantClear(&propVar);
    }

    // Subject
    if (SUCCEEDED(pPropertyStore->GetValue(PKEY_Subject, &propVar))) {
        if (propVar.vt == VT_LPWSTR) {
            metadata.subject = propVar.pwszVal;
        }
        PropVariantClear(&propVar);
    }

    // Keywords
    if (SUCCEEDED(pPropertyStore->GetValue(PKEY_Keywords, &propVar))) {
        if (propVar.vt == VT_LPWSTR) {
            metadata.keywords = propVar.pwszVal;
        }
        PropVariantClear(&propVar);
    }

    // Comments
    if (SUCCEEDED(pPropertyStore->GetValue(PKEY_Comment, &propVar))) {
        if (propVar.vt == VT_LPWSTR) {
            metadata.comments = propVar.pwszVal;
        }
        PropVariantClear(&propVar);
    }

    // Page count (for documents)
    if (SUCCEEDED(pPropertyStore->GetValue(PKEY_Document_PageCount, &propVar))) {
        if (propVar.vt == VT_I4) {
            metadata.pageCount = propVar.lVal;
        }
        PropVariantClear(&propVar);
    }

    // Word count
    if (SUCCEEDED(pPropertyStore->GetValue(PKEY_Document_WordCount, &propVar))) {
        if (propVar.vt == VT_I4) {
            metadata.wordCount = propVar.lVal;
        }
        PropVariantClear(&propVar);
    }

    return true;
}

// Detect document type from extension
DocumentThumbnail::DocumentType DocumentThumbnail::GetDocumentType(const std::wstring& filePath)
{
    std::wstring ext = PathFindExtension(filePath.c_str());
    if (ext.empty())
        return DocumentType::Unknown;

    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    if (ext == L".docx" || ext == L".doc" || ext == L".docm" || ext == L".dot" || ext == L".dotx") {
        return DocumentType::Word;
    }
    if (ext == L".xlsx" || ext == L".xls" || ext == L".xlsm" || ext == L".xlt" || ext == L".xltx" || ext == L".csv") {
        return DocumentType::Excel;
    }
    if (ext == L".pptx" || ext == L".ppt" || ext == L".pptm" || ext == L".pot" || ext == L".potx") {
        return DocumentType::PowerPoint;
    }
    if (ext == L".pdf") {
        return DocumentType::PDF;
    }
    if (ext == L".txt" || ext == L".rtf" || ext == L".log") {
        return DocumentType::Text;
    }
    if (ext == L".xps" || ext == L".oxps") {
        return DocumentType::XPS;
    }

    return DocumentType::Unknown;
}

// Check if Windows.Storage API is available (Windows 8+)
bool DocumentThumbnail::IsStorageThumbnailAvailable()
{
    // Use RtlGetVersion (ntdll) — avoids the deprecated GetVersionEx API
    using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
    auto rtlGetVersion = ntdll ? reinterpret_cast<RtlGetVersionFn>(
                                     ::GetProcAddress(ntdll, "RtlGetVersion")) : nullptr;

    RTL_OSVERSIONINFOW vi{};
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (rtlGetVersion && rtlGetVersion(&vi) == 0) {
        // Windows 8 is version 6.2; require 6.2 or later
        return vi.dwMajorVersion > 6 || (vi.dwMajorVersion == 6 && vi.dwMinorVersion >= 2);
    }

    return false;
}

}  // namespace ExplorerLens
