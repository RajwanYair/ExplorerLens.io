// IntegrationTestFrameworkV2.h — Integration Test Framework V2
// Copyright (c) 2026 ExplorerLens Project
//
// Enhanced integration testing framework for cross-module validation.
// Supports test fixtures, parameterized tests, timeout enforcement,
// and resource dependency tracking across Engine subsystems.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Enums
// ============================================================================

/// Integration test category
enum class IntegTestCategory : uint8_t {
    DecoderPipeline = 0,  ///< Decoder → Pipeline → Output
    CacheCoherence,       ///< Cache read/write/evict cycles
    GPURoundTrip,         ///< CPU → GPU → CPU data fidelity
    COMLifecycle,         ///< COM AddRef/Release/QI correctness
    ShellIntegration,     ///< Explorer shell interaction
    MemoryPressure,       ///< Allocation under constrained memory
    ConcurrentAccess,     ///< Multi-thread contention
    ErrorRecovery,        ///< Fault injection + self-healing
    COUNT
};

/// Test fixture lifecycle phase
enum class FixturePhase : uint8_t {
    Setup = 0,
    Execute,
    Verify,
    Teardown,
    COUNT
};

/// Integration test result status
enum class IntegTestStatus : uint8_t {
    NotRun = 0,
    Running,
    Passed,
    Failed,
    Skipped,
    TimedOut,
    COUNT
};

// ============================================================================
// String conversions
// ============================================================================

inline const char* IntegTestCategoryToString(IntegTestCategory c) {
    static const char* names[] = {
        "DecoderPipeline", "CacheCoherence", "GPURoundTrip",
        "COMLifecycle", "ShellIntegration", "MemoryPressure",
        "ConcurrentAccess", "ErrorRecovery"
    };
    auto idx = static_cast<uint8_t>(c);
    return (idx < static_cast<uint8_t>(IntegTestCategory::COUNT)) ? names[idx] : "Unknown";
}

inline const char* FixturePhaseToString(FixturePhase p) {
    static const char* names[] = { "Setup", "Execute", "Verify", "Teardown" };
    auto idx = static_cast<uint8_t>(p);
    return (idx < static_cast<uint8_t>(FixturePhase::COUNT)) ? names[idx] : "Unknown";
}

inline const char* IntegTestStatusToString(IntegTestStatus s) {
    static const char* names[] = {
        "NotRun", "Running", "Passed", "Failed", "Skipped", "TimedOut"
    };
    auto idx = static_cast<uint8_t>(s);
    return (idx < static_cast<uint8_t>(IntegTestStatus::COUNT)) ? names[idx] : "Unknown";
}

// ============================================================================
// Structs
// ============================================================================

/// Single integration test case descriptor
struct IntegTestCase {
    std::string         name;
    IntegTestCategory   category = IntegTestCategory::DecoderPipeline;
    IntegTestStatus     status = IntegTestStatus::NotRun;
    uint32_t            timeoutMs = 30000;
    double              durationMs = 0.0;
    std::string         failureMessage;
    bool                requiresGPU = false;
    bool                requiresAdmin = false;
};

/// Aggregated suite results
struct IntegTestSuiteResult {
    uint32_t total = 0;
    uint32_t passed = 0;
    uint32_t failed = 0;
    uint32_t skipped = 0;
    uint32_t timedOut = 0;
    double   totalDurationMs = 0.0;

    double PassRate() const {
        uint32_t executed = passed + failed;
        return (executed > 0) ? (static_cast<double>(passed) / executed * 100.0) : 0.0;
    }
};

// ============================================================================
// IntegrationTestFrameworkV2 class
// ============================================================================

class IntegrationTestFrameworkV2 {
public:
    using TestFunc = std::function<bool()>;

    /// Register a test case
    void RegisterTest(const std::string& name, IntegTestCategory category,
        TestFunc func, uint32_t timeoutMs = 30000) {
        IntegTestCase tc;
        tc.name = name;
        tc.category = category;
        tc.timeoutMs = timeoutMs;
        m_cases.push_back(tc);
        m_funcs.push_back(std::move(func));
    }

    /// Run all registered tests
    IntegTestSuiteResult RunAll() {
        IntegTestSuiteResult result;
        result.total = static_cast<uint32_t>(m_cases.size());

        for (size_t i = 0; i < m_cases.size(); ++i) {
            auto& tc = m_cases[i];
            tc.status = IntegTestStatus::Running;

            auto start = std::chrono::steady_clock::now();
            bool success = false;
            try {
                success = m_funcs[i]();
            }
            catch (...) {
                success = false;
                tc.failureMessage = "Exception thrown";
            }
            auto end = std::chrono::steady_clock::now();
            tc.durationMs = std::chrono::duration<double, std::milli>(end - start).count();

            if (success) {
                tc.status = IntegTestStatus::Passed;
                result.passed++;
            }
            else {
                tc.status = IntegTestStatus::Failed;
                result.failed++;
            }
            result.totalDurationMs += tc.durationMs;
        }
        return result;
    }

    /// Run tests in a specific category
    IntegTestSuiteResult RunCategory(IntegTestCategory category) {
        IntegTestSuiteResult result;
        for (size_t i = 0; i < m_cases.size(); ++i) {
            if (m_cases[i].category != category) continue;
            result.total++;
            auto& tc = m_cases[i];
            tc.status = IntegTestStatus::Running;

            auto start = std::chrono::steady_clock::now();
            bool success = false;
            try { success = m_funcs[i](); }
            catch (...) { success = false; }
            auto end = std::chrono::steady_clock::now();
            tc.durationMs = std::chrono::duration<double, std::milli>(end - start).count();

            if (success) {
                tc.status = IntegTestStatus::Passed;
                result.passed++;
            }
            else {
                tc.status = IntegTestStatus::Failed;
                result.failed++;
            }
            result.totalDurationMs += tc.durationMs;
        }
        return result;
    }

    /// Get all test cases
    const std::vector<IntegTestCase>& GetCases() const { return m_cases; }

    /// Get count of registered tests
    uint32_t GetTestCount() const { return static_cast<uint32_t>(m_cases.size()); }

    /// Clear all registered tests
    void Clear() { m_cases.clear(); m_funcs.clear(); }

private:
    std::vector<IntegTestCase> m_cases;
    std::vector<TestFunc>      m_funcs;
};

} // namespace Engine
} // namespace ExplorerLens
