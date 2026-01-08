/******************************************************************************
 * webp_decoder.cpp
 * WebP Image Decoder Implementation for DarkThumbs
 * Optimized for Windows 11 with static library linkage
 ******************************************************************************/

#include "StdAfx.h"
#include "webp_decoder.h"

// Link with libwebp static library (libwebp 1.5.0)
// Note: For dynamic DLL, use #pragma comment(linker, "/DELAYLOAD:webp.dll")
#pragma comment(lib, "webp.lib")
#pragma comment(lib, "sharpyuv.lib")

#include <webp/decode.h>

namespace DarkThumbs {

bool WebPDecoder::IsWebPFormat(const BYTE *data, size_t size) {
  if (!data || size < 12) {
    return false;
  }

  // Check RIFF header: "RIFF"
  if (memcmp(data, "RIFF", 4) != 0) {
    return false;
  }

  // Check WEBP signature at offset 8: "WEBP"
  if (memcmp(data + 8, "WEBP", 4) != 0) {
    return false;
  }

  return true;
}

bool WebPDecoder::GetDimensions(const BYTE *data, size_t size, int *width,
                                int *height) {
  if (!IsWebPFormat(data, size) || !width || !height) {
    return false;
  }

  // Use libwebp's fast dimension getter (no full decode)
  return WebPGetInfo(data, size, width, height) != 0;
}

HRESULT WebPDecoder::DecodeToHBITMAP(const BYTE *data, size_t size,
                                     HBITMAP *phBitmap) {
  if (!data || size == 0 || !phBitmap) {
    return E_INVALIDARG;
  }

  *phBitmap = nullptr;

  // Verify format
  if (!IsWebPFormat(data, size)) {
    return E_FAIL;
  }

  // Decode to RGBA (libwebp allocates the buffer)
  int width = 0, height = 0;
  BYTE *rgba = WebPDecodeRGBA(data, size, &width, &height);

  if (!rgba) {
    return E_FAIL;
  }

  // Convert to Windows HBITMAP
  HBITMAP hBitmap = CreateBitmapFromRGBA(rgba, width, height);

  // Free libwebp buffer
  WebPFree(rgba);

  if (!hBitmap) {
    return E_FAIL;
  }

  *phBitmap = hBitmap;
  return S_OK;
}

HBITMAP WebPDecoder::CreateBitmapFromRGBA(const BYTE *rgba, int width,
                                          int height) {
  // Setup DIB header (Windows bitmap format)
  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height; // Negative = top-down DIB
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32; // 32-bit ARGB
  bmi.bmiHeader.biCompression = BI_RGB;

  // Create DIB section
  void *pBits = nullptr;
  HDC hdc = GetDC(nullptr);
  HBITMAP hBitmap =
      CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
  ReleaseDC(nullptr, hdc);

  if (!hBitmap || !pBits) {
    return nullptr;
  }

  // Convert RGBA to BGRA (Windows expects BGR byte order)
  const BYTE *src = rgba;
  BYTE *dst = static_cast<BYTE *>(pBits);

  for (int i = 0; i < width * height; i++) {
    dst[0] = src[2]; // B
    dst[1] = src[1]; // G
    dst[2] = src[0]; // R
    dst[3] = src[3]; // A
    src += 4;
    dst += 4;
  }

  return hBitmap;
}

} // namespace DarkThumbs
