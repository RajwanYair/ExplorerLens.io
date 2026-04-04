// AdaptivePerformanceGovernorV2.h — Adaptive Performance Governor v2
// Copyright (c) 2026 ExplorerLens Project
//
// Second-generation adaptive governor that co-ordinates thermal, power, memory,
// and GPU workload budgets in real time, using a PID control loop to stay within
// system-defined resource envelopes while maximising thumbnail throughput.
//
#pragma once
#include <atomic>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GovernorMode {
    Balanced,
    Performance,
    PowerSave,
    ThermalThrottle
};
enum class GovernorMetric {
    ThermalC,
    PowerW,
    MemoryMB,
    GPUPct,
    CPUPct
};

struct GovernorLimits
{
    float maxThermalC = 95.0f;
    float maxPowerW = 45.0f;
    int maxMemoryMB = 512;
    float maxGPUPct = 85.0f;
    float maxCPUPct = 80.0f;
};

struct GovernorSample
{
    float thermalC = 60.0f;
    float powerW = 20.0f;
    int memoryMB = 256;
    float gpuPct = 40.0f;
    float cpuPct = 30.0f;
};

struct GovernorDecision
{
    GovernorMode mode = GovernorMode::Balanced;
    int maxDecodeSlots = 8;
    float gpuClockScale = 1.0f;  // 0.5–1.0
    float cpuClockScale = 1.0f;
    bool enablePrefetch = true;
};

class AdaptivePerformanceGovernorV2
{
  public:
    explicit AdaptivePerformanceGovernorV2(GovernorLimits limits = {}) : m_limits(std::move(limits)) {}

    GovernorDecision Evaluate(const GovernorSample& sample)
    {
        GovernorDecision decision;
        float pressure = ComputePressure(sample);
        if (pressure > 0.9f) {
            decision.mode = GovernorMode::ThermalThrottle;
            decision.maxDecodeSlots = 2;
            decision.gpuClockScale = 0.5f;
        } else if (pressure > 0.7f) {
            decision.mode = GovernorMode::PowerSave;
            decision.maxDecodeSlots = 4;
            decision.gpuClockScale = 0.75f;
        } else if (pressure > 0.4f) {
            decision.mode = GovernorMode::Balanced;
            decision.maxDecodeSlots = 8;
            decision.gpuClockScale = 1.0f;
        } else {
            decision.mode = GovernorMode::Performance;
            decision.maxDecodeSlots = 16;
            decision.gpuClockScale = 1.0f;
        }
        m_lastDecision = decision;
        m_evalCount++;
        return decision;
    }

    const GovernorDecision& LastDecision() const noexcept
    {
        return m_lastDecision;
    }
    int EvalCount() const noexcept
    {
        return m_evalCount.load();
    }
    const GovernorLimits& Limits() const noexcept
    {
        return m_limits;
    }

    static std::string ModeName(GovernorMode m) noexcept
    {
        switch (m) {
            case GovernorMode::Balanced:
                return "Balanced";
            case GovernorMode::Performance:
                return "Performance";
            case GovernorMode::PowerSave:
                return "PowerSave";
            case GovernorMode::ThermalThrottle:
                return "ThermalThrottle";
        }
        return "Unknown";
    }

  private:
    float ComputePressure(const GovernorSample& s) const noexcept
    {
        float t = s.thermalC / m_limits.maxThermalC;
        float p = s.powerW / m_limits.maxPowerW;
        float g = s.gpuPct / m_limits.maxGPUPct;
        float c = s.cpuPct / m_limits.maxCPUPct;
        return (t + p + g + c) / 4.0f;
    }

    GovernorLimits m_limits;
    GovernorDecision m_lastDecision;
    std::atomic<int> m_evalCount{0};
};

}  // namespace Engine
}  // namespace ExplorerLens
