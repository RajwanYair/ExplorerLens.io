#pragma once
// MultiMonitorColorProfile.h — Multi-Monitor Color Profile Engine
// Per-monitor ICC color profile application for thumbnails, ensuring
// accurate color reproduction across displays with different gamuts.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Monitor color profile type
enum class MonitorProfileType : uint8_t {
  sRGB = 0,     // Standard sRGB (most monitors)
  AdobeRGB,     // Photography monitors
  DCI_P3,       // Wide-gamut creative monitors
  Rec2020,      // HDR/broadcast monitors
  Custom_ICC,   // User-loaded ICC profile
  Uncalibrated, // No profile detected
  COUNT
};

/// Color matching method
enum class ColorMatchMethod : uint8_t {
  None = 0,   // No color management
  ICM_System, // Windows ICM 2.0
  WCS,        // Windows Color System
  LittleCMS,  // LittleCMS library (if available)
  COUNT
};

struct MonitorColorInfo {
  uint32_t monitorIndex = 0;
  const wchar_t *deviceName = nullptr;
  MonitorProfileType profileType = MonitorProfileType::sRGB;
  float gamutCoverage = 0.0f; // sRGB % coverage
  float deltaEAvg = 0.0f;     // average color error
  bool hdrEnabled = false;
  uint32_t maxLuminance = 0; // nits
};

struct ColorProfileConfig {
  bool enabled = true;
  ColorMatchMethod method = ColorMatchMethod::ICM_System;
  bool perMonitor = true;
  bool softProof = false;
  const wchar_t *fallbackProfile = nullptr;
};

class MultiMonitorColorProfile {
public:
  static constexpr size_t ProfileTypeCount() {
    return static_cast<size_t>(MonitorProfileType::COUNT);
  }
  static constexpr size_t MethodCount() {
    return static_cast<size_t>(ColorMatchMethod::COUNT);
  }

  static const wchar_t *ProfileTypeName(MonitorProfileType p) {
    switch (p) {
    case MonitorProfileType::sRGB:
      return L"sRGB";
    case MonitorProfileType::AdobeRGB:
      return L"Adobe RGB";
    case MonitorProfileType::DCI_P3:
      return L"DCI-P3";
    case MonitorProfileType::Rec2020:
      return L"Rec.2020";
    case MonitorProfileType::Custom_ICC:
      return L"Custom ICC";
    case MonitorProfileType::Uncalibrated:
      return L"Uncalibrated";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *MethodName(ColorMatchMethod m) {
    switch (m) {
    case ColorMatchMethod::None:
      return L"None";
    case ColorMatchMethod::ICM_System:
      return L"ICM 2.0";
    case ColorMatchMethod::WCS:
      return L"WCS";
    case ColorMatchMethod::LittleCMS:
      return L"LittleCMS";
    default:
      return L"Unknown";
    }
  }

  /// Check if wide-gamut conversion is needed
  static bool NeedsGamutMapping(MonitorProfileType source,
                                MonitorProfileType display) {
    return source != display && source != MonitorProfileType::Uncalibrated;
  }
};

} // namespace Engine
} // namespace ExplorerLens
