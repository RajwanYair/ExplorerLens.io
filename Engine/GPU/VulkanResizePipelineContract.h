// VulkanResizePipelineContract.h -- S297 / ROADMAP v6.0 Phase 6 GPU
// Vulkan compute-based image resize pipeline stub for Linux GPU path.
//
// Phase 6 (Linux Nautilus/KIO) requires a GPU resize backend that works
// without DirectX. This contract defines the Vulkan compute pipeline
// interface using a Lanczos-3 kernel (matching the D3D11 pipeline from S249).
//
// Not compiled on Windows (guarded by #ifndef _WIN32 at build time in CMake).
// Type definitions are unconditionally available for cross-platform test stubs.
//
// Rule: contract header only — no implementation, no platform headers.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Vulkan Pixel Format ───────────────────────────────────────────────────────
// Canonical subset of VkFormat values used by the resize pipeline.

enum class VulkanResizeFormat : uint32_t {
    B8G8R8A8_UNORM   = 44,   // VK_FORMAT_B8G8R8A8_UNORM — matches GDI+ BGRA
    R8G8B8A8_UNORM   = 37,   // VK_FORMAT_R8G8B8A8_UNORM — standard RGBA
    R16G16B16A16_UNORM = 91, // VK_FORMAT_R16G16B16A16_UNORM — 16-bit pipeline
    R32G32B32A32_SFLOAT = 109, // VK_FORMAT_R32G32B32A32_SFLOAT — HDR
};

// ── Resize Filter ─────────────────────────────────────────────────────────────

enum class VulkanResizeFilter : uint8_t {
    LANCZOS3    = 0,  // Lanczos-3 (default — matches D3D11 pipeline)
    BILINEAR    = 1,  // Fast bilinear (low quality)
    BICUBIC     = 2,  // Bicubic Mitchell-Netravali
    NEAREST     = 3,  // Nearest neighbor (test only)
};

// ── Pipeline Config ───────────────────────────────────────────────────────────

struct VulkanResizePipelinePolicy {
    VulkanResizeFormat inputFormat   = VulkanResizeFormat::B8G8R8A8_UNORM;
    VulkanResizeFormat outputFormat  = VulkanResizeFormat::B8G8R8A8_UNORM;
    VulkanResizeFilter filter        = VulkanResizeFilter::LANCZOS3;
    uint32_t           maxBatchSize  = 64;    // Images per dispatch (matches D3D11 S249)
    uint32_t           workgroupSizeX = 16;
    uint32_t           workgroupSizeY = 16;
    bool               usePipelineCache = true;  // VkPipelineCache for reuse
};

// ── Probe ─────────────────────────────────────────────────────────────────────

struct VulkanResizePipelineProbe {
    bool     pipelineCompiled   = false;
    bool     deviceSupportsHdr  = false;
    uint32_t dispatchesCompleted = 0;
    uint32_t fallbackToSoftware  = 0;  // Fallback count (GPU unavailable)
    uint64_t totalPixelsResized  = 0;
};

// ── Constants ─────────────────────────────────────────────────────────────────

static constexpr uint32_t kVulkanResizeMinApiVersion        = 0x00401000u; // Vulkan 1.1.0
static constexpr uint32_t kVulkanResizeMaxBatchSizeHard     = 256u;
static constexpr uint32_t kVulkanResizeHardTimeoutMs        = 5000u;
static constexpr uint8_t  kVulkanResizeSchemaVersion        = 1;

} // namespace Engine
} // namespace ExplorerLens
