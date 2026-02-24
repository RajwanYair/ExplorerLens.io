#pragma once
//==============================================================================
// WMFDecoder.h — Windows Metafile (WMF/EMF) Thumbnail Decoder
// ExplorerLens.io Engine v14.0.0
//
// Renders WMF (Windows Metafile) and EMF (Enhanced Metafile) vector graphics
// using GDI+ PlayMetaFile/PlayEnhMetaFile. Outputs rasterized HBITMAP
// thumbnail.
//==============================================================================

#include "../Core/IThumbnailDecoder.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Decoder for WMF (Windows Metafile) and EMF (Enhanced Metafile) vector
/// formats.
///
/// WMF (.wmf) — 16-bit legacy vector format (GDI commands)
/// EMF (.emf) — 32-bit enhanced metafile (GDI+ compatible)
///
/// Rendering pipeline:
///   1. Load metafile via GDI+ Metafile class
///   2. Create target bitmap at requested thumbnail size
///   3. Render metafile into bitmap using DrawImage (preserving aspect ratio)
///   4. Convert to HBITMAP for shell integration
class WMFDecoder {
public:
  WMFDecoder() = default;
  ~WMFDecoder() = default;

  // IThumbnailDecoder interface
  bool CanDecode(const wchar_t *filePath) const;
  HRESULT Decode(const wchar_t *filePath, uint32_t requestedSize,
                 HBITMAP &hBitmap);
  const wchar_t *GetName() const { return L"WMFDecoder"; }
  bool SupportsGPU() const { return false; }
  bool IsArchiveDecoder() const { return false; }

  static constexpr const wchar_t *s_extensions[] = {L".wmf", L".emf"};
  static constexpr uint32_t s_extensionCount = 2;

  const wchar_t **GetSupportedExtensions() const {
    return const_cast<const wchar_t **>(s_extensions);
  }
  uint32_t GetExtensionCount() const { return s_extensionCount; }

private:
  /// Render metafile to HBITMAP using GDI+
  HRESULT RenderMetafile(const wchar_t *filePath, uint32_t size,
                         HBITMAP &hBitmap);

  /// Calculate aspect-ratio-preserving render rect
  static Gdiplus::RectF CalculateRenderRect(uint32_t targetSize,
                                            float sourceWidth,
                                            float sourceHeight);
};

} // namespace Engine
} // namespace ExplorerLens
