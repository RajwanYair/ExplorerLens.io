#pragma once
//==============================================================================
// TestSuiteExpansion — Sprint 196
// Comprehensive test framework expansion with format-specific test archives,
// property-based testing infrastructure, and test coverage tracking
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace DarkThumbs { namespace Engine {

/// Test category for organizing test suites
enum class TestCategory : uint8_t {
    UnitTest,           ///< Single function/class test
    IntegrationTest,    ///< Multi-component test
    DecoderTest,        ///< Format-specific decoder test
    PerformanceTest,    ///< Benchmark/timing test
    FuzzTest,           ///< Fuzz/randomized test
    RegressionTest,     ///< Bug regression test
    StressTest,         ///< High-load/concurrency test
    EndToEndTest,       ///< Full pipeline test
    COMTest,            ///< COM interface test
    PlatformTest        ///< Platform-specific test
};

/// Test result verdict
enum class TestVerdict : uint8_t {
    Pass,
    Fail,
    Skip,
    Error,
    Timeout,
    Flaky       ///< Intermittent pass/fail
};

/// Decoder test specification
struct DecoderTestSpec {
    std::wstring formatName;        ///< e.g., "PNG", "JPEG", "RAW"
    std::wstring decoderClass;      ///< e.g., "ImageDecoder", "RAWDecoder"
    uint32_t minTestCount = 10;     ///< Target tests per decoder
    uint32_t currentTestCount = 0;
    bool hasValidFile = false;      ///< Has valid test file
    bool hasTruncatedFile = false;  ///< Has truncated input test
    bool hasCorruptFile = false;    ///< Has corrupt header test
    bool hasZeroByteFile = false;   ///< Has zero-byte test
    bool hasLargeFile = false;      ///< Has oversized file test
    bool hasAnimated = false;       ///< Has animated/multi-frame test
    bool hasExifTest = false;       ///< Has EXIF orientation test
    bool hasPerformance = false;    ///< Has decode timing test
    double coveragePercent = 0.0;
};

/// Test execution result
struct TestResult {
    std::wstring testName;
    TestCategory category = TestCategory::UnitTest;
    TestVerdict verdict = TestVerdict::Pass;
    double durationMs = 0.0;
    std::wstring message;
    std::wstring filePath;       ///< Source file
    uint32_t lineNumber = 0;
};

/// Test suite summary
struct TestSuiteSummary {
    uint32_t totalTests = 0;
    uint32_t passed = 0;
    uint32_t failed = 0;
    uint32_t skipped = 0;
    uint32_t errors = 0;
    uint32_t timeouts = 0;
    uint32_t flaky = 0;
    double totalDurationMs = 0.0;
    double passRate = 0.0;
    std::unordered_map<std::wstring, uint32_t> testsPerDecoder;
    std::vector<TestResult> failures;
};

/// Target coverage per component
struct CoverageTarget {
    std::wstring component;
    uint32_t currentTests = 0;
    uint32_t targetTests = 0;
    uint32_t gap = 0;
    double coveragePercent = 0.0;
};

/// Test suite expansion configuration
struct TestExpansionConfig {
    uint32_t targetTestsPerDecoder = 10;
    uint32_t targetTotalTests = 600;
    bool generatePropertyTests = true;
    bool generateFuzzTests = true;
    bool generateStressTests = true;
    std::wstring testArchiveDir = L"test-archives";
};

//==============================================================================
// TestSuiteExpansion
//==============================================================================
class TestSuiteExpansion {
public:
    TestSuiteExpansion();
    explicit TestSuiteExpansion(const TestExpansionConfig& config);

    /// Get decoder test specifications for all decoders
    std::vector<DecoderTestSpec> GetDecoderTestSpecs() const;

    /// Calculate coverage gaps
    std::vector<CoverageTarget> CalculateCoverageGaps() const;

    /// Get total test count (estimated from specs)
    uint32_t GetTotalTestCount() const;

    /// Get suite summary from results
    TestSuiteSummary ComputeSummary(const std::vector<TestResult>& results) const;

    /// Check if test suite meets expansion targets
    bool MeetsTargets() const;

    /// Generate test file manifest for a decoder
    std::vector<std::wstring> GetTestFilesForDecoder(const std::wstring& decoderName) const;

    /// Get config
    const TestExpansionConfig& GetConfig() const { return m_config; }

    /// Static name helpers
    static const wchar_t* GetCategoryName(TestCategory category);
    static const wchar_t* GetVerdictName(TestVerdict verdict);

private:
    TestExpansionConfig m_config;
};

}} // namespace DarkThumbs::Engine
