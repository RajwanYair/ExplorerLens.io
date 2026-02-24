//==============================================================================
// ExplorerLens Engine — Supply Chain Integrity V2
// SBOM v2 with enhanced provenance, VEX (Vulnerability Exploitability
// eXchange) advisories, dependency pinning, and reproducible build gates.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SBOMFormat : uint8_t {
  SPDX_2_3 = 0,
  CycloneDX_1_6,
  SWID,
  SPDX = SPDX_2_3, // compat alias
  COUNT = SWID + 1
};
enum class DepVulnStatus : uint8_t {
  NotAffected = 0,
  Affected,
  Fixed,
  UnderInvestigation,
  Clean = Fixed, // compat alias
  COUNT = UnderInvestigation + 1
};
enum class ReproducibleBuildCheck : uint8_t {
  DeterministicLinker = 0,
  StableTimestamps,
  HashLocked,
  SignedManifest,
  HashMatch = HashLocked, // compat alias
  COUNT = SignedManifest + 1
};

struct SBOMDependency {
  std::wstring name;
  std::wstring version;
  std::wstring license;
  std::wstring sourceURL;
  std::wstring sha256;
  bool direct = true;
};

struct VEXAdvisory {
  std::wstring cveId;
  std::wstring componentName;
  DepVulnStatus status = DepVulnStatus::NotAffected;
  std::wstring justification;
};

struct SupplyChainReport {
  SBOMFormat format = SBOMFormat::SPDX_2_3;
  uint32_t directDeps = 0;
  uint32_t transitiveDeps = 0;
  uint32_t pinned = 0;
  uint32_t vulnerabilities = 0;
  bool reproducible = false;
};

class SupplyChainIntegrityV2 {
public:
  static const wchar_t *SBOMFormatName(SBOMFormat f) {
    switch (f) {
    case SBOMFormat::SPDX_2_3:
      return L"SPDX 2.3";
    case SBOMFormat::CycloneDX_1_6:
      return L"CycloneDX 1.6";
    case SBOMFormat::SWID:
      return L"SWID";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *VulnStatusName(DepVulnStatus v) {
    switch (v) {
    case DepVulnStatus::NotAffected:
      return L"Not Affected";
    case DepVulnStatus::Affected:
      return L"Affected";
    case DepVulnStatus::Fixed:
      return L"Fixed";
    case DepVulnStatus::UnderInvestigation:
      return L"Under Investigation";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *ReprodCheckName(ReproducibleBuildCheck c) {
    switch (c) {
    case ReproducibleBuildCheck::DeterministicLinker:
      return L"Deterministic Linker";
    case ReproducibleBuildCheck::StableTimestamps:
      return L"Stable Timestamps";
    case ReproducibleBuildCheck::HashLocked:
      return L"Hash Locked Deps";
    case ReproducibleBuildCheck::SignedManifest:
      return L"Signed Manifest";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t SBOMFormatCount() {
    return static_cast<size_t>(SBOMFormat::COUNT);
  }
  static constexpr size_t VulnStatusCount() {
    return static_cast<size_t>(DepVulnStatus::COUNT);
  }
  static constexpr size_t ReprodCheckCount() {
    return static_cast<size_t>(ReproducibleBuildCheck::COUNT);
  }

  // Compatibility aliases (tests)
  static const wchar_t *ReproducibleCheckName(ReproducibleBuildCheck c) {
    return ReprodCheckName(c);
  }
};

} // namespace Engine
} // namespace ExplorerLens
