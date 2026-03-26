// RegressionTestSuite.h — Full Regression Test Matrix for v16.0.0 Horizon
// Copyright (c) 2026 ExplorerLens Project
//
// Defines the regression test execution and comparison framework used to
// validate that no previously-passing tests regress between releases.
// Produces a baseline snapshot (JSON) and a diff report against prior builds.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

// Categories of regression checks performed.
enum class RegressionCategory : uint8_t {
    Correctness  = 0x01,  // Pixel-accurate thumbnail regression (perceptual hash)
    Performance  = 0x02,  // Latency regressions > 10% vs baseline
    API          = 0x04,  // COM interface ABI compat
    Memory       = 0x08,  // Peak RSS regressions > 5%
    Stability    = 0x10,  // Crash/hang detection
    All          = 0xFF,
};

// Per-format regression result.
struct FormatRegressionResult {
    std::string  formatName;
    std::string  decoderName;
    bool         correctnessPass { true };
    bool         performancePass { true };
    bool         stabilityPass   { true };
    double       latencyMs       { 0.0 };
    double       baselineLatencyMs { 0.0 };
    double       latencyDeltaPct { 0.0 };
    uint32_t     pHashDistance   { 0 };  // Perceptual hash vs baseline (0 = identical)
};

// Summary of a full regression run.
struct RegressionRunSummary {
    std::string               version;
    std::string               baselineVersion;
    std::vector<FormatRegressionResult> results;
    uint32_t                  totalFormats   { 0 };
    uint32_t                  regressedFormats { 0 };
    uint32_t                  newFormats     { 0 };
    bool                      passed         { false };

    std::string Report() const;
};

// Callback invoked per format during regression run.
using RegressionProgressFn = std::function<void(const std::string& format,
                                                  uint32_t current,
                                                  uint32_t total)>;

// RegressionTestSuite — Automated format regression runner.
//
// Reads corpus from data/corpus/, decodes each sample with the current engine,
// compares pixel hashes and latency against a baseline snapshot JSON,
// and produces a structured diff report.
class RegressionTestSuite {
public:
    RegressionTestSuite() noexcept;
    ~RegressionTestSuite() noexcept;

    RegressionTestSuite(const RegressionTestSuite&)            = delete;
    RegressionTestSuite& operator=(const RegressionTestSuite&) = delete;

    // Load baseline snapshot produced by a previous SaveBaseline() call.
    bool LoadBaseline(const std::string& jsonPath) noexcept;

    // Save a new baseline from the current engine outputs.
    bool SaveBaseline(const std::string& jsonPath) noexcept;

    // Run the regression pass.  Returns false if any regressions found.
    bool Run(RegressionCategory  categories   = RegressionCategory::All,
             RegressionProgressFn onProgress  = nullptr) noexcept;

    // Retrieve last run summary.
    const RegressionRunSummary& LastResult() const noexcept { return m_last; }

    // Write full HTML regression report to path.
    bool WriteHtmlReport(const std::string& outputPath) const noexcept;

    // Set corpus directory (default: data/corpus/).
    void SetCorpusPath(const std::string& path) noexcept;

    // Set latency regression threshold (default: 10%).
    void SetLatencyThresholdPct(double pct) noexcept;

    // Set perceptual hash distance tolerance (default: 8).
    void SetPHashTolerance(uint32_t maxDistance) noexcept;

private:
    std::string          m_corpusPath;
    double               m_latencyThresholdPct { 10.0 };
    uint32_t             m_pHashTolerance      { 8 };
    RegressionRunSummary m_last;

    struct BaselineData;
    BaselineData*        m_baseline { nullptr };
};

}} // namespace ExplorerLens::Engine
