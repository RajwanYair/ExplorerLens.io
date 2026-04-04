// DPIScalingManager.h — Per-Monitor DPI Awareness
// Copyright (c) 2026 ExplorerLens Project
//
// Manages per-monitor DPI awareness, providing scale factors and
// scaled dimension calculations for multi-monitor setups.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DPIAwarenessLevel : uint32_t {
    Unaware = 0,
    System = 1,
    PerMonitor = 2,
    PerMonitorV2 = 3
};

struct DPIMonitorEntry
{
    HMONITOR handle = nullptr;
    uint32_t dpiX = 96;
    uint32_t dpiY = 96;
    double scaleFactor = 1.0;
    RECT workArea = {};
    RECT monitorRect = {};
    bool isPrimary = false;
    std::wstring deviceName;

    uint32_t ScaledWidth() const
    {
        return static_cast<uint32_t>((monitorRect.right - monitorRect.left) / scaleFactor);
    }

    uint32_t ScaledHeight() const
    {
        return static_cast<uint32_t>((monitorRect.bottom - monitorRect.top) / scaleFactor);
    }
};

class DPIScalingManager
{
  public:
    static DPIScalingManager& Instance()
    {
        static DPIScalingManager s;
        return s;
    }

    DPIMonitorEntry GetDPIForMonitor(HMONITOR hMonitor) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& info : m_monitors) {
            if (info.handle == hMonitor)
                return info;
        }
        // Return default 96 DPI info
        DPIMonitorEntry def;
        def.handle = hMonitor;
        return def;
    }

    SIZE ScaleSize(uint32_t baseWidth, uint32_t baseHeight, HMONITOR hMonitor) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        double scale = 1.0;
        for (const auto& info : m_monitors) {
            if (info.handle == hMonitor) {
                scale = info.scaleFactor;
                break;
            }
        }
        SIZE result;
        result.cx = static_cast<LONG>(baseWidth * scale);
        result.cy = static_cast<LONG>(baseHeight * scale);
        return result;
    }

    int32_t ScaleValue(int32_t value, HMONITOR hMonitor) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& info : m_monitors) {
            if (info.handle == hMonitor) {
                return static_cast<int32_t>(value * info.scaleFactor);
            }
        }
        return value;
    }

    std::vector<DPIMonitorEntry> GetAllMonitors()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_monitors.clear();
        EnumDisplayMonitors(nullptr, nullptr, MonitorEnumCallback, reinterpret_cast<LPARAM>(this));
        return m_monitors;
    }

    DPIAwarenessLevel GetAwarenessLevel() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_awarenessLevel;
    }

    bool SetAwareness(DPIAwarenessLevel level)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_awarenessLevel = level;
        // In production, would call SetProcessDpiAwarenessContext
        return true;
    }

    size_t GetMonitorCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_monitors.size();
    }

    bool Validate() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& info : m_monitors) {
            if (info.dpiX == 0 || info.dpiY == 0)
                return false;
            if (info.scaleFactor <= 0.0)
                return false;
            if (info.scaleFactor > 10.0)
                return false;
        }
        return static_cast<uint32_t>(m_awarenessLevel) <= 3;
    }

  private:
    DPIScalingManager() = default;
    ~DPIScalingManager() = default;
    DPIScalingManager(const DPIScalingManager&) = delete;
    DPIScalingManager& operator=(const DPIScalingManager&) = delete;

    static BOOL CALLBACK MonitorEnumCallback(HMONITOR hMonitor, HDC /*hdc*/, LPRECT lpRect, LPARAM lParam)
    {
        auto* self = reinterpret_cast<DPIScalingManager*>(lParam);
        DPIMonitorEntry info;
        info.handle = hMonitor;

        MONITORINFOEXW mi{};
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoW(hMonitor, &mi)) {
            info.monitorRect = mi.rcMonitor;
            info.workArea = mi.rcWork;
            info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
            info.deviceName = mi.szDevice;
        }

        if (lpRect) {
            info.monitorRect = *lpRect;
        }

        // Get DPI via device caps (works without ShellScalingApi)
        HDC hdc = CreateDCW(L"DISPLAY", nullptr, nullptr, nullptr);
        if (hdc) {
            info.dpiX = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSX));
            info.dpiY = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSY));
            DeleteDC(hdc);
        }

        info.scaleFactor = static_cast<double>(info.dpiX) / 96.0;

        self->m_monitors.push_back(info);
        return TRUE;
    }

    mutable std::mutex m_mutex;
    std::vector<DPIMonitorEntry> m_monitors;
    DPIAwarenessLevel m_awarenessLevel = DPIAwarenessLevel::PerMonitorV2;
};

enum class DPITier : uint8_t {
    Low = 0,
    Standard = 1,
    High = 2,
    VeryHigh = 3,
    Ultra = 4
};

class AdaptiveDPIScaler
{
  public:
    static int StrategyCount()
    {
        return 5;
    }

    static DPITier ClassifyDPI(int dpi)
    {
        if (dpi <= 96)
            return DPITier::Standard;
        if (dpi <= 120)
            return DPITier::High;
        if (dpi <= 192)
            return DPITier::VeryHigh;
        return DPITier::Ultra;
    }

    static int ScaledSize(int baseSize, float dpiRatio)
    {
        return static_cast<int>(baseSize * dpiRatio);
    }

    AdaptiveDPIScaler() = delete;
};

}  // namespace Engine
}  // namespace ExplorerLens
