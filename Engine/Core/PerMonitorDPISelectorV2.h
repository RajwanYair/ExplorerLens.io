// PerMonitorDPISelectorV2.h — Per-Monitor DPI Thumbnail Resolution Selector v2
// Copyright (c) 2026 ExplorerLens Project
//
// Selects the optimal thumbnail pixel resolution for each attached monitor based on
// physical DPI, scale factor, and Explorer window placement. v2 adds mixed-DPI
// multi-monitor support and per-monitor compositor hint injection.
//
#pragma once
#include <vector>
#include <cmath>
#include <stdint.h>

namespace ExplorerLens {
namespace Engine {

enum class PerMonitorDPIScale : int { Scale100 = 100, Scale125 = 125, Scale150 = 150,
                            Scale175 = 175, Scale200 = 200, Scale250 = 250,
                            Scale300 = 300, Custom = -1 };

struct MonitorDPIProfile {
    uint32_t  monitorHandle = 0;
    uint32_t  dpiX          = 96;
    uint32_t  dpiY          = 96;
    PerMonitorDPIScale  scalePercent  = PerMonitorDPIScale::Scale100;
    bool      isPrimary     = false;
    int       width         = 1920;
    int       height        = 1080;

    double ScaleFactor()    const noexcept { return dpiX / 96.0; }
    int    ScaledSize(int logical) const noexcept {
        return static_cast<int>(std::ceil(logical * ScaleFactor()));
    }
};

struct ThumbnailResolution {
    int        widthPx      = 256;
    int        heightPx     = 256;
    uint32_t   monitorHandle = 0;
    double     scaleFactor  = 1.0;
};

class PerMonitorDPISelectorV2 {
public:
    static constexpr int LOGICAL_THUMB_SIZE = 256;

    void RegisterMonitor(const MonitorDPIProfile& profile) {
        for (auto& m : m_monitors)
            if (m.monitorHandle == profile.monitorHandle) { m = profile; return; }
        m_monitors.push_back(profile);
    }

    ThumbnailResolution SelectResolution(uint32_t monitorHandle) const {
        for (const auto& m : m_monitors) {
            if (m.monitorHandle == monitorHandle)
                return { m.ScaledSize(LOGICAL_THUMB_SIZE), m.ScaledSize(LOGICAL_THUMB_SIZE),
                         monitorHandle, m.ScaleFactor() };
        }
        const MonitorDPIProfile* primary = GetPrimary();
        if (primary) return { primary->ScaledSize(LOGICAL_THUMB_SIZE),
                              primary->ScaledSize(LOGICAL_THUMB_SIZE),
                              primary->monitorHandle, primary->ScaleFactor() };
        return { LOGICAL_THUMB_SIZE, LOGICAL_THUMB_SIZE, 0, 1.0 };
    }

    bool IsMixedDPI() const noexcept {
        if (m_monitors.size() <= 1) return false;
        return m_monitors[0].dpiX != m_monitors.back().dpiX;
    }

    const MonitorDPIProfile* GetPrimary() const noexcept {
        for (const auto& m : m_monitors) if (m.isPrimary) return &m;
        return m_monitors.empty() ? nullptr : &m_monitors[0];
    }

    int MonitorCount() const noexcept { return (int)m_monitors.size(); }

private:
    std::vector<MonitorDPIProfile> m_monitors;
};

} // namespace Engine
} // namespace ExplorerLens
