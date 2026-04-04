// VulkanComputeAccelerator.h — Vulkan Compute Decode Acceleration
// Copyright (c) 2026 ExplorerLens Project
//
// Provides Vulkan compute-shader based decode acceleration as an alternative
// to DirectX 11/12 on systems with Vulkan 1.3+ but no DX12 (e.g., WSL2,
// non-Windows future ports). Falls back gracefully to CPU.
//
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class VulkanBackendStatus {
    NotInitialized,
    Unavailable,    // Vulkan loader not present
    NoSuitableGPU,  // No compute-capable device found
    Ready,
    Error,
};

struct VulkanDeviceInfo
{
    std::string deviceName;
    uint32_t vendorId;
    uint32_t deviceId;
    uint32_t apiVersionMajor;
    uint32_t apiVersionMinor;
    uint64_t deviceLocalMemoryBytes;
    bool supportsTimestamps;
};

struct VulkanComputeConfig
{
    uint32_t maxWorkgroupsX{256};
    uint32_t maxWorkgroupsY{256};
    uint32_t stagingBufferMB{64};
    bool enableValidationLayers{false};  // always false in production
    bool preferDiscreteGPU{true};
    int32_t preferredDeviceIndex{-1};  // -1 = auto-select
};

struct VulkanDecodeJob
{
    const void* compressedData{nullptr};
    size_t dataSize{0};
    uint32_t reqWidth{0};
    uint32_t reqHeight{0};
    uint32_t format{0};  // LENS_PIXEL_FORMAT_* constant
};

struct VulkanDecodeResult
{
    std::vector<uint8_t> pixels;
    uint32_t width{0};
    uint32_t height{0};
    uint32_t stride{0};
    double gpuMs{0.0};
    double uploadMs{0.0};
    double downloadMs{0.0};
};

class VulkanComputeAccelerator
{
  public:
    explicit VulkanComputeAccelerator(VulkanComputeConfig cfg = {}) : m_cfg(cfg) {}
    ~VulkanComputeAccelerator() {}

    VulkanComputeAccelerator(const VulkanComputeAccelerator&) = delete;
    VulkanComputeAccelerator& operator=(const VulkanComputeAccelerator&) = delete;

    [[nodiscard]] VulkanBackendStatus Initialize();
    void Shutdown();

    [[nodiscard]] std::optional<VulkanDecodeResult> Decode(const VulkanDecodeJob& job);

    // Batch decode — submits N jobs to command buffer, executes, reads back.
    [[nodiscard]] std::vector<VulkanDecodeResult> DecodeBatch(const std::vector<VulkanDecodeJob>& jobs);

    [[nodiscard]] VulkanBackendStatus Status() const noexcept
    {
        return m_status;
    }
    [[nodiscard]] const VulkanDeviceInfo& DeviceInfo() const noexcept
    {
        return m_deviceInfo;
    }

    // Available without initialization — just enumerate adapters.
    static std::vector<VulkanDeviceInfo> EnumerateDevices();

  private:
    VulkanComputeConfig m_cfg;
    VulkanBackendStatus m_status{VulkanBackendStatus::NotInitialized};
    VulkanDeviceInfo m_deviceInfo{};

    struct VkImpl;
    VkImpl* m_impl{nullptr};
};

}  // namespace Engine
}  // namespace ExplorerLens
