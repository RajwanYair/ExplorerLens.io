#pragma once
//==============================================================================
// ReleaseGateV10 — Release Quality Gate
// v10.0.0 Release validation gate with extended KPI matrix.
//
// Builds on ReleaseGateV3 with:
//   1. Scientific format validation (DICOM/FITS)
//   2. Advanced 3D format coverage
//   3. Plugin marketplace readiness
//   4. Cross-API GPU validation (Vulkan + D3D12 + D3D11)
//   5. Python SDK interop verification
//   6. 30+ decoder coverage matrix
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

/// v10 KPI category
enum class V10KPICategory : uint8_t {
    BuildSystem,
    TestCoverage,
    FormatCoverage,
    Performance,
    GPUBackends,
    PluginEcosystem,
    PythonSDK,
    Documentation,
    Packaging,
    Security,
    Compatibility,
    Scientific
};

/// v10 format coverage entry
struct FormatCoverageEntry {
    std::wstring category;      ///< e.g., "Image", "Archive", "Scientific"
    uint32_t totalFormats = 0;
    uint32_t supportedFormats = 0;
    uint32_t registeredInShell = 0;
    double coveragePercent = 0.0;
};

/// v10 GPU backend test result
struct GPUBackendResult {
    std::wstring backend;       ///< "Vulkan", "D3D12", "D3D11", "CPU"
    bool available = false;
    bool passed = false;
    double resizeTimeMs = 0.0;
    std::wstring notes;
};

/// v10 release thresholds
struct ReleaseThresholdsV10 {
    uint32_t minDecoderCount = 30;
    uint32_t minTestCount = 600;
    double minTestPassRate = 99.8;
    double maxSingleDecodeMs = 15.0;
    double minBatchThroughput = 300.0;
    uint32_t minShellRegistrations = 110;
    uint32_t minFormatCategories = 15;
    double minFormatCoverage = 90.0;
    uint32_t minGPUBackends = 2;
    bool requirePluginMarketplace = true;
    bool requirePythonSDK = true;
    bool requireScientificFormats = true;
    bool requireVulkanSupport = false;  ///< Optional — fallback OK
    uint32_t maxBuildWarnings = 0;
};

/// v10 comprehensive release result
struct V10ReleaseResult {
    std::wstring version = L"v10.0.0";
    bool passed = false;
    double overallScore = 0.0;
    uint32_t totalChecks = 0;
    uint32_t passedChecks = 0;
    std::vector<FormatCoverageEntry> formatCoverage;
    std::vector<GPUBackendResult> gpuBackends;
    std::vector<std::wstring> blockers;
    std::vector<std::wstring> warnings;
    std::vector<std::wstring> achievements; ///< Notable milestones
    std::wstring changelog;
};

//==============================================================================
// ReleaseGateV10
//==============================================================================
class ReleaseGateV10 {
public:
    ReleaseGateV10();
    explicit ReleaseGateV10(const ReleaseThresholdsV10& thresholds);

    /// Run comprehensive v10 release evaluation
    V10ReleaseResult Evaluate() const;

    /// Add format coverage data
    void AddFormatCoverage(const FormatCoverageEntry& entry);

    /// Add GPU backend test result
    void AddGPUBackend(const GPUBackendResult& result);

    /// Set test metrics
    void SetTestMetrics(uint32_t count, uint32_t passed, double passRate);

    /// Set decoder count
    void SetDecoderCount(uint32_t count);

    /// Set shell registration count
    void SetShellRegistrations(uint32_t count);

    /// Get thresholds
    const ReleaseThresholdsV10& GetThresholds() const { return m_thresholds; }

    /// Generate v10 changelog
    std::wstring GenerateChangelog() const;

    /// Generate v10 release notes
    std::wstring GenerateReleaseNotes(const V10ReleaseResult& result) const;

    /// Get category name
    static const wchar_t* GetCategoryName(V10KPICategory cat);

    /// Create default v10 thresholds
    static ReleaseThresholdsV10 ForV10();

private:
    ReleaseThresholdsV10 m_thresholds;
    std::vector<FormatCoverageEntry> m_formatCoverage;
    std::vector<GPUBackendResult> m_gpuBackends;
    uint32_t m_testCount = 0;
    uint32_t m_testsPassed = 0;
    double m_testPassRate = 0.0;
    uint32_t m_decoderCount = 0;
    uint32_t m_shellRegistrations = 0;
};

}} // namespace ExplorerLens::Engine
