#pragma once
// ARM64 Library Cross-Compilation Matrix
// Tracks the build status of all external static libraries for ARM64 target.
// Provides -Architecture ARM64 parameter semantics and build script metadata.

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens::Platform {

// ─── Library build result
// ──────────────────────────────────────────────────────

enum class LibBuildStatus : uint32_t {
  NotStarted = 0,
  Success = 1,
  Failed = 2,
  Skipped = 3,  // library not needed for ARM64 target
  Fallback = 4, // using alternative implementation
};

inline std::string ToString(LibBuildStatus s) {
  switch (s) {
  case LibBuildStatus::NotStarted:
    return "NotStarted";
  case LibBuildStatus::Success:
    return "Success";
  case LibBuildStatus::Failed:
    return "Failed";
  case LibBuildStatus::Skipped:
    return "Skipped";
  case LibBuildStatus::Fallback:
    return "Fallback";
  default:
    return "Unknown";
  }
}

// ─── Library matrix entry
// ─────────────────────────────────────────────────────

struct LibMatrixEntry {
  std::string name;
  std::string version;
  std::string buildScript; // relative path from project root
  LibBuildStatus x64Status{LibBuildStatus::Success}; // baseline
  LibBuildStatus arm64Status{LibBuildStatus::NotStarted};
  std::string outputLib; // expected .lib path for ARM64
  std::string notes;

  bool IsArm64Ready() const {
    return arm64Status == LibBuildStatus::Success ||
           arm64Status == LibBuildStatus::Fallback;
  }
};

// ─── Build parameter ─────────────────────────────────────────────────────────

struct CrossBuildParameters {
  std::string architecture{"ARM64"};
  std::string toolset{"v145"};
  std::string generator{"Visual Studio 18 2026"};
  std::string toolchainFile; // e.g., cmake/toolchain-windows-arm64.cmake
  bool clean{false};
  bool verbose{false};

  std::string ToPSArgString() const {
    return "-Architecture " + architecture + " -Toolset " + toolset;
  }
};

// ─── Full ARM64 library matrix
// ────────────────────────────────────────────────

struct ARM64LibraryMatrix {
  std::vector<LibMatrixEntry> libraries;
  CrossBuildParameters buildParams;

  uint32_t ReadyCount() const {
    uint32_t n = 0;
    for (const auto &e : libraries)
      if (e.IsArm64Ready())
        ++n;
    return n;
  }

  uint32_t FailedCount() const {
    uint32_t n = 0;
    for (const auto &e : libraries)
      if (e.arm64Status == LibBuildStatus::Failed)
        ++n;
    return n;
  }

  static constexpr uint32_t kMinPassThreshold =
      7; // at least 7 of 12 must succeed

  bool MeetsPassThreshold() const { return ReadyCount() >= kMinPassThreshold; }

  std::string SummaryReport() const {
    return "ARM64 Library Matrix: " + std::to_string(ReadyCount()) + "/" +
           std::to_string(libraries.size()) + " ready — " +
           (MeetsPassThreshold() ? "PASS" : "FAIL");
  }

  static ARM64LibraryMatrix CreateDefault() {
    ARM64LibraryMatrix m;
    m.buildParams = CrossBuildParameters{};
    m.libraries = {
        {"zlib", "1.3.1", "build-scripts/external-libs/Build-Zlib.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/compression-libs/zlib-1.3.1/lib/ARM64/Release/"
         "zlibstatic.lib",
         ""},

        {"LZ4", "1.10.0", "build-scripts/external-libs/Build-LZ4.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/compression-libs/lz4-1.10.0/lib/ARM64/Release/lz4.lib", ""},

        {"zstd", "1.5.7", "build-scripts/external-libs/Build-Zstd.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/compression-libs/zstd-1.5.7/lib/ARM64/Release/"
         "zstd_static.lib",
         ""},

        {"LZMA SDK", "26.00",
         "build-scripts/external-libs/Build-LZMA-SDK-26.00.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/compression-libs/lzma-26.00/lib/ARM64/Release/7za.lib", ""},

        {"minizip-ng", "4.0.10",
         "build-scripts/external-libs/Build-MinizipNG.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/compression-libs/minizip-ng-4.0.10/lib/ARM64/Release/"
         "minizip.lib",
         ""},

        {"UnRAR", "7.2.2", "build-scripts/external-libs/Build-UnRAR.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/compression-libs/unrar-7.2.2/lib/ARM64/Release/unrar.lib",
         "LGPL source — ARM64 MSVC not fully tested"},

        {"libwebp", "1.5.0",
         "build-scripts/external-libs/Build-LibWebP-NMake.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/image-libs/libwebp-1.5.0/lib/ARM64/Release/webp.lib", ""},

        {"libavif", "1.3.0", "build-scripts/external-libs/build-libavif.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/image-libs/libavif-1.3.0/lib/ARM64/Release/avif.lib", ""},

        {"libjxl", "0.11.1", "build-scripts/external-libs/build-libjxl.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/image-libs/libjxl-0.11.1/lib/ARM64/Release/jxl.lib",
         "Requires Brotli + HWY ARM64 support"},

        {"libheif", "1.19.5", "build-scripts/external-libs/Build-LibHEIF.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/image-libs/libheif-1.19.5/lib/ARM64/Release/heif.lib", ""},

        {"libde265", "1.0.15", "build-scripts/external-libs/Build-LibHEIF.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/image-libs/libde265-1.0.15/lib/ARM64/Release/de265.lib", ""},

        {"LibRaw", "0.21.3", "build-scripts/external-libs/Build-LibRaw.ps1",
         LibBuildStatus::Success, LibBuildStatus::NotStarted,
         "external/camera-libs/libraw-0.21.3/lib/ARM64/Release/raw.lib", ""},
    };
    return m;
  }
};

} // namespace ExplorerLens::Platform
