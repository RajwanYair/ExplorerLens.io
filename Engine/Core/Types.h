//==============================================================================
// ExplorerLens Engine - Common Types & Structures
// Version: 1.0.0
// Copyright (c) 2026 - ExplorerLens Project
//
// This file defines core types, enumerations, and structures used throughout
// the ExplorerLens thumbnail generation engine. All decoder implementations
// and pipeline components depend on these foundational types.
//
// Thread Safety: All types defined here are thread-safe for read operations.
// Structures with mutable fields must be synchronized externally.
//==============================================================================

#pragma once

#include <stdint.h>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// Version Information
//==============================================================================

// Version Information — canonical version defined in Engine/Engine.h
// These are kept in sync; use EXPLORERLENS_ENGINE_VERSION_MAJOR/MINOR/PATCH
// from Engine.h for numeric checks, or EXPLORERLENS_ENGINE_VERSION_STRING
// below.

//==============================================================================
// Forward Declarations
//==============================================================================

class IThumbnailDecoder;
class IFormatDetector;
class ICacheProvider;
class IGPURenderer;

//==============================================================================
// Enumerations
//==============================================================================

/// Detected format types for the FormatDetector pipeline
///
/// Each format type corresponds to one or more file extensions.
/// Decoders register themselves to handle specific format types.
/// NOTE: This is distinct from FormatType in FormatRegistry.h (legacy LENSTYPE
/// mapping).
enum class DetectedFormat : uint32_t {
 Unknown = 0, ///< Unknown or unsupported format

 // Image Formats
 ImageJPEG, ///< JPEG (.jpg, .jpeg)
 ImagePNG, ///< PNG (.png) with alpha support
 ImageBMP, ///< Windows Bitmap (.bmp)
 ImageGIF, ///< GIF (.gif) - first frame only
 ImageTIFF, ///< TIFF (.tif, .tiff) multi-page support
 ImageWEBP, ///< WebP (.webp) modern image format
 ImageAVIF, ///< AVIF (.avif) AV1-based format
 ImageHEIF, ///< HEIF/HEIC (.heif, .heic) iPhone photos
 ImageJXL, ///< JPEG XL (.jxl) next-gen compression
 ImageICO, ///< Icon (.ico)
 ImageRAW, ///< Camera RAW (.cr2, .nef, .arw, .dng, etc.)
 ImagePSD, ///< Adobe Photoshop (.psd, .psb)
 ImageDDS, ///< DirectDraw Surface (.dds) game textures
 ImageHDR, ///< Radiance RGBE (.hdr) HDR images
 ImageEXR, ///< OpenEXR (.exr) HDR/VFX images
 ImagePPM, ///< Netpbm (.ppm, .pgm, .pbm, .pnm, .pfm)
 ImageTGA, ///< Targa (.tga) already decoded by TGADecoder
 ImageQOI, ///< Quite OK Image (.qoi) already decoded
 ImageSVG, ///< SVG vector (.svg, .svgz)

 // Archive Formats (containing images)
 ArchiveZIP, ///< ZIP archives (.zip, .cbz)
 ArchiveRAR, ///< RAR archives (.rar, .cbr)
 Archive7Z, ///< 7-Zip archives (.7z, .cb7)
 ArchiveTAR, ///< TAR archives (.tar, .cbt)

 // Document Formats
 DocumentPDF,
 DocumentEPUB,

 // Video Formats
 VideoMP4,
 VideoMKV,
 VideoAVI,

 // Audio Formats (with album art)
 AudioMP3,
 AudioFLAC,

 // Font Formats
 FontTTF, ///< TrueType/OpenType fonts (.ttf, .otf, .woff, .ttc)
};

/// Thumbnail generation flags
enum class ThumbnailFlags : uint32_t {
 None = 0,
 FastMode = 1 << 0, // Prioritize speed over quality
 UseGPU = 1 << 1, // Enable GPU acceleration if available
 UseCache = 1 << 2, // Check cache before generating
 HighQuality = 1 << 3, // Use high-quality filtering
 PreserveAspect = 1 << 4, // Preserve aspect ratio (default)
};

inline ThumbnailFlags operator|(ThumbnailFlags a, ThumbnailFlags b) {
 return static_cast<ThumbnailFlags>(static_cast<uint32_t>(a) |
 static_cast<uint32_t>(b));
}

inline bool operator&(ThumbnailFlags a, ThumbnailFlags b) {
 return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

//==============================================================================
// Core Structures
//==============================================================================

/// Request structure for thumbnail generation
struct ThumbnailRequest {
 /// Full path to the source file
 const wchar_t *filePath;

 /// Desired thumbnail width (pixels)
 union {
 uint32_t width;
 uint32_t outputWidth; // Backward compatibility alias
 };

 /// Desired thumbnail height (pixels)
 union {
 uint32_t height;
 uint32_t outputHeight; // Backward compatibility alias
 };

 /// Generation flags (combination of ThumbnailFlags)
 ThumbnailFlags flags;

 /// Optional: Archive entry path (for ZIP/RAR files, e.g., "page001.jpg")
 const wchar_t *archiveEntry;

 ThumbnailRequest()
 : filePath(nullptr), width(256), height(256),
 flags(ThumbnailFlags::PreserveAspect | ThumbnailFlags::UseCache),
 archiveEntry(nullptr) {}
};

/// Result structure for thumbnail generation
struct ThumbnailResult {
 /// Generated thumbnail bitmap (caller must delete with DeleteObject)
 union {
 HBITMAP hBitmap;
 HBITMAP bitmap; // Backward compatibility alias
 };

 /// Actual thumbnail width (may differ from request if aspect ratio preserved)
 uint32_t width;

 /// Actual thumbnail height
 uint32_t height;

 /// Result status code (S_OK on success, error HRESULT on failure)
 HRESULT status;

 /// True if thumbnail was loaded from cache
 bool fromCache;

 /// True if GPU acceleration was used
 bool usedGPU;

 /// Generation time in milliseconds
 uint32_t generationTimeMs;

 ThumbnailResult()
 : hBitmap(nullptr), width(0), height(0), status(E_FAIL), fromCache(false),
 usedGPU(false), generationTimeMs(0) {}
};

/// Decoder capability information
struct DecoderInfo {
 /// Decoder name (e.g., "Image Decoder", "ZIP Archive Decoder")
 const wchar_t *name;

 /// Decoder version (e.g., "1.0.0")
 const wchar_t *version;

 /// Supported file extensions (null-terminated array)
 const wchar_t **supportedExtensions;

 /// Number of extensions in supportedExtensions array
 uint32_t extensionCount;

 /// True if decoder supports GPU acceleration
 bool supportsGPU;

 /// True if decoder can handle archive formats
 bool isArchiveDecoder;
};

//==============================================================================
// Error Codes
//==============================================================================

/// Engine-specific HRESULTs
#define DT_E_UNSUPPORTED_FORMAT \
 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0201)
#define DT_E_DECODER_NOT_FOUND \
 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0202)
#define DT_E_INVALID_IMAGE_DATA \
 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0203)
#define DT_E_GPU_NOT_AVAILABLE \
 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0204)
#define DT_E_CACHE_ERROR MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0205)
#define DT_E_ARCHIVE_EXTRACT_FAILED \
 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0206)

} // namespace Engine
} // namespace ExplorerLens
