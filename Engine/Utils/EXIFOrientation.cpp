//==============================================================================
// ExplorerLens Engine - EXIF Orientation Utilities
// Copyright (c) 2026 - ExplorerLens Project
// Version: 6.0.0
//==============================================================================

#include "EXIFOrientation.h"

namespace ExplorerLens {
namespace Engine {
namespace Utils {

HBITMAP ApplyEXIFOrientation(HBITMAP hBitmap, int orientation) {
 if (!hBitmap || orientation == 0 || orientation == 1) {
 // No transformation needed for normal orientation or invalid input
 return hBitmap;
 }
 
 // Get bitmap dimensions
 BITMAP bm;
 if (!GetObject(hBitmap, sizeof(BITMAP), &bm)) {
 return NULL;
 }
 
 HDC hdc = GetDC(NULL);
 HDC hdcSrc = CreateCompatibleDC(hdc);
 HDC hdcDst = CreateCompatibleDC(hdc);
 
 HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, hBitmap);
 
 HBITMAP hRotated = NULL;
 HBITMAP hOldDst = NULL;
 
 // Apply transformation based on EXIF orientation
 // EXIF orientation values: 1=normal, 2=flip-h, 3=180, 4=flip-v,
 // 5=transpose, 6=90CW, 7=transverse, 8=270CW
 switch (orientation) {
 case 1: // Normal - no transformation needed
 hRotated = hBitmap;
 break;
 
 case 2: // Flip horizontal
 hRotated = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);
 hOldDst = (HBITMAP)SelectObject(hdcDst, hRotated);
 StretchBlt(hdcDst, bm.bmWidth, 0, -bm.bmWidth, bm.bmHeight,
 hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
 break;
 
 case 3: // 180 degrees rotation
 hRotated = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);
 hOldDst = (HBITMAP)SelectObject(hdcDst, hRotated);
 StretchBlt(hdcDst, bm.bmWidth, bm.bmHeight, -bm.bmWidth, -bm.bmHeight,
 hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
 break;
 
 case 4: // Flip vertical
 hRotated = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);
 hOldDst = (HBITMAP)SelectObject(hdcDst, hRotated);
 StretchBlt(hdcDst, 0, bm.bmHeight, bm.bmWidth, -bm.bmHeight,
 hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
 break;
 
 case 5: // Transpose (flip horizontal + rotate 90 CW)
 hRotated = CreateCompatibleBitmap(hdc, bm.bmHeight, bm.bmWidth);
 hOldDst = (HBITMAP)SelectObject(hdcDst, hRotated);
 SetGraphicsMode(hdcDst, GM_ADVANCED);
 {
 XFORM xform;
 xform.eM11 = 0.0f; xform.eM12 = -1.0f;
 xform.eM21 = 1.0f; xform.eM22 = 0.0f;
 xform.eDx = 0.0f; xform.eDy = (float)bm.bmWidth;
 SetWorldTransform(hdcDst, &xform);
 BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
 }
 break;
 
 case 6: // 90 degrees clockwise
 hRotated = CreateCompatibleBitmap(hdc, bm.bmHeight, bm.bmWidth);
 hOldDst = (HBITMAP)SelectObject(hdcDst, hRotated);
 SetGraphicsMode(hdcDst, GM_ADVANCED);
 {
 XFORM xform;
 xform.eM11 = 0.0f; xform.eM12 = 1.0f;
 xform.eM21 = -1.0f; xform.eM22 = 0.0f;
 xform.eDx = (float)bm.bmHeight; xform.eDy = 0.0f;
 SetWorldTransform(hdcDst, &xform);
 BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
 }
 break;
 
 case 7: // Transverse (flip horizontal + rotate 270 CW)
 hRotated = CreateCompatibleBitmap(hdc, bm.bmHeight, bm.bmWidth);
 hOldDst = (HBITMAP)SelectObject(hdcDst, hRotated);
 SetGraphicsMode(hdcDst, GM_ADVANCED);
 {
 XFORM xform;
 xform.eM11 = 0.0f; xform.eM12 = 1.0f;
 xform.eM21 = 1.0f; xform.eM22 = 0.0f;
 xform.eDx = (float)bm.bmHeight; xform.eDy = (float)bm.bmWidth;
 SetWorldTransform(hdcDst, &xform);
 BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
 }
 break;
 
 case 8: // 270 degrees clockwise (90 CCW)
 hRotated = CreateCompatibleBitmap(hdc, bm.bmHeight, bm.bmWidth);
 hOldDst = (HBITMAP)SelectObject(hdcDst, hRotated);
 SetGraphicsMode(hdcDst, GM_ADVANCED);
 {
 XFORM xform;
 xform.eM11 = 0.0f; xform.eM12 = -1.0f;
 xform.eM21 = 1.0f; xform.eM22 = 0.0f;
 xform.eDx = 0.0f; xform.eDy = (float)bm.bmWidth;
 SetWorldTransform(hdcDst, &xform);
 BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
 }
 break;
 
 default:
 // Unknown orientation, return original
 hRotated = hBitmap;
 break;
 }
 
 if (hOldDst) SelectObject(hdcDst, hOldDst);
 SelectObject(hdcSrc, hOldSrc);
 DeleteDC(hdcDst);
 DeleteDC(hdcSrc);
 ReleaseDC(NULL, hdc);
 
 return hRotated;
}

} // namespace Utils
} // namespace Engine
} // namespace ExplorerLens

