#pragma once
// =============================================================================
// ReleaseGateV14.h — Sprint 244: v10.4 Release Quality Gate
// DarkThumbs Engine — Utils Module
// =============================================================================

#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs {

/// v10.4 KPI dimensions (18 total)
enum class GateKPIV14 : uint32_t {
    BuildClean          = 0,
    TestPass            = 1,
    CodeCoverage        = 2,
    PerformanceTarget   = 3,
    MemoryBudget        = 4,
    BinarySize          = 5,
    DocumentationSync   = 6,
    APIStability        = 7,
    SecurityScan        = 8,
    PluginCompat        = 9,
    ARM64Validation     = 10,
    CachEfficiency      = 11,
    FormatCoverage      = 12,
    HashVerification    = 13,
    RegistryIntegrity   = 14,
    RecoverySuccess     = 15,
    ResourcePoolHealth  = 16,   ///< NEW — pool hit rate & leak check
    MetadataAccuracy    = 17,   ///< NEW — metadata extraction correctness
    Count               = 18
};

/// KPI evaluation result
struct KPIV14Result {
    GateKPIV14  kpi         = GateKPIV14::BuildClean;
    bool        passed      = false;
    double      value       = 0.0;
    double      threshold   = 0.0;
    std::wstring details;
};

/// ReleaseGateV14 — v10.4 release quality gate with 18 KPIs
class ReleaseGateV14 {
public:
    ReleaseGateV14();

    // Evaluation
    void SetKPIResult(GateKPIV14 kpi, bool passed, double value = 0.0);
    KPIV14Result GetKPIResult(GateKPIV14 kpi) const;
    bool Evaluate() const;
    bool IsApproved() const;

    // Reporting
    uint32_t GetPassedCount() const;
    uint32_t GetFailedCount() const;
    std::vector<KPIV14Result> GetAllResults() const;
    std::wstring GetVersion() const { return L"10.4.0"; }

    // Static helpers
    static const wchar_t* GetKPIName(GateKPIV14 kpi);
    static constexpr uint32_t GetKPICount() { return static_cast<uint32_t>(GateKPIV14::Count); }

private:
    KPIV14Result m_results[static_cast<uint32_t>(GateKPIV14::Count)];
    void InitializeDefaults();
};

} // namespace DarkThumbs
