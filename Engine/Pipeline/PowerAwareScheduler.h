// PowerAwareScheduler.h — Power-Aware Work-Item Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Routes AI inference work-items to the best available accelerator based on
// current power state — aggressively offloads to NPU when plugged in, uses
// CPU-only paths on battery-saver to extend runtime.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class NPUPowerMode {
    AC,
    Battery,
    BatterySaver,
    CriticalBattery
};
enum class SchedulerMode {
    Aggressive,
    Balanced,
    Conservative,
    BatterySaver
};

struct WorkItem
{
    uint32_t id = 0;
    float estimatedMs = 0.0f;
    bool canDeferToNPU = true;
    std::string formatHint;
};

struct NPUScheduleDecision
{
    uint32_t workItemId = 0;
    AcceleratorType assignedTo = AcceleratorType::CPU;
    bool deferred = false;
    std::string reason;
};

struct PowerSchedulerConfig
{
    SchedulerMode defaultMode = SchedulerMode::Balanced;
    float batteryPctForConservative = 20.0f;
    bool enableNPUOnBattery = true;
};

class PowerAwareScheduler
{
  public:
    explicit PowerAwareScheduler(const PowerSchedulerConfig& cfg = {}) : m_cfg(cfg) {}

    void SetPowerState(NPUPowerMode ps)
    {
        m_powerState = ps;
        switch (ps) {
            case NPUPowerMode::BatterySaver:
            case NPUPowerMode::CriticalBattery:
                m_mode = SchedulerMode::BatterySaver;
                break;
            case NPUPowerMode::Battery:
                m_mode = SchedulerMode::Conservative;
                break;
            default:
                m_mode = m_cfg.defaultMode;
        }
    }

    NPUScheduleDecision Schedule(const WorkItem& item)
    {
        NPUScheduleDecision d;
        d.workItemId = item.id;
        if (m_mode == SchedulerMode::BatterySaver) {
            d.assignedTo = AcceleratorType::CPU;
            d.reason = "Battery saver mode";
        } else if (item.canDeferToNPU && m_npuAvailable) {
            d.assignedTo = AcceleratorType::NPU;
            d.reason = "NPU available";
        } else {
            d.assignedTo = AcceleratorType::CPU;
            d.reason = "CPU fallback";
        }
        ++m_scheduled;
        return d;
    }

    NPUPowerMode GetPowerState() const
    {
        return m_powerState;
    }
    SchedulerMode GetMode() const
    {
        return m_mode;
    }
    void SetNPUAvailable(bool v)
    {
        m_npuAvailable = v;
    }
    uint32_t Scheduled() const
    {
        return m_scheduled;
    }
    void Reset()
    {
        m_scheduled = 0;
    }
    const PowerSchedulerConfig& GetConfig() const
    {
        return m_cfg;
    }

  private:
    PowerSchedulerConfig m_cfg;
    NPUPowerMode m_powerState = NPUPowerMode::AC;
    SchedulerMode m_mode = SchedulerMode::Balanced;
    bool m_npuAvailable = false;
    uint32_t m_scheduled = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
