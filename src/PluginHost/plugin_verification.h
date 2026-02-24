// plugin_verification.h - ExplorerLens Verified Plugin Compatibility Test Kit v1.0
// Part of Sprint 20: Plugin Sandbox v2 + Trust/Signing (v6.2)
//
// PURPOSE:
//   Comprehensive testing framework for plugin certification and verification.
//   Defines compatibility requirements, performance benchmarks, stability tests,
//   and certification process for third-party plugins to achieve "Verified" status.
//
// VERIFIED STATUS BENEFITS:
//   - Listed in official marketplace
//   - Higher trust level and capabilities
//   - Recommended to users
//   - Stability and performance guarantees
//
// TEST CATEGORIES:
//   1. Functional Tests - Correct decoding, format support
//   2. Performance Tests - Speed, memory, GPU usage benchmarks
//   3. Stability Tests - Crash resistance, leak detection, edge cases
//   4. Security Tests - Capability compliance, sandbox escape attempts
//   5. Compatibility Tests - Windows versions, GPU drivers, file variants
//
// Created: January 6, 2026
// Version: 1.0.0

#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <map>
#include <optional>

namespace ExplorerLens {
namespace PluginVerification {

// Version
constexpr uint32_t VERIFICATION_KIT_VERSION = 1;

//=============================================================================
// TEST RESULT
//=============================================================================

enum class TestResult : uint32_t {
    PASSED = 0,         // Test passed
    FAILED = 1,         // Test failed
    SKIPPED = 2,        // Test skipped (not applicable)
    WARNING = 3,        // Passed with warnings
    ERROR = 4,          // Test error (couldn't run)
    TIMEOUT = 5         // Test timed out
};

inline const wchar_t* ToString(TestResult result) {
    switch (result) {
        case TestResult::PASSED: return L"PASSED";
        case TestResult::FAILED: return L"FAILED";
        case TestResult::SKIPPED: return L"SKIPPED";
        case TestResult::WARNING: return L"WARNING";
        case TestResult::ERROR: return L"ERROR";
        case TestResult::TIMEOUT: return L"TIMEOUT";
        default: return L"UNKNOWN";
    }
}

//=============================================================================
// TEST CASE
//=============================================================================

// Individual test case result
struct TestCaseResult {
    std::wstring testName;
    std::wstring category;
    TestResult result = TestResult::FAILED;
    
    std::chrono::milliseconds duration{0};
    std::wstring message;
    std::wstring details;
    
    // Metrics
    std::map<std::wstring, double> metrics;
    
    // Attachments (screenshots, logs, dumps)
    std::vector<std::wstring> attachments;
    
    TestCaseResult() = default;
    explicit TestCaseResult(const std::wstring& name, const std::wstring& cat)
        : testName(name), category(cat) {}
    
    bool IsPassed() const { return result == TestResult::PASSED; }
    bool IsFailed() const { return result == TestResult::FAILED || result == TestResult::ERROR; }
};

// Test case definition
struct TestCase {
    std::wstring name;
    std::wstring category;
    std::wstring description;
    bool required = true;  // Required for certification
    
    std::chrono::milliseconds timeout{30000};  // 30 seconds default
    
    using TestFunction = std::function<TestCaseResult()>;
    TestFunction testFunc;
    
    TestCase() = default;
};

//=============================================================================
// VERIFICATION REQUIREMENTS
//=============================================================================

// Performance requirements for verified plugins
struct PerformanceRequirements {
    // Decoding speed (average across test images)
    std::chrono::milliseconds maxDecodeTime{100};       // P50 < 100ms
    std::chrono::milliseconds maxDecodeTimeP95{250};    // P95 < 250ms
    std::chrono::milliseconds maxDecodeTimeP99{500};    // P99 < 500ms
    
    // Memory usage
    uint64_t maxMemoryPerDecode = 64 * 1024 * 1024;     // 64 MB per decode
    uint64_t maxTotalMemory = 256 * 1024 * 1024;        // 256 MB total
    uint64_t maxMemoryLeak = 1024 * 1024;               // 1 MB leak tolerance
    
    // GPU usage (if applicable)
    uint32_t maxGPUMemory = 128 * 1024 * 1024;          // 128 MB GPU memory
    uint32_t maxGPUUsagePercent = 80;                   // 80% GPU utilization
    
    // Throughput
    uint32_t minThroughput = 20;                        // 20 images/sec minimum
    
    // Startup
    std::chrono::milliseconds maxStartupTime{2000};     // 2 seconds max
    
    // Resource cleanup
    std::chrono::milliseconds maxCleanupTime{1000};     // 1 second max
    
    PerformanceRequirements() = default;
};

// Stability requirements
struct StabilityRequirements {
    uint32_t maxCrashCount = 0;                 // Zero crashes allowed
    uint32_t maxHangCount = 0;                  // Zero hangs allowed
    uint32_t minSuccessRate = 95;               // 95% success rate minimum
    
    // Stress test requirements
    uint32_t consecutiveDecodes = 1000;         // Must handle 1000 consecutive decodes
    uint32_t concurrentDecodes = 10;            // Must handle 10 concurrent decodes
    
    // Edge case handling
    bool mustHandleCorruptedFiles = true;
    bool mustHandleTruncatedFiles = true;
    bool mustHandleZeroSizeFiles = true;
    bool mustHandleMalformedHeaders = true;
    
    StabilityRequirements() = default;
};

// Security requirements
struct SecurityRequirements {
    bool mustRespectCapabilities = true;        // No capability violations
    bool mustNotEscapeSandbox = true;           // No sandbox escape attempts
    bool mustValidateInputs = true;             // Input validation required
    bool mustHandleUntrustedData = true;        // Safe handling of untrusted files
    
    uint32_t maxSecurityViolations = 0;         // Zero violations
    
    SecurityRequirements() = default;
};

// Compatibility requirements
struct CompatibilityRequirements {
    // Windows versions
    std::vector<std::wstring> supportedWindowsVersions = {
        L"Windows 10 20H2",
        L"Windows 10 21H2",
        L"Windows 11 21H2",
        L"Windows 11 22H2",
        L"Windows 11 23H2"
    };
    
    // Architectures
    std::vector<std::wstring> supportedArchitectures = {
        L"x64"
    };
    
    // GPU vendors (if GPU acceleration used)
    std::vector<std::wstring> supportedGPUVendors = {
        L"NVIDIA",
        L"AMD",
        L"Intel"
    };
    
    // Format variants (for each supported format)
    bool mustHandleAllVariants = true;
    
    CompatibilityRequirements() = default;
};

// Complete verification requirements
struct VerificationRequirements {
    PerformanceRequirements performance;
    StabilityRequirements stability;
    SecurityRequirements security;
    CompatibilityRequirements compatibility;
    
    VerificationRequirements() = default;
};

//=============================================================================
// TEST SUITE
//=============================================================================

// Test suite configuration
struct TestSuiteConfig {
    std::wstring pluginPath;
    std::wstring testDataPath;
    std::wstring outputPath;
    
    VerificationRequirements requirements;
    
    bool enableFunctionalTests = true;
    bool enablePerformanceTests = true;
    bool enableStabilityTests = true;
    bool enableSecurityTests = true;
    bool enableCompatibilityTests = true;
    
    bool verbose = false;
    bool saveArtifacts = true;
    
    TestSuiteConfig() = default;
};

// Test suite results
struct TestSuiteResult {
    std::wstring pluginPath;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    std::chrono::milliseconds totalDuration{0};
    
    std::vector<TestCaseResult> testResults;
    
    uint32_t totalTests = 0;
    uint32_t passedTests = 0;
    uint32_t failedTests = 0;
    uint32_t skippedTests = 0;
    uint32_t warningTests = 0;
    
    bool isVerified = false;
    std::vector<std::wstring> verificationErrors;
    std::vector<std::wstring> verificationWarnings;
    
    // Summary metrics
    std::map<std::wstring, double> summaryMetrics;
    
    TestSuiteResult() : startTime(std::chrono::system_clock::now()) {}
    
    void Finalize();
    bool IsFullyPassed() const { return failedTests == 0 && passedTests > 0; }
    double GetPassRate() const {
        return totalTests > 0 ? (static_cast<double>(passedTests) / totalTests * 100.0) : 0.0;
    }
};

//=============================================================================
// TEST SUITE RUNNER
//=============================================================================

// Main test suite runner
class TestSuiteRunner {
public:
    explicit TestSuiteRunner(const TestSuiteConfig& config);
    ~TestSuiteRunner();
    
    // Run all tests
    TestSuiteResult RunAll();
    
    // Run specific test categories
    TestSuiteResult RunFunctionalTests();
    TestSuiteResult RunPerformanceTests();
    TestSuiteResult RunStabilityTests();
    TestSuiteResult RunSecurityTests();
    TestSuiteResult RunCompatibilityTests();
    
    // Run single test
    TestCaseResult RunTest(const TestCase& testCase);
    
    // Configuration
    const TestSuiteConfig& GetConfig() const { return config_; }
    void SetConfig(const TestSuiteConfig& config) { config_ = config; }
    
    // Progress callback
    using ProgressCallback = std::function<void(
        const std::wstring& testName,
        uint32_t current,
        uint32_t total
    )>;
    
    void SetProgressCallback(ProgressCallback callback) { progressCallback_ = callback; }
    
private:
    // Test generators
    std::vector<TestCase> GenerateFunctionalTests();
    std::vector<TestCase> GeneratePerformanceTests();
    std::vector<TestCase> GenerateStabilityTests();
    std::vector<TestCase> GenerateSecurityTests();
    std::vector<TestCase> GenerateCompatibilityTests();
    
    // Test execution
    void ExecuteTests(std::vector<TestCase>& tests, TestSuiteResult& result);
    
    // Verification
    bool VerifyRequirements(const TestSuiteResult& result);
    
    TestSuiteConfig config_;
    ProgressCallback progressCallback_;
};

//=============================================================================
// FUNCTIONAL TESTS
//=============================================================================

class FunctionalTests {
public:
    // Basic decoding
    static TestCaseResult TestBasicDecode(const std::wstring& pluginPath, const std::wstring& testFile);
    static TestCaseResult TestMultipleSizes(const std::wstring& pluginPath, const std::wstring& testFile);
    static TestCaseResult TestFormatDetection(const std::wstring& pluginPath, const std::wstring& testFile);
    
    // Color accuracy
    static TestCaseResult TestColorAccuracy(const std::wstring& pluginPath, const std::wstring& testFile);
    static TestCaseResult TestAlphaChannel(const std::wstring& pluginPath, const std::wstring& testFile);
    static TestCaseResult TestColorSpaces(const std::wstring& pluginPath, const std::wstring& testFile);
    
    // Metadata
    static TestCaseResult TestMetadataExtraction(const std::wstring& pluginPath, const std::wstring& testFile);
    static TestCaseResult TestOrientation(const std::wstring& pluginPath, const std::wstring& testFile);
};

//=============================================================================
// PERFORMANCE TESTS
//=============================================================================

class PerformanceTests {
public:
    // Speed benchmarks
    static TestCaseResult TestDecodeSpeed(const std::wstring& pluginPath, const std::vector<std::wstring>& testFiles);
    static TestCaseResult TestThroughput(const std::wstring& pluginPath, const std::vector<std::wstring>& testFiles);
    static TestCaseResult TestStartupTime(const std::wstring& pluginPath);
    
    // Memory benchmarks
    static TestCaseResult TestMemoryUsage(const std::wstring& pluginPath, const std::vector<std::wstring>& testFiles);
    static TestCaseResult TestMemoryLeaks(const std::wstring& pluginPath, uint32_t iterations);
    
    // GPU benchmarks (if applicable)
    static TestCaseResult TestGPUUsage(const std::wstring& pluginPath, const std::vector<std::wstring>& testFiles);
    static TestCaseResult TestGPUMemory(const std::wstring& pluginPath, const std::vector<std::wstring>& testFiles);
};

//=============================================================================
// STABILITY TESTS
//=============================================================================

class StabilityTests {
public:
    // Stress tests
    static TestCaseResult TestConsecutiveDecodes(const std::wstring& pluginPath, uint32_t count);
    static TestCaseResult TestConcurrentDecodes(const std::wstring& pluginPath, uint32_t concurrency);
    static TestCaseResult TestRandomizedDecodes(const std::wstring& pluginPath, uint32_t count);
    
    // Edge cases
    static TestCaseResult TestCorruptedFiles(const std::wstring& pluginPath);
    static TestCaseResult TestTruncatedFiles(const std::wstring& pluginPath);
    static TestCaseResult TestMalformedHeaders(const std::wstring& pluginPath);
    static TestCaseResult TestZeroSizeFile(const std::wstring& pluginPath);
    static TestCaseResult TestExtremelyLargeFile(const std::wstring& pluginPath);
    
    // Recovery
    static TestCaseResult TestErrorRecovery(const std::wstring& pluginPath);
    static TestCaseResult TestGracefulDegradation(const std::wstring& pluginPath);
};

//=============================================================================
// SECURITY TESTS
//=============================================================================

class SecurityTests {
public:
    // Capability compliance
    static TestCaseResult TestCapabilityCompliance(const std::wstring& pluginPath);
    static TestCaseResult TestFileAccessRestrictions(const std::wstring& pluginPath);
    static TestCaseResult TestNetworkRestrictions(const std::wstring& pluginPath);
    static TestCaseResult TestRegistryRestrictions(const std::wstring& pluginPath);
    
    // Sandbox escape attempts
    static TestCaseResult TestSandboxEscape(const std::wstring& pluginPath);
    static TestCaseResult TestPrivilegeEscalation(const std::wstring& pluginPath);
    
    // Input validation
    static TestCaseResult TestBufferOverflow(const std::wstring& pluginPath);
    static TestCaseResult TestPathTraversal(const std::wstring& pluginPath);
    static TestCaseResult TestCodeInjection(const std::wstring& pluginPath);
};

//=============================================================================
// COMPATIBILITY TESTS
//=============================================================================

class CompatibilityTests {
public:
    // Platform compatibility
    static TestCaseResult TestWindowsVersions(const std::wstring& pluginPath);
    static TestCaseResult TestArchitectures(const std::wstring& pluginPath);
    
    // GPU compatibility
    static TestCaseResult TestGPUVendors(const std::wstring& pluginPath);
    static TestCaseResult TestDriverVersions(const std::wstring& pluginPath);
    
    // Format variants
    static TestCaseResult TestFormatVariants(const std::wstring& pluginPath);
    static TestCaseResult TestFileExtensions(const std::wstring& pluginPath);
};

//=============================================================================
// CERTIFICATION
//=============================================================================

// Certification level
enum class CertificationLevel : uint32_t {
    NONE = 0,           // Not certified
    BASIC = 1,          // Basic functionality verified
    STANDARD = 2,       // Standard performance and stability
    VERIFIED = 3,       // Full verification passed
    GOLD = 4            // Exceptional quality and performance
};

inline const wchar_t* ToString(CertificationLevel level) {
    switch (level) {
        case CertificationLevel::NONE: return L"NONE";
        case CertificationLevel::BASIC: return L"BASIC";
        case CertificationLevel::STANDARD: return L"STANDARD";
        case CertificationLevel::VERIFIED: return L"VERIFIED";
        case CertificationLevel::GOLD: return L"GOLD";
        default: return L"UNKNOWN";
    }
}

// Certification result
struct CertificationResult {
    CertificationLevel level = CertificationLevel::NONE;
    std::wstring pluginPath;
    std::wstring pluginVersion;
    
    std::chrono::system_clock::time_point certificationDate;
    std::chrono::system_clock::time_point expirationDate;
    
    TestSuiteResult testResults;
    
    std::vector<std::wstring> achievements;     // Gold: "Best Performance", "Zero Defects"
    std::vector<std::wstring> recommendations;  // Suggested improvements
    
    std::wstring certificateId;                 // Unique certificate ID
    std::wstring signature;                     // Digital signature
    
    CertificationResult() = default;
    
    bool IsValid() const;
    bool IsExpired() const;
};

// Certification manager
class CertificationManager {
public:
    static CertificationManager& Instance();
    
    // Run certification
    CertificationResult Certify(const std::wstring& pluginPath);
    CertificationResult Certify(const std::wstring& pluginPath, const VerificationRequirements& requirements);
    
    // Certificate management
    bool SaveCertificate(const CertificationResult& result, const std::wstring& outputPath);
    std::optional<CertificationResult> LoadCertificate(const std::wstring& certificatePath);
    bool VerifyCertificate(const CertificationResult& result);
    
    // Certification database
    bool RegisterCertification(const CertificationResult& result);
    std::optional<CertificationResult> GetCertification(const std::wstring& pluginPath);
    bool RevokeCertification(const std::wstring& pluginPath, const std::wstring& reason);
    
private:
    CertificationManager() = default;
    ~CertificationManager() = default;
    
    CertificationLevel DetermineCertificationLevel(const TestSuiteResult& results);
    std::wstring GenerateCertificateId();
    std::wstring SignCertificate(const CertificationResult& result);
};

//=============================================================================
// HELPER FUNCTIONS
//=============================================================================

// Generate test report
bool GenerateHTMLReport(const TestSuiteResult& result, const std::wstring& outputPath);
bool GenerateJSONReport(const TestSuiteResult& result, const std::wstring& outputPath);
bool GenerateMarkdownReport(const TestSuiteResult& result, const std::wstring& outputPath);

// Test data management
std::vector<std::wstring> LoadTestFiles(const std::wstring& testDataPath, const std::wstring& format);
bool ValidateTestData(const std::wstring& testDataPath);

} // namespace PluginVerification
} // namespace ExplorerLens

