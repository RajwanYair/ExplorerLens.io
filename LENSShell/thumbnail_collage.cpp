/******************************************************************************
 * thumbnail_collage.cpp
 * Multi-Page Thumbnail Collage Implementation for ExplorerLens
 * Renders multiple pages in a grid layout (2x2, 3x3, 4x4)
 ******************************************************************************/

#include "thumbnail_collage.h"

#include <atlbase.h>

#include "StdAfx.h"

#define LENS_APP_KEY _T("Software\\T800 Productions\\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}")

namespace ExplorerLens {

void ThumbnailCollage::GetGridDimensions(CollageMode mode, int& cols, int& rows)
{
    switch (mode) {
        case MODE_SINGLE:
            cols = 1;
            rows = 1;
            break;
        case MODE_2X2:
            cols = 2;
            rows = 2;
            break;
        case MODE_3X3:
            cols = 3;
            rows = 3;
            break;
        case MODE_4X4:
            cols = 4;
            rows = 4;
            break;
        default:
            cols = 2;
            rows = 2;
    }
}

ThumbnailCollage::CollageMode ThumbnailCollage::GetCollageModeFromRegistry()
{
    DWORD mode = MODE_SINGLE;  // Default: single image
    CRegKey regKey;

    if (ERROR_SUCCESS == regKey.Open(HKEY_CURRENT_USER, LENS_APP_KEY, KEY_READ)) {
        regKey.QueryDWORDValue(_T("CollagePages"), mode);
        regKey.Close();
    }

    // Validate mode
    if (mode != MODE_SINGLE && mode != MODE_2X2 && mode != MODE_3X3 && mode != MODE_4X4) {
        mode = MODE_SINGLE;
    }

    return static_cast<CollageMode>(mode);
}

bool ThumbnailCollage::SetCollageModeToRegistry(CollageMode mode)
{
    CRegKey regKey;

    if (ERROR_SUCCESS == regKey.Create(HKEY_CURRENT_USER, LENS_APP_KEY)) {
        DWORD modeValue = static_cast<DWORD>(mode);
        LONG result = regKey.SetDWORDValue(_T("CollagePages"), modeValue);
        regKey.Close();
        return (result == ERROR_SUCCESS);
    }

    return false;
}

bool ThumbnailCollage::DrawBitmapToComposite(HDC hdcDest, int x, int y, int width, int height, HBITMAP hSrc)
{
    if (!hSrc || !hdcDest)
        return false;

    // Get source bitmap info
    BITMAP bm;
    if (!GetObject(hSrc, sizeof(BITMAP), &bm))
        return false;

    // Create compatible DC for source
    HDC hdcSrc = CreateCompatibleDC(hdcDest);
    if (!hdcSrc)
        return false;

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcSrc, hSrc);

    // Use high-quality stretching
    SetStretchBltMode(hdcDest, HALFTONE);
    SetBrushOrgEx(hdcDest, 0, 0, NULL);

    // Stretch source to destination
    BOOL result = StretchBlt(hdcDest, x, y, width, height, hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    SelectObject(hdcSrc, hOldBitmap);
    DeleteDC(hdcSrc);

    return (result != 0);
}

HBITMAP ThumbnailCollage::CreateCollage(const std::vector<HBITMAP>& pages, int targetWidth, int targetHeight,
                                        CollageMode mode)
{
    if (pages.empty())
        return nullptr;

    // If single page mode or only one page available, return first page
    if (mode == MODE_SINGLE || pages.size() == 1) {
        // Clone the bitmap
        HDC hdcScreen = GetDC(NULL);
        HDC hdcSrc = CreateCompatibleDC(hdcScreen);
        HDC hdcDest = CreateCompatibleDC(hdcScreen);

        BITMAP bm;
        GetObject(pages[0], sizeof(BITMAP), &bm);

        HBITMAP hNewBitmap = CreateCompatibleBitmap(hdcScreen, bm.bmWidth, bm.bmHeight);

        HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, pages[0]);
        HBITMAP hOldDest = (HBITMAP)SelectObject(hdcDest, hNewBitmap);

        BitBlt(hdcDest, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

        SelectObject(hdcSrc, hOldSrc);
        SelectObject(hdcDest, hOldDest);
        DeleteDC(hdcSrc);
        DeleteDC(hdcDest);
        ReleaseDC(NULL, hdcScreen);

        return hNewBitmap;
    }

    // Get grid dimensions
    int cols, rows;
    GetGridDimensions(mode, cols, rows);

    // Calculate cell dimensions
    int cellWidth = targetWidth / cols;
    int cellHeight = targetHeight / rows;

    // Create composite bitmap
    HDC hdcScreen = GetDC(NULL);
    HDC hdcComposite = CreateCompatibleDC(hdcScreen);

    HBITMAP hComposite = CreateCompatibleBitmap(hdcScreen, targetWidth, targetHeight);
    if (!hComposite) {
        DeleteDC(hdcComposite);
        ReleaseDC(NULL, hdcScreen);
        return nullptr;
    }

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcComposite, hComposite);

    // Fill background with white
    RECT rc = {0, 0, targetWidth, targetHeight};
    FillRect(hdcComposite, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Draw each page into grid
    int pageIndex = 0;
    for (int row = 0; row < rows && pageIndex < (int)pages.size(); row++) {
        for (int col = 0; col < cols && pageIndex < (int)pages.size(); col++) {
            int x = col * cellWidth;
            int y = row * cellHeight;

            DrawBitmapToComposite(hdcComposite, x, y, cellWidth, cellHeight, pages[pageIndex]);

            pageIndex++;
        }
    }

    SelectObject(hdcComposite, hOldBitmap);
    DeleteDC(hdcComposite);
    ReleaseDC(NULL, hdcScreen);

    return hComposite;
}

}  // namespace ExplorerLens
