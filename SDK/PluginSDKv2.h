// PluginSDKv2.h — ExplorerLens Plugin SDK v2.0 C++ Wrapper
// Copyright (c) 2026 ExplorerLens Project
//
// C++ RAII + type-safe wrapper over the C ABI defined in plugin_api.h.
// Third-party plugins may use either the raw C ABI or this convenience header.
// This header requires C++17 or later.
//
#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

// ---------------------------------------------------------------------------
// C ABI forward declarations (from plugin_api.h)
// ---------------------------------------------------------------------------
extern "C" {

#define LENS_SDK_VERSION_MAJOR 2
#define LENS_SDK_VERSION_MINOR 0

typedef struct LENS_PixelBuffer {
    void*    pixels;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t pixelFormat;   // LENS_PIXEL_FORMAT_* constants
    uint32_t reserved[4];
} LENS_PixelBuffer;

typedef struct LENS_FormatInfo {
    const char* extension;      // e.g. ".webp"
    const char* mimeType;       // e.g. "image/webp"
    uint32_t    maxWidthHint;   // 0 = no limit
    uint32_t    maxHeightHint;
    uint32_t    flags;          // LENS_FORMAT_FLAG_* bitmask (v2.0+)
    uint32_t    reserved[3];
} LENS_FormatInfo;

typedef struct LENS_DecodeRequest {
    const void* fileData;
    size_t      fileSize;
    uint32_t    requestedWidth;
    uint32_t    requestedHeight;
    uint32_t    upscaleHint;    // 0 = none, 1 = AI upscale if available (v2.0+)
    uint32_t    flags;
} LENS_DecodeRequest;

// Format flags (v2.0+)
#define LENS_FORMAT_FLAG_ANIMATED     0x0001u
#define LENS_FORMAT_FLAG_HDR          0x0002u
#define LENS_FORMAT_FLAG_LAYERED      0x0004u
#define LENS_FORMAT_FLAG_MULTIPAGE    0x0008u
#define LENS_FORMAT_FLAG_DEPTH_MAP    0x0010u

// Plugin export table — plugin must export LENS_GetPluginInfoV2 (v2) or LENS_GetPluginInfo (v1)
typedef uint32_t (*LENS_PFN_Decode)(const LENS_DecodeRequest* req, LENS_PixelBuffer* out);
typedef void     (*LENS_PFN_FreeBuffer)(LENS_PixelBuffer* buf);
typedef uint32_t (*LENS_PFN_GetSupportedFormats)(const LENS_FormatInfo** formats, uint32_t* count);

typedef struct LENS_PluginInfoV2 {
    uint32_t                  structSize;        // sizeof(LENS_PluginInfoV2)
    uint32_t                  sdkVersionMajor;   // must == LENS_SDK_VERSION_MAJOR
    uint32_t                  sdkVersionMinor;
    const char*               pluginName;
    const char*               pluginVersion;
    const char*               authorName;
    const char*               licenseIdentifier; // SPDX identifier, e.g. "MIT"
    LENS_PFN_Decode           pfnDecode;
    LENS_PFN_FreeBuffer       pfnFreeBuffer;
    LENS_PFN_GetSupportedFormats pfnGetSupportedFormats;
    uint32_t                  capabilities;      // LENS_CAP_* bitmask
    uint32_t                  reserved[8];
} LENS_PluginInfoV2;

// Capability flags
#define LENS_CAP_GPU_ACCELERATED   0x0001u
#define LENS_CAP_THREAD_SAFE       0x0002u
#define LENS_CAP_STREAMING         0x0004u
#define LENS_CAP_METADATA          0x0008u

} // extern "C"

// ---------------------------------------------------------------------------
// C++ wrapper
// ---------------------------------------------------------------------------
namespace ExplorerLens {
namespace SDK {

// RAII pixel buffer — automatically freeing via the plugin's pfnFreeBuffer
class PixelBuffer {
public:
    PixelBuffer() = default;
    ~PixelBuffer() {
        if (m_buf.pixels && m_free) {
            m_free(&m_buf);
        }
    }

    PixelBuffer(const PixelBuffer&) = delete;
    PixelBuffer& operator=(const PixelBuffer&) = delete;

    PixelBuffer(PixelBuffer&& o) noexcept
        : m_buf(o.m_buf), m_free(o.m_free)
    {
        o.m_buf  = {};
        o.m_free = nullptr;
    }

    PixelBuffer& operator=(PixelBuffer&& o) noexcept {
        if (this != &o) {
            if (m_buf.pixels && m_free) m_free(&m_buf);
            m_buf  = o.m_buf;
            m_free = o.m_free;
            o.m_buf  = {};
            o.m_free = nullptr;
        }
        return *this;
    }

    [[nodiscard]] bool         IsValid()  const noexcept { return m_buf.pixels != nullptr; }
    [[nodiscard]] uint32_t     Width()    const noexcept { return m_buf.width; }
    [[nodiscard]] uint32_t     Height()   const noexcept { return m_buf.height; }
    [[nodiscard]] uint32_t     Stride()   const noexcept { return m_buf.stride; }
    [[nodiscard]] uint32_t     Format()   const noexcept { return m_buf.pixelFormat; }
    [[nodiscard]] const void*  Pixels()   const noexcept { return m_buf.pixels; }

    // Internal — used by Plugin::Decode
    LENS_PixelBuffer&          Raw()      noexcept { return m_buf; }
    void                       SetFree(LENS_PFN_FreeBuffer fn) noexcept { m_free = fn; }

private:
    LENS_PixelBuffer     m_buf  = {};
    LENS_PFN_FreeBuffer  m_free = nullptr;
};


// Lightweight non-owning view of a plugin's export table
class Plugin {
public:
    explicit Plugin(const LENS_PluginInfoV2* info) : m_info(info) {}

    [[nodiscard]] std::string_view Name()    const noexcept { return m_info ? m_info->pluginName    : ""; }
    [[nodiscard]] std::string_view Version() const noexcept { return m_info ? m_info->pluginVersion : ""; }
    [[nodiscard]] std::string_view Author()  const noexcept { return m_info ? m_info->authorName    : ""; }
    [[nodiscard]] bool IsGPUAccelerated()    const noexcept { return m_info && (m_info->capabilities & LENS_CAP_GPU_ACCELERATED); }
    [[nodiscard]] bool IsThreadSafe()        const noexcept { return m_info && (m_info->capabilities & LENS_CAP_THREAD_SAFE); }

    [[nodiscard]] std::span<const LENS_FormatInfo> SupportedFormats() const noexcept {
        if (!m_info || !m_info->pfnGetSupportedFormats) return {};
        const LENS_FormatInfo* fmts = nullptr;
        uint32_t count = 0;
        m_info->pfnGetSupportedFormats(&fmts, &count);
        return { fmts, count };
    }

    [[nodiscard]] std::optional<PixelBuffer> Decode(
        const void* data, size_t size,
        uint32_t reqW = 0, uint32_t reqH = 0,
        uint32_t upscaleHint = 0) const
    {
        if (!m_info || !m_info->pfnDecode) return std::nullopt;

        LENS_DecodeRequest req{};
        req.fileData       = data;
        req.fileSize       = size;
        req.requestedWidth = reqW;
        req.requestedHeight= reqH;
        req.upscaleHint    = upscaleHint;

        PixelBuffer out;
        if (m_info->pfnDecode(&req, &out.Raw()) != 0) return std::nullopt;
        out.SetFree(m_info->pfnFreeBuffer);
        return out;
    }

    [[nodiscard]] bool IsValid() const noexcept { return m_info != nullptr; }

private:
    const LENS_PluginInfoV2* m_info = nullptr;
};


// Compatibility helper — V2 validation function
inline bool ValidatePluginInfo(const LENS_PluginInfoV2* info) noexcept {
    if (!info) return false;
    if (info->structSize < sizeof(LENS_PluginInfoV2)) return false;
    if (info->sdkVersionMajor != LENS_SDK_VERSION_MAJOR) return false;
    if (!info->pfnDecode || !info->pfnFreeBuffer || !info->pfnGetSupportedFormats) return false;
    if (!info->pluginName || !info->pluginVersion) return false;
    return true;
}

} // namespace SDK
} // namespace ExplorerLens
