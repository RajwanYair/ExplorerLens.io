#pragma once
// Sprint 150 — Plugin Runtime E2E Test Matrix
// Comprehensive end-to-end test matrix covering all plugin activation paths,
// IPC channels, and failure/recovery modes.

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <optional>

namespace DarkThumbs::Plugin {

// ─── Test scenario classification ───────────────────────────────────────────

enum class PluginTestScenario : uint32_t {
    // Lifecycle
    PluginLoad          = 0x01,
    PluginUnload        = 0x02,
    PluginReload        = 0x03,  // unload → load cycle

    // Decode operations
    DecodeValidImage    = 0x10,
    DecodeCorruptData   = 0x11,
    DecodeLargePayload  = 0x12,  // payload > 64 MB
    DecodeEmptyPayload  = 0x13,

    // IPC channel
    IPCRoundTrip        = 0x20,
    IPCLatencyMeasure   = 0x21,
    IPCReconnect        = 0x22,  // lost + re-established pipe

    // Failure & recovery
    PluginCrash         = 0x30,  // simulated crash via bad access
    PluginTimeout       = 0x31,  // exceed wall-clock limit
    PluginOOM           = 0x32,  // exceed memory quota

    // Soak
    SoakIterations1K    = 0x40,
    SoakMemoryProfile   = 0x41,
    SoakOrphanCheck     = 0x42,  // no orphaned processes after soak
};

enum class PluginTestResultCode : uint32_t {
    Pass            = 0,
    Fail            = 1,
    Skip            = 2,
    Error           = 3,
    Timeout         = 4,
    NotImplemented  = 5,
};

// ─── IPC channel descriptor ──────────────────────────────────────────────────

enum class IPCChannelType : uint32_t {
    NamedPipe   = 0,
    SharedMemory = 1,
    COM         = 2,
};

struct IPCChannelConfig {
    IPCChannelType  type            { IPCChannelType::NamedPipe };
    uint32_t        bufferSizeBytes { 65536 };
    uint32_t        timeoutMs       { 5000 };
    bool            asyncMode       { true };

    // Named pipe defaults
    std::wstring    pipeName        { L"\\\\.\\pipe\\DarkThumbs_Plugin_Test" };

    static IPCChannelConfig Default() { return {}; }
    static IPCChannelConfig FastLocal() {
        IPCChannelConfig c;
        c.bufferSizeBytes = 131072;
        c.timeoutMs       = 2000;
        return c;
    }
};

// ─── Individual test entry ────────────────────────────────────────────────────

struct PluginTestEntry {
    PluginTestScenario  scenario;
    std::string         name;
    std::string         description;
    uint32_t            timeoutMs   { 10000 };
    bool                isRequired  { true };
};

struct PluginTestResult {
    PluginTestScenario  scenario;
    PluginTestResultCode code       { PluginTestResultCode::NotImplemented };
    std::string         message;
    double              durationMs  { 0.0 };
    uint64_t            peakMemory  { 0 };   // bytes

    bool Passed() const { return code == PluginTestResultCode::Pass; }
    bool Skipped() const { return code == PluginTestResultCode::Skip; }
};

// ─── IPC latency measurement ─────────────────────────────────────────────────

struct IPCLatencyResult {
    double  minMs       { 0.0 };
    double  maxMs       { 0.0 };
    double  meanMs      { 0.0 };
    double  p95Ms       { 0.0 };
    double  p99Ms       { 0.0 };
    int32_t sampleCount { 0 };

    static constexpr double kLatencyBudgetMs = 50.0;  // SLA: p95 < 50ms

    bool MeetsSLA() const { return p95Ms < kLatencyBudgetMs; }

    std::string Summary() const {
        return "IPC Latency p95=" + std::to_string(p95Ms) + "ms / SLA=" +
               std::to_string(kLatencyBudgetMs) + "ms / " +
               (MeetsSLA() ? "PASS" : "FAIL");
    }
};

// ─── Soak test configuration ──────────────────────────────────────────────────

struct SoakTestConfig {
    uint32_t    iterations      { 1000 };
    uint32_t    concurrency     { 4 };
    uint32_t    maxMemoryMB     { 512 };
    bool        checkOrphans    { true };
    bool        profileMemory   { true };

    static SoakTestConfig Quick()       { SoakTestConfig c; c.iterations = 100; c.concurrency = 2; return c; }
    static SoakTestConfig Standard()    { return {}; }
    static SoakTestConfig Stress()      { SoakTestConfig c; c.iterations = 5000; c.concurrency = 8; return c; }
};

struct SoakTestResult {
    uint32_t    totalIterations     { 0 };
    uint32_t    successCount        { 0 };
    uint32_t    failureCount        { 0 };
    uint32_t    orphanProcessCount  { 0 };
    uint64_t    peakMemoryBytes     { 0 };
    double      totalDurationMs     { 0.0 };

    double SuccessRate() const {
        return totalIterations ? (100.0 * successCount / totalIterations) : 0.0;
    }
    bool PassedSoak() const {
        return failureCount == 0 && orphanProcessCount == 0;
    }
    std::string Summary() const {
        return "Soak: " + std::to_string(successCount) + "/" +
               std::to_string(totalIterations) + " OK, " +
               std::to_string(orphanProcessCount) + " orphans";
    }
};

// ─── Complete test matrix runner ─────────────────────────────────────────────

struct TestMatrixConfig {
    std::vector<PluginTestEntry>    entries;
    IPCChannelConfig                ipcConfig;
    SoakTestConfig                  soakConfig;
    bool                            stopOnFirstFailure  { false };
    bool                            verboseLogging      { false };

    static TestMatrixConfig Default();
    static TestMatrixConfig CIMinimal();
    static TestMatrixConfig FullSoak();
};

inline TestMatrixConfig TestMatrixConfig::Default() {
    TestMatrixConfig cfg;
    cfg.ipcConfig = IPCChannelConfig::Default();
    cfg.soakConfig = SoakTestConfig::Standard();
    cfg.entries = {
        { PluginTestScenario::PluginLoad,        "Load",          "Load plugin from directory",   5000,  true  },
        { PluginTestScenario::PluginUnload,       "Unload",        "Unload plugin cleanly",        5000,  true  },
        { PluginTestScenario::PluginReload,       "Reload",        "Unload then reload cycle",     8000,  true  },
        { PluginTestScenario::DecodeValidImage,   "DecodeValid",   "Decode valid test image",      5000,  true  },
        { PluginTestScenario::DecodeCorruptData,  "DecodeCorrupt", "Decode corrupt payload",       5000,  true  },
        { PluginTestScenario::DecodeLargePayload, "DecodeLarge",   "Decode >64 MB payload",        15000, false },
        { PluginTestScenario::IPCRoundTrip,       "IPCRoundTrip",  "IPC ping-pong round-trip",     5000,  true  },
        { PluginTestScenario::IPCLatencyMeasure,  "IPCLatency",    "Measure IPC latency p95",      10000, true  },
        { PluginTestScenario::IPCReconnect,       "IPCReconnect",  "Reconnect after broken pipe",  8000,  false },
        { PluginTestScenario::PluginCrash,        "CrashRecover",  "Crash + restart sandbox",      10000, true  },
        { PluginTestScenario::PluginTimeout,      "TimeoutKill",   "Kill plugin exceeding limit",  8000,  true  },
        { PluginTestScenario::SoakIterations1K,   "Soak1K",        "1000-iteration plugin soak",   120000,true  },
        { PluginTestScenario::SoakOrphanCheck,    "OrphanCheck",   "No orphaned processes after",  10000, true  },
    };
    return cfg;
}

inline TestMatrixConfig TestMatrixConfig::CIMinimal() {
    auto cfg = Default();
    cfg.soakConfig = SoakTestConfig::Quick();
    // Remove long-running tests from CI
    for (auto& e : cfg.entries) {
        if (e.scenario == PluginTestScenario::SoakIterations1K)
            e.isRequired = false;
    }
    return cfg;
}

inline TestMatrixConfig TestMatrixConfig::FullSoak() {
    auto cfg = Default();
    cfg.soakConfig = SoakTestConfig::Stress();
    return cfg;
}

struct TestMatrixReport {
    std::vector<PluginTestResult>   results;
    IPCLatencyResult                ipcLatency;
    SoakTestResult                  soakResult;
    uint32_t                        passCount   { 0 };
    uint32_t                        failCount   { 0 };
    uint32_t                        skipCount   { 0 };
    double                          totalMs     { 0.0 };

    bool AllRequiredPassed() const { return failCount == 0; }

    std::string Summary() const {
        return "Matrix: " + std::to_string(passCount) + "P / " +
               std::to_string(failCount) + "F / " +
               std::to_string(skipCount) + "S — " +
               (AllRequiredPassed() ? "PASS" : "FAIL");
    }
};

// ─── Mock runner for unit tests ───────────────────────────────────────────────

class PluginRuntimeTestMatrixRunner {
public:
    explicit PluginRuntimeTestMatrixRunner(TestMatrixConfig cfg) : m_config(std::move(cfg)) {}

    /// Execute all matrix entries (no real plugin process — designed for unit test validation).
    TestMatrixReport RunDry() const {
        TestMatrixReport report;
        for (const auto& entry : m_config.entries) {
            PluginTestResult r;
            r.scenario   = entry.scenario;
            r.code       = PluginTestResultCode::Pass;   // dry run always pass
            r.message    = "dry-run";
            r.durationMs = 0.5;
            report.results.push_back(r);
            ++report.passCount;
        }
        report.ipcLatency = { 1.2, 8.4, 3.1, 6.2, 7.8, (int32_t)m_config.entries.size() };
        report.soakResult = { m_config.soakConfig.iterations,
                              m_config.soakConfig.iterations, 0, 0, 0, 10.0 };
        return report;
    }

    const TestMatrixConfig& Config() const { return m_config; }

private:
    TestMatrixConfig m_config;
};

} // namespace DarkThumbs::Plugin
