// Engine/GPU/Dxva2JpegSession.h
// ExplorerLens — DXVA2 JPEG decode session (ROADMAP v8.0 Phase 4)
// Sprint S355 (companion to Dxva2SurfacePool).
//
// Purpose:
//   Manages a single DXVA2 IDirectXVideoDecoder session for JPEG decode.
//   One session is created per D3D device; surfaces are obtained from
//   Dxva2SurfacePool.
//   Phase 4 stub — not wired until Sprint S355.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_DXVA2_JPEG_SESSION_H
#define EXPLORERLENS_ENGINE_DXVA2_JPEG_SESSION_H

#include <cstdint>
#include <span>
#include <vector>
#include <cstddef>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// Dxva2JpegSessionStatus
// ---------------------------------------------------------------------------
enum class Dxva2JpegSessionStatus : std::uint8_t {
    OK                  = 0,
    NOT_AVAILABLE       = 1,   ///< Phase 4 stub
    SESSION_NOT_OPEN    = 2,
    DECODE_FAILED       = 3,
    SURFACE_UNAVAILABLE = 4,
};

// ---------------------------------------------------------------------------
// Dxva2JpegSession — Phase 4 stub
// ---------------------------------------------------------------------------
class Dxva2JpegSession final {
public:
    Dxva2JpegSession() noexcept  = default;
    ~Dxva2JpegSession() noexcept = default;

    Dxva2JpegSession(const Dxva2JpegSession&)            = delete;
    Dxva2JpegSession& operator=(const Dxva2JpegSession&) = delete;

    [[nodiscard]] bool Open() noexcept { return false; }
    void Close() noexcept {}
    [[nodiscard]] bool IsOpen() const noexcept { return false; }

    [[nodiscard]] Dxva2JpegSessionStatus Decode(
        std::span<const std::byte> /*bitstream*/,
        std::vector<std::byte>&    /*outPixels*/,
        std::uint32_t&             /*outWidth*/,
        std::uint32_t&             /*outHeight*/) const noexcept
    {
        return Dxva2JpegSessionStatus::NOT_AVAILABLE;
    }

    [[nodiscard]] static bool IsHardwareAvailable() noexcept { return false; }

    static constexpr std::uint32_t kMaxJpegSidePixels = 8192u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DXVA2_JPEG_SESSION_H
