//==============================================================================
// WMFDecoder.cpp — Windows Metafile (WMF/EMF) Thumbnail Decoder
// ExplorerLens.io Engine v14.0.0
//==============================================================================

// GDI+ requires full Windows headers (COM/OLE2/RPC) excluded by
// WIN32_LEAN_AND_MEAN
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif

// Force-include Windows + COM headers before WMFDecoder.h
#include <ole2.h>
#include <windows.h>

#include "WMFDecoder.h"
#include <algorithm>
#include <memory>

#pragma comment(lib, "gdiplus.lib")

namespace ExplorerLens {
namespace Engine {

bool WMFDecoder::CanDecode(const wchar_t *filePath) const {
  if (!filePath)
    return false;

  std::wstring path(filePath);
  // Convert to lowercase for extension matching
  std::transform(path.begin(), path.end(), path.begin(), ::towlower);

  for (uint32_t i = 0; i < s_extensionCount; ++i) {
    size_t extLen = wcslen(s_extensions[i]);
    if (path.length() >= extLen &&
        path.compare(path.length() - extLen, extLen, s_extensions[i]) == 0) {
      return true;
    }
  }
  return false;
}

HRESULT WMFDecoder::Decode(const wchar_t *filePath, uint32_t requestedSize,
                           HBITMAP &hBitmap) {
  if (!filePath || requestedSize == 0)
    return E_INVALIDARG;
  return RenderMetafile(filePath, requestedSize, hBitmap);
}

Gdiplus::RectF WMFDecoder::CalculateRenderRect(uint32_t targetSize,
                                               float sourceWidth,
                                               float sourceHeight) {

  if (sourceWidth <= 0.0f || sourceHeight <= 0.0f) {
    return Gdiplus::RectF(0, 0, static_cast<float>(targetSize),
                          static_cast<float>(targetSize));
  }

  float scale = (std::min)(static_cast<float>(targetSize) / sourceWidth,
                           static_cast<float>(targetSize) / sourceHeight);

  float renderW = sourceWidth * scale;
  float renderH = sourceHeight * scale;
  float offsetX = (static_cast<float>(targetSize) - renderW) / 2.0f;
  float offsetY = (static_cast<float>(targetSize) - renderH) / 2.0f;

  return Gdiplus::RectF(offsetX, offsetY, renderW, renderH);
}

HRESULT WMFDecoder::RenderMetafile(const wchar_t *filePath, uint32_t size,
                                   HBITMAP &hBitmap) {
  // Initialize GDI+
  Gdiplus::GdiplusStartupInput gdipInput;
  ULONG_PTR gdipToken = 0;
  Gdiplus::Status status =
      Gdiplus::GdiplusStartup(&gdipToken, &gdipInput, nullptr);
  if (status != Gdiplus::Ok)
    return E_FAIL;

  HRESULT hr = E_FAIL;

  {
    // Load metafile
    auto metafile = std::make_unique<Gdiplus::Metafile>(filePath);
    if (metafile->GetLastStatus() != Gdiplus::Ok) {
      Gdiplus::GdiplusShutdown(gdipToken);
      return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    // Get source dimensions
    float srcWidth = static_cast<float>(metafile->GetWidth());
    float srcHeight = static_cast<float>(metafile->GetHeight());

    // Create target bitmap
    Gdiplus::Bitmap bitmap(size, size, PixelFormat32bppARGB);
    Gdiplus::Graphics graphics(&bitmap);

    // White background
    graphics.Clear(Gdiplus::Color(255, 255, 255, 255));

    // High-quality rendering
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    // Calculate aspect-ratio-preserving rect
    Gdiplus::RectF destRect = CalculateRenderRect(size, srcWidth, srcHeight);

    // Render metafile
    status = graphics.DrawImage(metafile.get(), destRect);
    if (status == Gdiplus::Ok) {
      bitmap.GetHBITMAP(Gdiplus::Color(255, 255, 255), &hBitmap);
      hr = S_OK;
    }
  }

  Gdiplus::GdiplusShutdown(gdipToken);
  return hr;
}

} // namespace Engine
} // namespace ExplorerLens
