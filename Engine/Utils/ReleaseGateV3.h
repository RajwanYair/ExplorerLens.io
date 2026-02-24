#pragma once
//==============================================================================
// ReleaseGateV3 — Sprint 198
// v9.2 Release validation gate with comprehensive KPI checks
//
// Builds on ReleaseGateV2 (Sprint 172) with:
//   1. Platform coverage validation (x64 + ARM64)
//   2. MSIX package validation
//   3. High-DPI compatibility matrix
//   4. Malformed input resilience score
//   5. Plugin ecosystem health
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

/// KPI dimension for release gate
enum class ReleaseKPIDimension : uint8_t {
    BuildQuality,        ///< Zero warnings, zero errors
    TestCoverage,        ///< Test pass rate and count
    Performance,         ///< Decode speed, cache hit rate
    Stability,           ///< Crash rate, memory leaks
    Security,            ///< Malformed input resilience
    Compatibility,       ///< Platform, DPI, format coverage
    Documentation,       ///< Doc sync, changelog
    Packaging,           ///< MSI/MSIX build success
    Observability        ///< ETW, logging coverage
};

/// Gate verdict
enum class GateVerdict : uint8_t {
    Pass,
    ConditionalPass,     ///< Minor issues, can ship with known issues
    Fail,
    Blocked              ///< Cannot evaluate (missing data)
};

/// Single KPI measurement
struct KPIMeasurement {
    ReleaseKPIDimension dimension = ReleaseKPIDimension::BuildQuality;
    std::wstring name;
    double value = 0.0;
    double threshold = 0.0;
    bool passed = false;
    std::wstring unit;
    std::wstring notes;
};

/// Release gate thresholds for v9.2
struct ReleaseThresholdsV92 {
    uint32_t minTestCount = 500;
    double minTestPassRate = 99.5;       ///< %
    double maxSingleDecodeMs = 20.0;     ///< ms
    double minBatchThroughput = 200.0;   ///< img/sec
    double maxCacheHitMs = 5.0;          ///< ms
    uint32_t minDecoderCount = 25;
    uint32_t minShellExtensions = 90;
    double minCodeCoverage = 70.0;       ///< %
    uint32_t maxBuildWarnings = 0;
    uint32_t maxBuildErrors = 0;
    double minMalformedResilience = 95.0; ///< % of fuzz inputs handled
    bool requireARM64CI = true;
    bool requireMSIXPackage = true;
    bool requireHighDPI = true;
    uint32_t minDocArtifacts = 10;
};

/// Platform validation entry
struct PlatformValidation {
    std::wstring platform;           ///< e.g., "x64", "ARM64"
    bool buildSucceeded = false;
    bool testsRan = false;
    uint32_t testsPassed = 0;
    uint32_t testsFailed = 0;
    std::wstring notes;
};

/// Release gate result
struct ReleaseGateResult {
    GateVerdict verdict = GateVerdict::Blocked;
    std::wstring version;
    std::vector<KPIMeasurement> measurements;
    std::vector<PlatformValidation> platforms;
    uint32_t totalKPIs = 0;
    uint32_t passedKPIs = 0;
    uint32_t failedKPIs = 0;
    double overallScore = 0.0;
    std::vector<std::wstring> blockers;
    std::vector<std::wstring> warnings;
    std::wstring releaseNotes;
};

//==============================================================================
// ReleaseGateV3
//==============================================================================
class ReleaseGateV3 {
public:
    ReleaseGateV3();
    explicit ReleaseGateV3(const ReleaseThresholdsV92& thresholds);

    /// Run full release gate evaluation
    ReleaseGateResult Evaluate() const;

    /// Add a KPI measurement
    void AddMeasurement(const KPIMeasurement& m);

    /// Add platform validation
    void AddPlatform(const PlatformValidation& p);

    /// Generate release notes markdown
    std::wstring GenerateReleaseNotes(const ReleaseGateResult& result) const;

    /// Generate release checklist
    std::wstring GenerateChecklist() const;

    /// Get thresholds
    const ReleaseThresholdsV92& GetThresholds() const { return m_thresholds; }

    /// Static helpers
    static const wchar_t* GetDimensionName(ReleaseKPIDimension dim);
    static const wchar_t* GetVerdictName(GateVerdict verdict);

    /// Create default v9.2 thresholds
    static ReleaseThresholdsV92 ForV92();

private:
    ReleaseThresholdsV92 m_thresholds;
    std::vector<KPIMeasurement> m_measurements;
    std::vector<PlatformValidation> m_platforms;

    GateVerdict ComputeVerdict(uint32_t passed, uint32_t failed,
                                bool hasBlockers) const;
};

}} // namespace ExplorerLens::Engine
