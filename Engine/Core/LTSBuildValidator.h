// LTSBuildValidator.h — LTS Freeze Validator
// Copyright (c) 2026 ExplorerLens Project
//
// Validates that all v34.x quality gates are met before an LTS build stamp
// is issued. Checks minimum test count, decoder count, zero-warning policy,
// memory targets, and soak-test results. Emits a structured report.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class LTSValidatorStatus : uint8_t
{
    Pass = 0,
    Fail = 1,
};

struct LTSGateCheck
{
    std::string  name;
    LTSValidatorStatus status;
    std::string  detail;

    bool Passed() const noexcept { return status == LTSValidatorStatus::Pass; }
};

struct LTSValidationReport
{
    std::vector<LTSGateCheck> checks;
    uint32_t                  passed;
    uint32_t                  failed;
    bool                      ltsStampIssued;  // true only if all gates pass

    bool AllGatesPassed() const noexcept { return failed == 0; }
    std::string Summary() const;
};

class LTSBuildValidator
{
public:
    // Minimum test count required for LTS stamp.
    static constexpr uint32_t MIN_TEST_COUNT    = 4500;
    // Minimum decoder count required.
    static constexpr uint32_t MIN_DECODER_COUNT = 200;
    // Maximum peak memory in MB.
    static constexpr double   MAX_PEAK_MEM_MB   = 120.0;
    // Minimum coverage percentage (line).
    static constexpr double   MIN_COVERAGE_PCT  = 95.0;

    // Override defaults for testing purposes.
    void SetMinTestCount(uint32_t n) noexcept        { m_minTestCount = n; }
    void SetMinDecoderCount(uint32_t n) noexcept     { m_minDecoderCount = n; }
    void SetMaxPeakMemoryMB(double mb) noexcept      { m_maxPeakMemMB = mb; }
    void SetMinCoveragePct(double pct) noexcept      { m_minCoveragePct = pct; }

    // Inject actual measurements before calling Validate().
    void SetTestCount(uint32_t n) noexcept    { m_testCount = n; }
    void SetDecoderCount(uint32_t n) noexcept { m_decoderCount = n; }
    void SetPeakMemoryMB(double mb) noexcept  { m_peakMemMB = mb; }
    void SetCoveragePct(double pct) noexcept  { m_coveragePct = pct; }
    void SetBuildWarnings(uint32_t n) noexcept { m_buildWarnings = n; }
    void SetSoakTestPassed(bool ok) noexcept  { m_soakTestPassed = ok; }

    // Run all LTS gate checks and return a validation report.
    LTSValidationReport Validate() const;

private:
    uint32_t m_minTestCount    = MIN_TEST_COUNT;
    uint32_t m_minDecoderCount = MIN_DECODER_COUNT;
    double   m_maxPeakMemMB    = MAX_PEAK_MEM_MB;
    double   m_minCoveragePct  = MIN_COVERAGE_PCT;

    uint32_t m_testCount     = 0;
    uint32_t m_decoderCount  = 0;
    double   m_peakMemMB     = 0.0;
    double   m_coveragePct   = 0.0;
    uint32_t m_buildWarnings = 0;
    bool     m_soakTestPassed = false;
};

} // namespace Engine
} // namespace ExplorerLens
