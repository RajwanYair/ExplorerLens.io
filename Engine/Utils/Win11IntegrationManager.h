//==============================================================================
// DarkThumbs Engine — Sprint 270: Windows 11 24H2 Integration
// Modern context menus, tabbed Explorer thumbnail refresh, dark mode awareness.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Windows 11 integration feature set
enum class Win11Feature : uint8_t {
    ModernContextMenu,      // IExplorerCommand for Win11 context menus
    TabbedExplorer,         // Multi-tab thumbnail refresh handling
    DarkModeAware,          // Auto-detect system dark/light mode
    RoundedCorners,         // Rounded thumbnail corners (Win11 style)
    MicaEffect,             // Mica/Acrylic backdrop support
    SnapLayouts,            // Snap layout aware thumbnail sizing
    FileRecommendations,    // Windows 11 Start recommendations
    WidgetIntegration,      // Widgets board integration
    COUNT
};

/// Windows version detection
enum class WindowsVersion : uint8_t {
    Windows10_1809,     // RS5 — minimum supported
    Windows10_1903,
    Windows10_2004,
    Windows10_21H2,     // Last Win10 feature update
    Windows11_21H2,     // Initial Win11
    Windows11_22H2,
    Windows11_23H2,
    Windows11_24H2,     // Latest — modern context menus, copilot
    Unknown
};

/// Windows 11 integration config
struct Win11IntegrationConfig {
    WindowsVersion detectedVersion  = WindowsVersion::Unknown;
    bool enableModernMenu           = true;
    bool enableRoundedCorners       = true;
    bool enableDarkMode             = true;
    uint32_t cornerRadius           = 8;    // Win11 default
};

/// Windows 11 integration manager
class Win11IntegrationManager {
public:
    /// Feature name
    static const wchar_t* FeatureName(Win11Feature f) {
        switch (f) {
            case Win11Feature::ModernContextMenu:   return L"Modern Context Menu";
            case Win11Feature::TabbedExplorer:      return L"Tabbed Explorer";
            case Win11Feature::DarkModeAware:       return L"Dark Mode Aware";
            case Win11Feature::RoundedCorners:      return L"Rounded Corners";
            case Win11Feature::MicaEffect:          return L"Mica Effect";
            case Win11Feature::SnapLayouts:         return L"Snap Layouts";
            case Win11Feature::FileRecommendations: return L"File Recommendations";
            case Win11Feature::WidgetIntegration:   return L"Widget Integration";
            default: return L"Unknown";
        }
    }

    /// Version display string
    static const wchar_t* VersionName(WindowsVersion v) {
        switch (v) {
            case WindowsVersion::Windows10_1809: return L"Windows 10 1809";
            case WindowsVersion::Windows10_1903: return L"Windows 10 1903";
            case WindowsVersion::Windows10_2004: return L"Windows 10 2004";
            case WindowsVersion::Windows10_21H2: return L"Windows 10 21H2";
            case WindowsVersion::Windows11_21H2: return L"Windows 11 21H2";
            case WindowsVersion::Windows11_22H2: return L"Windows 11 22H2";
            case WindowsVersion::Windows11_23H2: return L"Windows 11 23H2";
            case WindowsVersion::Windows11_24H2: return L"Windows 11 24H2";
            default: return L"Unknown";
        }
    }

    /// Check if Win11 feature is available for detected version
    static bool IsFeatureAvailable(Win11Feature f, WindowsVersion v) {
        if (v < WindowsVersion::Windows11_21H2) return false;
        if (f == Win11Feature::TabbedExplorer && v < WindowsVersion::Windows11_22H2) return false;
        if (f == Win11Feature::WidgetIntegration && v < WindowsVersion::Windows11_23H2) return false;
        return true;
    }

    /// Feature count
    static constexpr size_t FeatureCount() { return static_cast<size_t>(Win11Feature::COUNT); }

    /// Version count
    static constexpr size_t VersionCount() { return 8; }
};

}} // namespace DarkThumbs::Engine
