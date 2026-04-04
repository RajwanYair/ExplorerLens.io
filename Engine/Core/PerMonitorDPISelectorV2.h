// PerMonitorDPISelectorV2.h — Per-Monitor DPI-Aware Thumbnail Resolution Selector
// Copyright (c) 2026 ExplorerLens Project
//
// Selects the optimal thumbnail pixel dimensions for a given monitor by
// applying the display's DPI scale factor to the logical thumbnail size.
// Handles mixed-DPI multi-monitor setups and falls back to LOGICAL_THUMB_SIZE
// when the target monitor is not registered.
//
#pragma once
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class PerMonitorDPIScale : uint32_t {
    Scale100 = 100,
    Scale125 = 125,
    Scale150 = 150,
    Scale175 = 175,
    Scale200 = 200,
    Scale225 = 225,
    Scale250 = 250,
};

struct MonitorDPIProfile
{
    uint64_t monitorHandle = 0;
    uint32_t dpiX = 96;
    uint32_t dpiY = 96;
    PerMonitorDPIScale scalePercent = PerMonitorDPIScale::Scale100;
    bool isPrimary = false;
};

struct ThumbResolution
{
    uint32_t widthPx = 0;
    uint32_t heightPx = 0;
};

class PerMonitorDPISelectorV2
{
  public:
    static constexpr uint32_t LOGICAL_THUMB_SIZE = 256;

    void RegisterMonitor(const MonitorDPIProfile& profile)
    {
        m_monitors[profile.monitorHandle] = profile;
    }

    ThumbResolution SelectResolution(uint64_t monitorHandle) const
    {
        auto it = m_monitors.find(monitorHandle);
        if (it == m_monitors.end()) {
            return {LOGICAL_THUMB_SIZE, LOGICAL_THUMB_SIZE};
        }
        const double factor = static_cast<double>(static_cast<uint32_t>(it->second.scalePercent)) / 100.0;
        const uint32_t px = static_cast<uint32_t>(LOGICAL_THUMB_SIZE * factor);
        return {px, px};
    }

    int MonitorCount() const noexcept
    {
        return static_cast<int>(m_monitors.size());
    }

    bool IsMixedDPI() const noexcept
    {
        if (m_monitors.size() < 2)
            return false;
        uint32_t firstDpi = 0;
        for (const auto& [_, p] : m_monitors) {
            if (firstDpi == 0) {
                firstDpi = p.dpiX;
                continue;
            }
            if (p.dpiX != firstDpi)
                return true;
        }
        return false;
    }

  private:
    std::unordered_map<uint64_t, MonitorDPIProfile> m_monitors;
};

}  // namespace Engine
}  // namespace ExplorerLens
