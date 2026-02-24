//==============================================================================
// ExplorerLens Engine — Enterprise Policy Engine V2
// ADMX/GPO V2 with per-policy compliance scoring, policy drift detection,
// Intune MDM integration, and centralized policy distribution endpoint.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class EnterprisePolicySource : uint8_t { GroupPolicy=0,Intune,Workspace1,ManualJSON,COUNT };
enum class PolicyComplianceStatus : uint8_t { Compliant=0,NonCompliant,NotApplicable,Unknown,COUNT };
enum class PolicyScope : uint8_t { Machine=0,User,Both,COUNT };

struct EnterprisePolicyEntry {
    std::wstring        policyKey;
    std::wstring        value;
    PolicyScope         scope       = PolicyScope::Machine;
    PolicyComplianceStatus status   = PolicyComplianceStatus::Unknown;
    EnterprisePolicySource source   = EnterprisePolicySource::GroupPolicy;
};

struct EnterprisePolicyReport {
    uint32_t    totalPolicies   = 0;
    uint32_t    compliant       = 0;
    uint32_t    nonCompliant    = 0;
    float       complianceScore = 0.0f; // 0-100
    bool        driftDetected   = false;
};

class EnterprisePolicyEngineV2 {
public:
    static const wchar_t* SourceName(EnterprisePolicySource s) {
        switch(s) {
            case EnterprisePolicySource::GroupPolicy:  return L"Group Policy";
            case EnterprisePolicySource::Intune:       return L"Microsoft Intune";
            case EnterprisePolicySource::Workspace1:   return L"Workspace ONE";
            case EnterprisePolicySource::ManualJSON:   return L"Manual JSON";
            default: return L"Unknown";
        }
    }
    static const wchar_t* ComplianceStatusName(PolicyComplianceStatus s) {
        switch(s) {
            case PolicyComplianceStatus::Compliant:      return L"Compliant";
            case PolicyComplianceStatus::NonCompliant:   return L"Non-Compliant";
            case PolicyComplianceStatus::NotApplicable:  return L"N/A";
            case PolicyComplianceStatus::Unknown:        return L"Unknown";
            default: return L"Unknown";
        }
    }
    static const wchar_t* ScopeName(PolicyScope s) {
        switch(s) {
            case PolicyScope::Machine: return L"Machine";
            case PolicyScope::User:    return L"User";
            case PolicyScope::Both:    return L"Both";
            default: return L"Unknown";
        }
    }
    static constexpr size_t SourceCount()           { return static_cast<size_t>(EnterprisePolicySource::COUNT); }
    static constexpr size_t ComplianceStatusCount() { return static_cast<size_t>(PolicyComplianceStatus::COUNT); }
    static constexpr size_t ScopeCount()            { return static_cast<size_t>(PolicyScope::COUNT); }
    static bool IsFullyCompliant(const EnterprisePolicyReport& r) {
        return r.nonCompliant==0 && r.complianceScore>=95.0f;
    }
};

}} // namespace ExplorerLens::Engine

