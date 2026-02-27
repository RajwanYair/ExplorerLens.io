#pragma once
// ============================================================================
// WindowsCompat.h — Consolidated Windows Compatibility & Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for Windows platform integration:
//   Section 1 — Win11CompatibilityLayer: build detection, DPI helpers, dark mode, GPU enum
//   Section 2 — Win11IntegrationManager: Win11 24H2 features, context menus, tabbed Explorer
//   Section 3 — Windows11CompatMatrix: comprehensive OS/HDR/GPU/ARM64/DPI compat matrix
//   Section 4 — Windows12Compatibility: Dynamic Island, Fluent V4, NPU APIs
//   Section 5 — CompatibilityMatrix: test-scenario matrix execution framework
//   Tail     — #include "Win11Integration.h" (has .cpp, kept separate)
// ============================================================================

// ============================================================================
// Section 1 — Win11CompatibilityLayer
// Windows 11 runtime version detection, DPI helpers, and feature queries
//
// Provides runtime detection of Win11 build variants (22H2/23H2/24H2),
// per-monitor DPI scaling helpers, dark mode state queries, and GPU
// capability enumeration for the shell extension and manager GUI.
//
// Thread Safety: All functions are thread-safe (read-only or use TLS).
// ============================================================================

#include <Windows.h>
#include <ShellScalingApi.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <optional>
#include <functional>
#include <cstddef>
#include <algorithm>

#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "dxgi.lib")

namespace ExplorerLens {

// ============================================================================
// Windows 11 Build Detection
// ============================================================================

/// Windows 11 release identifier
enum class Win11Release {
    NOT_WIN11, ///< Windows 10 or earlier
    WIN11_21H2, ///< Build 22000 (initial release)
    WIN11_22H2, ///< Build 22621
    WIN11_23H2, ///< Build 22631
    WIN11_24H2, ///< Build 26100+
    WIN11_FUTURE ///< Unknown future build
};

/// GPU adapter information
struct GPUInfo {
    std::wstring description;
    UINT vendorId;
    size_t dedicatedVideoMemoryMB;
    bool isDiscrete; ///< true = dGPU, false = iGPU
    bool supportsD3D11;
    D3D_FEATURE_LEVEL featureLevel;
};

/// System compatibility report
struct CompatibilityReport {
    Win11Release win11Release = Win11Release::NOT_WIN11;
    DWORD buildNumber = 0;
    std::wstring architecture;
    bool isARM64 = false;
    bool isDarkMode = false;
    bool isHDRSupported = false;
    bool isPerMonitorV2 = false;
    int monitorCount = 0;
    std::vector<int> dpiScales; ///< Per-monitor DPI scale percentages
    std::vector<GPUInfo> gpus;
    bool hasDiscreteGPU = false;
    bool hasIntegratedGPU = false;
};

/// Detect Windows 11 release from build number
inline Win11Release DetectWin11Release() {
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) return Win11Release::NOT_WIN11;

    auto RtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
        GetProcAddress(hNtdll, "RtlGetVersion"));
    if (!RtlGetVersion) return Win11Release::NOT_WIN11;

    RTL_OSVERSIONINFOW osInfo = {};
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    RtlGetVersion(&osInfo);

    DWORD build = osInfo.dwBuildNumber;

    if (build < 22000) return Win11Release::NOT_WIN11;
    if (build < 22621) return Win11Release::WIN11_21H2;
    if (build < 22631) return Win11Release::WIN11_22H2;
    if (build < 26000) return Win11Release::WIN11_23H2;
    if (build < 30000) return Win11Release::WIN11_24H2;
    return Win11Release::WIN11_FUTURE;
}

/// Get build number
inline DWORD GetWindowsBuildNumber() {
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) return 0;

    auto RtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
        GetProcAddress(hNtdll, "RtlGetVersion"));
    if (!RtlGetVersion) return 0;

    RTL_OSVERSIONINFOW osInfo = {};
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    RtlGetVersion(&osInfo);

    return osInfo.dwBuildNumber;
}

/// Get Win11 release name as string
inline const char* Win11ReleaseName(Win11Release rel) {
    switch (rel) {
    case Win11Release::NOT_WIN11: return "Not Windows 11";
    case Win11Release::WIN11_21H2: return "Windows 11 21H2";
    case Win11Release::WIN11_22H2: return "Windows 11 22H2";
    case Win11Release::WIN11_23H2: return "Windows 11 23H2";
    case Win11Release::WIN11_24H2: return "Windows 11 24H2";
    case Win11Release::WIN11_FUTURE: return "Windows 11 (Future)";
    default: return "Unknown";
    }
}

// ============================================================================
// DPI Helpers
// ============================================================================

/// Get DPI for a specific monitor
inline UINT GetMonitorDPI(HMONITOR hMonitor) {
    UINT dpiX = 96, dpiY = 96;
    GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    return dpiX;
}

/// Get DPI scale percentage for a monitor (100%, 125%, 150%, 200%, etc.)
inline int GetMonitorScalePercent(HMONITOR hMonitor) {
    return (GetMonitorDPI(hMonitor) * 100) / 96;
}

/// Scale a pixel value for the current DPI context
inline int ScaleForDPI(int value, UINT dpi) {
    return MulDiv(value, dpi, 96);
}

/// Scale a pixel value for a specific window's DPI
inline int ScaleForWindow(int value, HWND hWnd) {
    UINT dpi = GetDpiForWindow(hWnd);
    return MulDiv(value, dpi, 96);
}

/// Get all monitor DPI scales
inline std::vector<int> GetAllMonitorDPIScales() {
    std::vector<int> scales;

    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM lParam) -> BOOL {
        auto* pScales = reinterpret_cast<std::vector<int>*>(lParam);
        pScales->push_back(GetMonitorScalePercent(hMon));
        return TRUE;
        }, reinterpret_cast<LPARAM>(&scales));

    return scales;
}

/// Check if system has mixed DPI monitors
inline bool HasMixedDPI() {
    auto scales = GetAllMonitorDPIScales();
    if (scales.size() <= 1) return false;

    int first = scales[0];
    for (int s : scales) {
        if (s != first) return true;
    }
    return false;
}

// ============================================================================
// Dark Mode Queries
// ============================================================================

/// Check if system-wide dark mode is enabled
inline bool IsSystemDarkMode() {
    DWORD value = 1;
    DWORD size = sizeof(DWORD);
    HKEY key;

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, nullptr,
            reinterpret_cast<BYTE*>(&value), &size);
        RegCloseKey(key);
    }

    return (value == 0);
}

// ============================================================================
// GPU Enumeration
// ============================================================================

/// Enumerate all GPU adapters with capability info
inline std::vector<GPUInfo> EnumerateGPUs() {
    std::vector<GPUInfo> gpus;

    IDXGIFactory1* pFactory = nullptr;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory)))
        return gpus;

    IDXGIAdapter1* pAdapter = nullptr;
    for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        GPUInfo info;
        info.description = desc.Description;
        info.vendorId = desc.VendorId;
        info.dedicatedVideoMemoryMB = desc.DedicatedVideoMemory / (1024 * 1024);
        info.isDiscrete = (desc.DedicatedVideoMemory >= 512ULL * 1024 * 1024);
        info.featureLevel = D3D_FEATURE_LEVEL_9_1;
        info.supportsD3D11 = false;

        // Test D3D11 device creation
        ID3D11Device* pDevice = nullptr;
        D3D_FEATURE_LEVEL fl;
        if (SUCCEEDED(D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_UNKNOWN,
            nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &pDevice, &fl, nullptr))) {
            info.supportsD3D11 = true;
            info.featureLevel = fl;
            pDevice->Release();
        }

        gpus.push_back(info);
        pAdapter->Release();
    }

    pFactory->Release();
    return gpus;
}

// ============================================================================
// Full Compatibility Report
// ============================================================================

/// Generate a complete compatibility report for the current system
inline CompatibilityReport GenerateCompatibilityReport() {
    CompatibilityReport report;

    // OS detection
    report.win11Release = DetectWin11Release();
    report.buildNumber = GetWindowsBuildNumber();

    // Architecture
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    report.isARM64 = (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64);
    report.architecture = report.isARM64 ? L"ARM64" : L"x64";

    // DPI
    report.dpiScales = GetAllMonitorDPIScales();
    report.monitorCount = static_cast<int>(report.dpiScales.size());
    report.isPerMonitorV2 = true; // We declare it in manifest

    // Dark mode
    report.isDarkMode = IsSystemDarkMode();

    // GPU
    report.gpus = EnumerateGPUs();
    for (const auto& gpu : report.gpus) {
        if (gpu.isDiscrete) report.hasDiscreteGPU = true;
        else report.hasIntegratedGPU = true;
    }

    return report;
}

/// Log compatibility report to OutputDebugString
inline void LogCompatibilityReport(const CompatibilityReport& report) {
    char buf[512];

    snprintf(buf, sizeof(buf),
        "[ExplorerLens] Win11 Compat: %s (build %lu), Arch=%ls, DarkMode=%s, HDR=%s, Monitors=%d, GPUs=%zu\n",
        Win11ReleaseName(report.win11Release),
        report.buildNumber,
        report.architecture.c_str(),
        report.isDarkMode ? "yes" : "no",
        report.isHDRSupported ? "yes" : "no",
        report.monitorCount,
        report.gpus.size());

    OutputDebugStringA(buf);
}

} // namespace ExplorerLens


// ============================================================================
// Section 2 — Win11IntegrationManager
// Windows 11 24H2 Integration: modern context menus, tabbed Explorer
// thumbnail refresh, dark mode awareness.
// ============================================================================

namespace ExplorerLens {
namespace Engine {

/// Windows 11 integration feature set (manager-level, distinct from
/// Win11Integration::Win11Feature)
enum class Win11MgrFeature : uint8_t {
    ModernContextMenu, // IExplorerCommand for Win11 context menus
    TabbedExplorer, // Multi-tab thumbnail refresh handling
    DarkModeAware, // Auto-detect system dark/light mode
    RoundedCorners, // Rounded thumbnail corners (Win11 style)
    MicaEffect, // Mica/Acrylic backdrop support
    SnapLayouts, // Snap layout aware thumbnail sizing
    FileRecommendations, // Windows 11 Start recommendations
    WidgetIntegration, // Widgets board integration
    COUNT
};

/// Windows version detection
enum class WindowsVersion : uint8_t {
    Windows10_1809, // RS5 — minimum supported
    Windows10_1903,
    Windows10_2004,
    Windows10_21H2, // Last Win10 feature update
    Windows11_21H2, // Initial Win11
    Windows11_22H2,
    Windows11_23H2,
    Windows11_24H2, // Latest — modern context menus, copilot
    Unknown
};

/// Windows 11 integration config
struct Win11IntegrationConfig {
    WindowsVersion detectedVersion = WindowsVersion::Unknown;
    bool enableModernMenu = true;
    bool enableRoundedCorners = true;
    bool enableDarkMode = true;
    uint32_t cornerRadius = 8; // Win11 default
};

/// Windows 11 integration manager
class Win11IntegrationManager {
public:
    /// Feature name
    static const wchar_t* FeatureName(Win11MgrFeature f) {
        switch (f) {
        case Win11MgrFeature::ModernContextMenu:
            return L"Modern Context Menu";
        case Win11MgrFeature::TabbedExplorer:
            return L"Tabbed Explorer";
        case Win11MgrFeature::DarkModeAware:
            return L"Dark Mode Aware";
        case Win11MgrFeature::RoundedCorners:
            return L"Rounded Corners";
        case Win11MgrFeature::MicaEffect:
            return L"Mica Effect";
        case Win11MgrFeature::SnapLayouts:
            return L"Snap Layouts";
        case Win11MgrFeature::FileRecommendations:
            return L"File Recommendations";
        case Win11MgrFeature::WidgetIntegration:
            return L"Widget Integration";
        default:
            return L"Unknown";
        }
    }

    /// Version display string
    static const wchar_t* VersionName(WindowsVersion v) {
        switch (v) {
        case WindowsVersion::Windows10_1809:
            return L"Windows 10 1809";
        case WindowsVersion::Windows10_1903:
            return L"Windows 10 1903";
        case WindowsVersion::Windows10_2004:
            return L"Windows 10 2004";
        case WindowsVersion::Windows10_21H2:
            return L"Windows 10 21H2";
        case WindowsVersion::Windows11_21H2:
            return L"Windows 11 21H2";
        case WindowsVersion::Windows11_22H2:
            return L"Windows 11 22H2";
        case WindowsVersion::Windows11_23H2:
            return L"Windows 11 23H2";
        case WindowsVersion::Windows11_24H2:
            return L"Windows 11 24H2";
        default:
            return L"Unknown";
        }
    }

    /// Check if Win11 feature is available for detected version
    static bool IsFeatureAvailable(Win11MgrFeature f, WindowsVersion v) {
        if (v < WindowsVersion::Windows11_21H2)
            return false;
        if (f == Win11MgrFeature::TabbedExplorer &&
            v < WindowsVersion::Windows11_22H2)
            return false;
        if (f == Win11MgrFeature::WidgetIntegration &&
            v < WindowsVersion::Windows11_23H2)
            return false;
        return true;
    }

    static constexpr size_t FeatureCount() {
        return static_cast<size_t>(Win11MgrFeature::COUNT);
    }

    /// Version count
    static constexpr size_t VersionCount() { return 8; }
};

} // namespace Engine
} // namespace ExplorerLens


// ============================================================================
// Section 3 — Windows11CompatMatrix
// Comprehensive compatibility layer for Windows 11 22H2/23H2/24H2, covering
// OS version detection, dark mode integration, HDR display awareness,
// multi-GPU enumeration, ARM64 feature detection, and DPI scaling.
// ============================================================================

namespace ExplorerLens {
namespace Engine {
namespace Compat {

//==============================================================================
// Windows 11 Build Thresholds
//==============================================================================
struct Win11Build {
    static constexpr DWORD Win11_21H2 = 22000;
    static constexpr DWORD Win11_22H2 = 22621;
    static constexpr DWORD Win11_23H2 = 22631;
    static constexpr DWORD Win11_24H2 = 26100;
    static constexpr DWORD Win10_Last = 19045;
};

//==============================================================================
// Windows Version Info
//==============================================================================
struct WindowsVersionInfo {
    DWORD majorVersion = 0;
    DWORD minorVersion = 0;
    DWORD buildNumber = 0;
    std::wstring displayVersion; // e.g., "23H2", "24H2"
    std::wstring editionId; // e.g., "Professional", "Enterprise"
    bool isWindows11 = false;
    bool isServer = false;
    bool isARM64 = false;

    std::string BuildLabel() const {
        if (!isWindows11) return "Windows 10 (or earlier)";
        if (buildNumber >= Win11Build::Win11_24H2) return "Windows 11 24H2";
        if (buildNumber >= Win11Build::Win11_23H2) return "Windows 11 23H2";
        if (buildNumber >= Win11Build::Win11_22H2) return "Windows 11 22H2";
        return "Windows 11 21H2";
    }
};

//==============================================================================
// Dark Mode State
//==============================================================================
enum class DarkModeState : uint32_t {
    Unavailable = 0, // OS doesn't support dark mode
    Light = 1, // User is in light mode
    Dark = 2, // User is in dark mode
    HighContrast = 3 // High contrast override theme
};

inline const char* DarkModeStateName(DarkModeState s) {
    static const char* names[] = {
    "Unavailable", "Light", "Dark", "HighContrast"
    };
    return names[static_cast<uint32_t>(s)];
}

//==============================================================================
// HDR Display Information
//==============================================================================
struct HDRDisplayInfo {
    bool hdrSupported = false;
    bool hdrEnabled = false;
    bool wideColorGamut = false;
    float maxLuminance = 0.0f;
    float minLuminance = 0.0f;
    float maxFullFrameLum = 0.0f;
    DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    std::wstring monitorName;
    uint32_t resolutionX = 0;
    uint32_t resolutionY = 0;
};

//==============================================================================
// GPU Adapter Information
//==============================================================================
struct GPUAdapterInfo {
    std::wstring description;
    uint32_t vendorId = 0;
    uint32_t deviceId = 0;
    SIZE_T dedicatedVRAM = 0;
    SIZE_T sharedVRAM = 0;
    bool isHardware = false;
    bool isSoftware = false;
    bool supportsD3D11 = false;
    bool supportsD3D12 = false;
    D3D_FEATURE_LEVEL maxFeatureLevel = D3D_FEATURE_LEVEL_9_1;

    std::string VRAMLabel() const {
        double gb = static_cast<double>(dedicatedVRAM) / (1024.0 * 1024.0 * 1024.0);
        char buf[64]{};
        snprintf(buf, sizeof(buf), "%.1f GB", gb);
        return buf;
    }

    std::string VendorName() const {
        switch (vendorId) {
        case 0x10DE: return "NVIDIA";
        case 0x1002: return "AMD";
        case 0x8086: return "Intel";
        case 0x1414: return "Microsoft (WARP)";
        default: return "Unknown";
        }
    }
};

//==============================================================================
// DPI Scaling Information
//==============================================================================
struct DPIScalingInfo {
    uint32_t systemDPI = 96;
    uint32_t effectiveDPI = 96;
    float scaleFactor = 1.0f;
    bool perMonitorV2 = false; // Per-Monitor DPI v2 support
    uint32_t monitorCount = 0;

    bool IsHighDPI() const { return scaleFactor > 1.0f; }
    uint32_t ScalePercent() const { return static_cast<uint32_t>(scaleFactor * 100.0f); }
};

//==============================================================================
// ARM64 Feature Detection
//==============================================================================
struct ARM64Features {
    bool isARM64 = false;
    bool hasNEON = false; // SIMD (always true on ARM64)
    bool hasCRC32 = false;
    bool hasAtomics = false; // LSE atomics
    bool hasSVE = false; // Scalable Vector Extensions
    bool hasSVE2 = false;
    bool isEmulated = false; // x64 on ARM64 via emulation

    bool CanRunNative() const { return isARM64 && !isEmulated; }
};

//==============================================================================
// Compatibility Test Result
//==============================================================================
struct CompatTestResult {
    std::string testName;
    bool passed = false;
    std::string details;
    std::string recommendation;
};

//==============================================================================
// Full Compatibility Matrix
//==============================================================================
struct CompatibilityMatrix {
    WindowsVersionInfo osInfo;
    DarkModeState darkMode = DarkModeState::Unavailable;
    HDRDisplayInfo hdrInfo;
    DPIScalingInfo dpiInfo;
    ARM64Features arm64Info;
    std::vector<GPUAdapterInfo> gpuAdapters;
    std::vector<CompatTestResult> testResults;

    bool AllTestsPassed() const {
        for (auto& t : testResults)
            if (!t.passed) return false;
        return !testResults.empty();
    }

    size_t PassCount() const {
        size_t c = 0;
        for (auto& t : testResults)
            if (t.passed) ++c;
        return c;
    }

    size_t FailCount() const {
        return testResults.size() - PassCount();
    }

    size_t GPUCount() const { return gpuAdapters.size(); }
    bool HasDiscreteGPU() const {
        for (auto& g : gpuAdapters)
            if (g.isHardware && g.vendorId != 0x8086) return true;
        return false;
    }
};

//==============================================================================
// Windows Version Detector
//==============================================================================
class WindowsVersionDetector {
public:
    static WindowsVersionInfo Detect() {
        WindowsVersionInfo info{};

        // Use RtlGetVersion for accurate version (avoids manifest issues)
        using RtlGetVersionFn = NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW);
        auto ntdll = GetModuleHandleW(L"ntdll.dll");
        if (ntdll) {
            auto RtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
                GetProcAddress(ntdll, "RtlGetVersion"));
            if (RtlGetVersion) {
                RTL_OSVERSIONINFOW osvi{};
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                if (RtlGetVersion(&osvi) == 0) {
                    info.majorVersion = osvi.dwMajorVersion;
                    info.minorVersion = osvi.dwMinorVersion;
                    info.buildNumber = osvi.dwBuildNumber;
                }
            }
        }

        info.isWindows11 = (info.majorVersion >= 10 &&
            info.buildNumber >= Win11Build::Win11_21H2);

        // Read display version from registry
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            wchar_t buf[64]{};
            DWORD bufSize = sizeof(buf);
            if (RegQueryValueExW(hKey, L"DisplayVersion", nullptr, nullptr,
                reinterpret_cast<LPBYTE>(buf), &bufSize) == ERROR_SUCCESS) {
                info.displayVersion = buf;
            }
            bufSize = sizeof(buf);
            if (RegQueryValueExW(hKey, L"EditionID", nullptr, nullptr,
                reinterpret_cast<LPBYTE>(buf), &bufSize) == ERROR_SUCCESS) {
                info.editionId = buf;
            }
            RegCloseKey(hKey);
        }

        // ARM64 detection
        SYSTEM_INFO si{};
        GetNativeSystemInfo(&si);
        info.isARM64 = (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64);

        return info;
    }
};

//==============================================================================
// Dark Mode Detector
//==============================================================================
class DarkModeDetector {
public:
    static DarkModeState Detect() {
        // Check high contrast first
        HIGHCONTRASTW hc{};
        hc.cbSize = sizeof(hc);
        if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0)) {
            if (hc.dwFlags & HCF_HIGHCONTRASTON) {
                return DarkModeState::HighContrast;
            }
        }

        // Read AppsUseLightTheme from registry
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD useLightTheme = 1;
            DWORD dataSize = sizeof(useLightTheme);
            if (RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr,
                reinterpret_cast<LPBYTE>(&useLightTheme), &dataSize) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return useLightTheme == 0 ? DarkModeState::Dark : DarkModeState::Light;
            }
            RegCloseKey(hKey);
        }

        return DarkModeState::Unavailable;
    }
};

//==============================================================================
// HDR Display Detector
//==============================================================================
class HDRDisplayDetector {
public:
    static HDRDisplayInfo Detect() {
        HDRDisplayInfo info{};

        IDXGIFactory6* factory = nullptr;
        HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory6),
            reinterpret_cast<void**>(&factory));
        if (FAILED(hr) || !factory) return info;

        IDXGIAdapter1* adapter = nullptr;
        if (SUCCEEDED(factory->EnumAdapters1(0, &adapter)) && adapter) {
            IDXGIOutput* output = nullptr;
            if (SUCCEEDED(adapter->EnumOutputs(0, &output)) && output) {
                IDXGIOutput6* output6 = nullptr;
                if (SUCCEEDED(output->QueryInterface(__uuidof(IDXGIOutput6),
                    reinterpret_cast<void**>(&output6))) && output6) {
                    DXGI_OUTPUT_DESC1 desc{};
                    if (SUCCEEDED(output6->GetDesc1(&desc))) {
                        info.hdrSupported = (desc.ColorSpace ==
                            DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
                        info.maxLuminance = desc.MaxLuminance;
                        info.minLuminance = desc.MinLuminance;
                        info.maxFullFrameLum = desc.MaxFullFrameLuminance;
                        info.colorSpace = desc.ColorSpace;
                        info.monitorName = desc.DeviceName;
                        info.resolutionX = desc.DesktopCoordinates.right -
                            desc.DesktopCoordinates.left;
                        info.resolutionY = desc.DesktopCoordinates.bottom -
                            desc.DesktopCoordinates.top;

                        // Wide color gamut if red primary > sRGB
                        info.wideColorGamut = (desc.RedPrimary[0] > 0.64f);
                    }
                    output6->Release();
                }
                output->Release();
            }
            adapter->Release();
        }
        factory->Release();

        // Check if HDR is enabled in Windows settings
        info.hdrEnabled = IsHDREnabled();

        return info;
    }

private:
    static bool IsHDREnabled() {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\AdvancedColor",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD enabled = 0;
            DWORD dataSize = sizeof(enabled);
            RegQueryValueExW(hKey, L"EnableAdvancedColor", nullptr, nullptr,
                reinterpret_cast<LPBYTE>(&enabled), &dataSize);
            RegCloseKey(hKey);
            return enabled != 0;
        }
        return false;
    }
};

//==============================================================================
// GPU Enumerator
//==============================================================================
class GPUEnumerator {
public:
    static std::vector<GPUAdapterInfo> EnumerateAll() {
        std::vector<GPUAdapterInfo> adapters;

        IDXGIFactory1* factory = nullptr;
        if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1),
            reinterpret_cast<void**>(&factory))) || !factory) {
            return adapters;
        }

        IDXGIAdapter1* adapter = nullptr;
        for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 desc{};
            if (SUCCEEDED(adapter->GetDesc1(&desc))) {
                GPUAdapterInfo info{};
                info.description = desc.Description;
                info.vendorId = desc.VendorId;
                info.deviceId = desc.DeviceId;
                info.dedicatedVRAM = desc.DedicatedVideoMemory;
                info.sharedVRAM = desc.SharedSystemMemory;
                info.isHardware = !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE);
                info.isSoftware = !info.isHardware;

                // Test D3D11 support
                D3D_FEATURE_LEVEL featureLevels[] = {
                D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0,
                D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
                D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_1
                };
                D3D_FEATURE_LEVEL achievedLevel = D3D_FEATURE_LEVEL_9_1;
                ID3D11Device* testDevice = nullptr;
                HRESULT hr = D3D11CreateDevice(
                    adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                    0, featureLevels, _countof(featureLevels),
                    D3D11_SDK_VERSION, &testDevice, &achievedLevel, nullptr);
                if (SUCCEEDED(hr)) {
                    info.supportsD3D11 = true;
                    info.maxFeatureLevel = achievedLevel;
                    if (achievedLevel >= D3D_FEATURE_LEVEL_12_0)
                        info.supportsD3D12 = true;
                    testDevice->Release();
                }

                adapters.push_back(std::move(info));
            }
            adapter->Release();
        }
        factory->Release();

        return adapters;
    }

    static std::optional<GPUAdapterInfo> GetPrimaryAdapter() {
        auto all = EnumerateAll();
        for (auto& a : all) {
            if (a.isHardware) return a;
        }
        return all.empty() ? std::nullopt : std::optional(all[0]);
    }
};

//==============================================================================
// DPI Scaling Detector
//==============================================================================
class DPIScalingDetector {
public:
    static DPIScalingInfo Detect() {
        DPIScalingInfo info{};

        // System DPI
        HDC hdc = GetDC(nullptr);
        if (hdc) {
            info.systemDPI = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(nullptr, hdc);
        }

        info.effectiveDPI = info.systemDPI;
        info.scaleFactor = static_cast<float>(info.systemDPI) / 96.0f;

        // Per-monitor DPI v2 support (Win10 1703+)
        info.perMonitorV2 = IsPerMonitorV2Available();

        // Count monitors
        info.monitorCount = GetSystemMetrics(SM_CMONITORS);

        return info;
    }

private:
    static bool IsPerMonitorV2Available() {
        // Per-Monitor v2 available since Win10 1703 (build 15063)
        auto ver = WindowsVersionDetector::Detect();
        return ver.buildNumber >= 15063;
    }
};

//==============================================================================
// ARM64 Feature Detector
//==============================================================================
class ARM64FeatureDetector {
public:
    static ARM64Features Detect() {
        ARM64Features features{};

        SYSTEM_INFO si{};
        GetNativeSystemInfo(&si);
        features.isARM64 = (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64);

        if (!features.isARM64) return features;

        // On ARM64, NEON is always available
        features.hasNEON = true;

        // Check if we're running as emulated x64
        BOOL isWow64 = FALSE;
        using IsWow64Process2Fn = BOOL(WINAPI*)(HANDLE, USHORT*, USHORT*);
        auto kernel32 = GetModuleHandleW(L"kernel32.dll");
        if (kernel32) {
            auto pIsWow64Process2 = reinterpret_cast<IsWow64Process2Fn>(
                GetProcAddress(kernel32, "IsWow64Process2"));
            if (pIsWow64Process2) {
                USHORT processMachine = 0, nativeMachine = 0;
                if (pIsWow64Process2(GetCurrentProcess(), &processMachine,
                    &nativeMachine)) {
                    features.isEmulated = (processMachine != 0); // 0 = native
                }
            }
        }

        // Feature registers (ARM64 specific)
#ifdef _M_ARM64
        features.hasCRC32 = IsProcessorFeaturePresent(PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE);
        features.hasAtomics = IsProcessorFeaturePresent(PF_ARM_V81_ATOMIC_INSTRUCTIONS_AVAILABLE);
#endif

        return features;
    }
};

//==============================================================================
// Compatibility Matrix Builder
//==============================================================================
class CompatibilityMatrixBuilder {
public:
    static CompatibilityMatrix Build() {
        CompatibilityMatrix matrix{};

        matrix.osInfo = WindowsVersionDetector::Detect();
        matrix.darkMode = DarkModeDetector::Detect();
        matrix.hdrInfo = HDRDisplayDetector::Detect();
        matrix.dpiInfo = DPIScalingDetector::Detect();
        matrix.arm64Info = ARM64FeatureDetector::Detect();
        matrix.gpuAdapters = GPUEnumerator::EnumerateAll();

        // Run compatibility tests
        RunCompatTests(matrix);

        return matrix;
    }

private:
    static void RunCompatTests(CompatibilityMatrix& matrix) {
        // Test 1: OS Version
        {
            CompatTestResult test;
            test.testName = "Windows 11 Version Check";
            test.passed = matrix.osInfo.isWindows11;
            test.details = matrix.osInfo.BuildLabel();
            if (!test.passed) test.recommendation = "Windows 11 22H2+ recommended";
            matrix.testResults.push_back(std::move(test));
        }

        // Test 2: Dark Mode Support
        {
            CompatTestResult test;
            test.testName = "Dark Mode Detection";
            test.passed = (matrix.darkMode != DarkModeState::Unavailable);
            test.details = DarkModeStateName(matrix.darkMode);
            matrix.testResults.push_back(std::move(test));
        }

        // Test 3: GPU Available
        {
            CompatTestResult test;
            test.testName = "GPU Hardware Adapter";
            test.passed = !matrix.gpuAdapters.empty();
            if (test.passed) {
                test.details = std::string("Found ") +
                    std::to_string(matrix.gpuAdapters.size()) + " adapter(s)";
                if (!matrix.gpuAdapters.empty()) {
                    auto& primary = matrix.gpuAdapters[0];
                    test.details += ": " + primary.VendorName() +
                        " (" + primary.VRAMLabel() + ")";
                }
            }
            else {
                test.recommendation = "Hardware GPU recommended for thumbnail acceleration";
            }
            matrix.testResults.push_back(std::move(test));
        }

        // Test 4: D3D11 Support
        {
            CompatTestResult test;
            test.testName = "Direct3D 11 Support";
            test.passed = false;
            for (auto& g : matrix.gpuAdapters) {
                if (g.supportsD3D11) { test.passed = true; break; }
            }
            test.details = test.passed ? "D3D11 available" : "No D3D11 support";
            matrix.testResults.push_back(std::move(test));
        }

        // Test 5: DPI Awareness
        {
            CompatTestResult test;
            test.testName = "DPI Scaling";
            test.passed = true; // Always pass, but note status
            auto& dpi = matrix.dpiInfo;
            test.details = std::to_string(dpi.ScalePercent()) + "% scale, " +
                std::to_string(dpi.monitorCount) + " monitor(s), " +
                (dpi.perMonitorV2 ? "Per-Monitor v2" : "System DPI");
            if (dpi.IsHighDPI() && !dpi.perMonitorV2) {
                test.recommendation = "Enable Per-Monitor DPI v2 in manifest";
            }
            matrix.testResults.push_back(std::move(test));
        }

        // Test 6: HDR Status
        {
            CompatTestResult test;
            test.testName = "HDR Display";
            test.passed = true; // Info only
            if (matrix.hdrInfo.hdrSupported) {
                test.details = std::string("HDR ") +
                    (matrix.hdrInfo.hdrEnabled ? "enabled" : "available but disabled");
                test.details += ", max " +
                    std::to_string(static_cast<int>(matrix.hdrInfo.maxLuminance)) + " nits";
            }
            else {
                test.details = "SDR display";
            }
            matrix.testResults.push_back(std::move(test));
        }

        // Test 7: ARM64 Status
        {
            CompatTestResult test;
            test.testName = "ARM64 Platform";
            test.passed = true; // Info only
            if (matrix.arm64Info.isARM64) {
                test.details = matrix.arm64Info.isEmulated
                    ? "ARM64 (running x64 emulated)"
                    : "ARM64 native";
            }
            else {
                test.details = "x64 native";
            }
            matrix.testResults.push_back(std::move(test));
        }
    }
};

} // namespace Compat
} // namespace Engine
} // namespace ExplorerLens


// ============================================================================
// Section 4 — Windows12Compatibility
// Runtime detection of Windows 12 shell APIs, adaptive rendering paths for
// new window compositor features, Fluent v4 style adaptation, and
// graceful fallback to Windows 11 behaviour.
// ============================================================================

namespace ExplorerLens {
namespace Engine {

enum class Win12Feature : uint8_t {
    DynamicIsland = 0, // Windows 12 Dynamic Island shell integration
    FluentV4 = 1, // Fluent Design System V4
    AIProcessingUnit = 2, // NPU tile APIs
    SmartDock = 3, // Snap / Smart Dock layouts V2
    UnifiedSearch = 4, // Unified search provider protocol
    COUNT
};

enum class Win12CompatMode : uint8_t { Native = 0, Adaptive, Fallback11, Fallback10, COUNT };
enum class Win12APIFamily : uint8_t { Shell = 0, Compositor, InputMethod, SystemTray, COUNT };

struct Win12FeatureAvailability {
    Win12Feature feature = Win12Feature::FluentV4;
    bool available = false;
    Win12CompatMode mode = Win12CompatMode::Fallback11;
    std::wstring minBuildNumber;
};

class Windows12Compatibility {
public:
    static const wchar_t* FeatureName(Win12Feature f) {
        switch (f) {
        case Win12Feature::DynamicIsland: return L"Dynamic Island Shell";
        case Win12Feature::FluentV4: return L"Fluent Design V4";
        case Win12Feature::AIProcessingUnit: return L"NPU AI Processing Unit";
        case Win12Feature::SmartDock: return L"Smart Dock Layouts V2";
        case Win12Feature::UnifiedSearch: return L"Unified Search Provider";
        default: return L"Unknown";
        }
    }
    static const wchar_t* CompatModeName(Win12CompatMode m) {
        switch (m) {
        case Win12CompatMode::Native: return L"Native";
        case Win12CompatMode::Adaptive: return L"Adaptive";
        case Win12CompatMode::Fallback11: return L"Fallback (Win11)";
        case Win12CompatMode::Fallback10: return L"Fallback (Win10)";
        default: return L"Unknown";
        }
    }
    static const wchar_t* APIFamilyName(Win12APIFamily a) {
        switch (a) {
        case Win12APIFamily::Shell: return L"Shell";
        case Win12APIFamily::Compositor: return L"Compositor";
        case Win12APIFamily::InputMethod: return L"Input Method";
        case Win12APIFamily::SystemTray: return L"System Tray";
        default: return L"Unknown";
        }
    }
    static constexpr size_t FeatureCount() { return static_cast<size_t>(Win12Feature::COUNT); }
    static constexpr size_t CompatModeCount() { return static_cast<size_t>(Win12CompatMode::COUNT); }
    static constexpr size_t APIFamilyCount() { return static_cast<size_t>(Win12APIFamily::COUNT); }
};

}
} // namespace ExplorerLens::Engine


// ============================================================================
// Section 5 — CompatibilityMatrix
// Win11 Compatibility Matrix Execution: full OS/GPU/DPI matrix validation
// framework for Windows 10/11 builds. Defines test scenarios, pass criteria,
// and compatibility reporting.
// ============================================================================

namespace ExplorerLens::Utils {

// --- Windows build identifiers ---
enum class WindowsBuild : uint16_t {
    Win10_21H2 = 19044,
    Win10_22H2 = 19045,
    Win11_21H2 = 22000,
    Win11_22H2 = 22621,
    Win11_23H2 = 22631,
    Win11_24H2 = 26100,
    Unknown = 0
};

inline const char* WindowsBuildName(WindowsBuild b) {
    switch (b) {
    case WindowsBuild::Win10_21H2: return "Windows 10 21H2 (19044)";
    case WindowsBuild::Win10_22H2: return "Windows 10 22H2 (19045)";
    case WindowsBuild::Win11_21H2: return "Windows 11 21H2 (22000)";
    case WindowsBuild::Win11_22H2: return "Windows 11 22H2 (22621)";
    case WindowsBuild::Win11_23H2: return "Windows 11 23H2 (22631)";
    case WindowsBuild::Win11_24H2: return "Windows 11 24H2 (26100)";
    default: return "Unknown";
    }
}

inline bool IsWindows11(WindowsBuild b) {
    return static_cast<uint16_t>(b) >= 22000;
}

// --- GPU vendor ---
enum class GPUVendor : uint8_t {
    NVIDIA = 0,
    AMD = 1,
    Intel = 2,
    Software = 3, // WARP / software renderer
    Unknown = 255
};

inline const char* GPUVendorName(GPUVendor v) {
    switch (v) {
    case GPUVendor::NVIDIA: return "NVIDIA";
    case GPUVendor::AMD: return "AMD";
    case GPUVendor::Intel: return "Intel";
    case GPUVendor::Software: return "Software (WARP)";
    default: return "Unknown";
    }
}

// --- DPI scaling factors ---
enum class DPIScale : uint8_t {
    Scale_100 = 100, // 96 DPI
    Scale_125 = 125, // 120 DPI
    Scale_150 = 150, // 144 DPI
    Scale_175 = 175, // 168 DPI
    Scale_200 = 200, // 192 DPI
    Scale_250 = 250, // 240 DPI
    Scale_300 = 0 // 288 DPI (overflow in uint8_t, use 0 as sentinel)
};

inline uint32_t DPIFromScale(DPIScale s) {
    if (s == DPIScale::Scale_300) return 288;
    return static_cast<uint32_t>(s) * 96 / 100;
}

// --- Compatibility test scenario ---
struct CompatTestScenario {
    std::string name;
    WindowsBuild osBuild = WindowsBuild::Unknown;
    GPUVendor gpu = GPUVendor::Unknown;
    DPIScale dpi = DPIScale::Scale_100;
    bool darkMode = false;
    bool multiMonitor = false;

    std::string Description() const {
        return name + " [" + WindowsBuildName(osBuild) + ", " +
            GPUVendorName(gpu) + ", DPI=" +
            std::to_string(DPIFromScale(dpi)) + "]";
    }
};

// --- Test result ---
enum class CompatResult : uint8_t {
    Pass = 0,
    Fail = 1,
    Warning = 2, // Works but with visual artifacts
    Skipped = 3, // Environment not available
    NotTested = 4
};

inline const char* CompatResultName(CompatResult r) {
    switch (r) {
    case CompatResult::Pass: return "PASS";
    case CompatResult::Fail: return "FAIL";
    case CompatResult::Warning: return "WARNING";
    case CompatResult::Skipped: return "SKIPPED";
    case CompatResult::NotTested: return "NOT TESTED";
    default: return "Unknown";
    }
}

// --- Test execution record ---
struct CompatTestExecution {
    CompatTestScenario scenario;
    CompatResult result = CompatResult::NotTested;
    std::string notes;
    double durationMs = 0.0;

    // Sub-checks
    bool comRegistrationOk = false;
    bool thumbnailRenderOk = false;
    bool darkModeOk = false;
    bool dpiScalingOk = false;
    bool gpuAccelOk = false;
    bool shellIntegrationOk = false;

    bool IsFullPass() const {
        return result == CompatResult::Pass &&
            comRegistrationOk && thumbnailRenderOk &&
            dpiScalingOk && gpuAccelOk && shellIntegrationOk;
    }
};

// --- Compatibility matrix ---
struct CompatMatrixStats {
    uint32_t totalScenarios = 0;
    uint32_t passed = 0;
    uint32_t failed = 0;
    uint32_t warnings = 0;
    uint32_t skipped = 0;
    uint32_t notTested = 0;

    double PassRate() const {
        uint32_t tested = passed + failed + warnings;
        return tested > 0 ? static_cast<double>(passed) / tested : 0.0;
    }

    bool MeetsMinimum(double threshold = 0.95) const {
        return PassRate() >= threshold;
    }
};

class CompatibilityMatrixExec {
public:
    CompatibilityMatrixExec() { BuildDefaultMatrix(); }

    void AddScenario(const CompatTestScenario& scenario) {
        CompatTestExecution exec;
        exec.scenario = scenario;
        m_executions.push_back(exec);
    }

    void RecordResult(size_t index, CompatResult result) {
        if (index < m_executions.size()) {
            m_executions[index].result = result;
        }
    }

    void RecordExecution(const CompatTestExecution& exec) {
        m_executions.push_back(exec);
    }

    CompatMatrixStats GetStats() const {
        CompatMatrixStats stats;
        stats.totalScenarios = static_cast<uint32_t>(m_executions.size());
        for (auto& e : m_executions) {
            switch (e.result) {
            case CompatResult::Pass: stats.passed++; break;
            case CompatResult::Fail: stats.failed++; break;
            case CompatResult::Warning: stats.warnings++; break;
            case CompatResult::Skipped: stats.skipped++; break;
            case CompatResult::NotTested: stats.notTested++; break;
            }
        }
        return stats;
    }

    size_t ScenarioCount() const { return m_executions.size(); }

    const std::vector<CompatTestExecution>& Executions() const { return m_executions; }

    static CompatibilityMatrixExec Create() { return CompatibilityMatrixExec(); }

private:
    void BuildDefaultMatrix() {
        // Core OS builds x GPU vendors
        static const WindowsBuild builds[] = {
        WindowsBuild::Win10_22H2,
        WindowsBuild::Win11_22H2,
        WindowsBuild::Win11_23H2,
        WindowsBuild::Win11_24H2
        };
        static const GPUVendor gpus[] = {
        GPUVendor::NVIDIA, GPUVendor::AMD, GPUVendor::Intel
        };
        static const DPIScale dpis[] = {
        DPIScale::Scale_100, DPIScale::Scale_150, DPIScale::Scale_200
        };

        int idx = 0;
        for (auto build : builds) {
            for (auto gpu : gpus) {
                for (auto dpi : dpis) {
                    CompatTestScenario s;
                    s.name = "Matrix_" + std::to_string(idx++);
                    s.osBuild = build;
                    s.gpu = gpu;
                    s.dpi = dpi;
                    s.darkMode = (idx % 2 == 0);
                    AddScenario(s);
                }
            }
        }
    }

    std::vector<CompatTestExecution> m_executions;
};

} // namespace ExplorerLens::Utils


// ============================================================================
// Tail — Related Windows platform headers (separate TUs for .cpp files)
// ============================================================================
#include "Win11Integration.h"
// NOTE: WindowsUI.h excluded — DPIScale enum conflicts with this header's DPIScale.
#include "MSIXPackageManager.h"
#include "PortableModeManager.h"
