// GPUVendorCapabilityMap.h — Vendor-Specific GPU Feature Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Maps GPU vendor capabilities for optimal decode path selection across
// NVIDIA, AMD, Intel, and Qualcomm hardware.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GPUVendorId : uint16_t {
    Unknown = 0,
    NVIDIA = 0x10DE,
    AMD = 0x1002,
    Intel = 0x8086,
    Qualcomm = 0x5143
};

enum class GPUCapability : uint32_t {
    None = 0,
    HardwareDecode = 1 << 0,
    ComputeShader = 1 << 1,
    SharedMemory = 1 << 2,
    AsyncCompute = 1 << 3,
    HalfPrecision = 1 << 4,
    RayTracing = 1 << 5,
    TensorOps = 1 << 6,
    DirectMLSupport = 1 << 7
};

inline GPUCapability operator|(GPUCapability a, GPUCapability b)
{
    return static_cast<GPUCapability>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline GPUCapability operator&(GPUCapability a, GPUCapability b)
{
    return static_cast<GPUCapability>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

struct GPUDeviceProfile
{
    GPUVendorId vendor = GPUVendorId::Unknown;
    std::string deviceName;
    uint32_t deviceId = 0;
    uint64_t vramBytes = 0;
    uint32_t computeUnits = 0;
    GPUCapability capabilities = GPUCapability::None;
};

class GPUVendorCapabilityMap
{
  public:
    GPUVendorCapabilityMap() = default;

    void RegisterDevice(const GPUDeviceProfile& profile)
    {
        m_devices.push_back(profile);
    }

    bool HasCapability(GPUCapability cap) const
    {
        for (const auto& dev : m_devices) {
            if ((dev.capabilities & cap) != GPUCapability::None)
                return true;
        }
        return false;
    }

    GPUDeviceProfile GetPrimaryDevice() const
    {
        return m_devices.empty() ? GPUDeviceProfile{} : m_devices[0];
    }

    std::vector<GPUDeviceProfile> GetDevicesByVendor(GPUVendorId vendor) const
    {
        std::vector<GPUDeviceProfile> result;
        for (const auto& dev : m_devices) {
            if (dev.vendor == vendor)
                result.push_back(dev);
        }
        return result;
    }

    size_t GetDeviceCount() const
    {
        return m_devices.size();
    }
    bool HasAnyGPU() const
    {
        return !m_devices.empty();
    }

  private:
    std::vector<GPUDeviceProfile> m_devices;
};

}  // namespace Engine
}  // namespace ExplorerLens
