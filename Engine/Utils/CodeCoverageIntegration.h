#pragma once
//==============================================================================
// CodeCoverageIntegration
// OpenCppCoverage integration + LibFuzzer harness for decoders
//
// Provides:
// 1. Coverage tracking infrastructure for CI integration
// 2. Fuzz target generation for all decoder formats
// 3. Coverage report merging and threshold validation
// 4. Structured fuzzing with format-aware seed corpus
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace ExplorerLens { namespace Engine {

/// Coverage metric types
enum class CoverageMetric : uint8_t {
 LineCoverage, ///< Lines executed / total lines
 BranchCoverage, ///< Branches taken / total branches
 FunctionCoverage, ///< Functions called / total functions
 RegionCoverage ///< Code regions executed / total regions
};

/// Fuzz target definition
enum class FuzzTargetType : uint8_t {
 HeaderParsing, ///< Test file header parsing
 FullDecode, ///< Test complete decode path
 MetadataExtraction, ///< Test metadata reading
 MalformedInput, ///< Test error handling
 BoundaryValues ///< Test dimension/size limits
};

/// Coverage result for a single module
struct ModuleCoverage {
 std::wstring moduleName;
 uint32_t totalLines = 0;
 uint32_t coveredLines = 0;
 uint32_t totalBranches = 0;
 uint32_t coveredBranches = 0;
 uint32_t totalFunctions = 0;
 uint32_t coveredFunctions = 0;
 double lineCoveragePercent = 0.0;
 double branchCoveragePercent = 0.0;
 double functionCoveragePercent = 0.0;
};

/// Fuzz test result
struct FuzzResult {
 std::wstring targetName;
 FuzzTargetType targetType = FuzzTargetType::HeaderParsing;
 uint64_t iterationsRun = 0;
 uint64_t crashesFound = 0;
 uint64_t timeoutsFound = 0;
 uint64_t oomFound = 0;
 double durationSeconds = 0.0;
 std::vector<std::wstring> crashInputPaths;
 double corpusCoverage = 0.0;
};

/// Coverage threshold configuration
struct CoverageThresholds {
 double minLineCoverage = 60.0; ///< Minimum line coverage %
 double minBranchCoverage = 40.0; ///< Minimum branch coverage %
 double minFunctionCoverage = 70.0; ///< Minimum function coverage %
 double targetLineCoverage = 80.0; ///< Target line coverage %
 double targetBranchCoverage = 60.0; ///< Target branch coverage %

 /// Default thresholds for CI gate
 static CoverageThresholds ForCI() {
 CoverageThresholds t;
 t.minLineCoverage = 60.0;
 t.minBranchCoverage = 40.0;
 t.minFunctionCoverage = 70.0;
 return t;
 }

 /// Strict thresholds for release gate
 static CoverageThresholds ForRelease() {
 CoverageThresholds t;
 t.minLineCoverage = 75.0;
 t.minBranchCoverage = 55.0;
 t.minFunctionCoverage = 85.0;
 return t;
 }
};

/// Fuzz configuration for a decoder format
struct FuzzTargetConfig {
 std::wstring decoderName;
 std::wstring seedCorpusDir;
 uint32_t maxInputSize = 65536; ///< Max input size in bytes
 uint32_t maxDurationSeconds = 300; ///< Max fuzz run duration
 uint64_t maxIterations = 100000; ///< Max fuzzing iterations
 std::vector<FuzzTargetType> targetTypes;
};

/// Overall coverage report
struct CoverageReport {
 std::vector<ModuleCoverage> modules;
 double overallLineCoverage = 0.0;
 double overallBranchCoverage = 0.0;
 double overallFunctionCoverage = 0.0;
 uint32_t totalModules = 0;
 bool meetsMinThresholds = false;
 bool meetsTargetThresholds = false;
 std::wstring reportPath;
 std::wstring timestamp;
};

//==============================================================================
// CodeCoverageIntegration
//==============================================================================
class CodeCoverageIntegration {
public:
 CodeCoverageIntegration();
 explicit CodeCoverageIntegration(const CoverageThresholds& thresholds);
 ~CodeCoverageIntegration() = default;

 /// Generate OpenCppCoverage command line for the engine
 std::wstring GenerateCoverageCommand(const std::wstring& testExecutable,
 const std::wstring& outputDir) const;

 /// Parse coverage results from OpenCppCoverage XML
 CoverageReport ParseCoverageReport(const std::wstring& xmlPath) const;

 /// Validate coverage against thresholds
 bool ValidateCoverage(const CoverageReport& report) const;

 /// Generate fuzz targets for all known decoders
 std::vector<FuzzTargetConfig> GenerateFuzzTargets() const;

 /// Get coverage for a specific module
 ModuleCoverage GetModuleCoverage(const std::wstring& moduleName,
 const CoverageReport& report) const;

 /// Merge multiple coverage reports
 CoverageReport MergeReports(const std::vector<CoverageReport>& reports) const;

 /// Get threshold configuration
 const CoverageThresholds& GetThresholds() const { return m_thresholds; }

 /// Static helpers
 static const wchar_t* GetMetricName(CoverageMetric metric);
 static const wchar_t* GetFuzzTargetName(FuzzTargetType type);

 /// Get list of all decoders that should be fuzzed
 static std::vector<std::wstring> GetFuzzableDecoders();

private:
 CoverageThresholds m_thresholds;
};

}} // namespace ExplorerLens::Engine

