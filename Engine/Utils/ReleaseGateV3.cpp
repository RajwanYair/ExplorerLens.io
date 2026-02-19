//==============================================================================
// ReleaseGateV3 — Sprint 198
// v9.2 Release gate implementation
//==============================================================================

#include "ReleaseGateV3.h"
#include <sstream>

namespace DarkThumbs { namespace Engine {

ReleaseGateV3::ReleaseGateV3()
    : m_thresholds(ForV92()) {}

ReleaseGateV3::ReleaseGateV3(const ReleaseThresholdsV92& thresholds)
    : m_thresholds(thresholds) {}

//------------------------------------------------------------------------------
ReleaseThresholdsV92 ReleaseGateV3::ForV92() {
    ReleaseThresholdsV92 t;
    t.minTestCount = 500;
    t.minTestPassRate = 99.5;
    t.maxSingleDecodeMs = 20.0;
    t.minBatchThroughput = 200.0;
    t.maxCacheHitMs = 5.0;
    t.minDecoderCount = 25;
    t.minShellExtensions = 90;
    t.minCodeCoverage = 70.0;
    t.maxBuildWarnings = 0;
    t.maxBuildErrors = 0;
    t.minMalformedResilience = 95.0;
    t.requireARM64CI = true;
    t.requireMSIXPackage = true;
    t.requireHighDPI = true;
    t.minDocArtifacts = 10;
    return t;
}

//------------------------------------------------------------------------------
void ReleaseGateV3::AddMeasurement(const KPIMeasurement& m) {
    m_measurements.push_back(m);
}

void ReleaseGateV3::AddPlatform(const PlatformValidation& p) {
    m_platforms.push_back(p);
}

//------------------------------------------------------------------------------
GateVerdict ReleaseGateV3::ComputeVerdict(uint32_t passed, uint32_t failed,
                                           bool hasBlockers) const {
    if (hasBlockers) return GateVerdict::Blocked;
    if (failed == 0) return GateVerdict::Pass;
    double passRate = (passed + failed > 0)
        ? (100.0 * passed / (passed + failed)) : 0.0;
    if (passRate >= 90.0) return GateVerdict::ConditionalPass;
    return GateVerdict::Fail;
}

//------------------------------------------------------------------------------
ReleaseGateResult ReleaseGateV3::Evaluate() const {
    ReleaseGateResult result;
    result.version = L"v9.2.0";
    result.measurements = m_measurements;
    result.platforms = m_platforms;

    uint32_t passed = 0, failed = 0;
    bool hasBlockers = false;

    for (const auto& m : m_measurements) {
        result.totalKPIs++;
        if (m.passed) {
            passed++;
        } else {
            failed++;
            if (m.dimension == ReleaseKPIDimension::BuildQuality ||
                m.dimension == ReleaseKPIDimension::Security) {
                hasBlockers = true;
                result.blockers.push_back(m.name + L" FAILED: " + m.notes);
            } else {
                result.warnings.push_back(m.name + L": " + m.notes);
            }
        }
    }

    result.passedKPIs = passed;
    result.failedKPIs = failed;
    result.overallScore = (result.totalKPIs > 0)
        ? (100.0 * passed / result.totalKPIs) : 0.0;
    result.verdict = ComputeVerdict(passed, failed, hasBlockers);
    return result;
}

//------------------------------------------------------------------------------
std::wstring ReleaseGateV3::GenerateReleaseNotes(const ReleaseGateResult& result) const {
    std::wstringstream ss;
    ss << L"# DarkThumbs " << result.version << L" Release Notes\n\n";
    ss << L"## Release Gate: " << GetVerdictName(result.verdict) << L"\n";
    ss << L"- Overall Score: " << result.overallScore << L"%\n";
    ss << L"- KPIs Passed: " << result.passedKPIs << L"/" << result.totalKPIs << L"\n\n";

    if (!result.blockers.empty()) {
        ss << L"## Blockers\n";
        for (const auto& b : result.blockers)
            ss << L"- ❌ " << b << L"\n";
        ss << L"\n";
    }

    if (!result.warnings.empty()) {
        ss << L"## Warnings\n";
        for (const auto& w : result.warnings)
            ss << L"- ⚠️ " << w << L"\n";
        ss << L"\n";
    }

    ss << L"## Platform Coverage\n";
    for (const auto& p : result.platforms) {
        ss << L"- " << p.platform << L": "
           << (p.buildSucceeded ? L"BUILD OK" : L"BUILD FAIL")
           << L" | Tests: " << p.testsPassed << L"/" 
           << (p.testsPassed + p.testsFailed) << L"\n";
    }

    return ss.str();
}

//------------------------------------------------------------------------------
std::wstring ReleaseGateV3::GenerateChecklist() const {
    std::wstringstream ss;
    ss << L"# v9.2 Release Checklist\n\n";
    ss << L"- [ ] Zero warnings build (x64)\n";
    ss << L"- [ ] Zero warnings build (ARM64)\n";
    ss << L"- [ ] All tests pass (>500)\n";
    ss << L"- [ ] Performance targets met\n";
    ss << L"- [ ] Malformed input resilience >95%\n";
    ss << L"- [ ] MSIX package builds\n";
    ss << L"- [ ] MSI package builds\n";
    ss << L"- [ ] High-DPI validation\n";
    ss << L"- [ ] Documentation sync audit\n";
    ss << L"- [ ] CHANGELOG updated\n";
    ss << L"- [ ] Version strings normalized\n";
    ss << L"- [ ] Git tagged\n";
    return ss.str();
}

//------------------------------------------------------------------------------
const wchar_t* ReleaseGateV3::GetDimensionName(ReleaseKPIDimension dim) {
    switch (dim) {
        case ReleaseKPIDimension::BuildQuality:   return L"BuildQuality";
        case ReleaseKPIDimension::TestCoverage:   return L"TestCoverage";
        case ReleaseKPIDimension::Performance:    return L"Performance";
        case ReleaseKPIDimension::Stability:      return L"Stability";
        case ReleaseKPIDimension::Security:       return L"Security";
        case ReleaseKPIDimension::Compatibility:  return L"Compatibility";
        case ReleaseKPIDimension::Documentation:  return L"Documentation";
        case ReleaseKPIDimension::Packaging:      return L"Packaging";
        case ReleaseKPIDimension::Observability:  return L"Observability";
        default:                                   return L"Unknown";
    }
}

const wchar_t* ReleaseGateV3::GetVerdictName(GateVerdict verdict) {
    switch (verdict) {
        case GateVerdict::Pass:            return L"Pass";
        case GateVerdict::ConditionalPass: return L"ConditionalPass";
        case GateVerdict::Fail:            return L"Fail";
        case GateVerdict::Blocked:         return L"Blocked";
        default:                            return L"Unknown";
    }
}

}} // namespace DarkThumbs::Engine
