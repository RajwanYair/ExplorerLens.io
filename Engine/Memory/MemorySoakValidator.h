#pragma once
// Sprint 140 — Memory Soak Validation
// 10K preview soak test framework with leak diffing and working-set gates.
// Validates memory stability under sustained thumbnail generation load.

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <cmath>

namespace DarkThumbs::Memory {

// ─── Memory snapshot ────────────────────────────────────────────
struct MemorySnapshot {
    size_t workingSetBytes = 0;
    size_t privateBytes = 0;
    size_t virtualBytes = 0;
    size_t heapAllocations = 0;
    size_t heapFrees = 0;
    uint64_t timestamp = 0;  // ms since test start

    size_t NetAllocations() const {
        return (heapAllocations > heapFrees) ? heapAllocations - heapFrees : 0;
    }

    double WorkingSetMB() const {
        return static_cast<double>(workingSetBytes) / (1024.0 * 1024.0);
    }

    double PrivateMB() const {
        return static_cast<double>(privateBytes) / (1024.0 * 1024.0);
    }
};

// ─── Memory diff between snapshots ──────────────────────────────
struct MemoryDiff {
    int64_t  workingSetDelta = 0;
    int64_t  privateDelta = 0;
    int64_t  netAllocationsDelta = 0;
    double   growthRateMBPerSec = 0.0;
    uint64_t durationMs = 0;

    bool HasLeak() const {
        // Leak heuristic: >5 MB growth and positive net allocations
        return workingSetDelta > 5 * 1024 * 1024 && netAllocationsDelta > 0;
    }

    bool IsStable() const {
        // Stable: less than 2 MB working set variation
        return std::abs(workingSetDelta) < 2 * 1024 * 1024;
    }

    static MemoryDiff Between(const MemorySnapshot& before, const MemorySnapshot& after) {
        MemoryDiff diff;
        diff.workingSetDelta = static_cast<int64_t>(after.workingSetBytes) -
                                static_cast<int64_t>(before.workingSetBytes);
        diff.privateDelta = static_cast<int64_t>(after.privateBytes) -
                             static_cast<int64_t>(before.privateBytes);
        diff.netAllocationsDelta = static_cast<int64_t>(after.NetAllocations()) -
                                    static_cast<int64_t>(before.NetAllocations());
        diff.durationMs = after.timestamp - before.timestamp;
        if (diff.durationMs > 0) {
            diff.growthRateMBPerSec = (diff.workingSetDelta / (1024.0 * 1024.0)) /
                                      (diff.durationMs / 1000.0);
        }
        return diff;
    }
};

// ─── Soak test configuration ────────────────────────────────────
struct SoakTestConfig {
    uint64_t iterationCount = 10000;
    uint32_t snapshotIntervalMs = 1000;   // Take snapshot every N ms
    size_t   workingSetLimitMB = 512;     // Max working set
    size_t   leakThresholdMB = 10;        // Max cumulative growth
    double   maxGrowthRateMBPerSec = 1.0; // Max sustained growth rate
    uint32_t warmupIterations = 100;      // Iterations before measurement starts

    static SoakTestConfig Quick() {
        SoakTestConfig c;
        c.iterationCount = 1000;
        c.warmupIterations = 50;
        return c;
    }

    static SoakTestConfig Standard() { return {}; }

    static SoakTestConfig Extended() {
        SoakTestConfig c;
        c.iterationCount = 50000;
        c.workingSetLimitMB = 256;
        c.leakThresholdMB = 5;
        c.warmupIterations = 500;
        return c;
    }
};

// ─── Soak test result ───────────────────────────────────────────
enum class SoakVerdict : uint8_t {
    Pass             = 0,
    MemoryLeakDetected = 1,
    WorkingSetExceeded = 2,
    GrowthRateExceeded = 3,
    Crashed          = 4,
    Timeout          = 5
};

inline const char* SoakVerdictName(SoakVerdict v) {
    switch (v) {
        case SoakVerdict::Pass:                return "PASS";
        case SoakVerdict::MemoryLeakDetected:  return "LEAK DETECTED";
        case SoakVerdict::WorkingSetExceeded:  return "WORKING SET EXCEEDED";
        case SoakVerdict::GrowthRateExceeded:  return "GROWTH RATE EXCEEDED";
        case SoakVerdict::Crashed:             return "CRASHED";
        case SoakVerdict::Timeout:             return "TIMEOUT";
        default: return "Unknown";
    }
}

struct SoakTestResult {
    SoakVerdict        verdict = SoakVerdict::Timeout;
    uint64_t           completedIterations = 0;
    MemorySnapshot     baseline;
    MemorySnapshot     final_;
    MemoryDiff         overallDiff;
    size_t             peakWorkingSetBytes = 0;
    double             totalDurationMs = 0.0;
    std::vector<MemorySnapshot> snapshots;

    bool IsPass() const { return verdict == SoakVerdict::Pass; }

    double PeakWorkingSetMB() const {
        return static_cast<double>(peakWorkingSetBytes) / (1024.0 * 1024.0);
    }

    double IterationsPerSecond() const {
        return totalDurationMs > 0
            ? completedIterations / (totalDurationMs / 1000.0)
            : 0.0;
    }

    std::string Summary() const {
        return "Verdict=" + std::string(SoakVerdictName(verdict)) +
               " Iterations=" + std::to_string(completedIterations) +
               " PeakWS=" + std::to_string(static_cast<int>(PeakWorkingSetMB())) + "MB" +
               " Speed=" + std::to_string(static_cast<int>(IterationsPerSecond())) + "/s";
    }
};

// ─── Memory Soak Validator ──────────────────────────────────────
class MemorySoakValidator {
public:
    explicit MemorySoakValidator(SoakTestConfig config = SoakTestConfig::Standard())
        : m_config(config) {}

    void RecordSnapshot(const MemorySnapshot& snapshot) {
        m_snapshots.push_back(snapshot);
        if (snapshot.workingSetBytes > m_peakWorkingSet) {
            m_peakWorkingSet = snapshot.workingSetBytes;
        }
    }

    SoakTestResult Evaluate() const {
        SoakTestResult result;
        result.completedIterations = m_config.iterationCount;
        result.peakWorkingSetBytes = m_peakWorkingSet;
        result.snapshots = m_snapshots;

        if (m_snapshots.size() < 2) {
            result.verdict = SoakVerdict::Pass;
            return result;
        }

        result.baseline = m_snapshots.front();
        result.final_ = m_snapshots.back();
        result.overallDiff = MemoryDiff::Between(result.baseline, result.final_);

        // Check working set limit
        if (m_peakWorkingSet > m_config.workingSetLimitMB * 1024 * 1024) {
            result.verdict = SoakVerdict::WorkingSetExceeded;
            return result;
        }

        // Check growth rate
        if (std::abs(result.overallDiff.growthRateMBPerSec) > m_config.maxGrowthRateMBPerSec) {
            result.verdict = SoakVerdict::GrowthRateExceeded;
            return result;
        }

        // Check leak threshold
        if (result.overallDiff.HasLeak() &&
            result.overallDiff.workingSetDelta > static_cast<int64_t>(m_config.leakThresholdMB * 1024 * 1024)) {
            result.verdict = SoakVerdict::MemoryLeakDetected;
            return result;
        }

        result.verdict = SoakVerdict::Pass;
        return result;
    }

    size_t SnapshotCount() const { return m_snapshots.size(); }
    SoakTestConfig GetConfig() const { return m_config; }

    static MemorySoakValidator Create(SoakTestConfig config = SoakTestConfig::Standard()) {
        return MemorySoakValidator(config);
    }

private:
    SoakTestConfig              m_config;
    std::vector<MemorySnapshot> m_snapshots;
    size_t                      m_peakWorkingSet = 0;
};

} // namespace DarkThumbs::Memory
