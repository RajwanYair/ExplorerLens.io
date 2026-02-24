//==============================================================================
// ExplorerLens.io Engine — Release Gate V26
// Security Excellence phase gate — validates threat model, memory safety,
// supply chain integrity, and runtime integrity verifier KPIs.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GateV26KPI : uint32_t {
  BuildClean = 0,
  TestsPass,
  ZeroWarnings,
  ThreatModelV2,
  MemorySafetyAuditV2,
  SupplyChainIntegrityV2,
  RuntimeIntegrity,
  ZeroCriticalThreats,
  ZeroOpenVulns,
  AuthenticodePassed,
  SBOMComplete,
  COUNT
};
struct GateV26Result {
  GateV26KPI kpi;
  bool passed = false;
  std::wstring detail;
};
struct GateV26Verdict {
  bool approved = false;
  uint32_t passed = 0;
  uint32_t failed = 0;
  std::wstring version = L"14.0.0";
  std::wstring milestone = L"v14.0 P4 - Security Excellence";
};
class ReleaseGateV26 {
public:
  static const wchar_t *KPIName(GateV26KPI k) {
    switch (k) {
    case GateV26KPI::BuildClean:
      return L"Build Clean";
    case GateV26KPI::TestsPass:
      return L"Tests Pass";
    case GateV26KPI::ZeroWarnings:
      return L"Zero Warnings";
    case GateV26KPI::ThreatModelV2:
      return L"Threat Model V2";
    case GateV26KPI::MemorySafetyAuditV2:
      return L"Memory Safety Audit V2";
    case GateV26KPI::SupplyChainIntegrityV2:
      return L"Supply Chain Integrity V2";
    case GateV26KPI::RuntimeIntegrity:
      return L"Runtime Integrity";
    case GateV26KPI::ZeroCriticalThreats:
      return L"Zero Critical Threats";
    case GateV26KPI::ZeroOpenVulns:
      return L"Zero Open Vulns";
    case GateV26KPI::AuthenticodePassed:
      return L"Authenticode Passed";
    case GateV26KPI::SBOMComplete:
      return L"SBOM Complete";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV26KPI::COUNT);
  }
  static GateV26Verdict Evaluate(const std::vector<GateV26Result> &r) {
    GateV26Verdict v;
    for (const auto &x : r) {
      if (x.passed)
        v.passed++;
      else
        v.failed++;
    }
    v.approved = (v.failed == 0 && v.passed == KPICount());
    return v;
  }
};

} // namespace Engine
} // namespace ExplorerLens
