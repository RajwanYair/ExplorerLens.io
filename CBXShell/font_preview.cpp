#include "StdAfx.h"
#include "font_preview.h"
#include <atlbase.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <propsys.h>
#include <propkey.h>
#include <propvarutil.h>
#include <wincodec.h>
#include <d2d1.h>
#include <dwrite.h>
#include <algorithm>

#pragma comment(lib, "shlwapi.lib")

namespace DarkThumbs
{

    // Main font preview generation
    HBITMAP FontPreview::GenerateFontPreview(
        const std::wstring &fontPath,
        int width,
        int height)
    {
        if (fontPath.empty() || !PathFileExists(fontPath.c_str()))
        {
            return NULL;
        }

        // Try DirectWrite first (best quality)
        if (IsDirectWriteAvailable())
        {
            HBITMAP hBitmap = RenderUsingDirectWrite(fontPath, width, height);
            if (hBitmap)
                return hBitmap;
        }

        // Fallback to GDI rendering
        return RenderUsingGDI(fontPath, width, height);
    }

    // DirectWrite rendering (Windows Vista+)
    HBITMAP FontPreview::RenderUsingDirectWrite(
        const std::wstring &fontPath,
        int width,
        int height)
    {
        // Initialize COM
        CoInitialize(NULL);

        CComPtr<ID2D1Factory> pD2DFactory;
        CComPtr<IDWriteFactory> pDWriteFactory;

        HRESULT hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            &pD2DFactory);

        if (FAILED(hr))
        {
            CoUninitialize();
            return NULL;
        }

        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown **>(&pDWriteFactory));

        if (FAILED(hr))
        {
            CoUninitialize();
            return NULL;
        }

        // Create WIC bitmap
        CComPtr<IWICImagingFactory> pWICFactory;
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pWICFactory));

        if (FAILED(hr))
        {
            CoUninitialize();
            return NULL;
        }

        CComPtr<IWICBitmap> pWICBitmap;
        hr = pWICFactory->CreateBitmap(
            width,
            height,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnDemand,
            &pWICBitmap);

        if (FAILED(hr))
        {
            CoUninitialize();
            return NULL;
        }

        // Create D2D render target
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.0f,
            96.0f);

        CComPtr<ID2D1RenderTarget> pRenderTarget;
        hr = pD2DFactory->CreateWicBitmapRenderTarget(
            pWICBitmap,
            &props,
            &pRenderTarget);

        if (FAILED(hr))
        {
            CoUninitialize();
            return NULL;
        }

        // KNOWN LIMITATION: DirectWrite font loading requires IDWriteFontSetBuilder (DirectWrite 3.0)
        // which needs Windows 10 SDK 10.0.15063.0 or later.
        // Current implementation uses GDI fallback for maximum compatibility.
        // GDI rendering works well for most font previews.

        // Fallback to GDI
        CoUninitialize();
        return RenderUsingGDI(fontPath, width, height);
    }

    // GDI rendering fallback
    HBITMAP FontPreview::RenderUsingGDI(
        const std::wstring &fontPath,
        int width,
        int height)
    {
        // Add font resource temporarily
        int fontCount = AddFontResourceEx(
            fontPath.c_str(),
            FR_PRIVATE,
            NULL);

        if (fontCount == 0)
        {
            // Could not load font
            return NULL;
        }

        // Create DC and bitmap
        HDC hdcScreen = GetDC(NULL);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

        // Dark theme background
        RECT rect = {0, 0, width, height};
        HBRUSH hBrush = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(hdcMem, &rect, hBrush);
        DeleteObject(hBrush);

        // Get font metadata (family name)
        FontMetadata metadata;
        GetFontMetadata(fontPath, metadata);

        std::wstring fontFamily = metadata.familyName;
        if (fontFamily.empty())
        {
            // Try to extract from filename
            fontFamily = PathFindFileName(fontPath.c_str());
            size_t extPos = fontFamily.find_last_of(L'.');
            if (extPos != std::wstring::npos)
            {
                fontFamily = fontFamily.substr(0, extPos);
            }
        }

        // Create large font for sample text
        HFONT hFont = CreateFont(
            height / 3,                  // Height
            0,                           // Width
            0,                           // Escapement
            0,                           // Orientation
            metadata.weight,             // Weight (400=normal, 700=bold)
            metadata.isItalic,           // Italic
            FALSE,                       // Underline
            FALSE,                       // Strikeout
            DEFAULT_CHARSET,             // Charset
            OUT_DEFAULT_PRECIS,          // Output precision
            CLIP_DEFAULT_PRECIS,         // Clipping precision
            CLEARTYPE_QUALITY,           // Quality
            DEFAULT_PITCH | FF_DONTCARE, // Pitch and family
            fontFamily.c_str()           // Font name
        );

        if (!hFont)
        {
            // Font creation failed, cleanup
            RemoveFontResourceEx(fontPath.c_str(), FR_PRIVATE, NULL);
            SelectObject(hdcMem, hOldBitmap);
            DeleteDC(hdcMem);
            ReleaseDC(NULL, hdcScreen);
            return NULL;
        }

        HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
        SetBkMode(hdcMem, TRANSPARENT);
        SetTextColor(hdcMem, RGB(240, 240, 240));

        // Sample text
        const wchar_t *sampleText = metadata.isMonospace ? L"Code 123" : L"AaBbCc";

        RECT textRect = {10, height / 6, width - 10, height / 2};
        DrawText(hdcMem, sampleText, -1, &textRect,
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdcMem, hOldFont);
        DeleteObject(hFont);

        // Draw smaller character set preview
        hFont = CreateFont(
            height / 8, // Smaller size
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            fontFamily.c_str());

        SelectObject(hdcMem, hFont);
        SetTextColor(hdcMem, RGB(180, 180, 180));

        const wchar_t *charSet = L"0123456789";
        RECT charRect = {10, height / 2 + 10, width - 10, height - height / 5};
        DrawText(hdcMem, charSet, -1, &charRect,
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdcMem, hOldFont);
        DeleteObject(hFont);

        // Draw font name at bottom
        hFont = CreateFont(
            height / 12,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI");

        SelectObject(hdcMem, hFont);
        SetTextColor(hdcMem, RGB(100, 150, 200));

        RECT nameRect = {10, height - height / 6, width - 10, height - 10};
        DrawText(hdcMem, fontFamily.c_str(), -1, &nameRect,
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        SelectObject(hdcMem, hOldFont);
        DeleteObject(hFont);

        // Cleanup
        SelectObject(hdcMem, hOldBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);

        // Remove font resource
        RemoveFontResourceEx(fontPath.c_str(), FR_PRIVATE, NULL);

        return hBitmap;
    }

    // Get font metadata
    bool FontPreview::GetFontMetadata(
        const std::wstring &fontPath,
        FontMetadata &metadata)
    {
        ZeroMemory(&metadata, sizeof(FontMetadata));

        // Try to get metadata from file properties first
        CComPtr<IPropertyStore> pPropertyStore;
        HRESULT hr = SHGetPropertyStoreFromParsingName(
            fontPath.c_str(),
            NULL,
            GPS_DEFAULT,
            IID_PPV_ARGS(&pPropertyStore));

        if (SUCCEEDED(hr))
        {
            PROPVARIANT propVar;
            PropVariantInit(&propVar);

            // Try to get font name
            if (SUCCEEDED(pPropertyStore->GetValue(PKEY_Title, &propVar)))
            {
                if (propVar.vt == VT_LPWSTR)
                {
                    metadata.familyName = propVar.pwszVal;
                }
                PropVariantClear(&propVar);
            }
        }

        // Default metadata if not found
        if (metadata.familyName.empty())
        {
            metadata.familyName = PathFindFileName(fontPath.c_str());
        }

        // Detect if monospace from filename
        std::wstring lowerName = metadata.familyName;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::towlower);

        metadata.isMonospace = (lowerName.find(L"mono") != std::wstring::npos ||
                                lowerName.find(L"code") != std::wstring::npos ||
                                lowerName.find(L"console") != std::wstring::npos ||
                                lowerName.find(L"courier") != std::wstring::npos);

        // Default weight
        metadata.weight = 400; // Normal

        // Detect bold/italic from filename
        metadata.isItalic = (lowerName.find(L"italic") != std::wstring::npos ||
                             lowerName.find(L"oblique") != std::wstring::npos);

        if (lowerName.find(L"bold") != std::wstring::npos)
        {
            metadata.weight = 700;
        }

        return true;
    }

    // Check if DirectWrite is available
    bool FontPreview::IsDirectWriteAvailable()
    {
        CComPtr<IDWriteFactory> pFactory;
        HRESULT hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown **>(&pFactory));
        return SUCCEEDED(hr);
    }

    const wchar_t *FontPreview::GetSampleText(bool isMonospace)
    {
        return isMonospace ? L"Code 123" : L"AaBbCc";
    }

} // namespace DarkThumbs
