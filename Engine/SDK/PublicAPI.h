// PublicAPI.h — Stable C ABI Public API Surface
// Copyright (c) 2026 ExplorerLens Project
//
// Declares the stable extern "C" ABI exported by ExplorerLensEngine.dll.
// All symbols here are guaranteed backward-compatible across minor versions.
// Third-party integrators and plugin authors use ONLY this header.
//
#pragma once
#include <windows.h>
#include <cstdint>

#ifdef EXPLORERLENS_EXPORTS
#  define LENS_API __declspec(dllexport)
#else
#  define LENS_API __declspec(dllimport)
#endif

#define LENS_API_VERSION_MAJOR 20
#define LENS_API_VERSION_MINOR 3
#define LENS_API_VERSION_PATCH 0
#define LENS_API_VERSION ((LENS_API_VERSION_MAJOR << 16) | (LENS_API_VERSION_MINOR << 8) | LENS_API_VERSION_PATCH)

// Opaque handle types
typedef void* LENS_ENGINE_HANDLE;
typedef void* LENS_THUMBNAIL_HANDLE;
typedef void* LENS_STREAM_HANDLE;

// Result codes
typedef enum LENS_RESULT : int32_t {
    LENS_OK               =  0,
    LENS_E_FAIL           = -1,
    LENS_E_INVALID_ARG    = -2,
    LENS_E_OUT_OF_MEMORY  = -3,
    LENS_E_FORMAT_UNKNOWN = -4,
    LENS_E_CORRUPT        = -5,
    LENS_E_NOT_LICENSED   = -6,
    LENS_E_TIMEOUT        = -7,
} LENS_RESULT;

// Image descriptor returned with thumbnails
typedef struct LENS_IMAGE_DESC {
    uint32_t width;
    uint32_t height;
    uint32_t stride;      // bytes per row
    uint32_t pixelFormat; // 0 = BGRA32, 1 = RGBA32, 2 = RGB24
    void*    pixels;      // owned by LENS_THUMBNAIL_HANDLE until LensReleaseThumbnail
    uint64_t reserved;
} LENS_IMAGE_DESC;

// Thumbnail request options
typedef struct LENS_THUMB_OPTIONS {
    uint32_t requestedWidth;
    uint32_t requestedHeight;
    uint32_t flags;    // bit 0: force CPU decode; bit 1: high quality
    uint32_t timeoutMs;
    wchar_t  reserved[32];
} LENS_THUMB_OPTIONS;

#ifdef __cplusplus
extern "C" {
#endif

// Engine lifecycle
LENS_API LENS_RESULT LensEngineCreate(LENS_ENGINE_HANDLE* phEngine);
LENS_API void        LensEngineDestroy(LENS_ENGINE_HANDLE hEngine);

// Version query — safe to call before LensEngineCreate
LENS_API uint32_t    LensGetAPIVersion(void);
LENS_API const char* LensGetVersionString(void); // "20.3.0"

// Thumbnail generation from path
LENS_API LENS_RESULT LensGenerateThumbnail(
    LENS_ENGINE_HANDLE  hEngine,
    const wchar_t*      filePath,
    const LENS_THUMB_OPTIONS* options,
    LENS_THUMBNAIL_HANDLE* phThumb);

// Thumbnail generation from IStream COM pointer (REFIID = IID_IStream)
LENS_API LENS_RESULT LensGenerateThumbnailFromStream(
    LENS_ENGINE_HANDLE  hEngine,
    void*               pStream,     // IStream*
    const wchar_t*      hintExt,     // e.g. L".psd"  (may be NULL)
    const LENS_THUMB_OPTIONS* options,
    LENS_THUMBNAIL_HANDLE* phThumb);

// Access thumbnail pixels — valid until LensReleaseThumbnail
LENS_API LENS_RESULT LensGetImageDesc(LENS_THUMBNAIL_HANDLE hThumb, LENS_IMAGE_DESC* pDesc);

// Release thumbnail memory
LENS_API void        LensReleaseThumbnail(LENS_THUMBNAIL_HANDLE hThumb);

// Format support query
LENS_API int         LensIsFormatSupported(const wchar_t* extension); // 1 = yes, 0 = no

// Cache management
LENS_API LENS_RESULT LensClearCache(LENS_ENGINE_HANDLE hEngine);
LENS_API LENS_RESULT LensGetCacheStats(LENS_ENGINE_HANDLE hEngine,
                                       uint64_t* bytesUsed, uint64_t* bytesCap);

#ifdef __cplusplus
} // extern "C"
#endif
