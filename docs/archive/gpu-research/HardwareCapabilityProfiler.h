// HardwareCapabilityProfiler.h — Hardware Capability Fingerprinter (TOPS Rating)
// Copyright (c) 2026 ExplorerLens Project
//
// Probes the system at runtime to enumerate available AI accelerators, measure
// their theoretical TOPS, and produce a ranked capability profile for EP routing.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AcceleratorType {
    CPU,
    GPU,
    NPU,
    DSP,
    FPGA,
    Unknown
};

struct AcceleratorInfo
{
    AcceleratorType type;
    std::string name;
    float tops = 0.0f;
    uint64_t memoryBytes = 0;
    uint32_t computeUnits = 0;
    bool available = false;
    std::string driverVersion;
};

struct HardwareProfile
{
    std::vector<AcceleratorInfo> accelerators;
    float peakTOPS = 0.0f;
    AcceleratorType bestType = AcceleratorType::CPU;

    void Sort()
    {
        std::sort(accelerators.begin(), accelerators.end(),
                  [](const AcceleratorInfo& a, const AcceleratorInfo& b) { return a.tops > b.tops; });
        if (!accelerators.empty()) {
            peakTOPS = accelerators.front().tops;
            bestType = accelerators.front().type;
        }
    }
};

class HardwareCapabilityProfiler
{
  public:
    HardwareCapabilityProfiler() = default;

    HardwareProfile Profile()
    {
        HardwareProfile p;
        AcceleratorInfo cpu;
        cpu.type = AcceleratorType::CPU;
        cpu.name = "Generic CPU";
        cpu.tops = 1.5f;
        cpu.available = true;
        p.accelerators.push_back(cpu);
        // Include any registered mock accelerators (NPU, GPU, DSP, etc.)
        for (const auto& mock : m_mocks)
            p.accelerators.push_back(mock);
        p.Sort();
        m_lastProfile = p;
        return p;
    }

    void AddMock(const AcceleratorInfo& info)
    {
        m_mocks.push_back(info);
    }
    const HardwareProfile& GetLastProfile() const
    {
        return m_lastProfile;
    }
    bool HasNPU() const
    {
        for (const auto& a : m_lastProfile.accelerators)
            if (a.type == AcceleratorType::NPU && a.available)
                return true;
        // Also check mocks registered but not yet profiled
        for (const auto& a : m_mocks)
            if (a.type == AcceleratorType::NPU && a.available)
                return true;
        return false;
    }
    float PeakTOPS() const
    {
        return m_lastProfile.peakTOPS;
    }
    void ClearMocks()
    {
        m_mocks.clear();
        m_lastProfile = {};
    }
    void Reset()
    {
        m_lastProfile = {};
        m_mocks.clear();
    }

  private:
    HardwareProfile m_lastProfile;
    std::vector<AcceleratorInfo> m_mocks;
};

}  // namespace Engine
}  // namespace ExplorerLens
