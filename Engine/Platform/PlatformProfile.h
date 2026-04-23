// PlatformProfile.h — Compile-time platform capability profile (PAL header)
// Copyright (c) 2026 ExplorerLens Project
//
// ROADMAP: §16.1 (cross-platform timeline), ADR-013 (cross-platform PAL),
//          §7.2 (Engine/Platform directory target)
//
// PURPOSE
// ───────
// Header-only, zero-cost, compile-time description of the current build
// target's platform capabilities.  Acts as the canonical answer to the
// question "on this build, what shell integration kind, GPU backend, thread-
// count hint, and filesystem semantics should I assume?"
//
// This header is:
//   • Pure header-only — no dependencies, no <windows.h>, no <Foundation.h>.
//   • `constexpr`-friendly — every query is evaluable at compile time.
//   • Namespace `ExplorerLens::Platform` — complementary to the existing
//     `PlatformDetector` (runtime) and `PlatformCapabilityProbe` (dynamic).
//   • Thread-safe — all operations are stateless.
//
// DESIGN NOTES
// ────────────
// Runtime probe and dynamic GPU routing belong in PlatformGPURouter and
// PlatformCapabilityProbe.  This header answers *compile-time* questions
// like "does this target support the Windows Shell IThumbnailProvider
// interface?" — used by static_assert guards and #ifdef-less code paths
// written with `if constexpr`.
//
// USAGE
// ─────
//   #include "Engine/Platform/PlatformProfile.h"
//   using namespace ExplorerLens::Platform;
//
//   if constexpr (CurrentProfile().shellKind == ShellIntegrationKind::WIN32_COM) {
//       // Windows COM IThumbnailProvider registration path
//   }
//
//   static_assert(CurrentProfile().hasDirectX,
//                 "DirectX required for Engine::GPU on this target");

#pragma once

#include <cstdint>
#include <string_view>

namespace ExplorerLens::Platform {

// ============================================================================
// Compile-time platform identifiers
// ============================================================================

enum class TargetPlatform : uint8_t {
    UNKNOWN = 0,
    WINDOWS = 1,
    MACOS   = 2,
    LINUX   = 3,
    WASM    = 4
};

enum class ShellIntegrationKind : uint8_t {
    NONE              = 0,
    WIN32_COM         = 1,  // IThumbnailProvider, IExtractImage2, IPropertyStore
    MACOS_QUICKLOOK   = 2,  // QLThumbnailProvider
    LINUX_TUMBLER     = 3,  // GNOME Tumbler D-Bus
    LINUX_NAUTILUS    = 4,  // Nautilus extension
    LINUX_KDE_KIO     = 5,  // KIO thumbnail plugin
    WASM_EMBEDDED     = 6   // No shell; in-browser host
};

enum class GraphicsBackend : uint8_t {
    NONE              = 0,
    DIRECTX_11        = 1,
    DIRECTX_12        = 2,
    METAL             = 3,
    VULKAN            = 4,
    OPENGL_ES         = 5,
    WEBGPU            = 6
};

enum class FilesystemSemantics : uint8_t {
    UNKNOWN           = 0,
    WIN32_NTFS        = 1,  // case-insensitive, backslash, drive letters, ADS
    POSIX             = 2,  // case-sensitive, forward slash, no drive letters
    WASM_MEMFS        = 3
};

// ============================================================================
// Immutable platform profile record
// ============================================================================

struct PlatformProfile {
    TargetPlatform       target{TargetPlatform::UNKNOWN};
    ShellIntegrationKind shellKind{ShellIntegrationKind::NONE};
    GraphicsBackend      primaryGpu{GraphicsBackend::NONE};
    GraphicsBackend      fallbackGpu{GraphicsBackend::NONE};
    FilesystemSemantics  fs{FilesystemSemantics::UNKNOWN};
    uint8_t              pointerSize{sizeof(void*)};
    bool                 hasDirectX{false};
    bool                 hasCOMInterop{false};
    bool                 hasRegistry{false};
    bool                 hasSymbolicLinks{false};
    bool                 supportsLongPaths{false};
    std::string_view     label{"unknown"};
};

// ============================================================================
// Compile-time selector — one definition per supported target
// ============================================================================

constexpr PlatformProfile MakeWindowsProfile() noexcept {
    return PlatformProfile{
        .target             = TargetPlatform::WINDOWS,
        .shellKind          = ShellIntegrationKind::WIN32_COM,
        .primaryGpu         = GraphicsBackend::DIRECTX_11,
        .fallbackGpu        = GraphicsBackend::DIRECTX_12,
        .fs                 = FilesystemSemantics::WIN32_NTFS,
        .pointerSize        = sizeof(void*),
        .hasDirectX         = true,
        .hasCOMInterop      = true,
        .hasRegistry        = true,
        .hasSymbolicLinks   = true,   // NTFS reparse points
        .supportsLongPaths  = true,   // with manifest opt-in
        .label              = "windows-x64"
    };
}

constexpr PlatformProfile MakeMacOSProfile() noexcept {
    return PlatformProfile{
        .target             = TargetPlatform::MACOS,
        .shellKind          = ShellIntegrationKind::MACOS_QUICKLOOK,
        .primaryGpu         = GraphicsBackend::METAL,
        .fallbackGpu        = GraphicsBackend::NONE,
        .fs                 = FilesystemSemantics::POSIX,
        .pointerSize        = sizeof(void*),
        .hasDirectX         = false,
        .hasCOMInterop      = false,
        .hasRegistry        = false,
        .hasSymbolicLinks   = true,
        .supportsLongPaths  = true,
        .label              = "macos"
    };
}

constexpr PlatformProfile MakeLinuxProfile() noexcept {
    return PlatformProfile{
        .target             = TargetPlatform::LINUX,
        .shellKind          = ShellIntegrationKind::LINUX_TUMBLER,
        .primaryGpu         = GraphicsBackend::VULKAN,
        .fallbackGpu        = GraphicsBackend::OPENGL_ES,
        .fs                 = FilesystemSemantics::POSIX,
        .pointerSize        = sizeof(void*),
        .hasDirectX         = false,
        .hasCOMInterop      = false,
        .hasRegistry        = false,
        .hasSymbolicLinks   = true,
        .supportsLongPaths  = true,
        .label              = "linux"
    };
}

constexpr PlatformProfile MakeWasmProfile() noexcept {
    return PlatformProfile{
        .target             = TargetPlatform::WASM,
        .shellKind          = ShellIntegrationKind::WASM_EMBEDDED,
        .primaryGpu         = GraphicsBackend::WEBGPU,
        .fallbackGpu        = GraphicsBackend::NONE,
        .fs                 = FilesystemSemantics::WASM_MEMFS,
        .pointerSize        = sizeof(void*),
        .hasDirectX         = false,
        .hasCOMInterop      = false,
        .hasRegistry        = false,
        .hasSymbolicLinks   = false,
        .supportsLongPaths  = false,
        .label              = "wasm"
    };
}

// ============================================================================
// Current target dispatch — selected via preprocessor at compile time
// ============================================================================

constexpr PlatformProfile CurrentProfile() noexcept {
#if defined(_WIN32) || defined(_WIN64)
    return MakeWindowsProfile();
#elif defined(__APPLE__)
    return MakeMacOSProfile();
#elif defined(__EMSCRIPTEN__) || defined(__wasm__)
    return MakeWasmProfile();
#elif defined(__linux__)
    return MakeLinuxProfile();
#else
    return PlatformProfile{};  // UNKNOWN
#endif
}

// ============================================================================
// Convenience predicates (all `constexpr`, zero runtime cost)
// ============================================================================

constexpr bool IsWindows() noexcept {
    return CurrentProfile().target == TargetPlatform::WINDOWS;
}
constexpr bool IsMacOS() noexcept {
    return CurrentProfile().target == TargetPlatform::MACOS;
}
constexpr bool IsLinux() noexcept {
    return CurrentProfile().target == TargetPlatform::LINUX;
}
constexpr bool IsWasm() noexcept {
    return CurrentProfile().target == TargetPlatform::WASM;
}
constexpr bool HasCOMIntegration() noexcept {
    return CurrentProfile().hasCOMInterop;
}
constexpr bool HasDirectX() noexcept {
    return CurrentProfile().hasDirectX;
}
constexpr bool IsPOSIXFilesystem() noexcept {
    return CurrentProfile().fs == FilesystemSemantics::POSIX;
}
constexpr bool IsNTFSFilesystem() noexcept {
    return CurrentProfile().fs == FilesystemSemantics::WIN32_NTFS;
}

// Path separator for the current target — compile-time constant.
constexpr char PathSeparator() noexcept {
    return IsWindows() ? '\\' : '/';
}

}  // namespace ExplorerLens::Platform
