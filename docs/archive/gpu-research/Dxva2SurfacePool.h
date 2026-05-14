// =============================================================================
// ExplorerLens Engine — Dxva2SurfacePool.h
// Sprint S355 | ROADMAP v8.0 Phase 4 (DXVA2 hardware JPEG decode)
// DXVA2 decode surface pool for IDXVAHD_Device / IDirectXVideoDecoder usage.
//
// Phase 4 exit criterion: "DXVA2 hardware JPEG decode"
// This header provides the surface allocation and lifetime management layer
// that backs the existing DXVA2JpegDecodeContract.h (S-earlier sprint).
//
// DXVA2 hardware JPEG decode pipeline:
//   Dxva2SurfacePool (allocate) → DXVA2JpegDecoder (execute) → GDI+ scale
//
// Windows-only. Non-Windows builds compile to stubs.
// =============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#ifndef EXPLORERLENS_ENGINE_DXVA2SURFACEPOOL_H
#define EXPLORERLENS_ENGINE_DXVA2SURFACEPOOL_H

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// Dxva2SurfaceFormat — pixel format for decode surfaces
// ---------------------------------------------------------------------------
enum class Dxva2SurfaceFormat : uint32_t {
    NV12  = 0x3231564Eu, ///< 'NV12' — most common DXVA2 output
    YUY2  = 0x32595559u, ///< 'YUY2' — packed YUV
    P010  = 0x30313050u, ///< 'P010' — 10-bit HDR
    ARGB  = 0x42475241u, ///< 'ARGB' — post-conversion RGB output
};

// ---------------------------------------------------------------------------
// Dxva2SurfacePoolStatus — operation result codes
// ---------------------------------------------------------------------------
enum class Dxva2SurfacePoolStatus : uint8_t {
    OK                 = 0,
    NOT_WIN32          = 1,
    DEVICE_NOT_INIT    = 2,  ///< IDirectXVideoDecoderService not created
    ALLOC_FAIL         = 3,  ///< CreateSurface returned E_OUTOFMEMORY
    INVALID_DIMENSION  = 4,  ///< Width or height is 0 or exceeds kMaxSurface
    POOL_EXHAUSTED     = 5,  ///< All surfaces are in use, none available
    SURFACE_IN_USE     = 6,  ///< Attempt to release an already-free surface
    NULL_DEVICE        = 7,  ///< IDirect3DDevice9 pointer is null
    FORMAT_UNSUPPORTED = 8,  ///< GPU does not support requested surface format
};

// ---------------------------------------------------------------------------
// Dxva2SurfaceDesc — describes a single allocated decode surface
// ---------------------------------------------------------------------------
struct Dxva2SurfaceDesc final {
    uint32_t         width{0};
    uint32_t         height{0};
    Dxva2SurfaceFormat format{Dxva2SurfaceFormat::NV12};
    void*            pSurface{nullptr};  ///< IDirect3DSurface9* (opaque pointer)
    uint32_t         index{0};           ///< Pool slot index
    bool             inUse{false};

    [[nodiscard]] bool IsValid()   const noexcept { return pSurface != nullptr && width > 0 && height > 0; }
    [[nodiscard]] uint64_t Bytes() const noexcept {
        // NV12: width * height * 1.5 bytes
        if (format == Dxva2SurfaceFormat::NV12)
            return static_cast<uint64_t>(width) * height * 3u / 2u;
        // YUY2: width * height * 2 bytes
        if (format == Dxva2SurfaceFormat::YUY2)
            return static_cast<uint64_t>(width) * height * 2u;
        // ARGB: width * height * 4 bytes
        return static_cast<uint64_t>(width) * height * 4u;
    }
};

// ---------------------------------------------------------------------------
// Dxva2PoolConfig — surface pool allocation parameters
// ---------------------------------------------------------------------------
struct Dxva2PoolConfig final {
    uint32_t           surfaceCount{4u};       ///< Number of surfaces to pre-allocate
    uint32_t           width{0};               ///< Surface width in pixels
    uint32_t           height{0};              ///< Surface height in pixels
    Dxva2SurfaceFormat format{Dxva2SurfaceFormat::NV12};
    bool               allowResize{true};      ///< Re-allocate pool if size changes

    static constexpr uint32_t kMinSurfaces = 1u;
    static constexpr uint32_t kMaxSurfaces = 32u;

    [[nodiscard]] static Dxva2PoolConfig ForThumbnail(
        uint32_t w, uint32_t h) noexcept
    {
        Dxva2PoolConfig c;
        c.surfaceCount = 4u;
        c.width        = w;
        c.height       = h;
        c.format       = Dxva2SurfaceFormat::NV12;
        c.allowResize  = true;
        return c;
    }
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kDxva2MaxSurfaceWidth   = 16384u;
static constexpr uint32_t kDxva2MaxSurfaceHeight  = 16384u;
static constexpr uint32_t kDxva2DefaultPoolCount  = 4u;
static constexpr uint32_t kDxva2MaxPoolCount      = 32u;

// ---------------------------------------------------------------------------
// Dxva2SurfacePool — manages a fixed pool of DXVA2 decode surfaces
// ---------------------------------------------------------------------------
class Dxva2SurfacePool final {
public:
    Dxva2SurfacePool() noexcept;
    ~Dxva2SurfacePool() noexcept;

    // Non-copyable; move is allowed
    Dxva2SurfacePool(const Dxva2SurfacePool&)            = delete;
    Dxva2SurfacePool& operator=(const Dxva2SurfacePool&) = delete;
    Dxva2SurfacePool(Dxva2SurfacePool&&)                 noexcept;
    Dxva2SurfacePool& operator=(Dxva2SurfacePool&&)      noexcept;

    /// Allocate surfaces using a DXVA2 decoder service pointer (IUnknown).
    /// @param pDecoderService  IDirectXVideoDecoderService* (cast to void*)
    /// @param cfg              Pool parameters
    [[nodiscard]] Dxva2SurfacePoolStatus Allocate(
        void* pDecoderService,
        const Dxva2PoolConfig& cfg) noexcept;

    /// Acquire a free surface for decode. Returns null desc on POOL_EXHAUSTED.
    [[nodiscard]] Dxva2SurfacePoolStatus AcquireSurface(
        Dxva2SurfaceDesc& outDesc) noexcept;

    /// Return a surface to the pool after decode completes.
    Dxva2SurfacePoolStatus ReleaseSurface(uint32_t index) noexcept;

    /// Release all surfaces and reset the pool.
    void Reset() noexcept;

    /// Returns true if pool has been allocated successfully.
    [[nodiscard]] bool IsAllocated()  const noexcept { return m_allocated; }
    [[nodiscard]] uint32_t PoolCount() const noexcept { return m_poolCount; }
    [[nodiscard]] uint32_t FreeCount() const noexcept;
    [[nodiscard]] uint32_t Width()    const noexcept { return m_width; }
    [[nodiscard]] uint32_t Height()   const noexcept { return m_height; }
    [[nodiscard]] Dxva2SurfaceFormat Format() const noexcept { return m_format; }

private:
    std::vector<Dxva2SurfaceDesc> m_surfaces;
    uint32_t                      m_poolCount{0u};
    uint32_t                      m_width{0u};
    uint32_t                      m_height{0u};
    Dxva2SurfaceFormat            m_format{Dxva2SurfaceFormat::NV12};
    bool                          m_allocated{false};
};

// ---------------------------------------------------------------------------
// Inline stub implementations for non-Windows
// ---------------------------------------------------------------------------
#ifndef _WIN32

inline Dxva2SurfacePool::Dxva2SurfacePool() noexcept {}
inline Dxva2SurfacePool::~Dxva2SurfacePool() noexcept {}
inline Dxva2SurfacePool::Dxva2SurfacePool(Dxva2SurfacePool&&) noexcept {}
inline Dxva2SurfacePool& Dxva2SurfacePool::operator=(Dxva2SurfacePool&&) noexcept { return *this; }

inline Dxva2SurfacePoolStatus Dxva2SurfacePool::Allocate(
    void* /*pDecoderService*/,
    const Dxva2PoolConfig& /*cfg*/) noexcept
{
    return Dxva2SurfacePoolStatus::NOT_WIN32;
}

inline Dxva2SurfacePoolStatus Dxva2SurfacePool::AcquireSurface(
    Dxva2SurfaceDesc& outDesc) noexcept
{
    outDesc = {};
    return Dxva2SurfacePoolStatus::NOT_WIN32;
}

inline Dxva2SurfacePoolStatus Dxva2SurfacePool::ReleaseSurface(
    uint32_t /*index*/) noexcept
{
    return Dxva2SurfacePoolStatus::NOT_WIN32;
}

inline void Dxva2SurfacePool::Reset() noexcept {}

inline uint32_t Dxva2SurfacePool::FreeCount() const noexcept { return 0u; }

#endif // !_WIN32

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DXVA2SURFACEPOOL_H
