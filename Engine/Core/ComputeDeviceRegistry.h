// ComputeDeviceRegistry.h — Compute Device Registry
// Copyright (c) 2026 ExplorerLens Project
//
// Enumerates and profiles all available AI accelerators at startup,
// providing a unified registry for NPU/GPU/CPU compute devices.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class ComputeDeviceClass : uint8_t {
    CPU = 0,
    GPU,
    NPU,
    DSP,
    FPGA,
    Unknown
};

struct ComputeDevice {
    std::string      name;
    ComputeDeviceClass deviceClass = ComputeDeviceClass::Unknown;
    float            tops         = 0.0f;
    uint32_t         memoryMB     = 0;
    bool             available    = false;
};

struct ComputeDeviceRegistryStats {
    uint32_t devicesEnumerated = 0;
    uint32_t npuCount          = 0;
    uint32_t gpuCount          = 0;
    uint32_t cpuCount          = 0;
};

class ComputeDeviceRegistry {
public:
    static ComputeDeviceRegistry& Instance() {
        static ComputeDeviceRegistry s;
        return s;
    }

    bool Initialize() {
        m_devices.clear();
        // Always add a CPU entry
        ComputeDevice cpu;
        cpu.name        = "Generic x64 CPU";
        cpu.deviceClass = ComputeDeviceClass::CPU;
        cpu.tops        = 4.0f;
        cpu.memoryMB    = 0; // system RAM
        cpu.available   = true;
        m_devices.push_back(cpu);
        ++m_stats.cpuCount;
#if defined(_WIN32)
        // Stub GPU and NPU entries for Windows 11 devices
        ComputeDevice gpu;
        gpu.name        = "DirectX 12 GPU";
        gpu.deviceClass = ComputeDeviceClass::GPU;
        gpu.tops        = 50.0f;
        gpu.memoryMB    = 8192;
        gpu.available   = true;
        m_devices.push_back(gpu);
        ++m_stats.gpuCount;

        ComputeDevice npu;
        npu.name        = "Windows NPU Device";
        npu.deviceClass = ComputeDeviceClass::NPU;
        npu.tops        = 40.0f;
        npu.memoryMB    = 2048;
        npu.available   = true;
        m_devices.push_back(npu);
        ++m_stats.npuCount;
#endif
        m_stats.devicesEnumerated = static_cast<uint32_t>(m_devices.size());
        m_ready = true;
        return true;
    }

    bool IsReady() const { return m_ready; }

    const std::vector<ComputeDevice>& GetDevices() const { return m_devices; }

    ComputeDevice* FindBestFor(ComputeDeviceClass preferredClass) {
        for (auto& d : m_devices) {
            if (d.deviceClass == preferredClass && d.available)
                return &d;
        }
        // Fallback to CPU
        for (auto& d : m_devices) {
            if (d.deviceClass == ComputeDeviceClass::CPU && d.available)
                return &d;
        }
        return nullptr;
    }

    const ComputeDeviceRegistryStats& GetStats() const { return m_stats; }

private:
    ComputeDeviceRegistry() = default;
    bool                         m_ready = false;
    std::vector<ComputeDevice>   m_devices;
    ComputeDeviceRegistryStats   m_stats;
};

}} // namespace ExplorerLens::Engine
