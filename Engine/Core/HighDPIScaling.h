#pragma once
//==============================================================================
// HighDPIScaling — Sprint 194
// Per-monitor DPI awareness and scaled thumbnail generation
//
// Architecture:
//   1. Per-monitor DPI detection (100%, 125%, 150%, 200%, 250%, 300%)
//   2. Logical-to-physical pixel conversion for thumbnail sizing
//   3. DPI-aware bitmap creation with correct resolution metadata
//   4. Multi-monitor DPI bridging for cross-monitor dragging
//   5. WM_DPICHANGED handling for dynamic scale changes
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs { namespace Engine {

/// DPI scale factor presets
enum class DPIScale : uint8_t {
    Scale100 = 0,    ///< 96 DPI (100%)
    Scale125,        ///< 120 DPI (125%)
    Scale150,        ///< 144 DPI (150%)
    Scale175,        ///< 168 DPI (175%)
    Scale200,        ///< 192 DPI (200%)
    Scale250,        ///< 240 DPI (250%)
    Scale300,        ///< 288 DPI (300%)
    Scale350,        ///< 336 DPI (350%)
    Scale400,        ///< 384 DPI (400%)
    Custom           ///< Non-standard DPI
};

/// DPI awareness mode
enum class DPIAwareness : uint8_t {
    Unaware,              ///< System scales (blurry)
    SystemAware,          ///< Single DPI for all monitors
    PerMonitorV1,         ///< Per-monitor, basic
    PerMonitorV2          ///< Per-monitor V2 (Win10 1703+)
};

/// Monitor DPI information
struct MonitorDPI {
    uint32_t monitorIndex = 0;
    uint32_t dpiX = 96;
    uint32_t dpiY = 96;
    DPIScale scale = DPIScale::Scale100;
    double scaleFactor = 1.0;
    int32_t left = 0;
    int32_t top = 0;
    int32_t width = 1920;
    int32_t height = 1080;
    bool isPrimary = false;
    std::wstring deviceName;
};

/// Thumbnail size request with DPI consideration
struct DPIAwareThumbnailRequest {
    uint32_t logicalWidth = 256;     ///< Requested logical pixels
    uint32_t logicalHeight = 256;
    uint32_t physicalWidth = 0;      ///< Computed physical pixels
    uint32_t physicalHeight = 0;
    uint32_t dpi = 96;
    double scaleFactor = 1.0;
    bool includeResolutionMetadata = true;
};

/// DPI scaling configuration
struct DPIConfig {
    DPIAwareness awareness = DPIAwareness::PerMonitorV2;
    bool enableDynamicScaling = true;       ///< Handle DPI changes at runtime
    bool snapToStandardScales = true;       ///< Round to nearest preset
    uint32_t maxPhysicalSize = 2048;        ///< Max output thumbnail size
    uint32_t minPhysicalSize = 32;          ///< Min output thumbnail size
    bool preferSharpScaling = true;         ///< Use nearest-neighbor for small sizes
    double customScaleFactor = 1.0;         ///< Override scale factor (0 = auto)
};

//==============================================================================
// HighDPIScaling
//==============================================================================
class HighDPIScaling {
public:
    HighDPIScaling();
    explicit HighDPIScaling(const DPIConfig& config);

    /// Get DPI for the primary monitor
    static uint32_t GetSystemDPI();

    /// Get DPI for a specific monitor
    static MonitorDPI GetMonitorDPI(uint32_t monitorIndex);

    /// Enumerate all monitors with DPI info
    static std::vector<MonitorDPI> EnumerateMonitors();

    /// Get current process DPI awareness mode
    static DPIAwareness GetProcessAwareness();

    /// Set process DPI awareness (must call before window creation)
    static bool SetProcessAwareness(DPIAwareness mode);

    /// Convert logical to physical pixels
    static uint32_t LogicalToPhysical(uint32_t logical, uint32_t dpi);
    static uint32_t PhysicalToLogical(uint32_t physical, uint32_t dpi);

    /// Scale a thumbnail request for the target DPI
    DPIAwareThumbnailRequest ScaleRequest(uint32_t logicalWidth,
                                           uint32_t logicalHeight,
                                           uint32_t targetDPI) const;

    /// Get nearest standard scale from raw DPI
    static DPIScale GetNearestScale(uint32_t dpi);

    /// Get DPI value for a scale preset
    static uint32_t GetDPIForScale(DPIScale scale);

    /// Get scale factor (1.0, 1.25, 1.5, etc.)
    static double GetScaleFactor(DPIScale scale);
    static double GetScaleFactorForDPI(uint32_t dpi);

    /// Get config
    const DPIConfig& GetConfig() const { return m_config; }

    /// Static name helpers
    static const wchar_t* GetScaleName(DPIScale scale);
    static const wchar_t* GetAwarenessName(DPIAwareness awareness);

private:
    DPIConfig m_config;
};

}} // namespace DarkThumbs::Engine
