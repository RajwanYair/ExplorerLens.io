//==============================================================================
// DarkThumbs Engine — Sprint 339: Release Gate V30
// Platform Hardening phase gate — validates all Phase 8 KPIs before
// advancing to Performance Summit (Phase 9).
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class GateV30KPI : uint8_t {
    Windows12CompatLayer        = 0,  // Win12 adaptive rendering verified
    ARM64SIMDAcceleration       = 1,  // NEON/SVE2 decode speedup ≥ 1.5×
    WinRTBootstrapSuccess       = 2,  // AppSDK 2.0 bootstrap in unpackaged mode
    MSIXPackagingValid          = 3,  // MSIX package passes Store certification
    SilentInstallComplete       = 4,  // Per-machine silent install passes UAC
    RollbackSmokeTest           = 5,  // Snapshot rollback fully restores state
    StagedRolloutManifestPublished = 6, // 10%/50%/100% rings configured
    ARM64CIGreenBuild           = 7,  // ARM64 CI pipeline: 0 errors, 0 warnings
    Windows12ShellRegistration  = 8,  // Full shell registration on Win12 preview
    PackagingDocUpdated         = 9,  // Packaging docs cite AppSDK 2.0 + Win12
    COUNT
};

struct ReleaseGateV30Result {
    bool    allKPIsPass         = false;
    uint8_t kpiPassCount        = 0;
    uint8_t kpiTotalCount       = static_cast<uint8_t>(GateV30KPI::COUNT);
    float   gateScore           = 0.0f;
    bool    advanceRecommended  = false;
};

class ReleaseGateV30 {
public:
    static constexpr size_t KPICount() { return static_cast<size_t>(GateV30KPI::COUNT); }
    static const wchar_t* KPIName(GateV30KPI k) {
        switch(k) {
            case GateV30KPI::Windows12CompatLayer:           return L"Windows 12 Compat Layer";
            case GateV30KPI::ARM64SIMDAcceleration:          return L"ARM64 SIMD Speedup ≥ 1.5×";
            case GateV30KPI::WinRTBootstrapSuccess:          return L"WinRT AppSDK 2.0 Bootstrap";
            case GateV30KPI::MSIXPackagingValid:             return L"MSIX Store Certification";
            case GateV30KPI::SilentInstallComplete:          return L"Silent Per-Machine Install";
            case GateV30KPI::RollbackSmokeTest:              return L"Rollback Snapshot Verified";
            case GateV30KPI::StagedRolloutManifestPublished: return L"Staged Rollout Manifest";
            case GateV30KPI::ARM64CIGreenBuild:              return L"ARM64 CI Green Build";
            case GateV30KPI::Windows12ShellRegistration:     return L"Win12 Shell Registration";
            case GateV30KPI::PackagingDocUpdated:            return L"Packaging Docs Updated";
            default: return L"Unknown KPI";
        }
    }
    static ReleaseGateV30Result Evaluate(bool kpiResults[]) {
        ReleaseGateV30Result r;
        for(size_t i=0;i<KPICount();++i) if(kpiResults[i]) ++r.kpiPassCount;
        r.gateScore = (static_cast<float>(r.kpiPassCount)/static_cast<float>(KPICount()))*100.0f;
        r.allKPIsPass = (r.kpiPassCount==r.kpiTotalCount);
        r.advanceRecommended = r.gateScore >= 90.0f;
        return r;
    }
};

}} // namespace DarkThumbs::Engine
