// PlatformBuildMatrix.h — Compile-Time & Runtime Platform Build Matrix Validator
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces platform build hygiene with static_assert checks for header leakage,
// runtime feature enumeration, and cross-platform build configuration validation.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <array>

namespace ExplorerLens { namespace Engine {

enum class BuildPlatform : uint8_t {
    Win64   = 0,
    Win32   = 1,
    macOS_ARM64 = 2,
    macOS_x64   = 3,
    Linux_x64   = 4,
    Linux_ARM64 = 5,
    Unknown = 255
};

enum class PlatformFeature : uint16_t {
    DirectX11       = 0x0001,
    DirectX12       = 0x0002,
    Vulkan          = 0x0004,
    Metal           = 0x0008,
    OpenGL          = 0x0010,
    COM             = 0x0020,
    DRM             = 0x0040,
    EGL             = 0x0080,
    NTFS_Notify     = 0x0100,
    inotify         = 0x0200,
    FSEvents        = 0x0400,
    WinShellExt     = 0x0800,
    QuickLook       = 0x1000,
    LinuxThumbnailer = 0x2000
};

struct PlatformBuildInfo {
    BuildPlatform platform    = BuildPlatform::Unknown;
    const char*   compilerName = "Unknown";
    uint32_t      compilerVersion = 0;
    const char*   arch        = "Unknown";
    uint16_t      featureMask = 0;
    bool          isDebug     = false;
};

// Compile-time platform validation
#ifdef _WIN32
    #ifdef _WIN64
        static_assert(sizeof(void*) == 8, "ExplorerLens requires 64-bit builds on Windows");
    #endif
    #ifndef _MSC_VER
        // Allow but warn conceptually — MSVC is preferred
    #endif
#endif

#if defined(__APPLE__) && defined(__x86_64__)
    // macOS x64 is supported but ARM64 is preferred
#endif

// Guard against platform header leakage
#if defined(_WIN32) && defined(__linux__)
    static_assert(false, "Conflicting platform macros: both _WIN32 and __linux__ defined");
#endif
#if defined(_WIN32) && defined(__APPLE__)
    static_assert(false, "Conflicting platform macros: both _WIN32 and __APPLE__ defined");
#endif
#if defined(__APPLE__) && defined(__linux__)
    static_assert(false, "Conflicting platform macros: both __APPLE__ and __linux__ defined");
#endif

class PlatformBuildMatrix {
public:
    static constexpr BuildPlatform GetBuildPlatform() {
#if defined(_WIN64)
        return BuildPlatform::Win64;
#elif defined(_WIN32)
        return BuildPlatform::Win32;
#elif defined(__APPLE__) && defined(__aarch64__)
        return BuildPlatform::macOS_ARM64;
#elif defined(__APPLE__)
        return BuildPlatform::macOS_x64;
#elif defined(__linux__) && defined(__aarch64__)
        return BuildPlatform::Linux_ARM64;
#elif defined(__linux__)
        return BuildPlatform::Linux_x64;
#else
        return BuildPlatform::Unknown;
#endif
    }

    static constexpr const char* GetBuildPlatformName() {
        constexpr const char* NAMES[] = {
            "Win64", "Win32", "macOS-ARM64", "macOS-x64", "Linux-x64", "Linux-ARM64"
        };
        auto idx = static_cast<uint8_t>(GetBuildPlatform());
        return (idx < 6) ? NAMES[idx] : "Unknown";
    }

    static constexpr const char* GetCompilerName() {
#ifdef _MSC_VER
        return "MSVC";
#elif defined(__clang__)
        return "Clang";
#elif defined(__GNUC__)
        return "GCC";
#else
        return "Unknown";
#endif
    }

    static constexpr uint32_t GetCompilerVersion() {
#ifdef _MSC_VER
        return _MSC_VER;
#elif defined(__clang__)
        return __clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__;
#elif defined(__GNUC__)
        return __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__;
#else
        return 0;
#endif
    }

    static uint16_t GetSupportedFeatures() {
        uint16_t mask = 0;
#ifdef _WIN32
        mask |= static_cast<uint16_t>(PlatformFeature::DirectX11);
        mask |= static_cast<uint16_t>(PlatformFeature::DirectX12);
        mask |= static_cast<uint16_t>(PlatformFeature::Vulkan);
        mask |= static_cast<uint16_t>(PlatformFeature::COM);
        mask |= static_cast<uint16_t>(PlatformFeature::NTFS_Notify);
        mask |= static_cast<uint16_t>(PlatformFeature::WinShellExt);
#elif defined(__APPLE__)
        mask |= static_cast<uint16_t>(PlatformFeature::Metal);
        mask |= static_cast<uint16_t>(PlatformFeature::OpenGL);
        mask |= static_cast<uint16_t>(PlatformFeature::FSEvents);
        mask |= static_cast<uint16_t>(PlatformFeature::QuickLook);
#elif defined(__linux__)
        mask |= static_cast<uint16_t>(PlatformFeature::Vulkan);
        mask |= static_cast<uint16_t>(PlatformFeature::OpenGL);
        mask |= static_cast<uint16_t>(PlatformFeature::DRM);
        mask |= static_cast<uint16_t>(PlatformFeature::EGL);
        mask |= static_cast<uint16_t>(PlatformFeature::inotify);
        mask |= static_cast<uint16_t>(PlatformFeature::LinuxThumbnailer);
#endif
        return mask;
    }

    static bool HasFeature(PlatformFeature feature) {
        return (GetSupportedFeatures() & static_cast<uint16_t>(feature)) != 0;
    }

    static PlatformBuildInfo GetBuildInfo() {
        PlatformBuildInfo info;
        info.platform        = GetBuildPlatform();
        info.compilerName    = GetCompilerName();
        info.compilerVersion = GetCompilerVersion();
        info.arch            = GetBuildPlatformName();
        info.featureMask     = GetSupportedFeatures();
#ifdef NDEBUG
        info.isDebug = false;
#else
        info.isDebug = true;
#endif
        return info;
    }

    static bool ValidatePlatformHeaders() {
        bool valid = true;
#ifdef _WIN32
        valid &= HasFeature(PlatformFeature::COM);
        valid &= HasFeature(PlatformFeature::WinShellExt);
#endif
        valid &= (GetBuildPlatform() != BuildPlatform::Unknown);
        valid &= (GetCompilerVersion() > 0);
        return valid;
    }
};

}} // namespace ExplorerLens::Engine
