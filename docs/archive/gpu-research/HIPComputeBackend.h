// HIPComputeBackend.h — AMD HIP Compute Back-End Adapter
// Copyright (c) 2026 ExplorerLens Project
//
// HIP runtime adapter for AMD GPUs — routes texture decode and upscale kernels via ROCm when NVIDIA is absent.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct HIPDeviceInfo
{
    std::string name;
    uint64_t totalMemBytes;
    uint32_t computeUnits;
    bool available;
};
struct HIPKernelResult
{
    bool success;
    double durationMs;
    std::vector<uint8_t> output;
};
class HIPComputeBackend
{
  public:
    HIPDeviceInfo QueryDevice(int index = 0) const
    {
        (void)index;
        return {"CPU Fallback", 0, 0, false};
    }
    HIPKernelResult Run(const std::string& kernelName, const std::vector<uint8_t>& input)
    {
        (void)kernelName;
        (void)input;
        return {false, 0.0, {}};
    }
    bool IsAvailable() const
    {
        return false;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens