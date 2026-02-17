//==============================================================================
// DarkThumbs — Sprint 7: Windows 11 Compatibility Matrix
//
// Comprehensive compatibility layer for Windows 11 22H2/23H2/24H2, covering
// OS version detection, dark mode integration, HDR display awareness,
// multi-GPU enumeration, ARM64 feature detection, and DPI scaling.
//==============================================================================

#pragma once

#include <windows.h>
#include <dxgi1_6.h>
#include <d3d11.h>
#include <shellscalingapi.h>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <optional>
#include <functional>

#pragma comment(lib, "dxgi.lib")

namespace DarkThumbs {
namespace Engine {
namespace Compat {

//==============================================================================
// Windows 11 Build Thresholds
//==============================================================================
struct Win11Build {
    static constexpr DWORD Win11_21H2  = 22000;
    static constexpr DWORD Win11_22H2  = 22621;
    static constexpr DWORD Win11_23H2  = 22631;
    static constexpr DWORD Win11_24H2  = 26100;
    static constexpr DWORD Win10_Last  = 19045;
};

//==============================================================================
// Windows Version Info
//==============================================================================
struct WindowsVersionInfo {
    DWORD majorVersion     = 0;
    DWORD minorVersion     = 0;
    DWORD buildNumber      = 0;
    std::wstring displayVersion;   // e.g., "23H2", "24H2"
    std::wstring editionId;        // e.g., "Professional", "Enterprise"
    bool  isWindows11      = false;
    bool  isServer         = false;
    bool  isARM64          = false;

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
    Unavailable = 0,      // OS doesn't support dark mode
    Light       = 1,      // User is in light mode
    Dark        = 2,      // User is in dark mode
    HighContrast = 3      // High contrast override theme
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
    bool   hdrSupported    = false;
    bool   hdrEnabled      = false;
    bool   wideColorGamut  = false;
    float  maxLuminance    = 0.0f;
    float  minLuminance    = 0.0f;
    float  maxFullFrameLum = 0.0f;
    DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    std::wstring monitorName;
    uint32_t resolutionX   = 0;
    uint32_t resolutionY   = 0;
};

//==============================================================================
// GPU Adapter Information
//==============================================================================
struct GPUAdapterInfo {
    std::wstring description;
    uint32_t     vendorId       = 0;
    uint32_t     deviceId       = 0;
    SIZE_T       dedicatedVRAM  = 0;
    SIZE_T       sharedVRAM     = 0;
    bool         isHardware     = false;
    bool         isSoftware     = false;
    bool         supportsD3D11  = false;
    bool         supportsD3D12  = false;
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
        default:     return "Unknown";
        }
    }
};

//==============================================================================
// DPI Scaling Information
//==============================================================================
struct DPIScalingInfo {
    uint32_t systemDPI     = 96;
    uint32_t effectiveDPI  = 96;
    float    scaleFactor   = 1.0f;
    bool     perMonitorV2  = false;   // Per-Monitor DPI v2 support
    uint32_t monitorCount  = 0;

    bool IsHighDPI() const { return scaleFactor > 1.0f; }
    uint32_t ScalePercent() const { return static_cast<uint32_t>(scaleFactor * 100.0f); }
};

//==============================================================================
// ARM64 Feature Detection
//==============================================================================
struct ARM64Features {
    bool isARM64           = false;
    bool hasNEON           = false;     // SIMD (always true on ARM64)
    bool hasCRC32          = false;
    bool hasAtomics        = false;     // LSE atomics
    bool hasSVE            = false;     // Scalable Vector Extensions
    bool hasSVE2           = false;
    bool isEmulated        = false;     // x64 on ARM64 via emulation

    bool CanRunNative() const { return isARM64 && !isEmulated; }
};

//==============================================================================
// Compatibility Test Result
//==============================================================================
struct CompatTestResult {
    std::string testName;
    bool        passed        = false;
    std::string details;
    std::string recommendation;
};

//==============================================================================
// Full Compatibility Matrix
//==============================================================================
struct CompatibilityMatrix {
    WindowsVersionInfo  osInfo;
    DarkModeState       darkMode     = DarkModeState::Unavailable;
    HDRDisplayInfo      hdrInfo;
    DPIScalingInfo      dpiInfo;
    ARM64Features       arm64Info;
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
            0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
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
            0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
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
                    reinterpret_cast<void**>(&output6))) && output6)
                {
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
            0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
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
                    D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_1
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
            } else {
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
            } else {
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
            } else {
                test.details = "x64 native";
            }
            matrix.testResults.push_back(std::move(test));
        }
    }
};

} // namespace Compat
} // namespace Engine
} // namespace DarkThumbs
