#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cstdint>

namespace ExplorerLens::Engine::Gpu {

    enum class GpuVendor {
        Unknown,
        Nvidia,
        Amd,
        Intel,
        Microsoft // e.g. WARP
    };

    struct GpuDeviceInfo {
        uint32_t adapterIndex;
        uint64_t luid;
        std::wstring description;
        GpuVendor vendor;
        uint64_t dedicatedVideoMemory;
        bool isSoftwareAdapter;
        bool isBlocked; // via driver blocklist
    };

    enum class DeviceSelectionPolicy {
        Performance,    // Prefer discrete high-power GPU
        PowerSaving,    // Prefer integrated / low-power
        Compatibility,  // Prefer WARP if stability issues detected
        Manual          // Use specific adapter index
    };

    class IDeviceManager {
    public:
        virtual ~IDeviceManager() = default;

        // Enumerate all available adapters (DX12/DX11/Vulkan capable)
        virtual std::vector<GpuDeviceInfo> EnumerateDevices() = 0;

        // Select the active device based on current policy
        virtual const GpuDeviceInfo& SelectDevice(DeviceSelectionPolicy policy) = 0;

        // Update the blocklist of known bad drivers/devices
        virtual void UpdateBlocklist(const std::vector<std::wstring>& blockedDeviceIds) = 0;

        // Metrics
        virtual float GetUsage(uint32_t adapterIndex) = 0;
    };

    // Factory
    std::shared_ptr<IDeviceManager> CreateDeviceManager();

}

