// SDKVersionGuard.h — ABI Compatibility Checking for Plugin SDK
// Copyright (c) 2026 ExplorerLens Project
//
// Validates at runtime that a loaded plugin DLL's ABI version is compatible
// with the engine's current SDK version. Provides forward (major) and backward
// (minor) compatibility checks per the Semantic Versioning contract.
//
#pragma once
#include <windows.h>
#include <string>
#include <cstdint>
#include "PublicAPI.h"

namespace ExplorerLens { namespace Engine {

// Compatibility classification
enum class SDKCompat {
    Compatible,         // Exact or older minor — safe to load
    NewerMinor,         // Plugin built against newer minor — ok with caution
    MajorMismatch,      // Major version mismatch — cannot load safely
    TooOld,             // Plugin ABI below minimum supported (v1.0)
    UnknownABI,         // Plugin did not export version information
};

struct SDKVersionInfo {
    uint16_t major      = 0;
    uint16_t minor      = 0;
    uint16_t patch      = 0;
    uint32_t fullVersion = 0;  // (major<<16)|(minor<<8)|patch
    std::wstring dllPath;
    std::wstring pluginName;
};

class SDKVersionGuard {
public:
    // Current engine SDK major version (must match plugin)
    static constexpr uint16_t ENGINE_SDK_MAJOR = LENS_API_VERSION_MAJOR;
    static constexpr uint16_t ENGINE_SDK_MINOR = LENS_API_VERSION_MINOR;

    // Minimum supported ABI version (plugins older than this are rejected)
    static constexpr uint32_t MIN_ABI_VERSION = (19u << 16) | (0u << 8) | 0u; // v19.0.0

    // Check a plugin's API version against the engine
    static SDKCompat Check(uint32_t pluginVersion) {
        if (pluginVersion == 0) return SDKCompat::UnknownABI;
        if (pluginVersion < MIN_ABI_VERSION) return SDKCompat::TooOld;

        uint16_t pMajor = (pluginVersion >> 16) & 0xFF;
        uint16_t pMinor = (pluginVersion >>  8) & 0xFF;

        if (pMajor != ENGINE_SDK_MAJOR) return SDKCompat::MajorMismatch;
        if (pMinor > ENGINE_SDK_MINOR)  return SDKCompat::NewerMinor;
        return SDKCompat::Compatible;
    }

    // Load plugin DLL and query its exported version symbol "LensPluginGetVersion"
    // Returns 0 if the DLL does not export the symbol
    static uint32_t QueryDLLVersion(HMODULE hDll) {
        if (!hDll) return 0;
        auto fn = reinterpret_cast<uint32_t(WINAPI*)()>(
            GetProcAddress(hDll, "LensPluginGetVersion"));
        return fn ? fn() : 0;
    }

    // Full validation: load + check + populate info
    static SDKCompat ValidateDLL(const std::wstring& dllPath, SDKVersionInfo* pInfo = nullptr) {
        HMODULE hDll = LoadLibraryExW(dllPath.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (!hDll) return SDKCompat::UnknownABI;

        uint32_t ver = QueryDLLVersion(hDll);
        SDKCompat compat = Check(ver);

        if (pInfo) {
            pInfo->fullVersion = ver;
            pInfo->major  = (ver >> 16) & 0xFF;
            pInfo->minor  = (ver >>  8) & 0xFF;
            pInfo->patch  = (ver)       & 0xFF;
            pInfo->dllPath = dllPath;

            // Try to read display name
            auto nameFn = reinterpret_cast<const wchar_t*(WINAPI*)()>(
                GetProcAddress(hDll, "LensPluginGetName"));
            if (nameFn) pInfo->pluginName = nameFn();
        }
        FreeLibrary(hDll);
        return compat;
    }

    // Human-readable compatibility string
    static const wchar_t* DescribeCompat(SDKCompat c) {
        switch (c) {
        case SDKCompat::Compatible:    return L"Compatible";
        case SDKCompat::NewerMinor:    return L"Compatible (newer minor — verify new features)";
        case SDKCompat::MajorMismatch: return L"Incompatible — major SDK version mismatch";
        case SDKCompat::TooOld:        return L"Incompatible — plugin ABI below minimum v19.0";
        case SDKCompat::UnknownABI:    return L"Unknown — plugin does not export version info";
        default:                       return L"Unknown";
        }
    }
};

}} // namespace ExplorerLens::Engine
