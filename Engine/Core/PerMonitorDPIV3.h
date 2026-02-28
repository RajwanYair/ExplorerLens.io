//==============================================================================
// ExplorerLens Engine — Per-Monitor DPI V3
// Per-monitor DPI awareness V3 with mixed-DPI multi-monitor support.
// Types (DPIScale, DPIAwareness) are defined in HighDPIScaling.h
//==============================================================================
#pragma once
#include "HighDPIScaling.h"

namespace ExplorerLens {
namespace Engine {

/// Monitor info for DPI
struct MonitorDPIInfo {
 uint32_t monitorIndex = 0;
 uint32_t dpiX = 96;
 uint32_t dpiY = 96;
 uint32_t widthPx = 1920;
 uint32_t heightPx = 1080;
 DPIScale scale = DPIScale::Scale100;
 bool isPrimary = false;
};

/// DPI scaling config
struct DPIScalingConfig {
 DPIAwareness awareness = DPIAwareness::PerMonitorV2;
 uint32_t baseThumbnailSize = 256;
 bool autoScale = true;
 bool crispRendering = true; // No blurring on scale
 float customScale = 1.0f;
};

/// Per-monitor DPI V3 manager
class PerMonitorDPIV3 {
public:
 static const wchar_t *AwarenessName(DPIAwareness a) {
 switch (a) {
 case DPIAwareness::Unaware:
 return L"DPI Unaware";
 case DPIAwareness::SystemAware:
 return L"System DPI Aware";
 case DPIAwareness::PerMonitorV1:
 return L"Per-Monitor V1";
 case DPIAwareness::PerMonitorV2:
 return L"Per-Monitor V2";
 case DPIAwareness::GDIScaled:
 return L"GDI Scaled";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *ScaleName(DPIScale s) {
 switch (s) {
 case DPIScale::Scale100:
 return L"100%";
 case DPIScale::Scale125:
 return L"125%";
 case DPIScale::Scale150:
 return L"150%";
 case DPIScale::Scale175:
 return L"175%";
 case DPIScale::Scale200:
 return L"200%";
 case DPIScale::Scale250:
 return L"250%";
 case DPIScale::Scale300:
 return L"300%";
 case DPIScale::Scale350:
 return L"350%";
 case DPIScale::Scale400:
 return L"400%";
 case DPIScale::Custom:
 return L"Custom";
 default:
 return L"Unknown";
 }
 }

 /// Calculate scaled thumbnail size
 static uint32_t ScaledSize(uint32_t baseSize, uint32_t dpi) {
 return (baseSize * dpi) / 96;
 }

 static constexpr size_t AwarenessCount() { return 5; } // Unaware..GDIScaled
 static constexpr size_t ScaleCount() { return 8; } // Scale100..Scale300
};

} // namespace Engine
} // namespace ExplorerLens
