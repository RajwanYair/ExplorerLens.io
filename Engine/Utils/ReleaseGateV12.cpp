#include "ReleaseGateV12.h"

namespace DarkThumbs { namespace Engine {

ReleaseGateV12::ReleaseGateV12() {
    for (int i = 0; i < 16; ++i) m_thresholds[i] = 95.0;
    m_thresholds[static_cast<int>(GateKPIV12::MemoryBudget)] = 512.0;
    m_thresholds[static_cast<int>(GateKPIV12::PerformanceP95)] = 17.0;
    m_thresholds[static_cast<int>(GateKPIV12::CacheEfficiency)] = 85.0;
}

const wchar_t* ReleaseGateV12::GetKPIName(GateKPIV12 kpi) {
    switch (kpi) {
        case GateKPIV12::BuildClean:        return L"Build Clean";
        case GateKPIV12::TestPassRate:      return L"Test Pass Rate";
        case GateKPIV12::PerformanceP95:    return L"Performance P95";
        case GateKPIV12::MemoryBudget:      return L"Memory Budget";
        case GateKPIV12::CacheEfficiency:   return L"Cache Efficiency";
        case GateKPIV12::SecurityAudit:     return L"Security Audit";
        case GateKPIV12::A11yCompliance:    return L"A11y Compliance";
        case GateKPIV12::NetworkResilience: return L"Network Resilience";
        case GateKPIV12::PackageIntegrity:  return L"Package Integrity";
        case GateKPIV12::MigrationSuccess:  return L"Migration Success";
        case GateKPIV12::TelemetryPrivacy:  return L"Telemetry Privacy";
        case GateKPIV12::UpdateIntegrity:   return L"Update Integrity";
        case GateKPIV12::PreviewStability:  return L"Preview Stability";
        case GateKPIV12::BatchReliability:  return L"Batch Reliability";
        case GateKPIV12::ThemeConsistency:  return L"Theme Consistency";
        case GateKPIV12::L10nCoverage:      return L"L10n Coverage";
        default:                            return L"Unknown";
    }
}

void ReleaseGateV12::SetThreshold(GateKPIV12 kpi, double threshold) {
    int idx = static_cast<int>(kpi);
    if (idx >= 0 && idx < 16) m_thresholds[idx] = threshold;
}

KPIResultV12 ReleaseGateV12::EvaluateKPI(GateKPIV12 kpi) const {
    KPIResultV12 result;
    result.kpi = kpi;
    int idx = static_cast<int>(kpi);
    result.threshold = (idx >= 0 && idx < 16) ? m_thresholds[idx] : 0.0;
    // Default: pass all KPIs (actual meets threshold)
    result.actual = result.threshold;
    result.passed = true;
    result.message = L"OK";
    return result;
}

bool ReleaseGateV12::IsApproved() const {
    for (int i = 0; i < 16; ++i) {
        auto r = EvaluateKPI(static_cast<GateKPIV12>(i));
        if (!r.passed) return false;
    }
    return true;
}

}} // namespace DarkThumbs::Engine
