// LTSBuildValidator.cpp — LTS Freeze Validator
// Copyright (c) 2026 ExplorerLens Project
//
#include "LTSBuildValidator.h"

namespace ExplorerLens {
namespace Engine {

LTSValidationReport LTSBuildValidator::Validate() const
{
    LTSValidationReport report;
    report.passed = 0;
    report.failed = 0;

    auto add = [&](const char* name, bool pass, const char* detail) {
        LTSGateCheck c;
        c.name   = name;
        c.status = pass ? LTSGateStatus::Pass : LTSGateStatus::Fail;
        c.detail = detail;
        report.checks.push_back(c);
        if (pass) ++report.passed; else ++report.failed;
    };

    // 1. Test count
    add("TestCount",
        m_testCount >= m_minTestCount,
        m_testCount >= m_minTestCount
            ? "Sufficient test coverage"
            : "Below minimum required test count");

    // 2. Decoder count
    add("DecoderCount",
        m_decoderCount >= m_minDecoderCount,
        m_decoderCount >= m_minDecoderCount
            ? "All format decoders present"
            : "Missing decoders — below minimum");

    // 3. Zero-warning build
    add("ZeroWarnings",
        m_buildWarnings == 0,
        m_buildWarnings == 0 ? "0 warnings" : "Build has warnings");

    // 4. Peak memory
    add("PeakMemory",
        m_maxPeakMemMB == 0.0 || m_peakMemMB <= m_maxPeakMemMB,
        m_peakMemMB <= m_maxPeakMemMB
            ? "Within memory budget"
            : "Peak memory exceeds LTS target");

    // 5. Code coverage
    add("CodeCoverage",
        m_coveragePct >= m_minCoveragePct,
        m_coveragePct >= m_minCoveragePct
            ? "Coverage target met"
            : "Insufficient code coverage for LTS stamp");

    // 6. Soak test
    add("SoakTest",
        m_soakTestPassed,
        m_soakTestPassed ? "48-hour soak test passed" : "Soak test not passed or not run");

    report.ltsStampIssued = (report.failed == 0);
    return report;
}

std::string LTSValidationReport::Summary() const
{
    std::string s;
    s += "LTS Validation: ";
    s += ltsStampIssued ? "ISSUED" : "BLOCKED";
    s += " (pass=" + std::to_string(passed) +
         " fail=" + std::to_string(failed) + ")";
    return s;
}

} // namespace Engine
} // namespace ExplorerLens
