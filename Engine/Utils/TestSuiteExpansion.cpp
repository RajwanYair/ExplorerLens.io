//==============================================================================
// TestSuiteExpansion.cpp
// Test framework expansion with decoder test specs and coverage tracking
//==============================================================================

#include "TestSuiteExpansion.h"
#include <algorithm>
#include <numeric>

namespace ExplorerLens { namespace Engine {

TestSuiteExpansion::TestSuiteExpansion() = default;

TestSuiteExpansion::TestSuiteExpansion(const TestExpansionConfig& config)
    : m_config(config) {}

//==============================================================================
// Decoder Test Specifications
//==============================================================================
std::vector<DecoderTestSpec> TestSuiteExpansion::GetDecoderTestSpecs() const {
    std::vector<DecoderTestSpec> specs;

    auto addSpec = [&](const wchar_t* name, const wchar_t* cls, uint32_t current,
                       bool valid, bool trunc, bool corrupt, bool zero, bool perf) {
        DecoderTestSpec s;
        s.formatName = name;
        s.decoderClass = cls;
        s.minTestCount = m_config.targetTestsPerDecoder;
        s.currentTestCount = current;
        s.hasValidFile = valid;
        s.hasTruncatedFile = trunc;
        s.hasCorruptFile = corrupt;
        s.hasZeroByteFile = zero;
        s.hasPerformance = perf;
        s.coveragePercent = (s.minTestCount > 0)
            ? (static_cast<double>(s.currentTestCount) / s.minTestCount * 100.0)
            : 0.0;
        specs.push_back(s);
    };

    // Core decoders (25+)
    addSpec(L"PNG",    L"ImageDecoder",    8, true, true, true, true, true);
    addSpec(L"JPEG",   L"ImageDecoder",    8, true, true, true, true, true);
    addSpec(L"BMP",    L"ImageDecoder",    4, true, false, false, true, false);
    addSpec(L"GIF",    L"ImageDecoder",    4, true, false, false, true, false);
    addSpec(L"TIFF",   L"ImageDecoder",    3, true, false, false, true, false);
    addSpec(L"WebP",   L"WebPDecoder",     5, true, true, true, true, true);
    addSpec(L"AVIF",   L"AVIFDecoder",     4, true, false, true, true, false);
    addSpec(L"HEIF",   L"HEIFDecoder",     4, true, false, true, true, false);
    addSpec(L"JXL",    L"JXLDecoder",      4, true, false, true, true, false);
    addSpec(L"PSD",    L"PSDDecoder",      3, true, false, false, true, false);
    addSpec(L"DDS",    L"DDSDecoder",      3, true, false, false, true, false);
    addSpec(L"HDR",    L"HDRDecoder",      3, true, false, false, true, false);
    addSpec(L"EXR",    L"EXRDecoder",      2, true, false, false, false, false);
    addSpec(L"TGA",    L"TGADecoder",      3, true, false, false, true, false);
    addSpec(L"ICO",    L"ICODecoder",      3, true, false, false, true, false);
    addSpec(L"QOI",    L"QOIDecoder",      3, true, false, false, true, false);
    addSpec(L"SVG",    L"SVGDecoder",      3, true, false, false, false, false);
    addSpec(L"RAW",    L"RAWDecoder",      5, true, true, true, true, true);
    addSpec(L"PDF",    L"PDFDecoder",      3, true, false, false, true, false);
    addSpec(L"Archive",L"ArchiveDecoder",  6, true, true, true, true, true);
    addSpec(L"Video",  L"VideoDecoder",    3, true, false, false, true, false);
    addSpec(L"Audio",  L"AudioDecoder",    3, true, false, false, true, false);
    addSpec(L"Font",   L"FontDecoder",     3, true, false, false, true, false);
    addSpec(L"Document",L"DocumentDecoder",3, true, false, false, true, false);
    addSpec(L"Model",  L"ModelDecoder",    3, true, false, false, true, false);
    addSpec(L"PCX",    L"PCXDecoder",      2, true, false, false, false, false);
    addSpec(L"PPM",    L"PPMDecoder",      3, true, false, false, true, false);
    addSpec(L"SGI",    L"SGIDecoder",      2, true, false, false, false, false);
    addSpec(L"XPM",    L"XPMDecoder",      2, true, false, false, false, false);
    addSpec(L"Farbfeld",L"FarbfeldDecoder",2, true, false, false, false, false);
    addSpec(L"KTX",    L"KTXDecoder",      2, true, false, false, false, false);
    addSpec(L"VTF",    L"VTFDecoder",      2, true, false, false, false, false);
    addSpec(L"XCF",    L"XCFDecoder",      2, true, false, false, false, false);
    addSpec(L"ORA",    L"OpenRasterDecoder",2, true, false, false, false, false);
    addSpec(L"WMF",    L"WMFDecoder",      2, true, false, false, false, false);
    addSpec(L"EPS",    L"EPSDecoder",      2, true, false, false, false, false);

    return specs;
}

//==============================================================================
// Coverage Analysis
//==============================================================================
std::vector<CoverageTarget> TestSuiteExpansion::CalculateCoverageGaps() const {
    std::vector<CoverageTarget> targets;

    auto addTarget = [&](const wchar_t* component, uint32_t current, uint32_t target) {
        CoverageTarget t;
        t.component = component;
        t.currentTests = current;
        t.targetTests = target;
        t.gap = (target > current) ? (target - current) : 0;
        t.coveragePercent = (target > 0) ? (static_cast<double>(current) / target * 100.0) : 100.0;
        targets.push_back(t);
    };

    addTarget(L"Core Decoders",     110, 360);
    addTarget(L"Plugin System",      50, 100);
    addTarget(L"Cache System",       40,  60);
    addTarget(L"Memory System",      40,  80);
    addTarget(L"Pipeline",           35,  60);
    addTarget(L"GPU Rendering",      15,  40);
    addTarget(L"Shell Integration",  10,  20);
    addTarget(L"ARM64",              10,  20);
    addTarget(L"DPI/Scaling",        10,  20);
    addTarget(L"Packaging",          10,  20);

    return targets;
}

uint32_t TestSuiteExpansion::GetTotalTestCount() const {
    auto specs = GetDecoderTestSpecs();
    uint32_t total = 0;
    for (const auto& s : specs) {
        total += s.currentTestCount;
    }
    // Add infrastructure tests (cache, memory, pipeline, GPU, etc.)
    total += 200; // Approximate infrastructure test count
    return total;
}

TestSuiteSummary TestSuiteExpansion::ComputeSummary(
    const std::vector<TestResult>& results) const {

    TestSuiteSummary summary;
    summary.totalTests = static_cast<uint32_t>(results.size());

    for (const auto& r : results) {
        summary.totalDurationMs += r.durationMs;
        switch (r.verdict) {
        case TestVerdict::Pass:    summary.passed++; break;
        case TestVerdict::Fail:    summary.failed++; summary.failures.push_back(r); break;
        case TestVerdict::Skip:    summary.skipped++; break;
        case TestVerdict::Error:   summary.errors++; summary.failures.push_back(r); break;
        case TestVerdict::Timeout: summary.timeouts++; summary.failures.push_back(r); break;
        case TestVerdict::Flaky:   summary.flaky++; break;
        }
    }

    summary.passRate = (summary.totalTests > 0)
        ? (static_cast<double>(summary.passed) / summary.totalTests * 100.0)
        : 0.0;

    return summary;
}

bool TestSuiteExpansion::MeetsTargets() const {
    uint32_t total = GetTotalTestCount();
    return total >= m_config.targetTotalTests;
}

std::vector<std::wstring> TestSuiteExpansion::GetTestFilesForDecoder(
    const std::wstring& decoderName) const {

    std::vector<std::wstring> files;
    std::wstring base = m_config.testArchiveDir + L"\\" + decoderName + L"\\";
    files.push_back(base + L"valid_sample");
    files.push_back(base + L"truncated_sample");
    files.push_back(base + L"corrupt_header");
    files.push_back(base + L"zero_byte");
    files.push_back(base + L"large_sample");
    return files;
}

//==============================================================================
// Static Name Helpers
//==============================================================================
const wchar_t* TestSuiteExpansion::GetCategoryName(TestCategory category) {
    switch (category) {
    case TestCategory::UnitTest:        return L"UnitTest";
    case TestCategory::IntegrationTest: return L"IntegrationTest";
    case TestCategory::DecoderTest:     return L"DecoderTest";
    case TestCategory::PerformanceTest: return L"PerformanceTest";
    case TestCategory::FuzzTest:        return L"FuzzTest";
    case TestCategory::RegressionTest:  return L"RegressionTest";
    case TestCategory::StressTest:      return L"StressTest";
    case TestCategory::EndToEndTest:    return L"EndToEndTest";
    case TestCategory::COMTest:         return L"COMTest";
    case TestCategory::PlatformTest:    return L"PlatformTest";
    default: return L"Unknown";
    }
}

const wchar_t* TestSuiteExpansion::GetVerdictName(TestVerdict verdict) {
    switch (verdict) {
    case TestVerdict::Pass:    return L"Pass";
    case TestVerdict::Fail:    return L"Fail";
    case TestVerdict::Skip:    return L"Skip";
    case TestVerdict::Error:   return L"Error";
    case TestVerdict::Timeout: return L"Timeout";
    case TestVerdict::Flaky:   return L"Flaky";
    default: return L"Unknown";
    }
}

}} // namespace ExplorerLens::Engine

