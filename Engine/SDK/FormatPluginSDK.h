// FormatPluginSDK.h — v2 Format Plugin Contract
// Copyright (c) 2026 ExplorerLens Project
//
// Defines the v2 format plugin contract for format decoders that ship as
// separate DLLs. A format plugin exports a single factory function conforming
// to LENS_FORMAT_PLUGIN_FACTORY_FN and can register multiple file extensions.
// V2 adds async decode, metadata extraction, and resource budget hints.
//
#pragma once
#include <windows.h>
#include <cstdint>
#include "../SDK/PublicAPI.h"

// Maximum number of extensions a single format plugin can handle
#define LENS_FORMAT_MAX_EXTS  64

// Format plugin version for ABI compatibility check
#define LENS_FORMAT_PLUGIN_ABI  0x0200  // v2.0

// Hint flags for resource scheduling
#define LENS_FMT_HINT_LARGE_FILE    0x0001  // Files > 50 MB expected
#define LENS_FMT_HINT_NEEDS_GPU     0x0002  // GPU-accelerated path available
#define LENS_FMT_HINT_SLOW_DECODE   0x0004  // CPU decode > 100ms typical
#define LENS_FMT_HINT_VECTOR        0x0008  // Vector / re-renderable format
#define LENS_FMT_HINT_MULTI_FRAME   0x0010  // Animated / multi-page

// Colorspace identifiers
typedef enum LENS_COLORSPACE : uint32_t {
    LENS_CS_SRGB   = 0,
    LENS_CS_LINEAR = 1,
    LENS_CS_P3     = 2,
    LENS_CS_REC2020= 3,
} LENS_COLORSPACE;

// Extended image descriptor returned by v2 format plugins
typedef struct LENS_FORMAT_FRAME_V2 {
    uint32_t         width;
    uint32_t         height;
    uint32_t         stride;
    uint32_t         pixelFormat;   // 0=BGRA32, 1=RGBA32, 2=RGB24, 3=RGBA64F
    void*            pixels;        // Owned by plugin; freed via FreeFrameV2
    uint32_t         frameIndex;
    uint32_t         frameCount;
    LENS_COLORSPACE  colorspace;
    float            displayGamma;  // 0 = unknown
    uint32_t         bitsPerChannel;
    uint64_t         reserved[2];
} LENS_FORMAT_FRAME_V2;

// Resource budget hint the engine passes to the plugin before decode
typedef struct LENS_RESOURCE_BUDGET {
    uint64_t maxMemoryBytes;  // Soft limit for decode scratch memory
    uint32_t timeoutMs;
    uint32_t gpuAdapterIndex; // 0xFFFFFFFF = CPU only
    uint64_t reserved[2];
} LENS_RESOURCE_BUDGET;

// V2 plugin vtable
typedef struct LENS_FORMAT_PLUGIN_VTABLE_V2 {
    uint16_t abiVersion;   // Must equal LENS_FORMAT_PLUGIN_ABI

    // Synchronous decode: file → first frame at requested size
    LENS_RESULT (WINAPI* DecodeFile)(
        void*                       pCtx,
        const wchar_t*              filePath,
        uint32_t                    requestedW,
        uint32_t                    requestedH,
        const LENS_RESOURCE_BUDGET* pBudget,
        LENS_FORMAT_FRAME_V2*       pFrame);   // out

    // Synchronous decode from IStream*
    LENS_RESULT (WINAPI* DecodeStream)(
        void*                       pCtx,
        void*                       pStream,
        const wchar_t*              hintExt,
        uint32_t                    requestedW,
        uint32_t                    requestedH,
        const LENS_RESOURCE_BUDGET* pBudget,
        LENS_FORMAT_FRAME_V2*       pFrame);

    void       (WINAPI* FreeFrameV2)(void* pCtx, LENS_FORMAT_FRAME_V2* pFrame);

    // Optional: return metadata as UTF-8 JSON; returns LENS_OK or LENS_E_FAIL
    LENS_RESULT (WINAPI* GetMetadataJSON)(void* pCtx, const wchar_t* filePath,
                                          char* jsonBuf, uint32_t bufLen);

    // Optional: async decode; returns handle polled via PollAsync
    LENS_RESULT (WINAPI* BeginDecodeAsync)(void* pCtx, const wchar_t* filePath,
                                           uint32_t w, uint32_t h,
                                           const LENS_RESOURCE_BUDGET* pBudget,
                                           void** phAsyncOp);

    LENS_RESULT (WINAPI* PollAsync)(void* pCtx, void* hAsyncOp,
                                    LENS_FORMAT_FRAME_V2* pFrame, uint32_t* pPercentDone);

    void        (WINAPI* CancelAsync)(void* pCtx, void* hAsyncOp);
    void        (WINAPI* Destroy)(void* pCtx);
    uint64_t    reserved[4];
} LENS_FORMAT_PLUGIN_VTABLE_V2;

// Plugin manifest attached to each format plugin DLL
typedef struct LENS_FORMAT_PLUGIN_MANIFEST {
    uint32_t  structSize;
    wchar_t   name[64];
    wchar_t   vendor[64];
    uint32_t  version;         // (major<<16)|(minor<<8)|patch
    uint32_t  hintFlags;       // LENS_FMT_HINT_* bitmask
    uint32_t  extensionCount;
    wchar_t   extensions[LENS_FORMAT_MAX_EXTS][16];
    uint32_t  priority;        // Lower = higher; default 100
    uint64_t  reserved[4];
} LENS_FORMAT_PLUGIN_MANIFEST;

// Factory function exported as "LensCreateFormatPlugin" from the plugin DLL
typedef LENS_RESULT (WINAPI* LENS_FORMAT_PLUGIN_FACTORY_FN)(
    void**                        ppCtx,
    LENS_FORMAT_PLUGIN_VTABLE_V2* pVTable,
    LENS_FORMAT_PLUGIN_MANIFEST*  pManifest);   // out
