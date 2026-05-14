// Engine/Core/OpenClResizePath.h
// ExplorerLens Engine — S388 (Phase 4, Sprint 8)
//
// Purpose:
//   OpenCL 1.2 GPU resize fallback path (ROADMAP H17).
//   Phase 4 exit criterion: enabled when D3D11 is unavailable (e.g. Hyper-V,
//   headless server, or older GPU without D3D11 support).
//
//   Resize pipeline:
//   1. OpenClResizePath::Initialize() — cl_platform_id → cl_device_id → cl_context
//   2. OpenClResizePath::CompileKernels() — bilinear or Lanczos-3 CLSL source
//   3. OpenClResizePath::Resize() — enqueue kernel, read back, measure latency
//
//   Availability check:
//   - clGetPlatformIDs()  → NO_PLATFORM if 0 platforms
//   - clGetDeviceIDs()    → NO_DEVICE if no GPU devices
//   - kernel compilation  → BUILD_FAILED if CLSL error
//
//   Non-Win32 stub: Initialize() returns NOT_SUPPORTED.
//   MSVC build: does not link OpenCL; all paths are inline stubs.
//   Clang build on Linux: links -lOpenCL (CI lane).

#pragma once
#ifndef EXPLORERLENS_ENGINE_OPENCLRESIZEPATH_H
#define EXPLORERLENS_ENGINE_OPENCLRESIZEPATH_H

#include <cstdint>
#include <cstddef>

namespace ExplorerLens::Engine {

// ─── Availability ────────────────────────────────────────────────────────────

enum class OpenClStatus : uint8_t {
    OK               = 0,
    NOT_INITIALIZED  = 1,
    NO_PLATFORM      = 2,   // clGetPlatformIDs returned 0 platforms
    NO_DEVICE        = 3,   // no GPU devices on found platform
    BUILD_FAILED     = 4,   // OpenCL kernel compilation failed
    ENQUEUE_FAILED   = 5,
    NULL_ARGUMENT    = 6,
    ZERO_DIMENSION   = 7,
    NOT_SUPPORTED    = 8,   // non-Win32 or OpenCL not installed
};

// ─── Resize filter ───────────────────────────────────────────────────────────

enum class OpenClFilter : uint8_t {
    NEAREST   = 0,
    BILINEAR  = 1,
    LANCZOS3  = 2,
};

// ─── Config ──────────────────────────────────────────────────────────────────

struct OpenClResizeConfig final {
    OpenClFilter    filter          = OpenClFilter::BILINEAR;
    uint32_t        deviceIndex     = 0;         // which OpenCL device to use
    bool            enableProfiling = false;
    bool            verbose         = false;

    static constexpr OpenClResizeConfig Default() noexcept {
        return OpenClResizeConfig{};
    }

    static constexpr OpenClResizeConfig LanczosQuality() noexcept {
        OpenClResizeConfig c{};
        c.filter          = OpenClFilter::LANCZOS3;
        c.enableProfiling = true;
        return c;
    }

    static constexpr OpenClResizeConfig FastBilinear() noexcept {
        OpenClResizeConfig c{};
        c.filter  = OpenClFilter::BILINEAR;
        return c;
    }
};

// ─── Resize result ───────────────────────────────────────────────────────────

struct OpenClResizeResult final {
    OpenClStatus    status      = OpenClStatus::OK;
    uint32_t        srcWidthPx  = 0;
    uint32_t        srcHeightPx = 0;
    uint32_t        dstWidthPx  = 0;
    uint32_t        dstHeightPx = 0;
    OpenClFilter    filter      = OpenClFilter::BILINEAR;
    uint64_t        resizeUs    = 0;

    bool IsOk() const noexcept { return status == OpenClStatus::OK; }
};

// ─── Device info ─────────────────────────────────────────────────────────────

struct OpenClDeviceInfo final {
    char         name[128]      = {};
    char         vendor[64]     = {};
    uint32_t     maxWorkGroupSz = 0;
    uint32_t     computeUnits   = 0;
    bool         isGpu          = false;

    bool IsValid() const noexcept { return name[0] != '\0'; }
};

// ─── Path class ──────────────────────────────────────────────────────────────

class OpenClResizePath final {
public:
    OpenClResizePath() = default;
    ~OpenClResizePath() noexcept { Shutdown(); }

    OpenClResizePath(const OpenClResizePath&) = delete;
    OpenClResizePath& operator=(const OpenClResizePath&) = delete;

    static OpenClResizePath& Global() noexcept {
        static OpenClResizePath s_instance;
        return s_instance;
    }

    void Configure(const OpenClResizeConfig& config) noexcept { m_config = config; }

    // Probe platform, create device context, compile kernels
    OpenClStatus Initialize() noexcept;
    void Shutdown() noexcept;

    // Resize pixels: src (BGRA32, srcW×srcH) → dst (BGRA32, dstW×dstH)
    OpenClResizeResult Resize(
        const uint8_t* src, uint32_t srcW, uint32_t srcH,
        uint8_t*       dst, uint32_t dstW, uint32_t dstH,
        uint32_t channels = 4u) noexcept;

    bool   IsReady()       const noexcept { return m_isReady; }
    OpenClDeviceInfo ActiveDevice() const noexcept { return m_device; }
    uint32_t TotalResamples() const noexcept { return m_totalResamples; }

    const OpenClResizeConfig& Config() const noexcept { return m_config; }

private:
    OpenClResizeConfig m_config{};
    OpenClDeviceInfo   m_device{};
    bool               m_isReady        = false;
    uint32_t           m_totalResamples = 0;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline OpenClStatus OpenClResizePath::Initialize() noexcept {
    // Stub: real impl links clGetPlatformIDs / clGetDeviceIDs / clBuildProgram
    m_isReady = false;
    return OpenClStatus::NO_PLATFORM;  // stub always returns no platform
}

inline void OpenClResizePath::Shutdown() noexcept {
    m_isReady = false;
}

inline OpenClResizeResult OpenClResizePath::Resize(
    const uint8_t* src, uint32_t srcW, uint32_t srcH,
    uint8_t*       dst, uint32_t dstW, uint32_t dstH,
    uint32_t /*channels*/) noexcept
{
    OpenClResizeResult r{};
    if (!m_isReady) { r.status = OpenClStatus::NOT_INITIALIZED; return r; }
    if (!src || !dst) { r.status = OpenClStatus::NULL_ARGUMENT; return r; }
    if (srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0) {
        r.status = OpenClStatus::ZERO_DIMENSION; return r;
    }
    // Stub: not reached until Initialize() succeeds
    r.status      = OpenClStatus::OK;
    r.srcWidthPx  = srcW;
    r.srcHeightPx = srcH;
    r.dstWidthPx  = dstW;
    r.dstHeightPx = dstH;
    r.filter      = m_config.filter;
    ++m_totalResamples;
    return r;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kOpenClMaxImageSidePx     = 65536u;
static constexpr uint32_t kOpenClMinWorkGroupSize   = 16u;
static constexpr const char* kOpenClBilinearKernel  = "lens_resize_bilinear";
static constexpr const char* kOpenClLanczosKernel   = "lens_resize_lanczos3";

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_OPENCLRESIZEPATH_H
