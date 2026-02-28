//==============================================================================
// ExplorerLens Engine — Modular Codec Interface (ICodecModule)
// Execution Optimization — Per-Format DLL Architecture
// Copyright (c) 2026 — ExplorerLens Project
//
// PURPOSE:
// Define the binary-stable C ABI that every codec DLL exports.
// Each codec DLL is a self-contained decoder for one family of file formats.
// The shell extension (LENSShell.dll) loads codec DLLs on demand so that
// browsing a directory containing only JPEGs never pages in the RAW or
// HEIF codec — cutting working-set by 10-40 MB per unused codec.
//
// DESIGN PRINCIPLES:
// • C ABI (extern "C") — no name mangling, no vtable layout dependency
// • Version-tagged structures — forward-compatible without recompilation
// • Minimal surface — 5 exported functions per DLL
// • No heap cross-talk — DLL allocates, DLL frees (DtCodec_FreeResult)
// • Thread-safe — codecs must be re-entrant for Explorer's thread pool
//
// MEMORY MODEL:
// Codec DLLs own all memory they allocate. The host calls
// DtCodec_FreeResult() to release pixel buffers. HBITMAPs are created
// by the host from the returned pixel data, so codec DLLs never need
// to link against GDI32 or user32.
//
// BUILD NOTES:
// Codec DLLs link ONLY against the libraries they need:
// ExplorerLens_Codec_WebP.dll → libwebp, libsharpyuv
// ExplorerLens_Codec_HEIF.dll → libheif, libde265
// ExplorerLens_Codec_JXL.dll → libjxl, brotli, highway
// ExplorerLens_Codec_RAW.dll → LibRaw
// … etc.
// The host (LENSShell.dll) links against NONE of these.
//==============================================================================

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// ABI Version — bump MINOR for additive changes, MAJOR for breaking changes
//==============================================================================
#define DTCODEC_ABI_VERSION_MAJOR 1
#define DTCODEC_ABI_VERSION_MINOR 0

//==============================================================================
// Export / Import macros
//==============================================================================
#ifdef DTCODEC_EXPORTS
# define DTCODEC_API __declspec(dllexport)
#else
# define DTCODEC_API __declspec(dllimport)
#endif

//==============================================================================
// Pixel format for decoded thumbnails
//==============================================================================
typedef enum DtPixelFormat
{
 DT_PIXEL_BGRA32 = 0, ///< 32-bit BGRA (GDI-native, pre-multiplied alpha OK)
 DT_PIXEL_BGR24 = 1, ///< 24-bit BGR (no alpha, most compact)
 DT_PIXEL_RGBA32 = 2, ///< 32-bit RGBA (needs swizzle before GDI blit)
} DtPixelFormat;

//==============================================================================
// Codec capability flags (bitmask)
//==============================================================================
typedef enum DtCodecCaps
{
 DT_CAP_NONE = 0,
 DT_CAP_GPU_ACCEL = 1 << 0, ///< Codec can use GPU (D3D11/DirectML)
 DT_CAP_ANIMATION = 1 << 1, ///< Codec handles animated formats
 DT_CAP_MULTI_PAGE = 1 << 2, ///< Codec handles multi-page containers
 DT_CAP_HDR = 1 << 3, ///< Codec can tone-map HDR to SDR
 DT_CAP_ARCHIVE = 1 << 4, ///< Codec extracts from archive container
 DT_CAP_METADATA = 1 << 5, ///< Codec can extract EXIF/XMP metadata
 DT_CAP_ICC_PROFILE = 1 << 6, ///< Codec applies ICC color correction
 DT_CAP_THREAD_SAFE = 1 << 7, ///< Codec is fully re-entrant
} DtCodecCaps;

//==============================================================================
// Codec module descriptor — returned by DtCodec_GetModuleInfo()
//
// Size-versioned: callers set `structSize = sizeof(DtCodecModuleInfo)` before
// calling. Codecs check structSize to decide which fields to fill.
//==============================================================================
typedef struct DtCodecModuleInfo
{
 uint32_t structSize; ///< Must be sizeof(DtCodecModuleInfo)
 uint32_t abiVersionMajor; ///< DTCODEC_ABI_VERSION_MAJOR
 uint32_t abiVersionMinor; ///< DTCODEC_ABI_VERSION_MINOR

 /// Human-readable codec name, e.g. "WebP Codec"
 const wchar_t* codecName;

 /// Codec version string, e.g. "1.5.0 (libwebp)"
 const wchar_t* codecVersion;

 /// Null-terminated array of supported extensions (with dots).
 /// Example: { L".webp", nullptr }
 const wchar_t** supportedExtensions;

 /// Number of extensions (excludes trailing nullptr)
 uint32_t extensionCount;

 /// Bitmask of DtCodecCaps
 uint32_t capabilities;

 /// Approximate working-set overhead when codec is loaded (bytes).
 /// Used by the loader for memory budgeting (0 = unknown).
 uint64_t estimatedMemoryBytes;

 /// Unique codec identifier for registry / logging.
 /// Convention: "explorerlens.codec.<family>", e.g. "explorerlens.codec.webp"
 const char* codecId;

} DtCodecModuleInfo;

//==============================================================================
// Decode request — passed to DtCodec_DecodeThumbnail()
//==============================================================================
typedef struct DtDecodeRequest
{
 uint32_t structSize; ///< Must be sizeof(DtDecodeRequest)

 /// Full path to file to decode (UTF-16)
 const wchar_t* filePath;

 /// Desired thumbnail width (pixels)
 uint32_t maxWidth;

 /// Desired thumbnail height (pixels)
 uint32_t maxHeight;

 /// Combination of DtDecodeFlags
 uint32_t flags;

 /// Reserved for future use — must be zero
 uint64_t reserved[4];

} DtDecodeRequest;

/// Flags for DtDecodeRequest.flags
typedef enum DtDecodeFlags
{
 DT_DECODE_DEFAULT = 0,
 DT_DECODE_FAST_MODE = 1 << 0, ///< Speed over quality (use 1/8 JPEG, half-size RAW)
 DT_DECODE_HIGH_QUALITY = 1 << 1, ///< Lanczos / high-quality rescale
 DT_DECODE_PRESERVE_ASPECT = 1 << 2, ///< Maintain aspect ratio
 DT_DECODE_FORCE_SDR = 1 << 3, ///< Tone-map HDR images to SDR
 DT_DECODE_EXTRACT_METADATA = 1 << 4, ///< Populate metadata fields in result
} DtDecodeFlags;

//==============================================================================
// Decode result — filled by DtCodec_DecodeThumbnail(), freed by DtCodec_FreeResult()
//==============================================================================
typedef struct DtDecodeResult
{
 uint32_t structSize; ///< Must be sizeof(DtDecodeResult)

 /// Decoded pixel buffer — CODEC-OWNED memory, freed by DtCodec_FreeResult()
 uint8_t* pixelData;

 /// Stride in bytes per row (>= width * bytesPerPixel, may include padding)
 uint32_t stride;

 /// Actual decoded width
 uint32_t width;

 /// Actual decoded height
 uint32_t height;

 /// Pixel format of pixelData
 DtPixelFormat pixelFormat;

 /// 0 on success, Win32 error code on failure
 uint32_t errorCode;

 /// Human-readable error message (nullptr on success)
 const wchar_t* errorMessage;

 /// Decode wall-clock time in microseconds
 uint64_t decodeTimeUs;

 /// Peak memory used during decode (bytes, 0 = unknown)
 uint64_t peakMemoryBytes;

 /// Reserved for metadata expansion
 uint64_t reserved[4];

} DtDecodeResult;

//==============================================================================
// Codec health / statistics — returned by DtCodec_GetHealth()
//==============================================================================
typedef struct DtCodecHealth
{
 uint32_t structSize; ///< Must be sizeof(DtCodecHealth)
 uint64_t totalDecodes; ///< Lifetime decode calls
 uint64_t totalErrors; ///< Lifetime decode errors
 uint64_t totalBytesDecoded; ///< Cumulative input bytes processed
 uint64_t currentMemoryBytes; ///< Current heap usage inside codec
 uint64_t peakMemoryBytes; ///< Peak heap usage inside codec
 uint64_t avgDecodeTimeUs; ///< Rolling average decode latency
 uint32_t threadCount; ///< Active decode threads inside codec
} DtCodecHealth;

//==============================================================================
// Exported functions — every codec DLL must export all five
//==============================================================================

/// 1) Initialize the codec. Called once after LoadLibrary.
/// Returns 0 on success, Win32 error code on failure.
typedef uint32_t (__stdcall *PFN_DtCodec_Initialize)(void);

/// 2) Fill module info descriptor.
/// Caller pre-sets info->structSize before calling.
typedef uint32_t (__stdcall *PFN_DtCodec_GetModuleInfo)(DtCodecModuleInfo* info);

/// 3) Decode a thumbnail.
/// Caller pre-sets request->structSize.
/// On success result->pixelData is non-null; call DtCodec_FreeResult to release.
typedef uint32_t (__stdcall *PFN_DtCodec_DecodeThumbnail)(
 const DtDecodeRequest* request,
 DtDecodeResult* result);

/// 4) Free a decode result (releases pixelData and errorMessage).
typedef void (__stdcall *PFN_DtCodec_FreeResult)(DtDecodeResult* result);

/// 5) Shut down the codec. Called once before FreeLibrary.
typedef void (__stdcall *PFN_DtCodec_Shutdown)(void);

/// 6) OPTIONAL — health telemetry (nullptr OK if not exported)
typedef uint32_t (__stdcall *PFN_DtCodec_GetHealth)(DtCodecHealth* health);


//==============================================================================
// Convenience: function names as string constants (for GetProcAddress)
//==============================================================================
#define DTCODEC_FN_INITIALIZE "DtCodec_Initialize"
#define DTCODEC_FN_GETMODULEINFO "DtCodec_GetModuleInfo"
#define DTCODEC_FN_DECODETHUMBNAIL "DtCodec_DecodeThumbnail"
#define DTCODEC_FN_FREERESULT "DtCodec_FreeResult"
#define DTCODEC_FN_SHUTDOWN "DtCodec_Shutdown"
#define DTCODEC_FN_GETHEALTH "DtCodec_GetHealth"

#ifdef __cplusplus
} // extern "C"
#endif

