//==============================================================================
// DarkThumbs Engine — Sprint 290: Per-Monitor DPI V3
// Per-monitor DPI awareness V3 with mixed-DPI multi-monitor support.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// DPI awareness level
enum class DPIAwareness : uint8_t {
    Unaware,            // System-scaled (legacy)
    SystemAware,        // Single DPI for all monitors
    PerMonitorV1,       // Per-monitor, top-level only
    PerMonitorV2,       // Per-monitor, child windows too
    PerMonitorV3,       // Per-monitor, dynamic DPI change
    COUNT
};

/// DPI scale factor
enum class DPIScale : uint8_t {
    Scale100,       // 96 DPI (100%)
    Scale125,       // 120 DPI (125%)
    Scale150,       // 144 DPI (150%)
    Scale175,       // 168 DPI (175%)
    Scale200,       // 192 DPI (200%)
    Scale250,       // 240 DPI (250%)
    Scale300,       // 288 DPI (300%)
    Scale350,       // 336 DPI (350%)
    COUNT
};

/// Monitor info for DPI
struct MonitorDPIInfo {
    uint32_t    monitorIndex    = 0;
    uint32_t    dpiX            = 96;
    uint32_t    dpiY            = 96;
    uint32_t    widthPx         = 1920;
    uint32_t    heightPx        = 1080;
    DPIScale    scale           = DPIScale::Scale100;
    bool        isPrimary       = false;
};

/// DPI scaling config
struct DPIScalingConfig {
    DPIAwareness awareness      = DPIAwareness::PerMonitorV3;
    uint32_t     baseThumbnailSize = 256;
    bool         autoScale      = true;
    bool         crispRendering = true;     // No blurring on scale
    float        customScale    = 1.0f;
};

/// Per-monitor DPI V3 manager
class PerMonitorDPIV3 {
public:
    static const wchar_t* AwarenessName(DPIAwareness a) {
        switch (a) {
            case DPIAwareness::Unaware:        return L"DPI Unaware";
            case DPIAwareness::SystemAware:    return L"System DPI Aware";
            case DPIAwareness::PerMonitorV1:   return L"Per-Monitor V1";
            case DPIAwareness::PerMonitorV2:   return L"Per-Monitor V2";
            case DPIAwareness::PerMonitorV3:   return L"Per-Monitor V3";
            default: return L"Unknown";
        }
    }

    static const wchar_t* ScaleName(DPIScale s) {
        switch (s) {
            case DPIScale::Scale100: return L"100%";
            case DPIScale::Scale125: return L"125%";
            case DPIScale::Scale150: return L"150%";
            case DPIScale::Scale175: return L"175%";
            case DPIScale::Scale200: return L"200%";
            case DPIScale::Scale250: return L"250%";
            case DPIScale::Scale300: return L"300%";
            case DPIScale::Scale350: return L"350%";
            default: return L"Unknown";
        }
    }

    /// Calculate scaled thumbnail size
    static uint32_t ScaledSize(uint32_t baseSize, uint32_t dpi) {
        return (baseSize * dpi) / 96;
    }

    static constexpr size_t AwarenessCount() { return static_cast<size_t>(DPIAwareness::COUNT); }
    static constexpr size_t ScaleCount() { return static_cast<size_t>(DPIScale::COUNT); }
};

}} // namespace DarkThumbs::Engine
