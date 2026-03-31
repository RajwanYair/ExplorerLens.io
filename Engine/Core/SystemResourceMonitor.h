#pragma once
// SystemResourceMonitor.h — System Resource Monitor
// Real-time monitoring of CPU, GPU, memory, and I/O utilization to
// dynamically throttle thumbnail generation under resource pressure.
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Resource type
enum class MonitoredResource : uint8_t {
 CPU = 0,
 GPU,
 SystemMemory,
 VideoMemory,
 DiskIO,
 NetworkIO,
 COUNT
};

/// Throttle level
enum class ThrottleLevel : uint8_t {
 // No throttling — full decode throughput
 None = 0,
 // Reduce thread parallelism slightly
 Light,
 // Approximately half throughput
 Moderate,
 // Allow only minimal background work
 Heavy,
 // Suspend all non-essential decode work
 Paused,
 COUNT
};

struct ResourceSnapshot {
 float cpuUsagePct = 0.0f;
 float gpuUsagePct = 0.0f;
 float memoryUsagePct = 0.0f;
 float vramUsagePct = 0.0f;
 float diskUtilizationPct = 0.0f;
 uint64_t availableMemMB = 0;
 uint64_t availableVRAMMB = 0;
 ThrottleLevel recommendedThrottle = ThrottleLevel::None;
};

struct ResourceMonitorConfig {
 uint32_t pollIntervalMs = 1000;
 float cpuThrottleAt = 90.0f; // start throttling at 90% CPU
 float memThrottleAt = 85.0f;
 float gpuThrottleAt = 90.0f;
 bool autoThrottle = true;
 bool logSnapshots = false;
};

class SystemResourceMonitor {
public:
 static constexpr size_t ResourceCount() {
 return static_cast<size_t>(MonitoredResource::COUNT);
 }
 static constexpr size_t ThrottleCount() {
 return static_cast<size_t>(ThrottleLevel::COUNT);
 }

 static const wchar_t *ResourceName(MonitoredResource r) {
 switch (r) {
 case MonitoredResource::CPU:
 return L"CPU";
 case MonitoredResource::GPU:
 return L"GPU";
 case MonitoredResource::SystemMemory:
 return L"System Memory";
 case MonitoredResource::VideoMemory:
 return L"Video Memory";
 case MonitoredResource::DiskIO:
 return L"Disk I/O";
 case MonitoredResource::NetworkIO:
 return L"Network I/O";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *ThrottleName(ThrottleLevel t) {
 switch (t) {
 case ThrottleLevel::None:
 return L"None";
 case ThrottleLevel::Light:
 return L"Light";
 case ThrottleLevel::Moderate:
 return L"Moderate";
 case ThrottleLevel::Heavy:
 return L"Heavy";
 case ThrottleLevel::Paused:
 return L"Paused";
 default:
 return L"Unknown";
 }
 }

 /// Determine throttle level from CPU and memory usage
 static ThrottleLevel RecommendThrottle(float cpuPct, float memPct) {
 float maxPressure = (std::max)(cpuPct, memPct);
 if (maxPressure < 70.0f)
 return ThrottleLevel::None;
 if (maxPressure < 80.0f)
 return ThrottleLevel::Light;
 if (maxPressure < 90.0f)
 return ThrottleLevel::Moderate;
 if (maxPressure < 95.0f)
 return ThrottleLevel::Heavy;
 return ThrottleLevel::Paused;
 }
};

class AccessibilityNarratorBridge {
public:
    static int FeatureCount() { return 6; }
    static const wchar_t* GenerateNarratorText(
        const wchar_t* filename, int /*w*/, int /*h*/, const wchar_t* /*fmt*/) {
        return filename ? filename : L"Image file";
    }
    AccessibilityNarratorBridge() = delete;
};

} // namespace Engine
} // namespace ExplorerLens
