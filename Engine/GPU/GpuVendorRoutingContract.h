// ============================================================================
// GpuVendorRoutingContract.h -- S271 / ROADMAP v6.0 H8 (GPU vendor path)
//
// Phase 3 GPU router contract.  Header-only.  Declares how a decode/resize
// job is dispatched between NVIDIA NVDEC, Intel QuickSync, AMD AMF, and the
// generic D3D11 path based on adapter query + format capability.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class GpuVendorId : uint16_t
{
    UNKNOWN      = 0x0000,
    NVIDIA       = 0x10DE,
    INTEL        = 0x8086,
    AMD          = 0x1002,
    MICROSOFT    = 0x1414,   // WARP
    QUALCOMM     = 0x5143,
};

enum class GpuVendorPath : uint8_t
{
    CPU_ONLY           = 0,
    D3D11_GENERIC      = 1,
    NVIDIA_NVDEC       = 2,
    INTEL_QUICKSYNC    = 3,
    AMD_AMF            = 4,
    WARP_FALLBACK      = 5,
};

enum class GpuVendorRoutingStatus : uint8_t
{
    OK                    = 0,
    ADAPTER_UNAVAILABLE   = 1,
    DRIVER_TOO_OLD        = 2,
    FORMAT_UNSUPPORTED    = 3,
    OUT_OF_VRAM           = 4,
    FALLBACK_TO_CPU       = 5,
};

struct GpuVendorRoutingPolicy
{
    GpuVendorId   preferredVendor      = GpuVendorId::UNKNOWN; // UNKNOWN = auto
    bool          allowHwVideoDecode   = true;
    bool          allowHwImageDecode   = true;
    bool          allowD3D11Resize     = true;
    bool          allowWarpFallback    = false;  // too slow, only for telemetry
    uint32_t      minDriverMajor       = 25;     // driver year 2025+
    uint32_t      vramBudgetMb         = 512;
};

struct GpuVendorRoutingDecision
{
    GpuVendorRoutingStatus status       = GpuVendorRoutingStatus::OK;
    GpuVendorId            chosenVendor = GpuVendorId::UNKNOWN;
    GpuVendorPath          chosenPath   = GpuVendorPath::CPU_ONLY;
    uint32_t               queryLatencyUs = 0;
    bool                   usedCache    = false;  // adapter caps cached
};

inline constexpr uint32_t kGpuVendorRoutingMaxAdapters = 8;
inline constexpr uint32_t kGpuVendorRoutingCacheTtlMs  = 30000;

static_assert(std::is_trivially_copyable_v<GpuVendorRoutingPolicy>,
              "GpuVendorRoutingPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<GpuVendorRoutingDecision>,
              "GpuVendorRoutingDecision must be trivially copyable");
static_assert(static_cast<uint16_t>(GpuVendorId::NVIDIA) == 0x10DE,
              "NVIDIA PCI vendor id must be 0x10DE");
static_assert(static_cast<uint16_t>(GpuVendorId::INTEL) == 0x8086,
              "Intel PCI vendor id must be 0x8086");
static_assert(static_cast<uint16_t>(GpuVendorId::AMD) == 0x1002,
              "AMD PCI vendor id must be 0x1002");

} // namespace ExplorerLens::Engine
