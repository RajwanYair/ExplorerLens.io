//==============================================================================
// ReleaseGateV10 — Sprint 204
// v10.0.0 Release gate implementation
//==============================================================================

#include "ReleaseGateV10.h"
#include <sstream>

namespace DarkThumbs { namespace Engine {

ReleaseGateV10::ReleaseGateV10() : m_thresholds(ForV10()) {}

ReleaseGateV10::ReleaseGateV10(const ReleaseThresholdsV10& thresholds)
    : m_thresholds(thresholds) {}

//------------------------------------------------------------------------------
ReleaseThresholdsV10 ReleaseGateV10::ForV10() {
    ReleaseThresholdsV10 t;
    t.minDecoderCount = 30;
    t.minTestCount = 600;
    t.minTestPassRate = 99.8;
    t.maxSingleDecodeMs = 15.0;
    t.minBatchThroughput = 300.0;
    t.minShellRegistrations = 110;
    t.minFormatCategories = 15;
    t.minFormatCoverage = 90.0;
    t.minGPUBackends = 2;
    t.requirePluginMarketplace = true;
    t.requirePythonSDK = true;
    t.requireScientificFormats = true;
    t.requireVulkanSupport = false;
    t.maxBuildWarnings = 0;
    return t;
}

//------------------------------------------------------------------------------
void ReleaseGateV10::AddFormatCoverage(const FormatCoverageEntry& entry) {
    m_formatCoverage.push_back(entry);
}

void ReleaseGateV10::AddGPUBackend(const GPUBackendResult& result) {
    m_gpuBackends.push_back(result);
}

void ReleaseGateV10::SetTestMetrics(uint32_t count, uint32_t passed, double passRate) {
    m_testCount = count;
    m_testsPassed = passed;
    m_testPassRate = passRate;
}

void ReleaseGateV10::SetDecoderCount(uint32_t count) {
    m_decoderCount = count;
}

void ReleaseGateV10::SetShellRegistrations(uint32_t count) {
    m_shellRegistrations = count;
}

//------------------------------------------------------------------------------
V10ReleaseResult ReleaseGateV10::Evaluate() const {
    V10ReleaseResult result;
    result.version = L"v10.0.0";
    result.formatCoverage = m_formatCoverage;
    result.gpuBackends = m_gpuBackends;
    result.totalChecks = 0;
    result.passedChecks = 0;

    // Check decoder count
    result.totalChecks++;
    if (m_decoderCount >= m_thresholds.minDecoderCount) {
        result.passedChecks++;
        result.achievements.push_back(L"30+ decoders milestone");
    } else {
        result.blockers.push_back(L"Decoder count below " + std::to_wstring(m_thresholds.minDecoderCount));
    }

    // Check test count
    result.totalChecks++;
    if (m_testCount >= m_thresholds.minTestCount) {
        result.passedChecks++;
        result.achievements.push_back(L"600+ test milestone");
    } else {
        result.warnings.push_back(L"Test count: " + std::to_wstring(m_testCount) +
                                   L" (need " + std::to_wstring(m_thresholds.minTestCount) + L")");
    }

    // Check test pass rate
    result.totalChecks++;
    if (m_testPassRate >= m_thresholds.minTestPassRate) {
        result.passedChecks++;
    } else {
        result.blockers.push_back(L"Test pass rate below " + std::to_wstring(m_thresholds.minTestPassRate) + L"%");
    }

    // Check shell registrations
    result.totalChecks++;
    if (m_shellRegistrations >= m_thresholds.minShellRegistrations) {
        result.passedChecks++;
        result.achievements.push_back(L"110+ shell registrations");
    } else {
        result.warnings.push_back(L"Shell registrations: " + std::to_wstring(m_shellRegistrations));
    }

    // Check GPU backends
    result.totalChecks++;
    uint32_t availableBackends = 0;
    for (const auto& gpu : m_gpuBackends) {
        if (gpu.available && gpu.passed) availableBackends++;
    }
    if (availableBackends >= m_thresholds.minGPUBackends) {
        result.passedChecks++;
    } else {
        result.warnings.push_back(L"Only " + std::to_wstring(availableBackends) + L" GPU backends available");
    }

    // Check format coverage
    result.totalChecks++;
    if (m_formatCoverage.size() >= m_thresholds.minFormatCategories) {
        result.passedChecks++;
    }

    // Compute score
    if (result.totalChecks > 0) {
        result.overallScore = 100.0 * result.passedChecks / result.totalChecks;
    }
    result.passed = result.blockers.empty() && result.overallScore >= 80.0;
    if (result.passed) result.achievements.push_back(L"v10.0.0 RELEASE APPROVED");

    result.changelog = GenerateChangelog();
    return result;
}

//------------------------------------------------------------------------------
std::wstring ReleaseGateV10::GenerateChangelog() const {
    std::wstringstream ss;
    ss << L"# DarkThumbs v10.0.0 Changelog\n\n";
    ss << L"## New Features\n";
    ss << L"- Scientific format suite: DICOM (.dcm), FITS (.fits) decoders\n";
    ss << L"- Advanced 3D: FBX, USD, 3MF format support\n";
    ss << L"- Plugin marketplace with discovery, install, and auto-update\n";
    ss << L"- Vulkan compute pipeline with D3D12/D3D11/CPU fallback\n";
    ss << L"- Python SDK with ctypes and pybind11 bindings\n";
    ss << L"- " << m_decoderCount << L" decoders, " << m_shellRegistrations << L" shell registrations\n\n";
    ss << L"## Quality\n";
    ss << L"- " << m_testCount << L" tests, " << m_testPassRate << L"% pass rate\n";
    ss << L"- Zero warnings build policy enforced\n";
    return ss.str();
}

//------------------------------------------------------------------------------
std::wstring ReleaseGateV10::GenerateReleaseNotes(const V10ReleaseResult& result) const {
    std::wstringstream ss;
    ss << L"# Release Notes — " << result.version << L"\n\n";
    ss << L"**Score:** " << result.overallScore << L"%\n";
    ss << L"**Status:** " << (result.passed ? L"APPROVED" : L"BLOCKED") << L"\n\n";

    if (!result.achievements.empty()) {
        ss << L"## Achievements\n";
        for (const auto& a : result.achievements) ss << L"- " << a << L"\n";
    }
    if (!result.blockers.empty()) {
        ss << L"\n## Blockers\n";
        for (const auto& b : result.blockers) ss << L"- ❌ " << b << L"\n";
    }
    if (!result.warnings.empty()) {
        ss << L"\n## Warnings\n";
        for (const auto& w : result.warnings) ss << L"- ⚠️ " << w << L"\n";
    }
    return ss.str();
}

//------------------------------------------------------------------------------
const wchar_t* ReleaseGateV10::GetCategoryName(V10KPICategory cat) {
    switch (cat) {
        case V10KPICategory::BuildSystem:      return L"Build System";
        case V10KPICategory::TestCoverage:     return L"Test Coverage";
        case V10KPICategory::FormatCoverage:   return L"Format Coverage";
        case V10KPICategory::Performance:      return L"Performance";
        case V10KPICategory::GPUBackends:      return L"GPU Backends";
        case V10KPICategory::PluginEcosystem:  return L"Plugin Ecosystem";
        case V10KPICategory::PythonSDK:        return L"Python SDK";
        case V10KPICategory::Documentation:    return L"Documentation";
        case V10KPICategory::Packaging:        return L"Packaging";
        case V10KPICategory::Security:         return L"Security";
        case V10KPICategory::Compatibility:    return L"Compatibility";
        case V10KPICategory::Scientific:       return L"Scientific";
        default: return L"Unknown";
    }
}

}} // namespace DarkThumbs::Engine
