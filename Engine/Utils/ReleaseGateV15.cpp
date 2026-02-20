// =============================================================================
// ReleaseGateV15.cpp — Sprint 248: v10.5 Release Quality Gate (Final)
// DarkThumbs Engine — Utils Module
// =============================================================================

#include "ReleaseGateV15.h"

namespace DarkThumbs {

ReleaseGateV15::ReleaseGateV15() {
    InitializeDefaults();
}

void ReleaseGateV15::InitializeDefaults() {
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateKPIV15::Count); ++i) {
        m_results[i].kpi = static_cast<GateKPIV15>(i);
        m_results[i].passed = false;
        m_results[i].value = 0.0;
        m_results[i].threshold = 0.0;
    }
}

void ReleaseGateV15::SetKPIResult(GateKPIV15 kpi, bool passed, double value) {
    uint32_t idx = static_cast<uint32_t>(kpi);
    if (idx < static_cast<uint32_t>(GateKPIV15::Count)) {
        m_results[idx].passed = passed;
        m_results[idx].value = value;
    }
}

KPIV15Result ReleaseGateV15::GetKPIResult(GateKPIV15 kpi) const {
    uint32_t idx = static_cast<uint32_t>(kpi);
    if (idx < static_cast<uint32_t>(GateKPIV15::Count)) return m_results[idx];
    KPIV15Result empty;
    return empty;
}

bool ReleaseGateV15::Evaluate() const {
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateKPIV15::Count); ++i) {
        if (!m_results[i].passed) return false;
    }
    return true;
}

bool ReleaseGateV15::IsApproved() const {
    return Evaluate();
}

uint32_t ReleaseGateV15::GetPassedCount() const {
    uint32_t count = 0;
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateKPIV15::Count); ++i) {
        if (m_results[i].passed) count++;
    }
    return count;
}

uint32_t ReleaseGateV15::GetFailedCount() const {
    return static_cast<uint32_t>(GateKPIV15::Count) - GetPassedCount();
}

std::vector<KPIV15Result> ReleaseGateV15::GetAllResults() const {
    std::vector<KPIV15Result> results;
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateKPIV15::Count); ++i) {
        results.push_back(m_results[i]);
    }
    return results;
}

const wchar_t* ReleaseGateV15::GetKPIName(GateKPIV15 kpi) {
    switch (kpi) {
        case GateKPIV15::BuildClean:         return L"Build Clean";
        case GateKPIV15::TestPass:           return L"Test Pass";
        case GateKPIV15::CodeCoverage:       return L"Code Coverage";
        case GateKPIV15::PerformanceTarget:  return L"Performance Target";
        case GateKPIV15::MemoryBudget:       return L"Memory Budget";
        case GateKPIV15::BinarySize:         return L"Binary Size";
        case GateKPIV15::DocumentationSync:  return L"Documentation Sync";
        case GateKPIV15::APIStability:       return L"API Stability";
        case GateKPIV15::SecurityScan:       return L"Security Scan";
        case GateKPIV15::PluginCompat:       return L"Plugin Compat";
        case GateKPIV15::ARM64Validation:    return L"ARM64 Validation";
        case GateKPIV15::CacheEfficiency:    return L"Cache Efficiency";
        case GateKPIV15::FormatCoverage:     return L"Format Coverage";
        case GateKPIV15::HashVerification:   return L"Hash Verification";
        case GateKPIV15::RegistryIntegrity:  return L"Registry Integrity";
        case GateKPIV15::RecoverySuccess:    return L"Recovery Success";
        case GateKPIV15::ResourcePoolHealth: return L"Resource Pool Health";
        case GateKPIV15::MetadataAccuracy:   return L"Metadata Accuracy";
        case GateKPIV15::ContentIndexHealth: return L"Content Index Health";
        case GateKPIV15::ConfigMigration:    return L"Config Migration";
        default:                             return L"Unknown";
    }
}

} // namespace DarkThumbs
