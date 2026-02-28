//==============================================================================
// HighDPIScaling.cpp
// Per-monitor DPI awareness and scaled thumbnail generation
//==============================================================================

#include "HighDPIScaling.h"
#include <algorithm>

#ifdef _WIN32
#include <shellscalingapi.h>
#include <windows.h>
#endif

namespace ExplorerLens {
namespace Engine {

HighDPIScaling::HighDPIScaling() = default;

HighDPIScaling::HighDPIScaling(const DPIConfig &config) : m_config(config) {}

//==============================================================================
// System DPI
//==============================================================================
uint32_t HighDPIScaling::GetSystemDPI() {
#ifdef _WIN32
 HDC hdc = GetDC(nullptr);
 if (hdc) {
 int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
 ReleaseDC(nullptr, hdc);
 return static_cast<uint32_t>(dpi);
 }
#endif
 return 96;
}

MonitorDPI HighDPIScaling::GetMonitorDPI(uint32_t monitorIndex) {
 MonitorDPI info;
 info.monitorIndex = monitorIndex;
 info.dpiX = GetSystemDPI();
 info.dpiY = info.dpiX;
 info.scale = GetNearestScale(info.dpiX);
 info.scaleFactor = GetScaleFactorForDPI(info.dpiX);
 info.isPrimary = (monitorIndex == 0);

#ifdef _WIN32
 info.width = GetSystemMetrics(SM_CXSCREEN);
 info.height = GetSystemMetrics(SM_CYSCREEN);
#endif

 return info;
}

std::vector<MonitorDPI> HighDPIScaling::EnumerateMonitors() {
 std::vector<MonitorDPI> monitors;

#ifdef _WIN32
 int count = GetSystemMetrics(SM_CMONITORS);
 if (count <= 0)
 count = 1;
 for (int i = 0; i < count; i++) {
 monitors.push_back(GetMonitorDPI(static_cast<uint32_t>(i)));
 }
#else
 monitors.push_back(GetMonitorDPI(0));
#endif

 return monitors;
}

//==============================================================================
// DPI Awareness
//==============================================================================
DPIAwareness HighDPIScaling::GetProcessAwareness() {
#ifdef _WIN32
 typedef HRESULT(WINAPI * PFN_GetProcessDpiAwareness)(HANDLE, int *);
 auto hMod = GetModuleHandleW(L"shcore.dll");
 if (hMod) {
 auto pFunc = reinterpret_cast<PFN_GetProcessDpiAwareness>(
 GetProcAddress(hMod, "GetProcessDpiAwareness"));
 if (pFunc) {
 int awareness = 0;
 if (SUCCEEDED(pFunc(nullptr, &awareness))) {
 switch (awareness) {
 case 0:
 return DPIAwareness::Unaware;
 case 1:
 return DPIAwareness::SystemAware;
 case 2:
 return DPIAwareness::PerMonitorV1;
 }
 }
 }
 }
#endif
 return DPIAwareness::Unaware;
}

bool HighDPIScaling::SetProcessAwareness(DPIAwareness mode) {
#ifdef _WIN32
 typedef HRESULT(WINAPI * PFN_SetProcessDpiAwareness)(int);
 auto hMod = GetModuleHandleW(L"shcore.dll");
 if (hMod) {
 auto pFunc = reinterpret_cast<PFN_SetProcessDpiAwareness>(
 GetProcAddress(hMod, "SetProcessDpiAwareness"));
 if (pFunc) {
 int awareness = 0;
 switch (mode) {
 case DPIAwareness::Unaware:
 awareness = 0;
 break;
 case DPIAwareness::SystemAware:
 awareness = 1;
 break;
 case DPIAwareness::PerMonitorV1:
 awareness = 2;
 break;
 case DPIAwareness::PerMonitorV2:
 awareness = 2;
 break; // Fallback to V1
 case DPIAwareness::GDIScaled:
 awareness = 2;
 break; // Fallback to V1
 }
 return SUCCEEDED(pFunc(awareness));
 }
 }
#endif
 (void)mode;
 return false;
}

//==============================================================================
// Pixel Conversion
//==============================================================================
uint32_t HighDPIScaling::LogicalToPhysical(uint32_t logical, uint32_t dpi) {
 if (dpi == 0)
 dpi = 96;
 return static_cast<uint32_t>(
 static_cast<double>(logical) * static_cast<double>(dpi) / 96.0 + 0.5);
}

uint32_t HighDPIScaling::PhysicalToLogical(uint32_t physical, uint32_t dpi) {
 if (dpi == 0)
 dpi = 96;
 return static_cast<uint32_t>(
 static_cast<double>(physical) * 96.0 / static_cast<double>(dpi) + 0.5);
}

DPIAwareThumbnailRequest
HighDPIScaling::ScaleRequest(uint32_t logicalWidth, uint32_t logicalHeight,
 uint32_t targetDPI) const {

 DPIAwareThumbnailRequest req;
 req.logicalWidth = logicalWidth;
 req.logicalHeight = logicalHeight;
 req.dpi = targetDPI;
 req.scaleFactor = GetScaleFactorForDPI(targetDPI);

 req.physicalWidth = LogicalToPhysical(logicalWidth, targetDPI);
 req.physicalHeight = LogicalToPhysical(logicalHeight, targetDPI);

 // Clamp to config limits
 req.physicalWidth = std::clamp(req.physicalWidth, m_config.minPhysicalSize,
 m_config.maxPhysicalSize);
 req.physicalHeight = std::clamp(req.physicalHeight, m_config.minPhysicalSize,
 m_config.maxPhysicalSize);

 return req;
}

//==============================================================================
// Scale Helpers
//==============================================================================
DPIScale HighDPIScaling::GetNearestScale(uint32_t dpi) {
 if (dpi <= 108)
 return DPIScale::Scale100; // ≤112%
 if (dpi <= 132)
 return DPIScale::Scale125;
 if (dpi <= 156)
 return DPIScale::Scale150;
 if (dpi <= 180)
 return DPIScale::Scale175;
 if (dpi <= 216)
 return DPIScale::Scale200;
 if (dpi <= 264)
 return DPIScale::Scale250;
 if (dpi <= 312)
 return DPIScale::Scale300;
 if (dpi <= 360)
 return DPIScale::Scale350;
 return DPIScale::Scale400;
}

uint32_t HighDPIScaling::GetDPIForScale(DPIScale scale) {
 switch (scale) {
 case DPIScale::Scale100:
 return 96;
 case DPIScale::Scale125:
 return 120;
 case DPIScale::Scale150:
 return 144;
 case DPIScale::Scale175:
 return 168;
 case DPIScale::Scale200:
 return 192;
 case DPIScale::Scale250:
 return 240;
 case DPIScale::Scale300:
 return 288;
 case DPIScale::Scale350:
 return 336;
 case DPIScale::Scale400:
 return 384;
 default:
 return 96;
 }
}

double HighDPIScaling::GetScaleFactor(DPIScale scale) {
 return static_cast<double>(GetDPIForScale(scale)) / 96.0;
}

double HighDPIScaling::GetScaleFactorForDPI(uint32_t dpi) {
 if (dpi == 0)
 return 1.0;
 return static_cast<double>(dpi) / 96.0;
}

//==============================================================================
// Name Helpers
//==============================================================================
const wchar_t *HighDPIScaling::GetScaleName(DPIScale scale) {
 switch (scale) {
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

const wchar_t *HighDPIScaling::GetAwarenessName(DPIAwareness awareness) {
 switch (awareness) {
 case DPIAwareness::Unaware:
 return L"Unaware";
 case DPIAwareness::SystemAware:
 return L"SystemAware";
 case DPIAwareness::PerMonitorV1:
 return L"PerMonitorV1";
 case DPIAwareness::PerMonitorV2:
 return L"PerMonitorV2";
 case DPIAwareness::GDIScaled:
 return L"GDIScaled";
 default:
 return L"Unknown";
 }
}

} // namespace Engine
} // namespace ExplorerLens
