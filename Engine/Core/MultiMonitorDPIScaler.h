#pragma once
// ============================================================================
// MultiMonitorDPIScaler.h — Per-monitor DPI-aware thumbnail scaling
//
// Purpose:   Per-monitor DPI-aware thumbnail scaling
// Provides:  ScaleMode enum, MonitorDPIScaleConfig struct,
//            MultiMonitorDPIScaler class
// Used by:   Render pipeline for multi-display setups
// ============================================================================

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// DPI scaling mode strategy
enum class DPIScaleMode : uint8_t {
    System = 0,  // System-wide DPI (legacy)
    PerMonitor = 1,  // Per-Monitor DPI (Win 8.1+)
    PerMonitorV2 = 2,  // Per-Monitor V2 (Win10 1703+)
    Unaware = 3,  // DPI unaware (legacy apps)
    CustomScale = 4   // User-defined custom scale factor
};

inline const char* DPIScaleModeName(DPIScaleMode m) noexcept {
    switch (m) {
    case DPIScaleMode::System:       return "System";
    case DPIScaleMode::PerMonitor:   return "PerMonitor";
    case DPIScaleMode::PerMonitorV2: return "PerMonitorV2";
    case DPIScaleMode::Unaware:      return "Unaware";
    case DPIScaleMode::CustomScale:  return "CustomScale";
    default:                         return "Unknown";
    }
}

/// Display monitor profile classification
enum class MonitorProfile : uint8_t {
    Standard = 0,  // 96 DPI (100%)
    HiDPI = 1,  // 144 DPI (150%)
    UltraHiDPI = 2,  // 192+ DPI (200%+)
    Mixed = 3,  // Multi-monitor with different DPIs
    Projector = 4   // Projector / external display
};

inline const char* MonitorProfileName(MonitorProfile p) noexcept {
    switch (p) {
    case MonitorProfile::Standard:   return "Standard";
    case MonitorProfile::HiDPI:      return "HiDPI";
    case MonitorProfile::UltraHiDPI: return "UltraHiDPI";
    case MonitorProfile::Mixed:      return "Mixed";
    case MonitorProfile::Projector:  return "Projector";
    default:                         return "Unknown";
    }
}

/// Information about a connected monitor
struct DPIMonitorInfo {
    uint32_t       monitorIndex = 0;
    uint32_t       dpiX = 96;
    uint32_t       dpiY = 96;
    float          scaleFactor = 1.0f;
    MonitorProfile profile = MonitorProfile::Standard;
    bool           isPrimary = false;
    std::wstring   deviceName;
};

/// Configuration for DPI scaling behavior
struct MonitorDPIScaleConfig {
    DPIScaleMode mode = DPIScaleMode::PerMonitorV2;
    float        customScale = 1.0f;     // Only used in CustomScale mode
    bool         roundToNearest = true;      // Round scaled sizes to nearest pixel
    uint32_t     baseDPI = 96;        // Reference DPI for 100%
};

/// Manages per-monitor DPI scaling for thumbnail rendering,
/// detecting display configurations and applying appropriate
/// scale factors for crisp thumbnails on HiDPI screens.
class MultiMonitorDPIScaler {
public:
    MultiMonitorDPIScaler() = default;
    ~MultiMonitorDPIScaler() = default;

    MultiMonitorDPIScaler(const MultiMonitorDPIScaler&) = delete;
    MultiMonitorDPIScaler& operator=(const MultiMonitorDPIScaler&) = delete;
    MultiMonitorDPIScaler(MultiMonitorDPIScaler&&) noexcept = default;
    MultiMonitorDPIScaler& operator=(MultiMonitorDPIScaler&&) noexcept = default;

    /// Get the scale factor for a given DPI value
    float GetScaleFactor(uint32_t dpi) const noexcept {
        if (dpi == 0) dpi = m_config.baseDPI;
        return static_cast<float>(dpi) / static_cast<float>(m_config.baseDPI);
    }

    /// Scale a thumbnail dimension for a specific monitor
    uint32_t ScaleForMonitor(uint32_t baseSizePx, uint32_t monitorDpi) const noexcept {
        float factor = GetScaleFactor(monitorDpi);
        float scaled = static_cast<float>(baseSizePx) * factor;
        if (m_config.roundToNearest) {
            return static_cast<uint32_t>(scaled + 0.5f);
        }
        return static_cast<uint32_t>(scaled);
    }

    /// Classify a monitor based on its DPI
    MonitorProfile GetMonitorProfile(uint32_t dpi) const noexcept {
        if (dpi <= 96)  return MonitorProfile::Standard;
        if (dpi <= 144) return MonitorProfile::HiDPI;
        return MonitorProfile::UltraHiDPI;
    }

    /// Get the current DPI mode
    DPIScaleMode GetMode() const noexcept { return m_config.mode; }

    /// Set the DPI scale mode
    void SetMode(DPIScaleMode mode) noexcept { m_config.mode = mode; }

    /// Apply configuration
    void SetConfig(const MonitorDPIScaleConfig& cfg) noexcept { m_config = cfg; }

    /// Get current configuration
    const MonitorDPIScaleConfig& GetConfig() const noexcept { return m_config; }

private:
    MonitorDPIScaleConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
