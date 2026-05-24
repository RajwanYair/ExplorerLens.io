// Engine/GPU/Dxva2SurfacePool.h
// ExplorerLens — DXVA2 decode surface pool (ROADMAP v8.0 Phase 4)
// Sprint S355.
//
// Purpose:
//   Manages a pool of IDirectXVideoDecoder decode surfaces to avoid the
//   overhead of allocating/releasing a DXVA2 surface per frame.
//   Phase 4 stub — surfaces are pre-allocated at initialisation time.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_DXVA2_SURFACE_POOL_H
#define EXPLORERLENS_ENGINE_DXVA2_SURFACE_POOL_H

#include <cstdint>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// Dxva2SurfacePoolStatus
// ---------------------------------------------------------------------------
enum class Dxva2SurfacePoolStatus : std::uint8_t {
    OK                  = 0,
    NOT_AVAILABLE       = 1,   ///< Phase 4 stub
    POOL_EXHAUSTED      = 2,
    INVALID_CONFIG      = 3,
};

// ---------------------------------------------------------------------------
// Dxva2SurfacePool — Phase 4 stub
// ---------------------------------------------------------------------------
class Dxva2SurfacePool final {
public:
    Dxva2SurfacePool() noexcept  = default;
    ~Dxva2SurfacePool() noexcept = default;

    Dxva2SurfacePool(const Dxva2SurfacePool&)            = delete;
    Dxva2SurfacePool& operator=(const Dxva2SurfacePool&) = delete;

    [[nodiscard]] bool IsAvailable() const noexcept { return false; }

    [[nodiscard]] Dxva2SurfacePoolStatus Initialize(
        std::uint32_t /*surfaceCount*/,
        std::uint32_t /*width*/,
        std::uint32_t /*height*/) noexcept
    {
        return Dxva2SurfacePoolStatus::NOT_AVAILABLE;
    }

    void Shutdown() noexcept {}

    // Constants
    static constexpr std::uint32_t kDefaultSurfaceCount = 8u;
    static constexpr std::uint32_t kMaxSurfaceCount     = 32u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DXVA2_SURFACE_POOL_H
