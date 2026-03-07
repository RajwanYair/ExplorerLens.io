// GPUDeviceSelector.h — Multi-GPU Device Selection Strategy
// Copyright (c) 2026 ExplorerLens Project
//
// Enumerates available GPU devices and selects the optimal one based on
// capabilities, power profile, and workload characteristics.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GPUVendorType : uint8_t {
    Unknown = 0,
    NVIDIA = 1,
    AMD = 2,
    Intel = 3,
    Microsoft = 4  // WARP
};

enum class GPUSelectionStrategy : uint8_t {
    HighestPerformance = 0,
    LowestPower = 1,
    PreferDiscrete = 2,
    PreferIntegrated = 3,
    SpecificVendor = 4
};

struct GPUDeviceDetail {
    uint32_t deviceIndex = 0;
    std::wstring name;
    GPUVendorType vendor = GPUVendorType::Unknown;
    uint64_t dedicatedVideoMemory = 0;
    uint64_t sharedSystemMemory = 0;
    bool isDiscrete = false;
    bool supportsCompute = false;
    bool supportsDecode = false;
    uint32_t computeUnits = 0;
    uint32_t driverVersion = 0;
};

struct DeviceSelection {
    bool found = false;
    uint32_t deviceIndex = UINT32_MAX;
    std::wstring deviceName;
    GPUVendorType vendor = GPUVendorType::Unknown;
    std::string reason;
};

class GPUDeviceSelector {
public:
    void SetStrategy(GPUSelectionStrategy strategy) { m_strategy = strategy; }

    DeviceSelection Select(const std::vector<GPUDeviceDetail>& devices) const {
        DeviceSelection result;
        if (devices.empty()) return result;

        const GPUDeviceDetail* best = nullptr;
        for (const auto& d : devices) {
            if (!best) { best = &d; continue; }
            switch (m_strategy) {
            case GPUSelectionStrategy::HighestPerformance:
            case GPUSelectionStrategy::PreferDiscrete:
                if (d.isDiscrete && !best->isDiscrete) best = &d;
                else if (d.isDiscrete == best->isDiscrete &&
                    d.dedicatedVideoMemory > best->dedicatedVideoMemory)
                    best = &d;
                break;
            case GPUSelectionStrategy::LowestPower:
            case GPUSelectionStrategy::PreferIntegrated:
                if (!d.isDiscrete && best->isDiscrete) best = &d;
                break;
            default:
                break;
            }
        }
        if (best) {
            result.found = true;
            result.deviceIndex = best->deviceIndex;
            result.deviceName = best->name;
            result.vendor = best->vendor;
            result.reason = "Strategy match";
        }
        return result;
    }

    static bool HasComputeSupport(const GPUDeviceDetail& info) {
        return info.supportsCompute && info.computeUnits > 0;
    }

private:
    GPUSelectionStrategy m_strategy = GPUSelectionStrategy::PreferDiscrete;
};

} // namespace Engine
} // namespace ExplorerLens
