// WinUI3Research.h — WinUI 3 / Windows App SDK Modernization Feasibility
// Copyright (c) 2026 ExplorerLens Project
//
// Documents the feasibility study for migrating LENSManager from WTL to
// WinUI 3. Evaluates compatibility with COM shell extensions, admin elevation,
// XAML Islands integration, and mixed WTL+WinUI hybrid approach.

#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// Migration feasibility assessment
enum class MigrationFeasibility : uint8_t {
    NotFeasible = 0, ///< Cannot migrate
    PartialOnly = 1, ///< Some components can migrate
    FullyFeasible = 2, ///< Complete migration possible
    Recommended = 3 ///< Migration recommended
};

/// WinUI 3 component assessment
struct WinUI3Assessment {
    const char* component = nullptr;
    const char* currentTech = nullptr;
    const char* proposedTech = nullptr;
    MigrationFeasibility feasibility = MigrationFeasibility::NotFeasible;
    const char* blockers = nullptr;
    const char* notes = nullptr;
    uint32_t effortDays = 0;
};

/// XAML Islands integration status
enum class XamlIslandsStatus : uint8_t {
    NotAvailable = 0, ///< OS version too old
    Available = 1, ///< Can be used
    Initialized = 2, ///< Hosting initialized
    Active = 3 ///< Controls rendered
};

/// WinUI 3 modernization research results
class WinUI3Research {
public:
    static WinUI3Research& Instance() {
        static WinUI3Research inst;
        return inst;
    }

    /// Overall migration assessment
    MigrationFeasibility GetOverallFeasibility() const {
        return MigrationFeasibility::PartialOnly; // Hybrid approach recommended
    }

    /// Get assessment for each component
    static constexpr uint32_t ASSESSMENT_COUNT = 8;

    WinUI3Assessment GetAssessment(uint32_t index) const {
        if (index >= ASSESSMENT_COUNT) return {};

        static const WinUI3Assessment assessments[] = {
        {
        "LENSManager GUI",
        "WTL 10.0 (ATL Dialog)",
        "WinUI 3 XAML",
        MigrationFeasibility::PartialOnly,
        "Admin elevation conflicts with WinUI 3 app model; "
        "WinUI 3 doesn't support running elevated by default",
        "Hybrid approach using XAML Islands is the recommended path",
        30
        },
        {
        "Shell Extension DLL",
        "ATL COM (IThumbnailProvider)",
        "Keep as-is (COM required)",
        MigrationFeasibility::NotFeasible,
        "IThumbnailProvider MUST be COM DLL loaded by explorer.exe; "
        "WinUI 3 cannot host in-process COM servers",
        "Shell extension stays as native COM DLL — non-negotiable",
        0
        },
        {
        "Settings Dialog",
        "WTL CPropertySheet",
        "WinUI 3 NavigationView",
        MigrationFeasibility::FullyFeasible,
        nullptr,
        "Settings page is the best candidate for XAML modernization. "
        "Can use ContentDialog + NavigationView for tabbed layout",
        15
        },
        {
        "Format Registration",
        "Win32 CheckBoxes + Registry",
        "WinUI 3 ToggleSwitch + TreeView",
        MigrationFeasibility::FullyFeasible,
        nullptr,
        "TreeView with ToggleSwitches would greatly improve UX. "
        "Category grouping becomes natural with TreeViewItem hierarchy",
        10
        },
        {
        "Theme Support",
        "Owner-draw dark mode (uxtheme hacks)",
        "WinUI 3 native dark/light theme",
        MigrationFeasibility::Recommended,
        nullptr,
        "WinUI 3 handles dark mode natively with RequestedTheme. "
        "Eliminates need for DarkModeRendererV2 hacks",
        5
        },
        {
        "Performance Charts",
        "GDI+ custom drawing",
        "WinUI 3 + Win2D charts",
        MigrationFeasibility::FullyFeasible,
        nullptr,
        "Win2D provides GPU-accelerated 2D rendering for charts. "
        "Significantly better than GDI+ for real-time stats",
        12
        },
        {
        "COM Registration",
        "RegSvr32 + manual registry",
        "MSIX sparse package + AppExtension",
        MigrationFeasibility::PartialOnly,
        "MSIX sparse packages support COM registration but require "
        "Windows 10 2004+. Classic reg approach must remain as fallback",
        "MSIX sparse package gives clean install/uninstall + auto-update. "
        "Dual-path: MSIX for modern OS, traditional for older",
        20
        },
        {
        "Installer",
        "WiX 6 MSI + Inno Setup",
        "MSIX + MSI bridge",
        MigrationFeasibility::PartialOnly,
        "MSIX doesn't support all COM registration scenarios. "
        "Shell extension COM needs SxS manifest or sparse package",
        "Recommended: Keep MSI as primary, add MSIX as optional modern path",
        15
        }
        };

        return assessments[index];
    }

    /// Total migration effort estimate
    uint32_t GetTotalEffortDays() const {
        uint32_t total = 0;
        for (uint32_t i = 0; i < ASSESSMENT_COUNT; ++i)
            total += GetAssessment(i).effortDays;
        return total;
    }

    /// Recommended approach
    const char* GetRecommendedApproach() const {
        return
            "HYBRID APPROACH (Recommended):\n"
            "1. Keep LENSShell.dll as native COM DLL (mandatory)\n"
            "2. Keep LENSManager as WTL for admin/registration operations\n"
            "3. Add XAML Islands for modern UI panels (settings, gallery)\n"
            "4. Use ContentIslands API (Win11 22H2+) for embedded WinUI 3 content\n"
            "5. Fallback to pure WTL on older Windows versions\n"
            "\n"
            "PREREQUISITES:\n"
            "- Windows App SDK 1.5+ NuGet package\n"
            "- WindowsAppRuntime redistributable or self-contained\n"
            "- C++/WinRT header generation for XAML interop\n"
            "\n"
            "TIMELINE: ~4-6 development cycles for initial settings panel migration";
    }

    /// Check Windows App SDK availability
    bool IsWindowsAppSDKAvailable() const {
        HMODULE hMod = LoadLibraryW(L"Microsoft.ui.xaml.dll");
        if (hMod) {
            FreeLibrary(hMod);
            return true;
        }
        return false;
    }

    /// Check minimum Windows version for XAML Islands
    /// Requires Windows 10 1903+ (build 18362)
    bool IsXamlIslandsSupported() const {
        // Use RtlGetVersion to avoid versionhelpers.h (WIN32_LEAN_AND_MEAN incompatible)
        using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (!hNtdll) return false;

        auto pRtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
            GetProcAddress(hNtdll, "RtlGetVersion"));
        if (!pRtlGetVersion) return false;

        RTL_OSVERSIONINFOW osvi = {};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        pRtlGetVersion(&osvi);

        // Windows 10 1903 = build 18362
        return osvi.dwBuildNumber >= 18362;
    }

private:
    WinUI3Research() = default;
};

} // namespace Engine
} // namespace ExplorerLens
