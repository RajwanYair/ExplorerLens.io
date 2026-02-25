// PDFDecoder.h - PDF Document Thumbnail Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: PDF (.pdf)
// Features:
// - MuPDF 1.24.11 native rendering (when HAS_MUPDF is defined)
// - Falls back to Windows Shell IThumbnailProvider (Edge/Acrobat)
// - Generates placeholder with page layout when no renderer available
// - High-quality anti-aliased rendering at arbitrary DPI

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>

#ifdef HAS_MUPDF
extern "C" {
#include <mupdf/fitz.h>
}
#endif

namespace ExplorerLens {
namespace Engine {

class PDFDecoder : public IThumbnailDecoder {
public:
  PDFDecoder();
  ~PDFDecoder() override;

  // IThumbnailDecoder interface
  bool CanDecode(const wchar_t *filePath) override;
  HRESULT Decode(const ThumbnailRequest &request,
                 ThumbnailResult &result) override;
  DecoderInfo GetInfo() const override;
  const wchar_t *GetName() const override { return L"PDFDecoder"; }
  const wchar_t **GetSupportedExtensions() const override;
  uint32_t GetExtensionCount() const override { return m_extensionCount; }
  bool SupportsGPU() const override { return false; }
  bool IsArchiveDecoder() const override { return false; }

  /// Returns true if MuPDF native rendering is available
  static bool HasNativeRenderer();

private:
#ifdef HAS_MUPDF
  // MuPDF native rendering (preferred path)
  HRESULT RenderWithMuPDF(const wchar_t *filePath, uint32_t width,
                          uint32_t height, HBITMAP *phBitmap);
#endif

  // Shell-based thumbnail extraction (fallback)
  HRESULT ExtractThumbnailShell(const wchar_t *filePath, uint32_t width,
                                uint32_t height, HBITMAP *phBitmap);

  // Placeholder generation
  HBITMAP CreatePDFPlaceholder(uint32_t width, uint32_t height,
                               const wchar_t *filePath);

  // Verify PDF signature (%PDF)
  bool IsPDFFormat(const wchar_t *path);

  static const wchar_t *m_extensions[];
  static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens
