// Engine/GPU/VulkanResizePipelineContract.h
// ExplorerLens — Vulkan resize pipeline stub (ROADMAP v6.0 Phase 6)
// Sprint S297.
//
// Purpose:
//   Contract for a Vulkan-based thumbnail resize pipeline as an alternative
//   to D3D11 on non-Windows platforms and for cross-vendor support.
//   Phase 6 stub — not implemented until Vulkan SDK integration sprint.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_VULKAN_RESIZE_PIPELINE_CONTRACT_H
#define EXPLORERLENS_ENGINE_VULKAN_RESIZE_PIPELINE_CONTRACT_H

#include <cstdint>
#include <vector>
#include <cstddef>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// VulkanResizeStatus
// ---------------------------------------------------------------------------
enum class VulkanResizeStatus : std::uint8_t {
    OK                  = 0,
    NOT_AVAILABLE       = 1,   ///< Vulkan SDK not linked (Phase 6 stub)
    DEVICE_LOST         = 2,
    OUT_OF_MEMORY       = 3,
    INVALID_DESCRIPTOR  = 4,
};

// ---------------------------------------------------------------------------
// VulkanResizeResult
// ---------------------------------------------------------------------------
struct VulkanResizeResult final {
    VulkanResizeStatus     status{ VulkanResizeStatus::NOT_AVAILABLE };
    std::uint32_t          widthPixels{};
    std::uint32_t          heightPixels{};
    std::vector<std::byte> pixels;
};

// ---------------------------------------------------------------------------
// VulkanResizePipelineContract — Phase 6 stub
// ---------------------------------------------------------------------------
class VulkanResizePipelineContract final {
public:
    // Vulkan path is not available until Phase 6.
    [[nodiscard]] static constexpr bool IsAvailable() noexcept { return false; }

    [[nodiscard]] static constexpr const char* BackendName() noexcept
    {
        return "Vulkan";
    }

    // Resize stub — always returns NOT_AVAILABLE.
    [[nodiscard]] VulkanResizeResult Resize(
        const std::byte* /*srcPixels*/,
        std::uint32_t    /*srcWidth*/,
        std::uint32_t    /*srcHeight*/,
        std::uint32_t    /*dstWidth*/,
        std::uint32_t    /*dstHeight*/) const noexcept
    {
        return {};
    }

    // Constants
    static constexpr std::uint32_t kMaxTextureDimension = 16384u;

private:
    VulkanResizePipelineContract() = delete;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_VULKAN_RESIZE_PIPELINE_CONTRACT_H
