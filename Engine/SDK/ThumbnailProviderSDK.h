// ThumbnailProviderSDK.h — Third-Party Thumbnail Provider Registration SDK
// Copyright (c) 2026 ExplorerLens Project
//
// Allows external DLLs to register as thumbnail providers for custom formats.
// The SDK consumer implements IThumbnailPlugin and registers a factory via
// LensRegisterThumbnailProvider() from PublicAPI.h.
//
#pragma once
#include <windows.h>
#include <cstdint>
#include "PublicAPI.h"

// Maximum extension count per provider
#define LENS_MAX_EXTENSIONS 32

// Provider capability flags
#define LENS_CAP_GPU_DECODE   0x0001  // Provider supports GPU-accelerated decode
#define LENS_CAP_ANIMATED     0x0002  // Provider can decode animated/multi-frame
#define LENS_CAP_METADATA     0x0004  // Provider returns embedded metadata
#define LENS_CAP_STREAMING    0x0008  // Provider accepts partial (streaming) data

// A decoded frame returned by IThumbnailPlugin
typedef struct LENS_PLUGIN_FRAME {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t pixelFormat;  // LENS_FMT_BGRA32=0, LENS_FMT_RGBA32=1
    void*    pixels;       // Plugin-owned; freed via LENS_PLUGIN_VTABLE::FreeFrame
    uint32_t frameIndex;
    uint32_t frameCount;
} LENS_PLUGIN_FRAME;

// V-table for a thumbnail plugin implementation
typedef struct LENS_PLUGIN_VTABLE {
    // ABI version — set to LENS_API_VERSION
    uint32_t abiVersion;

    // Required: decode file at path → first frame
    LENS_RESULT (WINAPI* Decode)(
        void*              pCtx,
        const wchar_t*     filePath,
        uint32_t           requestedWidth,
        uint32_t           requestedHeight,
        LENS_PLUGIN_FRAME* pFrame);         // out

    // Required: free memory allocated by Decode
    void (WINAPI* FreeFrame)(void* pCtx, LENS_PLUGIN_FRAME* pFrame);

    // Optional (may be NULL): decode from IStream*
    LENS_RESULT (WINAPI* DecodeStream)(
        void*              pCtx,
        void*              pStream,
        const wchar_t*     hintExt,
        uint32_t           requestedWidth,
        uint32_t           requestedHeight,
        LENS_PLUGIN_FRAME* pFrame);

    // Optional (may be NULL): return metadata as UTF-8 JSON
    LENS_RESULT (WINAPI* GetMetadata)(void* pCtx, const wchar_t* filePath,
                                      char* jsonBuf, uint32_t bufLen);

    // Required: free pCtx allocated by factory
    void (WINAPI* Destroy)(void* pCtx);

    uint64_t reserved[4];
} LENS_PLUGIN_VTABLE;

// Provider descriptor submitted to LensRegisterThumbnailProvider
typedef struct LENS_PROVIDER_DESC {
    uint32_t      structSize;       // sizeof(LENS_PROVIDER_DESC)
    wchar_t       name[64];         // Display name e.g. L"MyCADThumb"
    wchar_t       vendor[64];
    uint32_t      version;          // Encoded: (major<<16)|(minor<<8)|patch
    uint32_t      capabilityFlags;  // LENS_CAP_* bitmask
    uint32_t      priority;         // Lower = higher priority; default 100
    uint32_t      extensionCount;
    wchar_t       extensions[LENS_MAX_EXTENSIONS][16]; // e.g. L".step", L".iges"

    // Factory: create a plugin context (pCtx) and fill pVTable
    LENS_RESULT (WINAPI* CreateInstance)(void** ppCtx, LENS_PLUGIN_VTABLE* pVTable);

    uint64_t      reserved[4];
} LENS_PROVIDER_DESC;

#ifdef __cplusplus
extern "C" {
#endif

// Register a provider — call from DllMain(DLL_PROCESS_ATTACH) or on first use
LENS_API LENS_RESULT LensRegisterThumbnailProvider(
    LENS_ENGINE_HANDLE          hEngine,
    const LENS_PROVIDER_DESC*   pDesc,
    uint32_t*                   pProviderId);  // out: assigned provider ID

// Unregister — call from DllMain(DLL_PROCESS_DETACH)
LENS_API LENS_RESULT LensUnregisterThumbnailProvider(
    LENS_ENGINE_HANDLE hEngine,
    uint32_t           providerId);

// Query all registered providers (pass NULL pDescs to get count only)
LENS_API LENS_RESULT LensEnumThumbnailProviders(
    LENS_ENGINE_HANDLE  hEngine,
    LENS_PROVIDER_DESC* pDescs,    // may be NULL
    uint32_t*           pCount);   // in: capacity; out: actual count

#ifdef __cplusplus
} // extern "C"
#endif
