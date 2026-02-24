// ============================================================================
// MemoryLeakTest.h, Task 6.4
// Memory leak regression test for decode loops
//
// Runs N decode iterations and asserts that peak working-set memory stays
// within a bounded multiplier of the baseline. Catches decoder leaks that
// would otherwise accumulate in long-running Explorer sessions.
//
// USAGE (in CTest):
//   ExplorerLens::Testing::MemoryLeakTest::Config cfg;
//   cfg.iterations = 100;
//   cfg.maxGrowthFactorPct = 20;  // Allow 20% growth
//   auto result = ExplorerLens::Testing::MemoryLeakTest::Run(cfg);
//   ASSERT_TRUE(result.passed) << result.report;
// ============================================================================

#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <functional>

#pragma comment(lib, "Psapi.lib")

namespace ExplorerLens {
namespace Testing {

/// Memory snapshot at a point in time
struct MemorySnapshot {
    double workingSetMB = 0.0;
    double privateBytesMB = 0.0;
    double pageFaultCount = 0;
    int iteration = 0;

    static MemorySnapshot Capture(int iter = 0) {
        MemorySnapshot snap;
        snap.iteration = iter;

        PROCESS_MEMORY_COUNTERS_EX pmc = {};
        pmc.cb = sizeof(pmc);
        if (GetProcessMemoryInfo(GetCurrentProcess(),
                                 reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                                 sizeof(pmc))) {
            snap.workingSetMB = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
            snap.privateBytesMB = static_cast<double>(pmc.PrivateUsage) / (1024.0 * 1024.0);
            snap.pageFaultCount = static_cast<double>(pmc.PageFaultCount);
        }
        return snap;
    }
};

/// Result of a memory leak test run
struct MemoryLeakResult {
    bool passed = false;              ///< True if memory growth within bounds
    double baselineMB = 0.0;          ///< Working set after warmup
    double peakMB = 0.0;              ///< Peak working set during test
    double finalMB = 0.0;             ///< Working set at test end
    double growthPct = 0.0;           ///< Percent growth from baseline to peak
    double allowedGrowthPct = 0.0;    ///< Configured limit
    int iterations = 0;              ///< Number of iterations run
    double elapsedSeconds = 0.0;
    std::string report;               ///< Human-readable summary
    std::vector<MemorySnapshot> snapshots;
};

/// Memory leak regression test runner
class MemoryLeakTest {
public:
    /// Test configuration
    struct Config {
        int iterations = 100;              ///< Number of decode loop iterations
        int warmupIterations = 5;          ///< Iterations before baseline capture
        int sampleInterval = 10;           ///< Capture memory every N iterations
        double maxGrowthFactorPct = 20.0;  ///< Fail if growth exceeds this percent
    };

    /// Run the memory leak test using a user-supplied decode function
    /// @param cfg Test configuration
    /// @param decodeFn Function to call each iteration (returns HRESULT)
    /// @return MemoryLeakResult with pass/fail and diagnostics
    static MemoryLeakResult Run(
        const Config& cfg,
        std::function<HRESULT()> decodeFn)
    {
        MemoryLeakResult result;
        result.allowedGrowthPct = cfg.maxGrowthFactorPct;
        result.iterations = cfg.iterations;

        auto startTime = std::chrono::steady_clock::now();

        // Warmup phase — let JIT, caches, allocators stabilize
        for (int i = 0; i < cfg.warmupIterations; i++) {
            decodeFn();
        }

        // Capture baseline after warmup
        auto baseline = MemorySnapshot::Capture(0);
        result.baselineMB = baseline.workingSetMB;
        result.peakMB = baseline.workingSetMB;
        result.snapshots.push_back(baseline);

        // Main test loop
        for (int i = 1; i <= cfg.iterations; i++) {
            decodeFn();

            if (i % cfg.sampleInterval == 0 || i == cfg.iterations) {
                auto snap = MemorySnapshot::Capture(i);
                result.snapshots.push_back(snap);

                if (snap.workingSetMB > result.peakMB) {
                    result.peakMB = snap.workingSetMB;
                }
            }
        }

        // Capture final state
        auto finalSnap = MemorySnapshot::Capture(cfg.iterations);
        result.finalMB = finalSnap.workingSetMB;

        // Calculate growth
        if (result.baselineMB > 0.0) {
            result.growthPct = ((result.peakMB - result.baselineMB) / result.baselineMB) * 100.0;
        }

        result.passed = (result.growthPct <= cfg.maxGrowthFactorPct);

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        result.elapsedSeconds = std::chrono::duration<double>(elapsed).count();

        // Build human-readable report
        result.report = FormatReport(result);

        return result;
    }

    /// Run a default memory leak test (100 iterations, 20% threshold)
    /// Useful when a default decode operation is wired up
    static MemoryLeakResult RunDefault(std::function<HRESULT()> decodeFn) {
        Config cfg;
        return Run(cfg, decodeFn);
    }

private:
    static std::string FormatReport(const MemoryLeakResult& result) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);

        oss << "=== Memory Leak Regression Test ===\n";
        oss << "Status:     " << (result.passed ? "PASSED" : "FAILED") << "\n";
        oss << "Iterations: " << result.iterations << "\n";
        oss << "Duration:   " << result.elapsedSeconds << "s\n";
        oss << "Baseline:   " << result.baselineMB << " MB\n";
        oss << "Peak:       " << result.peakMB << " MB\n";
        oss << "Final:      " << result.finalMB << " MB\n";
        oss << "Growth:     " << result.growthPct << "% "
            << "(limit: " << result.allowedGrowthPct << "%)\n";

        if (!result.passed) {
            oss << "\n*** LEAK DETECTED: Memory grew " << result.growthPct
                << "% which exceeds the " << result.allowedGrowthPct
                << "% threshold ***\n";
        }

        oss << "\n--- Memory Snapshots ---\n";
        oss << "Iter     WorkingSet(MB)  Private(MB)\n";
        for (const auto& snap : result.snapshots) {
            oss << std::setw(6) << snap.iteration
                << "   " << std::setw(12) << snap.workingSetMB
                << "   " << std::setw(12) << snap.privateBytesMB
                << "\n";
        }

        return oss.str();
    }
};

} // namespace Testing
} // namespace ExplorerLens

