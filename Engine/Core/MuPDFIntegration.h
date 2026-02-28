// MuPDFIntegration.h — MuPDF PDF Rendering Integration Layer
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a high-level wrapper around MuPDF's C API for PDF thumbnail
// generation. Handles library initialization, page rendering, error recovery,
// and resource cleanup. Gated behind HAS_MUPDF compile flag; falls back to stub
// when MuPDF is not built.
//
// Build: Requires Build-MuPDF.ps1 to produce libmupdf.lib in external/pdf-libs/

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// PDF rendering backend
enum class MuPDFBackend : uint8_t {
 Native = 0, ///< MuPDF native renderer
 WIC = 1, ///< Windows Imaging Component fallback
 None = 2, ///< No backend available
 COUNT
};

/// PDF page configuration
struct PDFPageConfig {
 uint32_t dpi = 150; ///< Render DPI
 uint32_t maxPages = 1; ///< Max pages to render
 bool antialias = true; ///< Enable anti-aliasing
 bool useAlpha = false; ///< Transparent background
};

/// PDF page rendering options
struct PDFRenderOptions {
 uint32_t targetWidth = 256;
 uint32_t targetHeight = 256;
 int pageIndex = 0; ///< 0-based page index to render
 float dpi = 150.0f; ///< Render DPI (higher = sharper, slower)
 bool antialias = true; ///< Enable anti-aliased rendering
 bool useTransparency = false; ///< Render with alpha channel
 bool fitToPage =
 true; ///< Scale to fit target dimensions preserving aspect ratio
};

/// Result of a PDF render operation
struct PDFRenderResult {
 bool success = false;
 uint32_t width = 0;
 uint32_t height = 0;
 uint32_t pageCount = 0;
 std::vector<uint8_t> pixelData; ///< BGRA8 pixel data
 std::string errorMessage;
 double renderTimeMs = 0.0;
};

/// PDF document metadata
struct PDFDocumentInfo {
 std::string title;
 std::string author;
 std::string subject;
 std::string creator;
 uint32_t pageCount = 0;
 float pageWidthPt = 0.0f;
 float pageHeightPt = 0.0f;
 bool isEncrypted = false;
 bool isLinearized = false;
};

/// MuPDF library integration — thread-safe singleton
class MuPDFIntegration {
public:
 static MuPDFIntegration &Instance() {
 static MuPDFIntegration instance;
 return instance;
 }

 /// Check if MuPDF library is available
 bool IsAvailable() const {
#ifdef HAS_MUPDF
 return m_initialized;
#else
 return false;
#endif
 }

 /// Get library version string
 const char *GetVersion() const {
#ifdef HAS_MUPDF
 return "1.24.11";
#else
 return "not available";
#endif
 }

 /// Render a page from a PDF file to BGRA8 pixel buffer
 PDFRenderResult RenderPage(const wchar_t *filePath,
 const PDFRenderOptions &options = {}) {
 PDFRenderResult result;
#ifdef HAS_MUPDF
 std::lock_guard<std::mutex> lock(m_renderMutex);
 result = RenderPageInternal(filePath, options);
#else
 (void)filePath;
 (void)options;
 result.success = false;
 result.errorMessage = "MuPDF not built (HAS_MUPDF=OFF)";
#endif
 return result;
 }

 /// Get document metadata without rendering
 PDFDocumentInfo GetDocumentInfo(const wchar_t *filePath) {
 PDFDocumentInfo info;
#ifdef HAS_MUPDF
 std::lock_guard<std::mutex> lock(m_renderMutex);
 info = GetDocumentInfoInternal(filePath);
#else
 (void)filePath;
#endif
 return info;
 }

 /// Generate HBITMAP thumbnail from PDF (convenience wrapper for COM shell
 /// extension)
 HBITMAP GenerateThumbnail(const wchar_t *filePath, uint32_t cx, uint32_t cy) {
 PDFRenderOptions opts;
 opts.targetWidth = cx;
 opts.targetHeight = cy;
 opts.fitToPage = true;
 opts.antialias = true;

 auto result = RenderPage(filePath, opts);
 if (!result.success || result.pixelData.empty())
 return nullptr;

 return CreateBitmapFromPixels(result.pixelData.data(), result.width,
 result.height);
 }

 /// Rendering capability flags
 enum class Capability : uint32_t {
 None = 0,
 BasicRender = 1 << 0, ///< Page rendering
 TextExtract = 1 << 1, ///< Text extraction
 Annotations = 1 << 2, ///< Annotation rendering
 Transparency = 1 << 3, ///< Alpha channel support
 ColorManagement = 1 << 4, ///< ICC profile handling
 HighDPI = 1 << 5, ///< DPI-aware rendering
 All = 0x3F
 };

 Capability GetCapabilities() const {
 if (!IsAvailable())
 return Capability::None;
 return Capability::All;
 }

 /// Backend name lookup
 static const wchar_t *BackendName(MuPDFBackend b) {
 switch (b) {
 case MuPDFBackend::Native:
 return L"MuPDF Native";
 case MuPDFBackend::WIC:
 return L"Windows Imaging";
 case MuPDFBackend::None:
 return L"None";
 default:
 return L"Unknown";
 }
 }

 /// Number of backends
 static constexpr size_t BackendCount() {
 return static_cast<size_t>(MuPDFBackend::COUNT);
 }

private:
 MuPDFIntegration() {
#ifdef HAS_MUPDF
 m_initialized = InitializeLibrary();
#endif
 }

 ~MuPDFIntegration() {
#ifdef HAS_MUPDF
 ShutdownLibrary();
#endif
 }

 MuPDFIntegration(const MuPDFIntegration &) = delete;
 MuPDFIntegration &operator=(const MuPDFIntegration &) = delete;

#ifdef HAS_MUPDF
 bool InitializeLibrary();
 void ShutdownLibrary();
 PDFRenderResult RenderPageInternal(const wchar_t *filePath,
 const PDFRenderOptions &options);
 PDFDocumentInfo GetDocumentInfoInternal(const wchar_t *filePath);
#endif

 HBITMAP CreateBitmapFromPixels(const uint8_t *pixels, uint32_t w,
 uint32_t h) {
 BITMAPINFO bmi = {};
 bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
 bmi.bmiHeader.biWidth = static_cast<LONG>(w);
 bmi.bmiHeader.biHeight = -static_cast<LONG>(h); // top-down
 bmi.bmiHeader.biPlanes = 1;
 bmi.bmiHeader.biBitCount = 32;
 bmi.bmiHeader.biCompression = BI_RGB;

 void *bits = nullptr;
 HBITMAP hbmp =
 CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
 if (hbmp && bits) {
 memcpy(bits, pixels, static_cast<size_t>(w) * h * 4);
 }
 return hbmp;
 }

 bool m_initialized = false;
 std::mutex m_renderMutex;
};

inline MuPDFIntegration::Capability operator|(MuPDFIntegration::Capability a,
 MuPDFIntegration::Capability b) {
 return static_cast<MuPDFIntegration::Capability>(static_cast<uint32_t>(a) |
 static_cast<uint32_t>(b));
}
inline MuPDFIntegration::Capability operator&(MuPDFIntegration::Capability a,
 MuPDFIntegration::Capability b) {
 return static_cast<MuPDFIntegration::Capability>(static_cast<uint32_t>(a) &
 static_cast<uint32_t>(b));
}

} // namespace Engine
} // namespace ExplorerLens
