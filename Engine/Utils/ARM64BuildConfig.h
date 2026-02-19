#pragma once
// Sprint 155 — ARM64 Build Configuration
// Architecture detection, NEON intrinsics availability, capability flags,
// CMake toolchain configuration descriptors.

#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs::Platform {

// ─── Architecture detection ───────────────────────────────────────────────────

enum class TargetArchitecture : uint32_t {
    x86         = 0,
    x64         = 1,
    ARM         = 2,
    ARM64       = 3,
    ARM64EC     = 4,   // ARM64 EC (emulation compatible)
    Unknown     = 99,
};

inline std::string ToString(TargetArchitecture arch) {
    switch (arch) {
        case TargetArchitecture::x86:     return "x86";
        case TargetArchitecture::x64:     return "x64";
        case TargetArchitecture::ARM:     return "ARM";
        case TargetArchitecture::ARM64:   return "ARM64";
        case TargetArchitecture::ARM64EC: return "ARM64EC";
        default: return "Unknown";
    }
}

// Compile-time detection
inline constexpr TargetArchitecture CurrentArchitecture() {
#if defined(_M_ARM64) || defined(__aarch64__)
    return TargetArchitecture::ARM64;
#elif defined(_M_ARM64EC)
    return TargetArchitecture::ARM64EC;
#elif defined(_M_ARM)   || defined(__arm__)
    return TargetArchitecture::ARM;
#elif defined(_M_X64)   || defined(__x86_64__)
    return TargetArchitecture::x64;
#elif defined(_M_IX86)  || defined(__i386__)
    return TargetArchitecture::x86;
#else
    return TargetArchitecture::Unknown;
#endif
}

// ─── SIMD capability flags ────────────────────────────────────────────────────

struct SIMDCapabilities {
    // x64
    bool    hasSSE2     { false };
    bool    hasSSE4_1   { false };
    bool    hasAVX2     { false };
    bool    hasAVX512   { false };
    // ARM64
    bool    hasNEON     { false };
    bool    hasSVE      { false };   // Scalable Vector Extension
    bool    hasBFLOAT16 { false };

    static SIMDCapabilities DetectForArch(TargetArchitecture arch) {
        SIMDCapabilities c;
        if (arch == TargetArchitecture::x64 || arch == TargetArchitecture::x86) {
            c.hasSSE2   = true;   // baseline on x64 Windows
            c.hasSSE4_1 = true;   // assumed on modern x64
            // AVX2 requires runtime CPUID check
        }
        if (arch == TargetArchitecture::ARM64 || arch == TargetArchitecture::ARM64EC) {
            c.hasNEON = true;     // mandatory on ARMv8-A
        }
        return c;
    }

    bool HasAnyVector() const { return hasSSE2 || hasNEON; }
};

// ─── ARM64 build config descriptor ───────────────────────────────────────────

struct ARM64BuildConfig {
    std::string         cmakeToolset        { "v145" };         // VS18 ARM64 toolset
    std::string         cmakePlatform       { "ARM64" };
    std::string         platformToolset     { "v145" };
    bool                crossCompileFromX64 { true };
    bool                enableNEON          { true };
    bool                enableSVE           { false };          // opt-in when supported
    bool                enableWinRT         { false };          // future

    // Toolchain file path (relative to project root)
    std::string         toolchainFilePath   { "cmake/toolchain-windows-arm64.cmake" };

    static ARM64BuildConfig Default() { return {}; }
    static ARM64BuildConfig WithSVE() {
        auto c = Default();
        c.enableSVE = true;
        return c;
    }
};

// ─── CMake ARM64 toolchain parameters ─────────────────────────────────────────

struct CMakeARM64Parameters {
    // CMAKE_SYSTEM_PROCESSOR
    std::string systemProcessor     { "ARM64" };
    // CMAKE_C_COMPILER / CMAKE_CXX_COMPILER
    // (handled by VS generator — not set explicitly)

    // Compile flags
    std::vector<std::string> compileFlags { "/favor:ARM64" };
    // Define flags
    std::vector<std::string> defineFlags  { "_ARM64_", "WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP" };

    std::string ToCMakeArgs() const {
        std::string args = "-DCMAKE_SYSTEM_PROCESSOR=ARM64";
        for (const auto& f : compileFlags)
            args += " -DCMAKE_C_FLAGS=\"" + f + "\" -DCMAKE_CXX_FLAGS=\"" + f + "\"";
        return args;
    }
};

// ─── MSBuild ARM64 platform metadata ─────────────────────────────────────────

struct MSBuildARM64Config {
    std::string platform            { "ARM64" };
    std::string configuration       { "Release" };
    std::string platformToolset     { "v145" };
    std::string windowsTargetPlatformVersion { "10.0" };

    std::string ToMSBuildArgs() const {
        return "/p:Platform=ARM64 /p:Configuration=" + configuration +
               " /p:PlatformToolset=" + platformToolset;
    }
};

// ─── Library cross-compilation status ─────────────────────────────────────────

enum class LibraryARM64Status : uint32_t {
    NotStarted      = 0,
    BuildSuccess    = 1,
    BuildFailed     = 2,
    Fallback        = 3,   // using WIC / inbox codec instead
    NotRequired     = 4,
};

struct LibraryARM64Entry {
    std::string         libraryName;
    std::string         version;
    LibraryARM64Status  status  { LibraryARM64Status::NotStarted };
    std::string         notes;
};

struct ARM64LibraryInventory {
    std::vector<LibraryARM64Entry>  entries;

    uint32_t BuildSuccessCount() const {
        uint32_t n = 0;
        for (const auto& e : entries)
            if (e.status == LibraryARM64Status::BuildSuccess) ++n;
        return n;
    }

    static ARM64LibraryInventory CreateDefault() {
        ARM64LibraryInventory inv;
        inv.entries = {
            { "zlib",       "1.3.1",  LibraryARM64Status::NotStarted, "" },
            { "LZ4",        "1.10.0", LibraryARM64Status::NotStarted, "" },
            { "zstd",       "1.5.7",  LibraryARM64Status::NotStarted, "" },
            { "LZMA SDK",   "26.00",  LibraryARM64Status::NotStarted, "" },
            { "minizip-ng", "4.0.10", LibraryARM64Status::NotStarted, "" },
            { "UnRAR",      "7.2.2",  LibraryARM64Status::NotStarted, "" },
            { "libwebp",    "1.5.0",  LibraryARM64Status::NotStarted, "" },
            { "libavif",    "1.3.0",  LibraryARM64Status::NotStarted, "" },
            { "libjxl",     "0.11.1", LibraryARM64Status::NotStarted, "" },
            { "libheif",    "1.19.5", LibraryARM64Status::NotStarted, "" },
            { "libde265",   "1.0.15", LibraryARM64Status::NotStarted, "" },
            { "LibRaw",     "0.21.3", LibraryARM64Status::NotStarted, "" },
        };
        return inv;
    }
};

} // namespace DarkThumbs::Platform
