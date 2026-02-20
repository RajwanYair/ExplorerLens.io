// =============================================================================
// ReleaseGateV14.cpp — Sprint 244: v10.4 Release Quality Gate
// DarkThumbs Engine — Utils Module
// =============================================================================

#include "ReleaseGateV14.h"

namespace DarkThumbs {

ReleaseGateV14::ReleaseGateV14() {
    InitializeDefaults();
}

void ReleaseGateV14::InitializeDefaults() {
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateKPIV14::Count); ++i) {
        m_results[i].kpi = static_cast<GateKPIV14>(i);
        m_results[i].passed = false;
        m_results[i].value = 0.0;
        m_results[i].threshold = 0.0;
    }
}

void ReleaseGateV14::SetKPIResult(GateKPIV14 kpi, bool passed, double value) {
    uint32_t idx = static_cast<uint32_t>(kpi);
    if (idx < static_cast<uint32_t>(GateKPIV14::Count)) {
        m_results[idx].passed = passed;
        m_results[idx].value = value;
    }
}

KPIV14Result ReleaseGateV14::GetKPIResult(GateKPIV14 kpi) const {
    uint32_t idx = static_cast<uint32_t>(kpi);
    if (idx < static_cast<uint32_t>(GateKPIV14::Count)) return m_results[idx];
    KPIV14Result empty;
    return empty;
}

bool ReleaseGateV14::Evaluate() const {
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateKPIV14::Count); ++i) {
        if (!m_results[i].passed) return false;
    }
    return true;
}

bool ReleaseGateV14::IsApproved() const {
    return Evaluate();
}

uint32_t ReleaseGateV14::GetPassedCount() const {
    uint32_t count = 0;
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateKPIV14::Count); ++i) {
        if (m_results[i].passed) count++;
    }
    return count;
}

uint32_t ReleaseGateV14::GetFailedCount() const {
    return static_cast<uint32_t>(GateKPIV14::Count) - GetPassedCount();
}

std::vector<KPIV14Result> ReleaseGateV14::GetAllResults() const {
    std::vector<KPIV14Result> results;
    for (uint32_t i = 0; i < static_cast<uint32_t>(GateKPIV14::Count); ++i) {
        results.push_back(m_results[i]);
    }
    return results;
}

const wchar_t* ReleaseGateV14::GetKPIName(GateKPIV14 kpi) {
    switch (kpi) {
        case GateKPIV14::BuildClean:         return L"Build Clean";
        case GateKPIV14::TestPass:           return L"Test Pass";
        case GateKPIV14::CodeCoverage:       return L"Code Coverage";
        case GateKPIV14::PerformanceTarget:  return L"Performance Target";
        case GateKPIV14::MemoryBudget:       return L"Memory Budget";
        case GateKPIV14::BinarySize:         return L"Binary Size";
        case GateKPIV14::DocumentationSync:  return L"Documentation Sync";
        case GateKPIV14::APIStability:       return L"API Stability";
        case GateKPIV14::SecurityScan:       return L"Security Scan";
        case GateKPIV14::PluginCompat:       return L"Plugin Compat";
        case GateKPIV14::ARM64Validation:    return L"ARM64 Validation";
        case GateKPIV14::CachEfficiency:     return L"Cache Efficiency";
        case GateKPIV14::FormatCoverage:     return L"Format Coverage";
        case GateKPIV14::HashVerification:   return L"Hash Verification";
        case GateKPIV14::RegistryIntegrity:  return L"Registry Integrity";
        case GateKPIV14::RecoverySuccess:    return L"Recovery Success";
        case GateKPIV14::ResourcePoolHealth: return L"Resource Pool Health";
        case GateKPIV14::MetadataAccuracy:   return L"Metadata Accuracy";
        default:                             return L"Unknown";
    }
}

} // namespace DarkThumbs
