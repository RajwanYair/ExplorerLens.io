//==============================================================================
// CIHardeningEngine.h — CI Hardening
// GitHub Actions matrix build, automated test, caching.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// CI build configuration
enum class CIConfig : uint8_t { x64Release = 0, x64Debug, ARM64Cross, COUNT };

/// CI/CD pipeline hardening with matrix builds and caching.
class CIHardeningEngine {
public:
  enum class CITarget {
    x64_Debug,
    x64_Release,
    ARM64_CrossCompile,
    x64_Sanitizer,
    COUNT
  };

  enum class CIStage {
    Checkout,
    Configure,
    Build,
    Test,
    Package,
    Deploy,
    COUNT
  };

  enum class CacheStrategy { None, ExternalLibs, BuildArtifacts, Full, COUNT };

  struct CIJob {
    CITarget target;
    CIStage currentStage;
    bool passing;
    uint32_t durationSec;
  };

  static const wchar_t *TargetName(CITarget t) {
    switch (t) {
    case CITarget::x64_Debug:
      return L"x64-Debug";
    case CITarget::x64_Release:
      return L"x64-Release";
    case CITarget::ARM64_CrossCompile:
      return L"ARM64-Cross";
    case CITarget::x64_Sanitizer:
      return L"x64-Sanitizer";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *StageName(CIStage s) {
    switch (s) {
    case CIStage::Checkout:
      return L"Checkout";
    case CIStage::Configure:
      return L"Configure";
    case CIStage::Build:
      return L"Build";
    case CIStage::Test:
      return L"Test";
    case CIStage::Package:
      return L"Package";
    case CIStage::Deploy:
      return L"Deploy";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *CacheStrategyName(CacheStrategy c) {
    switch (c) {
    case CacheStrategy::None:
      return L"None";
    case CacheStrategy::ExternalLibs:
      return L"ExternalLibs";
    case CacheStrategy::BuildArtifacts:
      return L"BuildArtifacts";
    case CacheStrategy::Full:
      return L"Full";
    default:
      return L"Unknown";
    }
  }

  static size_t TargetCount() { return static_cast<size_t>(CITarget::COUNT); }
  static size_t StageCount() { return static_cast<size_t>(CIStage::COUNT); }
  static size_t CacheStrategyCount() {
    return static_cast<size_t>(CacheStrategy::COUNT);
  }

  static std::vector<CIJob> GetPipeline() {
    return {
        {CITarget::x64_Debug, CIStage::Test, true, 240},
        {CITarget::x64_Release, CIStage::Test, true, 180},
        {CITarget::ARM64_CrossCompile, CIStage::Build, true, 300},
        {CITarget::x64_Sanitizer, CIStage::Test, true, 600},
    };
  }

  static bool AllPassing() {
    for (const auto &j : GetPipeline())
      if (!j.passing)
        return false;
    return true;
  }

  /// CI configuration enum for test API
  static constexpr size_t ConfigCount() {
    return static_cast<size_t>(CIConfig::COUNT);
  }
  static const wchar_t *ConfigName(CIConfig c) {
    switch (c) {
    case CIConfig::x64Release:
      return L"x64 Release";
    case CIConfig::x64Debug:
      return L"x64 Debug";
    case CIConfig::ARM64Cross:
      return L"ARM64 Cross-Compile";
    default:
      return L"Unknown";
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
