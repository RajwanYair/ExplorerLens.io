// MultiMonitorContext.h — Multi-Monitor DPI Context Tracker
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks all connected monitors, their HMONITOR handles, DPI scales, physical
// rectangles, and color profiles to enable per-monitor thumbnail sizing.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Monitor Descriptor -----------------------------------------------------

enum class HDRDisplayCapability : uint8_t {
    None        = 0,
    HDR10       = 1,   // ST.2084 (PQ) + BT.2020
    DolbyVision = 2,
    HybridLogGamma = 3,
};

struct MonitorInfo {
    void*       hmonitor        = nullptr;  // HMONITOR — opaque handle
    uint32_t    dpiX            = 96;
    uint32_t    dpiY            = 96;
    float       scaleFactor     = 1.0f;     // dpiX / 96.0f
    int32_t     left            = 0;
    int32_t     top             = 0;
    int32_t     widthPx         = 0;
    int32_t     heightPx        = 0;
    bool        isPrimary       = false;
    bool        isHDR           = false;
    HDRDisplayCapability hdrCap = HDRDisplayCapability::None;
    float       peakLuminance   = 80.0f;    // Nits (SDR default 80 nit)
    std::string deviceName;                 // e.g. "\\.\DISPLAY1"
    std::string iccProfilePath;
};

// ---- Change Notification ----------------------------------------------------

using MonitorChangeCallback = std::function<void(const std::vector<MonitorInfo>&)>;

// ---- MultiMonitorContext ----------------------------------------------------

class MultiMonitorContext {
public:
    // Enumerate all monitors via EnumDisplayMonitors + GetDpiForMonitor.
    void Refresh();

    const std::vector<MonitorInfo>& Monitors() const { return m_monitors; }

    // Find monitor containing the given screen point (pixels).
    const MonitorInfo* MonitorForPoint(int32_t x, int32_t y) const;

    // Find monitor nearest to a given HWND.
    const MonitorInfo* MonitorForWindow(void* hwnd) const;

    // Primary monitor (first with isPrimary = true).
    const MonitorInfo* PrimaryMonitor() const;

    // Average DPI across all monitors.
    float AverageScaleFactor() const;

    // Register / unregister WM_DISPLAYCHANGE watcher (see MonitorConfigWatcher).
    void SetChangeCallback(MonitorChangeCallback cb);
    void ClearChangeCallback();

    // Singleton
    static MultiMonitorContext& Instance();

private:
    MultiMonitorContext()  = default;
    ~MultiMonitorContext() = default;

    MultiMonitorContext(const MultiMonitorContext&) = delete;
    MultiMonitorContext& operator=(const MultiMonitorContext&) = delete;

    std::vector<MonitorInfo> m_monitors;
    MonitorChangeCallback    m_changeCallback;

    static bool MonitorEnumProc(void* hmon, void* hdc, void* rect, void* ctx);
};

} // namespace Engine
} // namespace ExplorerLens
