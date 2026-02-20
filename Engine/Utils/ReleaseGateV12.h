#pragma once
// Sprint 234: Release Gate V12 — v10.2 release quality gate with 16 KPIs
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// v10.2 KPI dimensions (16 total)
enum class GateKPIV12 : uint32_t {
    BuildClean        = 0,
    TestPassRate      = 1,
    PerformanceP95    = 2,
    MemoryBudget      = 3,
    CacheEfficiency   = 4,
    SecurityAudit     = 5,
    A11yCompliance    = 6,
    NetworkResilience = 7,
    PackageIntegrity  = 8,
    MigrationSuccess  = 9,
    TelemetryPrivacy  = 10,
    UpdateIntegrity   = 11,
    PreviewStability  = 12,
    BatchReliability  = 13,
    ThemeConsistency  = 14,
    L10nCoverage      = 15,
    COUNT             = 16
};

struct KPIResultV12 {
    GateKPIV12 kpi      = GateKPIV12::BuildClean;
    bool       passed   = false;
    double     actual   = 0.0;
    double     threshold = 0.0;
    std::wstring message;
};

class ReleaseGateV12 {
public:
    ReleaseGateV12();

    static const wchar_t* GetKPIName(GateKPIV12 kpi);
    static uint32_t GetKPICount() { return static_cast<uint32_t>(GateKPIV12::COUNT); }

    void SetThreshold(GateKPIV12 kpi, double threshold);
    KPIResultV12 EvaluateKPI(GateKPIV12 kpi) const;
    bool IsApproved() const;
    std::wstring GetVersion() const { return L"10.2.0"; }

private:
    double m_thresholds[16];
};

}} // namespace DarkThumbs::Engine
