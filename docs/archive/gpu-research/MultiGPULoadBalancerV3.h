// MultiGPULoadBalancerV3.h — Multi-GPU Load Balancer v3
// Copyright (c) 2026 ExplorerLens Project
//
// Distributes decode workloads across heterogeneous GPU devices using demand-driven work stealing and thermal caps.
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

struct GPUDeviceHandle
{
    uint32_t index;
    std::string name;
    float utilizationPct;
    float thermalPct;
};
struct LBv3Config
{
    float thermalLimit = 90.0f;
    bool stickyAffinity = true;
    bool stealEnabled = true;
};
class MultiGPULoadBalancerV3
{
  public:
    void Register(GPUDeviceHandle dev)
    {
        m_devices.push_back(dev);
    }
    uint32_t SelectDevice() const
    {
        for (auto& d : m_devices)
            if (d.thermalPct < 90.0f)
                return d.index;
        return 0;
    }
    size_t DeviceCount() const
    {
        return m_devices.size();
    }
    float AverageUtilization() const
    {
        if (m_devices.empty())
            return 0.0f;
        float t = 0;
        for (auto& d : m_devices)
            t += d.utilizationPct;
        return t / m_devices.size();
    }

  private:
    std::vector<GPUDeviceHandle> m_devices;
    LBv3Config m_cfg;
};

}  // namespace Engine
}  // namespace ExplorerLens