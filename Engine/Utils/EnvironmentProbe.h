// EnvironmentProbe.h — System environment detection for CI and diagnostics
// Copyright (c) 2026 ExplorerLens Project
//
// Detects CI environment, GPU capabilities, SIMD features, Windows version,
// and display DPI. Uses RtlGetVersion (never versionhelpers.h) and runtime
// LoadLibrary for GPU probing to avoid hard link dependencies.
//
#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <intrin.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace ExplorerLens { namespace Engine {

// Safe getenv wrapper using _dupenv_s (avoids C4996)
inline std::string SafeGetEnvVar(const char* varName) {
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, varName) == 0 && buf != nullptr) {
        std::string result(buf);
        free(buf);
        return result;
    }
    return {};
}

// ── CI environment detection ───────────────────────────────────

enum class CIProvider : uint8_t {
    None,
    GitHubActions,
    AzureDevOps,
    Jenkins,
    Other
};

inline const char* CIProviderToString(CIProvider p) {
    switch (p) {
        case CIProvider::GitHubActions: return "GitHub Actions";
        case CIProvider::AzureDevOps:  return "Azure DevOps";
        case CIProvider::Jenkins:      return "Jenkins";
        case CIProvider::Other:        return "Other";
        default:                       return "None";
    }
}

inline CIProvider DetectCIProvider() {
    // GitHub Actions sets GITHUB_ACTIONS=true
    std::string gh = SafeGetEnvVar("GITHUB_ACTIONS");
    if (gh == "true") return CIProvider::GitHubActions;

    // Azure DevOps sets TF_BUILD=True
    std::string az = SafeGetEnvVar("TF_BUILD");
    if (az == "True") return CIProvider::AzureDevOps;

    // Jenkins sets JENKINS_URL
    if (!SafeGetEnvVar("JENKINS_URL").empty()) return CIProvider::Jenkins;

    // Generic CI check
    std::string ci = SafeGetEnvVar("CI");
    if (ci == "true" || ci == "1")
        return CIProvider::Other;

    return CIProvider::None;
}

inline bool IsRunningInCI() {
    return DetectCIProvider() != CIProvider::None;
}

// ── SIMD capability detection (via CPUID intrinsics) ──────────

struct ProbedSIMDCaps {
    bool hasSSE2    = false;
    bool hasSSE41   = false;
    bool hasSSE42   = false;
    bool hasAVX     = false;
    bool hasAVX2    = false;
    bool hasAVX512F = false;
    bool hasFMA     = false;

    std::string ToString() const {
        std::string result;
        if (hasSSE2)    result += "SSE2 ";
        if (hasSSE41)   result += "SSE4.1 ";
        if (hasSSE42)   result += "SSE4.2 ";
        if (hasAVX)     result += "AVX ";
        if (hasAVX2)    result += "AVX2 ";
        if (hasFMA)     result += "FMA ";
        if (hasAVX512F) result += "AVX-512F ";
        if (result.empty()) result = "(none)";
        return result;
    }
};

inline ProbedSIMDCaps ProbeSIMDCapabilities() {
    ProbedSIMDCaps caps;
    int cpuInfo[4] = {};

    // Basic CPUID: function 1 — SSE2, SSE4.1, SSE4.2, AVX, FMA
    __cpuid(cpuInfo, 1);
    caps.hasSSE2  = (cpuInfo[3] & (1 << 26)) != 0;  // EDX bit 26
    caps.hasSSE41 = (cpuInfo[2] & (1 << 19)) != 0;  // ECX bit 19
    caps.hasSSE42 = (cpuInfo[2] & (1 << 20)) != 0;  // ECX bit 20
    caps.hasAVX   = (cpuInfo[2] & (1 << 28)) != 0;  // ECX bit 28
    caps.hasFMA   = (cpuInfo[2] & (1 << 12)) != 0;  // ECX bit 12

    // Extended CPUID: function 7, sub-leaf 0 — AVX2, AVX-512F
    __cpuidex(cpuInfo, 7, 0);
    caps.hasAVX2    = (cpuInfo[1] & (1 << 5)) != 0;   // EBX bit 5
    caps.hasAVX512F = (cpuInfo[1] & (1 << 16)) != 0;  // EBX bit 16

    return caps;
}

// ── Windows version detection (via RtlGetVersion) ─────────────

struct ProbedWindowsVersion {
    uint32_t majorVersion = 0;
    uint32_t minorVersion = 0;
    uint32_t buildNumber  = 0;
    bool detected = false;

    std::string ToString() const {
        if (!detected) return "(unknown)";
        return "Windows " + std::to_string(majorVersion) + "." +
               std::to_string(minorVersion) + " build " +
               std::to_string(buildNumber);
    }

    bool IsWindows10_1809OrLater() const {
        if (!detected) return false;
        return (majorVersion > 10) ||
               (majorVersion == 10 && buildNumber >= 17763);
    }
};

inline ProbedWindowsVersion DetectWindowsVersion() {
    ProbedWindowsVersion info;
    typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) return info;

    auto pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
        GetProcAddress(hNtdll, "RtlGetVersion"));
    if (!pRtlGetVersion) return info;

    RTL_OSVERSIONINFOW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (pRtlGetVersion(&osvi) == 0) {
        info.majorVersion = osvi.dwMajorVersion;
        info.minorVersion = osvi.dwMinorVersion;
        info.buildNumber  = osvi.dwBuildNumber;
        info.detected     = true;
    }

    return info;
}

// ── GPU capability detection (runtime, no hard link) ──────────

enum class GPUFeatureLevel : uint8_t {
    None,
    D3D11_FeatureLevel_9_1,
    D3D11_FeatureLevel_10_0,
    D3D11_FeatureLevel_11_0,
    D3D11_FeatureLevel_11_1,
    D3D11_FeatureLevel_12_0,
    D3D11_FeatureLevel_12_1
};

inline const char* GPUFeatureLevelToString(GPUFeatureLevel level) {
    switch (level) {
        case GPUFeatureLevel::D3D11_FeatureLevel_9_1:  return "9.1";
        case GPUFeatureLevel::D3D11_FeatureLevel_10_0: return "10.0";
        case GPUFeatureLevel::D3D11_FeatureLevel_11_0: return "11.0";
        case GPUFeatureLevel::D3D11_FeatureLevel_11_1: return "11.1";
        case GPUFeatureLevel::D3D11_FeatureLevel_12_0: return "12.0";
        case GPUFeatureLevel::D3D11_FeatureLevel_12_1: return "12.1";
        default:                                        return "None";
    }
}

struct GPUCapabilities {
    bool gpuAvailable = false;
    GPUFeatureLevel featureLevel = GPUFeatureLevel::None;
    std::string adapterDescription;

    std::string ToString() const {
        if (!gpuAvailable) return "No GPU detected";
        return "GPU: " + adapterDescription +
               " (Feature Level " + GPUFeatureLevelToString(featureLevel) + ")";
    }
};

inline GPUCapabilities DetectGPUCapabilities() {
    GPUCapabilities caps;

    // Load d3d11.dll at runtime to avoid hard link dependency
    HMODULE hD3D11 = LoadLibraryW(L"d3d11.dll");
    if (!hD3D11) return caps;

    // D3D11CreateDevice function pointer
    typedef HRESULT(WINAPI* PFN_EL_D3D11CreateDevice)(
        void* pAdapter,
        UINT DriverType,  // D3D_DRIVER_TYPE
        HMODULE Software,
        UINT Flags,
        const UINT* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        void** ppDevice,
        UINT* pFeatureLevel,
        void** ppImmediateContext);

    auto pD3D11CreateDevice = reinterpret_cast<PFN_EL_D3D11CreateDevice>(
        GetProcAddress(hD3D11, "D3D11CreateDevice"));

    if (pD3D11CreateDevice) {
        // Feature levels to try (D3D_FEATURE_LEVEL values)
        UINT featureLevels[] = {
            0xc100,  // D3D_FEATURE_LEVEL_12_1
            0xc000,  // D3D_FEATURE_LEVEL_12_0
            0xb100,  // D3D_FEATURE_LEVEL_11_1
            0xb000,  // D3D_FEATURE_LEVEL_11_0
            0xa000,  // D3D_FEATURE_LEVEL_10_0
            0x9100,  // D3D_FEATURE_LEVEL_9_1
        };

        void* pDevice = nullptr;
        void* pContext = nullptr;
        UINT achievedLevel = 0;

        // D3D_DRIVER_TYPE_HARDWARE = 1, D3D11_SDK_VERSION = 7
        HRESULT hr = pD3D11CreateDevice(
            nullptr,           // default adapter
            1,                 // D3D_DRIVER_TYPE_HARDWARE
            nullptr,           // no software module
            0,                 // flags
            featureLevels,
            _countof(featureLevels),
            7,                 // D3D11_SDK_VERSION
            &pDevice,
            &achievedLevel,
            &pContext);

        if (SUCCEEDED(hr) && pDevice) {
            caps.gpuAvailable = true;

            switch (achievedLevel) {
                case 0xc100: caps.featureLevel = GPUFeatureLevel::D3D11_FeatureLevel_12_1; break;
                case 0xc000: caps.featureLevel = GPUFeatureLevel::D3D11_FeatureLevel_12_0; break;
                case 0xb100: caps.featureLevel = GPUFeatureLevel::D3D11_FeatureLevel_11_1; break;
                case 0xb000: caps.featureLevel = GPUFeatureLevel::D3D11_FeatureLevel_11_0; break;
                case 0xa000: caps.featureLevel = GPUFeatureLevel::D3D11_FeatureLevel_10_0; break;
                case 0x9100: caps.featureLevel = GPUFeatureLevel::D3D11_FeatureLevel_9_1;  break;
                default:     caps.featureLevel = GPUFeatureLevel::None; break;
            }

            caps.adapterDescription = "Hardware GPU (D3D11)";

            // Release COM objects
            // IUnknown::Release is at vtable index 2
            typedef ULONG(WINAPI* ReleasePtr)(void*);
            auto** vtable = *reinterpret_cast<void***>(pDevice);
            reinterpret_cast<ReleasePtr>(vtable[2])(pDevice);

            if (pContext) {
                auto** ctxVtable = *reinterpret_cast<void***>(pContext);
                reinterpret_cast<ReleasePtr>(ctxVtable[2])(pContext);
            }
        }
    }

    FreeLibrary(hD3D11);
    return caps;
}

// ── Display DPI detection ─────────────────────────────────────

struct DisplayDPIInfo {
    uint32_t dpiX = 96;
    uint32_t dpiY = 96;
    bool detected = false;

    float ScaleFactor() const {
        return static_cast<float>(dpiX) / 96.0f;
    }

    std::string ToString() const {
        if (!detected) return "DPI: unknown";
        return "DPI: " + std::to_string(dpiX) + "x" + std::to_string(dpiY) +
               " (" + std::to_string(static_cast<int>(ScaleFactor() * 100.0f)) + "% scale)";
    }
};

inline DisplayDPIInfo DetectDisplayDPI() {
    DisplayDPIInfo info;

    // Use GetDpiForSystem if available (Win10 1607+)
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef UINT(WINAPI* GetDpiForSystemPtr)();
        auto pGetDpi = reinterpret_cast<GetDpiForSystemPtr>(
            GetProcAddress(hUser32, "GetDpiForSystem"));
        if (pGetDpi) {
            UINT dpi = pGetDpi();
            info.dpiX = dpi;
            info.dpiY = dpi;
            info.detected = true;
            return info;
        }
    }

    // Fallback: use GetDeviceCaps on screen DC
    HDC hdc = GetDC(nullptr);
    if (hdc) {
        info.dpiX = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSX));
        info.dpiY = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSY));
        info.detected = true;
        ReleaseDC(nullptr, hdc);
    }

    return info;
}

// ── Aggregate environment report ──────────────────────────────

struct EnvironmentReport {
    CIProvider ciProvider = CIProvider::None;
    std::string ciProviderName;
    bool isCI = false;

    ProbedWindowsVersion windowsVersion;
    ProbedSIMDCaps simdCaps;
    GPUCapabilities gpuCaps;
    DisplayDPIInfo displayDPI;

    std::string Summary() const {
        std::string summary;
        summary += "CI: " + std::string(isCI ? ciProviderName : "Local") + "\n";
        summary += "OS: " + windowsVersion.ToString() + "\n";
        summary += "SIMD: " + simdCaps.ToString() + "\n";
        summary += "GPU: " + gpuCaps.ToString() + "\n";
        summary += "Display: " + displayDPI.ToString() + "\n";
        return summary;
    }
};

inline EnvironmentReport ProbeEnvironment() {
    EnvironmentReport report;

    report.ciProvider     = DetectCIProvider();
    report.ciProviderName = CIProviderToString(report.ciProvider);
    report.isCI           = IsRunningInCI();

    report.windowsVersion = DetectWindowsVersion();
    report.simdCaps       = ProbeSIMDCapabilities();
    report.gpuCaps        = DetectGPUCapabilities();
    report.displayDPI     = DetectDisplayDPI();

    return report;
}

}} // namespace ExplorerLens::Engine
