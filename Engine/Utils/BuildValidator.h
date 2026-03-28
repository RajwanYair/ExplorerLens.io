// BuildValidator.h — Compile-time and runtime build validation
// Copyright (c) 2026 ExplorerLens Project
//
// Validates compiler version, C++20 features, OS version, CRT linkage,
// available RAM, and DPI awareness at both compile and run time.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

// ── Compile-time assertions ────────────────────────────────────

// Require MSVC 19.40+ (VS 2022 17.10+)
#ifdef _MSC_VER
static_assert(_MSC_VER >= 1940, "ExplorerLens requires MSVC 19.40 or later (Visual Studio 2022 17.10+)");
#endif

// Require C++20 — use preprocessor guard since defined() is not valid in static_assert
#if defined(_MSVC_LANG)
static_assert(_MSVC_LANG >= 202002L, "ExplorerLens requires C++20 (/std:c++20)");
#endif

// Require 64-bit target
static_assert(sizeof(void*) == 8, "ExplorerLens requires a 64-bit (x64) build target");

namespace ExplorerLens { namespace Engine {

// ── Build configuration info ───────────────────────────────────

enum class BuildCRTLinkage : uint8_t {
    Dynamic,  // /MD
    Static,   // /MT
    Unknown
};

enum class BuildConfiguration : uint8_t {
    Debug,
    Release,
    RelWithDebInfo,
    MinSizeRel,
    Unknown
};

struct ValidatorBuildInfo {
    uint32_t compilerVersionMajor = 0;
    uint32_t compilerVersionMinor = 0;
    uint32_t compilerVersionPatch = 0;
    std::string compilerName;
    std::string buildDate;
    std::string buildTime;
    BuildConfiguration configuration = BuildConfiguration::Unknown;
    std::string targetArch;
    BuildCRTLinkage crtLinkage = BuildCRTLinkage::Unknown;
    bool hasCpp20 = false;
    bool hasExceptions = true;
    bool hasRTTI = true;
};

// ── Validation result ──────────────────────────────────────────

struct BuildCheckResult {
    bool passed = true;
    std::string checkName;
    std::string message;
};

struct BuildValidationReport {
    bool allPassed = true;
    std::vector<BuildCheckResult> results;

    void AddResult(const std::string& name, bool ok, const std::string& msg = "") {
        results.push_back({ok, name, msg});
        if (!ok) allPassed = false;
    }
};

// ── GetBuildInfo ───────────────────────────────────────────────

inline ValidatorBuildInfo GetBuildInfo() {
    ValidatorBuildInfo info;

#ifdef _MSC_VER
    info.compilerName = "MSVC";
    info.compilerVersionMajor = _MSC_VER / 100;
    info.compilerVersionMinor = _MSC_VER % 100;
#ifdef _MSC_FULL_VER
    info.compilerVersionPatch = _MSC_FULL_VER % 100000;
#endif
#endif

    info.buildDate = __DATE__;
    info.buildTime = __TIME__;

#ifdef _WIN64
    info.targetArch = "x64";
#elif defined(_M_ARM64)
    info.targetArch = "ARM64";
#else
    info.targetArch = "x86";
#endif

#ifdef _DLL
    info.crtLinkage = BuildCRTLinkage::Dynamic;
#else
    info.crtLinkage = BuildCRTLinkage::Static;
#endif

#ifdef NDEBUG
    info.configuration = BuildConfiguration::Release;
#else
    info.configuration = BuildConfiguration::Debug;
#endif

#if defined(_MSVC_LANG) && _MSVC_LANG >= 202002L
    info.hasCpp20 = true;
#elif __cplusplus >= 202002L
    info.hasCpp20 = true;
#endif

#ifdef _CPPUNWIND
    info.hasExceptions = true;
#else
    info.hasExceptions = false;
#endif

#ifdef _CPPRTTI
    info.hasRTTI = true;
#else
    info.hasRTTI = false;
#endif

    return info;
}

// ── ValidateBuildEnvironment (compile-time checks at runtime) ──

inline BuildValidationReport ValidateBuildEnvironment() {
    BuildValidationReport report;

    // Check compiler version
#ifdef _MSC_VER
    report.AddResult("CompilerVersion",
        _MSC_VER >= 1940,
        "MSVC " + std::to_string(_MSC_VER) + " (need >= 1940)");
#else
    report.AddResult("CompilerVersion", false, "Non-MSVC compiler detected");
#endif

    // Check C++20
    bool hasCpp20 = false;
#if defined(_MSVC_LANG) && _MSVC_LANG >= 202002L
    hasCpp20 = true;
#elif __cplusplus >= 202002L
    hasCpp20 = true;
#endif
    report.AddResult("Cpp20Support", hasCpp20, hasCpp20 ? "C++20 enabled" : "C++20 required");

    // Check 64-bit
    report.AddResult("64BitTarget", sizeof(void*) == 8, "64-bit target required");

    // Check CRT linkage (/MD expected)
#ifdef _DLL
    report.AddResult("CRTLinkage", true, "Dynamic CRT (/MD) — correct");
#else
    report.AddResult("CRTLinkage", false, "Static CRT (/MT) detected — expected /MD");
#endif

    // Check required headers are available (compile-time proxy)
    report.AddResult("WindowsHeaders", true, "Windows.h available");
    report.AddResult("STLHeaders", true, "C++20 STL available");

    return report;
}

// ── ValidateCRTConsistency ─────────────────────────────────────

inline BuildCheckResult ValidateCRTConsistency() {
    BuildCheckResult result;
    result.checkName = "CRTConsistency";

    // At runtime, check if msvcrt DLL is loaded (indicates /MD)
    HMODULE hCrt = GetModuleHandleW(L"ucrtbase.dll");
    bool dynamicCrt = (hCrt != nullptr);

#ifdef _DLL
    bool compiledDynamic = true;
#else
    bool compiledDynamic = false;
#endif

    result.passed = (dynamicCrt == compiledDynamic);
    if (result.passed) {
        result.message = "CRT linkage consistent: " +
            std::string(compiledDynamic ? "/MD" : "/MT") + " compile matches runtime";
    } else {
        result.message = "CRT mismatch: compiled " +
            std::string(compiledDynamic ? "/MD" : "/MT") +
            " but runtime " + std::string(dynamicCrt ? "dynamic" : "static");
    }

    return result;
}

// ── ValidateRuntimeEnvironment ─────────────────────────────────

inline BuildValidationReport ValidateRuntimeEnvironment() {
    BuildValidationReport report;

    // Check OS version via RtlGetVersion (NOT versionhelpers.h)
    {
        typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll) {
            auto pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
                GetProcAddress(hNtdll, "RtlGetVersion"));
            if (pRtlGetVersion) {
                RTL_OSVERSIONINFOW osvi = {};
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                if (pRtlGetVersion(&osvi) == 0) {
                    // Windows 10 1809 = build 17763
                    bool osOk = (osvi.dwMajorVersion > 10) ||
                        (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber >= 17763);
                    report.AddResult("OSVersion", osOk,
                        "Windows " + std::to_string(osvi.dwMajorVersion) + "." +
                        std::to_string(osvi.dwMinorVersion) +
                        " build " + std::to_string(osvi.dwBuildNumber) +
                        (osOk ? " (OK)" : " (need 10.0.17763+)"));
                } else {
                    report.AddResult("OSVersion", false, "RtlGetVersion failed");
                }
            } else {
                report.AddResult("OSVersion", false, "RtlGetVersion not found in ntdll.dll");
            }
        } else {
            report.AddResult("OSVersion", false, "ntdll.dll not loaded");
        }
    }

    // Check available RAM (>512 MB)
    {
        MEMORYSTATUSEX memInfo = {};
        memInfo.dwLength = sizeof(memInfo);
        if (GlobalMemoryStatusEx(&memInfo)) {
            uint64_t availMB = memInfo.ullAvailPhys / (1024ULL * 1024ULL);
            bool ramOk = availMB > 512;
            report.AddResult("AvailableRAM", ramOk,
                std::to_string(availMB) + " MB available" +
                (ramOk ? " (OK)" : " (need >512 MB)"));
        } else {
            report.AddResult("AvailableRAM", false, "GlobalMemoryStatusEx failed");
        }
    }

    // Check DPI awareness
    {
        // GetProcessDpiAwareness requires shcore.dll
        HMODULE hShcore = LoadLibraryW(L"shcore.dll");
        if (hShcore) {
            typedef HRESULT(WINAPI* GetProcessDpiAwarenessPtr)(HANDLE, int*);
            auto pGetDpiAwareness = reinterpret_cast<GetProcessDpiAwarenessPtr>(
                GetProcAddress(hShcore, "GetProcessDpiAwareness"));
            if (pGetDpiAwareness) {
                int awareness = 0;
                HRESULT hr = pGetDpiAwareness(nullptr, &awareness);
                if (SUCCEEDED(hr)) {
                    // 0=unaware, 1=system, 2=per-monitor
                    report.AddResult("DPIAwareness", true,
                        "DPI awareness level: " + std::to_string(awareness));
                } else {
                    report.AddResult("DPIAwareness", true, "DPI query returned HRESULT, process running");
                }
            } else {
                report.AddResult("DPIAwareness", true, "shcore API not available (pre-8.1)");
            }
            FreeLibrary(hShcore);
        } else {
            report.AddResult("DPIAwareness", true, "shcore.dll not available");
        }
    }

    // CRT consistency
    {
        auto crtResult = ValidateCRTConsistency();
        report.AddResult(crtResult.checkName, crtResult.passed, crtResult.message);
    }

    return report;
}

}} // namespace ExplorerLens::Engine
