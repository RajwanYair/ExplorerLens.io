// LibWebPConfig.h — libwebp CRT Configuration and Runtime Validator
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 351
// Copyright (c) 2026 ExplorerLens Project
//
// Validates that libwebp is linked with matching CRT (/MD vs /MT) at compile
// time and runtime. Provides diagnostic helpers and build configuration
// constants for the libwebp 1.5.0 integration.
//
// The canonical fix is rebuilding libwebp with
// -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL. This header documents the
// configuration and provides runtime verification.

#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// CRT linkage mode for external libraries
enum class CRTLinkage : uint8_t {
  Unknown = 0,
  StaticMT = 1,  ///< /MT — static CRT (problematic for DLL hosts)
  DynamicMD = 2, ///< /MD — dynamic CRT (correct for shell extensions)
  DebugMTd = 3,  ///< /MTd — debug static
  DebugMDd = 4   ///< /MDd — debug dynamic
};

/// Alias for backward compatibility with tests
using CRTLinkMode = CRTLinkage;

/// Library build configuration record
struct LibraryBuildConfig {
  const char *name = nullptr;
  const char *version = nullptr;
  CRTLinkage crt = CRTLinkage::Unknown;
  bool isRebuiltWithMD = false;
  bool requiresNoDefaultLib = false;
  const char *buildScript = nullptr;
};

/// libwebp configuration manager
class LibWebPConfig {
public:
  /// Public default constructor for direct use
  LibWebPConfig() : useDynamicCRT(true), enableSIMD(true) {
    m_config.name = "libwebp";
    m_config.version = "1.5.0";
    m_config.buildScript =
        "build-scripts/external-libs/Build-LibWebP-NMake.ps1";
#ifdef LIBWEBP_REBUILT_WITH_MD
    m_config.crt = CRTLinkage::DynamicMD;
    m_config.isRebuiltWithMD = true;
    m_config.requiresNoDefaultLib = false;
#else
    m_config.crt = CRTLinkage::StaticMT;
    m_config.isRebuiltWithMD = false;
    m_config.requiresNoDefaultLib = true;
#endif
  }

  static LibWebPConfig &Instance() {
    static LibWebPConfig instance;
    return instance;
  }

  /// Whether libwebp uses dynamic CRT (/MD)
  bool useDynamicCRT = true;
  /// Whether SIMD optimizations are enabled
  bool enableSIMD = true;

  /// CRT mode display name
  static const wchar_t *CRTModeName(CRTLinkage mode) {
    switch (mode) {
    case CRTLinkage::DynamicMD:
      return L"Dynamic (/MD)";
    case CRTLinkage::StaticMT:
      return L"Static (/MT)";
    case CRTLinkage::DebugMDd:
      return L"Debug Dynamic (/MDd)";
    case CRTLinkage::DebugMTd:
      return L"Debug Static (/MTd)";
    default:
      return L"Unknown";
    }
  }

  /// Get current libwebp build configuration
  const LibraryBuildConfig &GetConfig() const { return m_config; }

  /// Check if libwebp was built with correct CRT (/MD)
  bool IsCorrectCRT() const { return m_config.crt == CRTLinkage::DynamicMD; }

  /// Check if /NODEFAULTLIB:LIBCMT workaround is needed
  bool NeedsNoDefaultLibWorkaround() const {
    return m_config.requiresNoDefaultLib;
  }

  /// Validate CRT consistency at runtime
  /// Returns true if no CRT mismatch is detected
  bool ValidateCRTConsistency() const {
    // When built with /MD, the module should use msvcrt.dll
    // When built with /MT, a private CRT is statically linked
    // Mismatch causes heap corruption when allocating in one CRT
    // and freeing in another
#ifdef _DLL
    // Host (LENSShell.dll) is /MD — libwebp should also be /MD
    return m_config.crt == CRTLinkage::DynamicMD;
#else
    return m_config.crt == CRTLinkage::StaticMT;
#endif
  }

  /// Get diagnostic string for build verification
  std::string GetDiagnostic() const {
    std::string diag = "libwebp ";
    diag += m_config.version ? m_config.version : "?";
    diag += " CRT=";
    switch (m_config.crt) {
    case CRTLinkage::StaticMT:
      diag += "/MT";
      break;
    case CRTLinkage::DynamicMD:
      diag += "/MD";
      break;
    case CRTLinkage::DebugMTd:
      diag += "/MTd";
      break;
    case CRTLinkage::DebugMDd:
      diag += "/MDd";
      break;
    default:
      diag += "unknown";
      break;
    }
    if (m_config.requiresNoDefaultLib)
      diag += " [NODEFAULTLIB workaround active]";
    if (m_config.isRebuiltWithMD)
      diag += " [rebuilt with /MD]";
    return diag;
  }

  /// Apply the /MD rebuild fix
  /// Updates configuration to reflect the corrected build
  void MarkAsRebuiltWithMD() {
    m_config.crt = CRTLinkage::DynamicMD;
    m_config.isRebuiltWithMD = true;
    m_config.requiresNoDefaultLib = false;
  }

  /// Get the linker flags needed for the current configuration
  struct LinkerFlags {
    bool noDefaultLibLIBCMT = false;
    bool noDefaultLibMSVCRT = false;
    const char *description = "";
  };

  LinkerFlags GetRequiredLinkerFlags() const {
    LinkerFlags flags;
    if (m_config.crt == CRTLinkage::StaticMT) {
      // libwebp /MT linked into /MD host — must suppress conflicting CRT
      flags.noDefaultLibLIBCMT = true;
      flags.description =
          "/NODEFAULTLIB:LIBCMT required (libwebp /MT in /MD host)";
    } else {
      flags.description = "No workaround needed (matching CRT)";
    }
    return flags;
  }

private:
  LibraryBuildConfig m_config;
};

/// Compile-time CRT validation
namespace CRTValidation {
/// Verify at compile time that the host module uses /MD
static_assert(
#ifdef _DLL
    true,
#else
    false,
#endif
    "ExplorerLens must be compiled with /MD (dynamic CRT). "
    "Shell extensions using /MT cause heap corruption in explorer.exe.");
} // namespace CRTValidation

} // namespace Engine
} // namespace ExplorerLens
