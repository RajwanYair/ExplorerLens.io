// Engine/GPU/GpuVendorRoutingContract.h
// ExplorerLens — GPU vendor routing contract (ROADMAP v6.0 Phase 1)
// Sprint S271.
//
// Purpose:
//   Routes GPU decode operations to the appropriate vendor acceleration path:
//   NVDEC (NVIDIA), QuickSync (Intel), AMF (AMD).
//   Phase 2 implementation wires actual DXGI adapter enumeration.
//   Until Sprint S330 the stub returns SOFTWARE_PATH for all formats.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_GPU_VENDOR_ROUTING_CONTRACT_H
#define EXPLORERLENS_ENGINE_GPU_VENDOR_ROUTING_CONTRACT_H

#include <cstdint>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// GpuVendor — detected GPU vendor
// ---------------------------------------------------------------------------
enum class GpuVendor : std::uint8_t {
    UNKNOWN  = 0,
    NVIDIA   = 1,   ///< NVDEC hardware decode
    INTEL    = 2,   ///< QuickSync hardware decode
    AMD      = 3,   ///< AMF hardware decode
    WARP     = 4,   ///< Microsoft WARP software rasteriser
};

// ---------------------------------------------------------------------------
// GpuDecodeRoute — which acceleration path to use
// ---------------------------------------------------------------------------
enum class GpuDecodeRoute : std::uint8_t {
    SOFTWARE_PATH   = 0,   ///< CPU decode (always available)
    NVDEC           = 1,   ///< NVIDIA Video Decode Engine
    QUICK_SYNC      = 2,   ///< Intel Quick Sync Video
    AMF             = 3,   ///< AMD Advanced Media Framework
    DXVA2           = 4,   ///< DXVA2 (vendor-agnostic)
};

// ---------------------------------------------------------------------------
// GpuVendorRoutingContract — stateless vendor routing query
// ---------------------------------------------------------------------------
class GpuVendorRoutingContract final {
public:
    // Detect the primary GPU vendor from DXGI adapter description.
    // Returns UNKNOWN until D3D11DeviceManager is initialised.
    [[nodiscard]] static GpuVendor DetectVendor() noexcept
    {
        return GpuVendor::UNKNOWN;  // Phase 2 stub
    }

    // Route a format to the best available decode path.
    [[nodiscard]] static GpuDecodeRoute RouteFormat(
        std::uint32_t /*formatFourCC*/) noexcept
    {
        return GpuDecodeRoute::SOFTWARE_PATH;  // Phase 2 stub
    }

    [[nodiscard]] static bool IsHardwareRoutingAvailable() noexcept
    {
        return false;  // Phase 2 stub
    }

    // Constants
    static constexpr std::uint32_t kNvidiaVendorId  = 0x10DEu;
    static constexpr std::uint32_t kIntelVendorId   = 0x8086u;
    static constexpr std::uint32_t kAmdVendorId     = 0x1002u;

private:
    GpuVendorRoutingContract() = delete;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_GPU_VENDOR_ROUTING_CONTRACT_H
