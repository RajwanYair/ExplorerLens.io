#pragma once
//==============================================================================
// Win11Integration — Sprint 209
// Windows 11 native features: mica, rounded corners, snap layouts, widget provider
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs { namespace Engine {

/// Windows version info
struct WindowsVersionInfo {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t build = 0;
    bool isWin11 = false;        // Build >= 22000
    bool isWin11_22H2 = false;   // Build >= 22621
    bool isWin11_23H2 = false;   // Build >= 22631
    bool isWin11_24H2 = false;   // Build >= 26100
    std::wstring displayName;
};

/// Shell integration capabilities
enum class Win11Feature : uint8_t {
    RoundedCorners = 0,
    MicaMaterial,
    SnapLayouts,
    Widgets,
    DarkMode,
    DirectStorage,
    AutoHDR,
    WindowedGames,
    FeatureCount
};

/// Mica material mode
enum class MicaMode : uint8_t {
    None = 0,
    Mica,
    MicaAlt,
    Acrylic
};

/// Snap layout zone
struct SnapZone {
    uint32_t zoneId = 0;
    int32_t  x = 0, y = 0;
    uint32_t width = 0, height = 0;
    float    scale = 1.0f;
};

//------------------------------------------------------------------------------
class Win11Integration {
public:
    Win11Integration();
    ~Win11Integration() = default;

    // Version detection
    WindowsVersionInfo DetectVersion() const;
    bool IsWindows11() const { return m_versionInfo.isWin11; }
    uint32_t GetBuildNumber() const { return m_versionInfo.build; }

    // Feature support
    bool IsFeatureAvailable(Win11Feature feature) const;
    std::vector<Win11Feature> GetAvailableFeatures() const;
    uint32_t GetAvailableFeatureCount() const;

    // Mica/Material
    MicaMode GetCurrentMicaMode() const { return m_micaMode; }
    void SetMicaMode(MicaMode mode) { m_micaMode = mode; }

    // Dark mode
    bool IsDarkModeEnabled() const;
    bool IsSystemDarkMode() const;

    // Snap layouts
    std::vector<SnapZone> GetSnapZones(uint32_t monitorWidth, uint32_t monitorHeight) const;

    // Static info
    static const wchar_t* GetFeatureName(Win11Feature feature);
    static const wchar_t* GetMicaModeName(MicaMode mode);
    static uint32_t GetFeatureCount();

private:
    WindowsVersionInfo m_versionInfo;
    MicaMode m_micaMode = MicaMode::None;
    void DetectOS();
};

}} // namespace DarkThumbs::Engine
