// PowerBudgetController.h — Power Budget Controller (DXGI / ACPI Integration)
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors and enforces per-process power budgets using DXGI power metrics and
// ACPI battery state, reducing thumbnail throughput when on battery to extend runtime.
//
#pragma once
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class PBCPowerSource {
    AC,
    Battery,
    UPS,
    Unknown
};
enum class PowerBudgetState {
    Unconstrained,
    Constrained,
    Strict,
    Emergency
};

struct PowerBudgetPolicy
{
    float maxACPowerW = 45.0f;
    float maxBatteryPowerW = 15.0f;
    float batteryWarningPct = 20.0f;  // %
    float batteryCriticalPct = 10.0f;
};

struct PowerSample
{
    PBCPowerSource source = PBCPowerSource::AC;
    float currentPowerW = 0.0f;
    float batteryLevelPct = 100.0f;
    float gpuPowerW = 0.0f;
    float cpuPowerW = 0.0f;
};

struct PowerBudgetDecision
{
    PowerBudgetState state = PowerBudgetState::Unconstrained;
    int maxDecodeSlots = 16;
    bool disableGPU = false;
    float targetWatts = 45.0f;
};

class PowerBudgetController
{
  public:
    explicit PowerBudgetController(PowerBudgetPolicy policy = {}) : m_policy(std::move(policy)) {}

    PowerBudgetDecision Evaluate(const PowerSample& sample) const noexcept
    {
        PowerBudgetDecision d;
        float limit = (sample.source == PBCPowerSource::Battery) ? m_policy.maxBatteryPowerW : m_policy.maxACPowerW;
        d.targetWatts = limit;

        if (sample.source == PBCPowerSource::Battery && sample.batteryLevelPct <= m_policy.batteryCriticalPct) {
            d.state = PowerBudgetState::Emergency;
            d.maxDecodeSlots = 1;
            d.disableGPU = true;
        } else if (sample.source == PBCPowerSource::Battery && sample.batteryLevelPct <= m_policy.batteryWarningPct) {
            d.state = PowerBudgetState::Strict;
            d.maxDecodeSlots = 2;
        } else if (sample.currentPowerW > limit * 0.9f) {
            d.state = PowerBudgetState::Constrained;
            d.maxDecodeSlots = 4;
        } else {
            d.state = PowerBudgetState::Unconstrained;
            d.maxDecodeSlots = 16;
        }
        return d;
    }

    const PowerBudgetPolicy& Policy() const noexcept
    {
        return m_policy;
    }

    static std::string StateName(PowerBudgetState s) noexcept
    {
        switch (s) {
            case PowerBudgetState::Unconstrained:
                return "Unconstrained";
            case PowerBudgetState::Constrained:
                return "Constrained";
            case PowerBudgetState::Strict:
                return "Strict";
            case PowerBudgetState::Emergency:
                return "Emergency";
        }
        return "Unknown";
    }

    static std::string SourceName(PBCPowerSource s) noexcept
    {
        switch (s) {
            case PBCPowerSource::AC:
                return "AC";
            case PBCPowerSource::Battery:
                return "Battery";
            case PBCPowerSource::UPS:
                return "UPS";
            case PBCPowerSource::Unknown:
                return "Unknown";
        }
        return "Unknown";
    }

  private:
    PowerBudgetPolicy m_policy;
};

}  // namespace Engine
}  // namespace ExplorerLens
