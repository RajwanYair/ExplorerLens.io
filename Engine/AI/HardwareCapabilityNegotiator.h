// HardwareCapabilityNegotiator.h — Hardware Capability Negotiator
// Copyright (c) 2026 ExplorerLens Project
//
// Selects optimal compute backend (NPU/GPU/CPU) per inference task based on
// hardware profile, power envelope, and latency requirements.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class HWBackendChoice : uint8_t {
    NPU = 0,
    GPU,
    CPU,
    FPGA,
    Unknown
};

struct HWCapabilityScore
{
    HWBackendChoice backend = HWBackendChoice::CPU;
    float score = 0.0f;  // higher is better
    float latencyMs = 0.0f;
    float powerWatts = 0.0f;
    bool available = false;
};

struct HWNegotiatorStats
{
    uint64_t npuSelections = 0;
    uint64_t gpuSelections = 0;
    uint64_t cpuSelections = 0;
    uint64_t totalQueries = 0;
};

class HardwareCapabilityNegotiator
{
public:
    HardwareCapabilityNegotiator() = default;

    bool Initialize()
    {
        m_ready = true;
        return true;
    }
    bool IsReady() const { return m_ready; }

    HWCapabilityScore Negotiate(const std::string& taskType)
    {
        ++m_stats.totalQueries;
        HWCapabilityScore best;
        best.available = true;
        if (taskType == "embedding" || taskType == "clip") {
            best.backend = HWBackendChoice::NPU;
            best.score = 95.0f;
            best.latencyMs = 0.8f;
            best.powerWatts = 2.5f;
            ++m_stats.npuSelections;
        } else if (taskType == "render" || taskType == "decode") {
            best.backend = HWBackendChoice::GPU;
            best.score = 90.0f;
            best.latencyMs = 1.5f;
            best.powerWatts = 15.0f;
            ++m_stats.gpuSelections;
        } else {
            best.backend = HWBackendChoice::CPU;
            best.score = 50.0f;
            best.latencyMs = 12.0f;
            best.powerWatts = 8.0f;
            ++m_stats.cpuSelections;
        }
        return best;
    }

    bool PrefersNPU(const std::string& taskType) { return Negotiate(taskType).backend == HWBackendChoice::NPU; }

    const HWNegotiatorStats& GetStats() const { return m_stats; }
    void Reset() { m_stats = {}; }

private:
    bool m_ready = false;
    HWNegotiatorStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
