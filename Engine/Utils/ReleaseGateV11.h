#pragma once
//==============================================================================
// ReleaseGateV11 — Sprint 224
// v10.1.0 release quality gate with 15 KPI dimensions.
// Evaluates build health, test coverage, performance, security, and docs.
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class GateKPIV11 : uint8_t {
    BuildClean      = 0,
    TestPassRate     = 1,
    TestCoverage     = 2,
    PerfRegression   = 3,
    MemoryLeaks      = 4,
    SecurityAudit    = 5,
    CodeSigning      = 6,
    DocSync          = 7,
    PluginCompat     = 8,
    ARM64Build       = 9,
    NetworkTests     = 10,
    AccessibilityAudit = 11,
    PackagingValid   = 12,
    MigrationTest    = 13,
    RegressionSuite  = 14,
    KPICount         = 15
};

struct KPIResultV11 {
    GateKPIV11 kpi = GateKPIV11::BuildClean;
    bool passed = false;
    double value = 0.0;
    double threshold = 0.0;
    std::wstring details;
};

struct ReleaseGateResultV11 {
    bool approved = false;
    uint32_t kpisEvaluated = 0;
    uint32_t kpisPassed = 0;
    uint32_t kpisFailed = 0;
    double evaluationTimeMs = 0.0;
    std::vector<KPIResultV11> results;
    std::wstring releaseVersion;
};

class ReleaseGateV11 {
public:
    ReleaseGateV11();

    ReleaseGateResultV11 Evaluate(const std::wstring& version = L"v10.1.0");
    KPIResultV11 EvaluateKPI(GateKPIV11 kpi) const;

    void SetThreshold(GateKPIV11 kpi, double threshold);
    double GetThreshold(GateKPIV11 kpi) const;

    static const wchar_t* GetKPIName(GateKPIV11 kpi);
    static uint32_t GetKPICount() { return static_cast<uint32_t>(GateKPIV11::KPICount); }

private:
    double m_thresholds[static_cast<uint32_t>(GateKPIV11::KPICount)];
    void InitializeThresholds();
};

}} // namespace ExplorerLens::Engine
