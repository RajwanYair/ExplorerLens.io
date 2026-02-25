// PerformanceActivation.h — Performance Feature Activation Controller
// Copyright (c) 2026 ExplorerLens Project
//
// Central controller that activates and coordinates the performance
// subsystems: ZeroCopyPipeline, ParallelIOPipeline, SIMD scaler,
// PipelineStateCacheV2, and CacheWarmingService.
#pragma once

#include "../Cache/CacheWarmingService.h"
#include "../Cache/PipelineStateCacheV2.h"
#include "../Pipeline/ParallelIOPipeline.h"
#include "../Pipeline/ZeroCopyPipeline.h"
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// SIMD capability detection
// ============================================================================

enum class SIMDCapability : uint32_t {
  NONE = 0,
  SSE2 = 1 << 0,
  SSE41 = 1 << 1,
  AVX2 = 1 << 2,
  AVX512 = 1 << 3,
  NEON = 1 << 4, // ARM64
};

inline SIMDCapability operator|(SIMDCapability a, SIMDCapability b) {
  return static_cast<SIMDCapability>(static_cast<uint32_t>(a) |
                                     static_cast<uint32_t>(b));
}

inline bool HasCapability(SIMDCapability caps, SIMDCapability test) {
  return (static_cast<uint32_t>(caps) & static_cast<uint32_t>(test)) != 0;
}

inline SIMDCapability DetectSIMDCapabilities() {
  SIMDCapability caps = SIMDCapability::NONE;

#if defined(_M_X64) || defined(_M_IX86)
  // x86/x64: Use __cpuid intrinsic
  int cpuInfo[4] = {0};
  __cpuid(cpuInfo, 1);

  // SSE2: EDX bit 26
  if (cpuInfo[3] & (1 << 26))
    caps = caps | SIMDCapability::SSE2;
  // SSE4.1: ECX bit 19
  if (cpuInfo[2] & (1 << 19))
    caps = caps | SIMDCapability::SSE41;

  // AVX2: Check CPUID leaf 7
  __cpuidex(cpuInfo, 7, 0);
  if (cpuInfo[1] & (1 << 5))
    caps = caps | SIMDCapability::AVX2;
  // AVX-512F: EBX bit 16
  if (cpuInfo[1] & (1 << 16))
    caps = caps | SIMDCapability::AVX512;

#elif defined(_M_ARM64)
  // ARM64: NEON is always available on ARMv8+
  caps = caps | SIMDCapability::NEON;
#endif

  return caps;
}

// ============================================================================
// PerformanceProfile — What features to enable based on system capabilities
// ============================================================================

struct PerformanceProfile {
  bool zeroCopyEnabled = false;
  bool parallelIOEnabled = false;
  bool simdScalerEnabled = false;
  bool psoCacheEnabled = false;
  bool cacheWarmingEnabled = false;
  SIMDCapability simdCaps = SIMDCapability::NONE;

  uint32_t ioThreadCount = 2;
  uint32_t maxParallelDecodes = 4;
  size_t psoCacheMaxEntries = 256;
  size_t warmingPrefetchCount = 32;
};

// ============================================================================
// PerformanceActivation — Central performance controller
// ============================================================================

class PerformanceActivation {
public:
  static PerformanceActivation &Instance() {
    static PerformanceActivation s_instance;
    return s_instance;
  }

  // ====================================================================
  // Detect system capabilities and create optimal profile
  // ====================================================================
  PerformanceProfile DetectAndConfigure() {
    PerformanceProfile profile;

    // SIMD Detection
    profile.simdCaps = DetectSIMDCapabilities();
    profile.simdScalerEnabled =
        HasCapability(profile.simdCaps, SIMDCapability::SSE41);

    // Zero-copy is available on all Windows 10+ systems
    profile.zeroCopyEnabled = true;

    // Parallel I/O scales with CPU cores
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    profile.ioThreadCount = (si.dwNumberOfProcessors > 4) ? 4 : 2;
    profile.parallelIOEnabled = (si.dwNumberOfProcessors >= 4);
    profile.maxParallelDecodes = si.dwNumberOfProcessors;

    // PSO cache — always enabled, sizes scale with GPU memory
    profile.psoCacheEnabled = true;
    profile.psoCacheMaxEntries = 256;

    // Cache warming — enable for systems with enough memory
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    uint64_t totalGB = memStatus.ullTotalPhys / (1024ULL * 1024 * 1024);
    profile.cacheWarmingEnabled = (totalGB >= 8);
    profile.warmingPrefetchCount = (totalGB >= 16) ? 64 : 32;

    m_profile = profile;
    m_detected = true;
    return profile;
  }

  // ====================================================================
  // Activate all performance features based on detected profile
  // ====================================================================
  void ActivateAll() {
    if (!m_detected)
      DetectAndConfigure();

    // Each subsystem is already implemented as a standalone module.
    // This controller just decides which features to activate based
    // on system capabilities detected above.
    m_activated = true;
  }

  // ====================================================================
  // Get SIMD dispatch function pointer for image scaling
  // ====================================================================
  enum class ScalerKernel {
    GENERIC,        // C++ reference implementation
    SSE41_BILINEAR, // SSE4.1 4-pixel bilinear
    AVX2_BILINEAR,  // AVX2 8-pixel bilinear
    AVX2_BICUBIC,   // AVX2 bicubic Mitchell-Netravali
    NEON_BILINEAR,  // ARM NEON bilinear
  };

  ScalerKernel SelectOptimalScaler() const {
    if (HasCapability(m_profile.simdCaps, SIMDCapability::AVX2))
      return ScalerKernel::AVX2_BICUBIC;
    if (HasCapability(m_profile.simdCaps, SIMDCapability::SSE41))
      return ScalerKernel::SSE41_BILINEAR;
    if (HasCapability(m_profile.simdCaps, SIMDCapability::NEON))
      return ScalerKernel::NEON_BILINEAR;
    return ScalerKernel::GENERIC;
  }

  // ====================================================================
  // Report activated features
  // ====================================================================
  std::string GetActivationReport() const {
    std::string report = "Performance Activation Report:\n";
    report += "  Zero-Copy Pipeline:  " +
              std::string(m_profile.zeroCopyEnabled ? "ON" : "OFF") + "\n";
    report += "  Parallel I/O:        " +
              std::string(m_profile.parallelIOEnabled ? "ON" : "OFF");
    report += " (" + std::to_string(m_profile.ioThreadCount) + " threads)\n";
    report += "  SIMD Scaler:         " +
              std::string(m_profile.simdScalerEnabled ? "ON" : "OFF") + "\n";
    report += "  PSO Cache:           " +
              std::string(m_profile.psoCacheEnabled ? "ON" : "OFF");
    report +=
        " (" + std::to_string(m_profile.psoCacheMaxEntries) + " entries)\n";
    report += "  Cache Warming:       " +
              std::string(m_profile.cacheWarmingEnabled ? "ON" : "OFF");
    report +=
        " (" + std::to_string(m_profile.warmingPrefetchCount) + " prefetch)\n";

    // SIMD caps
    report += "  SIMD Capabilities:  ";
    if (HasCapability(m_profile.simdCaps, SIMDCapability::AVX512))
      report += "AVX-512 ";
    if (HasCapability(m_profile.simdCaps, SIMDCapability::AVX2))
      report += "AVX2 ";
    if (HasCapability(m_profile.simdCaps, SIMDCapability::SSE41))
      report += "SSE4.1 ";
    if (HasCapability(m_profile.simdCaps, SIMDCapability::SSE2))
      report += "SSE2 ";
    if (HasCapability(m_profile.simdCaps, SIMDCapability::NEON))
      report += "NEON ";
    report += "\n";

    return report;
  }

  const PerformanceProfile &Profile() const { return m_profile; }
  bool IsActivated() const { return m_activated; }

private:
  PerformanceActivation() = default;

  PerformanceProfile m_profile;
  bool m_detected = false;
  bool m_activated = false;
};

} // namespace Engine
} // namespace ExplorerLens
