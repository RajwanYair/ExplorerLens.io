//==============================================================================
// ExplorerLens.io Engine — Release Gate V29
// Enterprise & Cloud V2 phase gate — validates all Phase 7 KPIs before
// advancing to Platform Hardening (Phase 8).
//==============================================================================
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class GateV29KPI : uint8_t {
  EnterprisePolicyCompliance = 0, // ≥ 95% policy compliance score
  SharePointTeamsThumbnails = 1,  // Graph API auth + thumbnail generation
  MultiTenantCacheIsolation = 2,  // Strict namespace isolation verified
  ComplianceLogImmutability = 3,  // Audit log tamper-proof under all regs
  GDPRRetentionEnforced = 4,      // Retention ≤ 365 days enforced
  IntuneGPOPoliciesApplied = 5,   // Intune + Group Policy coexistence
  CloudSyncDeltaEnabled = 6,      // Delta sync active for SharePoint
  TenantQuotasEnforced = 7,       // No tenant exceeds allocated cache quota
  DSRRedactionComplete = 8,       // Data subject request redaction passes
  EnterpriseDocumentation = 9,    // Enterprise admin guide up-to-date
  CloudIntegrationTests = 10,     // 100% cloud integration tests pass
  COUNT
};

struct ReleaseGateV29Result {
  bool allKPIsPass = false;
  uint8_t kpiPassCount = 0;
  uint8_t kpiTotalCount = static_cast<uint8_t>(GateV29KPI::COUNT);
  float gateScore = 0.0f; // 0-100
  bool advanceRecommended = false;
};

class ReleaseGateV29 {
public:
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV29KPI::COUNT);
  }
  static const wchar_t *KPIName(GateV29KPI k) {
    switch (k) {
    case GateV29KPI::EnterprisePolicyCompliance:
      return L"Enterprise Policy Compliance ≥ 95%";
    case GateV29KPI::SharePointTeamsThumbnails:
      return L"SharePoint/Teams Thumbnail Generation";
    case GateV29KPI::MultiTenantCacheIsolation:
      return L"Multi-Tenant Cache Isolation";
    case GateV29KPI::ComplianceLogImmutability:
      return L"Compliance Log Immutability";
    case GateV29KPI::GDPRRetentionEnforced:
      return L"GDPR Retention Policy Enforced";
    case GateV29KPI::IntuneGPOPoliciesApplied:
      return L"Intune + GPO Coexistence";
    case GateV29KPI::CloudSyncDeltaEnabled:
      return L"Cloud Delta Sync Active";
    case GateV29KPI::TenantQuotasEnforced:
      return L"Tenant Cache Quotas Enforced";
    case GateV29KPI::DSRRedactionComplete:
      return L"DSR Redaction Complete";
    case GateV29KPI::EnterpriseDocumentation:
      return L"Enterprise Admin Documentation";
    case GateV29KPI::CloudIntegrationTests:
      return L"Cloud Integration Tests 100%";
    default:
      return L"Unknown KPI";
    }
  }
  static ReleaseGateV29Result Evaluate(bool kpiResults[]) {
    ReleaseGateV29Result r;
    for (size_t i = 0; i < KPICount(); ++i)
      if (kpiResults[i])
        ++r.kpiPassCount;
    r.gateScore =
        (static_cast<float>(r.kpiPassCount) / static_cast<float>(KPICount())) *
        100.0f;
    r.allKPIsPass = (r.kpiPassCount == r.kpiTotalCount);
    r.advanceRecommended = r.gateScore >= 90.0f;
    return r;
  }
};

} // namespace Engine
} // namespace ExplorerLens
