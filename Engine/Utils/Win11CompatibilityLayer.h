// ============================================================================
// Win11CompatibilityLayer.h
// Windows 11 runtime version detection, DPI helpers, and feature queries
//
// Provides runtime detection of Win11 build variants (22H2/23H2/24H2),
// per-monitor DPI scaling helpers, dark mode state queries, and GPU
// capability enumeration for the shell extension and manager GUI.
//
// Thread Safety: All functions are thread-safe (read-only or use TLS).
// ============================================================================

#pragma once

#include <Windows.h>
#include <ShellScalingApi.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <string>
#include <vector>

#pragma comment(lib, "Shcore.lib")

namespace ExplorerLens {

// ============================================================================
// Windows 11 Build Detection
// ============================================================================

/// Windows 11 release identifier
enum class Win11Release {
    NOT_WIN11,       ///< Windows 10 or earlier
    WIN11_21H2,      ///< Build 22000 (initial release)
    WIN11_22H2,      ///< Build 22621
    WIN11_23H2,      ///< Build 22631
    WIN11_24H2,      ///< Build 26100+
    WIN11_FUTURE     ///< Unknown future build
};

/// GPU adapter information
struct GPUInfo {
    std::wstring description;
    UINT vendorId;
    size_t dedicatedVideoMemoryMB;
    bool isDiscrete;             ///< true = dGPU, false = iGPU
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
    std::vector<int> dpiScales;      ///< Per-monitor DPI scale percentages
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
    case Win11Release::NOT_WIN11:   return "Not Windows 11";
    case Win11Release::WIN11_21H2:  return "Windows 11 21H2";
    case Win11Release::WIN11_22H2:  return "Windows 11 22H2";
    case Win11Release::WIN11_23H2:  return "Windows 11 23H2";
    case Win11Release::WIN11_24H2:  return "Windows 11 24H2";
    case Win11Release::WIN11_FUTURE: return "Windows 11 (Future)";
    default:                        return "Unknown";
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
        0, KEY_READ, &key) == ERROR_SUCCESS)
    {
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
            nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &pDevice, &fl, nullptr)))
        {
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

