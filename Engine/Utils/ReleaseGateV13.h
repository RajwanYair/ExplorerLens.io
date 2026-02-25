#pragma once
// ReleaseGateV13.h — Release Gate V13 — v10.3 release quality gate with 17 KPIs
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class GateKPIV13 : uint32_t {
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
    HashVerification  = 14,
    RegistryIntegrity = 15,
    RecoverySuccess   = 16,
    COUNT             = 17
};

struct KPIResultV13 {
    GateKPIV13 kpi    = GateKPIV13::BuildClean;
    bool       passed = false;
    double     actual = 0.0;
    double     threshold = 0.0;
    std::wstring message;
};

class ReleaseGateV13 {
public:
    ReleaseGateV13();

    static const wchar_t* GetKPIName(GateKPIV13 kpi);
    static uint32_t GetKPICount() { return static_cast<uint32_t>(GateKPIV13::COUNT); }

    void SetThreshold(GateKPIV13 kpi, double threshold);
    KPIResultV13 EvaluateKPI(GateKPIV13 kpi) const;
    bool IsApproved() const;
    std::wstring GetVersion() const { return L"10.3.0"; }

private:
    double m_thresholds[17];
};

}} // namespace ExplorerLens::Engine
