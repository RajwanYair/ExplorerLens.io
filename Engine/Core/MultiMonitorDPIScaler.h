// MultiMonitorDPIScaler.h — Multi-monitor DPI scaling support
// Copyright (c) 2026 ExplorerLens Project
//
// Per-monitor DPI awareness v2 scaling utilities.
//
#pragma once
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class DPIScaleMode : uint8_t {
    Unaware = 0,
    SystemAware = 1,
    PerMonitor = 2,
    PerMonitorV2 = 3
};

inline const char* DPIScaleModeName(DPIScaleMode m) noexcept
{
    switch (m) {
        case DPIScaleMode::Unaware:
            return "Unaware";
        case DPIScaleMode::SystemAware:
            return "SystemAware";
        case DPIScaleMode::PerMonitor:
            return "PerMonitor";
        case DPIScaleMode::PerMonitorV2:
            return "PerMonitorV2";
        default:
            return "Unknown";
    }
}

enum class MonitorProfile : uint8_t {
    Standard = 0,
    HiDPI = 1,
    UltraHiDPI = 2
};

inline const char* MonitorProfileName(MonitorProfile p) noexcept
{
    switch (p) {
        case MonitorProfile::Standard:
            return "Standard";
        case MonitorProfile::HiDPI:
            return "HiDPI";
        case MonitorProfile::UltraHiDPI:
            return "UltraHiDPI";
        default:
            return "Unknown";
    }
}

class MultiMonitorDPIScaler
{
  public:
    float GetScaleFactor(uint32_t dpi) const noexcept
    {
        return static_cast<float>(dpi) / 96.0f;
    }

    uint32_t ScaleForMonitor(uint32_t baseSize, uint32_t dpi) const noexcept
    {
        return static_cast<uint32_t>(baseSize * GetScaleFactor(dpi));
    }

    MonitorProfile GetMonitorProfile(uint32_t dpi) const noexcept
    {
        if (dpi >= 192)
            return MonitorProfile::UltraHiDPI;
        if (dpi >= 144)
            return MonitorProfile::HiDPI;
        return MonitorProfile::Standard;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
