//==============================================================================
// ExplorerLens Engine — ARM64 Validation
// Real ARM64 hardware testing, NEON SIMD paths, platform-specific fixes.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>
// ARM64Feature enum is defined in ARM64HardwareValidator.h
#include "ARM64HardwareValidator.h"

namespace ExplorerLens {
namespace Engine {

// ARM64Feature is imported from ARM64HardwareValidator.h (uint32_t bitmask)

/// ARM64 validation test category
enum class ARM64TestCategory : uint8_t {
  BasicBoot,         // DLL loads on ARM64
  DecoderFunctional, // All decoders produce correct output
  GPURendering,      // D3D11/GDI rendering works
  MemoryLayout,      // Struct alignment correct
  SIMDPaths,         // NEON code paths active
  Endianness,        // File format byte order handling
  Performance,       // Meets baseline perf targets
  COMRegistration,   // Shell extension registers correctly
  COUNT
};

/// ARM64 validation config
struct ARM64ValidationConfig {
  bool isEmulated = false;    // Running under x64 emulation
  bool isNativeARM64 = false; // True ARM64 hardware
  uint32_t coreCount = 0;
  uint64_t systemMemory = 0;
  std::wstring processorName;
  std::wstring osVersion;
};

/// ARM64 platform validation result (distinct from
/// ARM64HardwareValidator::ARM64ValidationResult)
struct ARM64PlatValidationResult {
  ARM64TestCategory category;
  bool passed = false;
  std::wstring detail;
  double perfMs = 0;
};

/// ARM64 platform validator
class ARM64PlatformValidator {
public:
  /// Feature name
  static const wchar_t *FeatureName(ARM64Feature f) {
    switch (f) {
    case ARM64Feature::NEON:
      return L"NEON";
    case ARM64Feature::CRC32:
      return L"CRC32";
    case ARM64Feature::AES:
      return L"AES";
    case ARM64Feature::SHA1:
      return L"SHA-1";
    case ARM64Feature::SHA256:
      return L"SHA-256";
    case ARM64Feature::LSE:
      return L"LSE Atomics";
    case ARM64Feature::FP16:
      return L"FP16";
    case ARM64Feature::DotProd:
      return L"Dot Product";
    case ARM64Feature::SVE:
      return L"SVE";
    default:
      return L"Unknown";
    }
  }

  /// Test category name
  static const wchar_t *CategoryName(ARM64TestCategory c) {
    switch (c) {
    case ARM64TestCategory::BasicBoot:
      return L"Basic Boot";
    case ARM64TestCategory::DecoderFunctional:
      return L"Decoder Functional";
    case ARM64TestCategory::GPURendering:
      return L"GPU Rendering";
    case ARM64TestCategory::MemoryLayout:
      return L"Memory Layout";
    case ARM64TestCategory::SIMDPaths:
      return L"SIMD Paths";
    case ARM64TestCategory::Endianness:
      return L"Endianness";
    case ARM64TestCategory::Performance:
      return L"Performance";
    case ARM64TestCategory::COMRegistration:
      return L"COM Registration";
    default:
      return L"Unknown";
    }
  }

  /// Feature count (matches ARM64HardwareValidator.h enum values)
  static constexpr size_t FeatureCount() { return 9; } // NEON..FLAGM

  /// Test category count
  static constexpr size_t CategoryCount() {
    return static_cast<size_t>(ARM64TestCategory::COUNT);
  }

  /// Check if running on ARM64
  static bool IsARM64() {
#if defined(_M_ARM64) || defined(__aarch64__)
    return true;
#else
    return false;
#endif
  }
};

} // namespace Engine
} // namespace ExplorerLens
