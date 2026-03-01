#pragma once
// CodeCoverage.h — Consolidated Code Coverage Infrastructure
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for code coverage concerns:
// - Coverage tool selection (OpenCppCoverage/MSVC), metrics collection
// - Per-module coverage tracking and threshold enforcement
// - CI integration with LibFuzzer harness generation

#include <cstdint>
#include <string>
#include <vector>

// ─── CodeCoverageConfig ──────────────────────────────────────────────────────

namespace ExplorerLens {
namespace Engine {

/// Coverage tool selection
enum class CodeCoverageTool : uint8_t {
 None = 0,
 OpenCppCoverage = 1, ///< Open source, works with MSVC
 MSVCProfile = 2, ///< /Profile instrumentation
 VSInstrumental = 3, ///< VS Performance Profiler
 BullseyeCoverage = 4 ///< Commercial tool
};

/// Coverage metric type
enum class CodeCoverageMetric : uint8_t {
 Line = 0, ///< Line coverage
 Function = 1, ///< Function coverage
 Branch = 2, ///< Branch coverage
 Region = 3 ///< Region coverage (LLVM-style)
};

/// Module coverage report
struct ModuleCoverageReport {
 const char* moduleName = nullptr; ///< e.g., "Engine/Core"
 uint32_t totalLines = 0;
 uint32_t coveredLines = 0;
 uint32_t totalFunctions = 0;
 uint32_t coveredFunctions = 0;
 uint32_t totalBranches = 0;
 uint32_t coveredBranches = 0;

 float GetLineCoverage() const {
 return totalLines > 0 ? static_cast<float>(coveredLines) * 100.0f / static_cast<float>(totalLines) : 0.0f;
 }
 float GetFunctionCoverage() const {
 return totalFunctions > 0 ? static_cast<float>(coveredFunctions) * 100.0f / static_cast<float>(totalFunctions) : 0.0f;
 }
 float GetBranchCoverage() const {
 return totalBranches > 0 ? static_cast<float>(coveredBranches) * 100.0f / static_cast<float>(totalBranches) : 0.0f;
 }
};

/// Coverage threshold targets
struct CodeCoverageThresholds {
 float minLineCoverage = 70.0f; ///< Minimum line coverage %
 float minFunctionCoverage = 80.0f; ///< Minimum function coverage %
 float minBranchCoverage = 50.0f; ///< Minimum branch coverage %
 float targetLineCoverage = 85.0f; ///< Target line coverage %
 bool failOnThresholdMiss = false; ///< Fail CI if below min
 bool warnOnTargetMiss = true; ///< Warn if below target
};

/// Exclusion patterns (files/dirs to skip)
struct CoverageExclusion {
 const char* pattern = nullptr; ///< Glob pattern
 const char* reason = nullptr; ///< Why excluded
};

/// Code coverage configuration
class CodeCoverageConfig {
public:
 static CodeCoverageConfig& Instance() {
 static CodeCoverageConfig inst;
 return inst;
 }

 /// Get OpenCppCoverage command line
 std::string GetOpenCppCoverageCommand(const char* testExe,
 const char* outputDir = "coverage") const {
 std::string cmd = "OpenCppCoverage.exe";
 cmd += " --sources Engine\\";
 cmd += " --sources LENSShell\\";
 cmd += " --excluded_sources external\\";
 cmd += " --excluded_sources gtest\\";
 cmd += " --excluded_sources build\\";

 // Add exclusion patterns
 for (uint32_t i = 0; i < EXCLUSION_COUNT; i++) {
 const auto& ex = GetExclusions()[i];
 cmd += " --excluded_sources ";
 cmd += ex.pattern;
 }

 cmd += " --export_type=html:";
 cmd += outputDir;
 cmd += "\\html";
 cmd += " --export_type=cobertura:";
 cmd += outputDir;
 cmd += "\\coverage.xml";
 cmd += " -- ";
 cmd += testExe;
 return cmd;
 }

 /// Check if coverage meets thresholds
 bool CheckThresholds(const ModuleCoverageReport& report) const {
 return report.GetLineCoverage() >= m_thresholds.minLineCoverage &&
 report.GetFunctionCoverage() >= m_thresholds.minFunctionCoverage &&
 report.GetBranchCoverage() >= m_thresholds.minBranchCoverage;
 }

 /// Get coverage thresholds
 const CodeCoverageThresholds& GetThresholds() const { return m_thresholds; }
 void SetThresholds(const CodeCoverageThresholds& t) { m_thresholds = t; }

 /// Default exclusion patterns
 static constexpr uint32_t EXCLUSION_COUNT = 6;
 static const CoverageExclusion* GetExclusions() {
 static const CoverageExclusion exclusions[] = {
 { "external\\*", "Third-party libraries" },
 { "gtest\\*", "Test framework" },
 { "build\\*", "Build artifacts" },
 { "Engine\\Tests\\*", "Test code itself" },
 { "packages\\*", "NuGet packages" },
 { "*_generated.*", "Auto-generated code" }
 };
 return exclusions;
 }

 /// Selected coverage tool
 CodeCoverageTool tool = CodeCoverageTool::OpenCppCoverage;

 /// Report format
 bool generateHTML = true;
 bool generateCobertura = true; ///< For CI integration (Azure DevOps/GitHub)
 bool generateLCOV = false; ///< For Codecov/Coveralls

 /// Module-specific targets
 struct ModuleTarget {
 const char* module = nullptr;
 float targetCoverage = 80.0f;
 };

 static constexpr uint32_t MODULE_TARGET_COUNT = 5;
 static const ModuleTarget* GetModuleTargets() {
 static const ModuleTarget targets[] = {
 { "Engine/Core", 85.0f },
 { "Engine/Decoders", 75.0f },
 { "Engine/Cache", 80.0f },
 { "Engine/Pipeline", 70.0f },
 { "Engine/Utils", 60.0f }
 };
 return targets;
 }

private:
 CodeCoverageConfig() = default;
 CodeCoverageThresholds m_thresholds;
};

} // namespace Engine
} // namespace ExplorerLens

// ─── CodeCoverageIntegration (separate — has .cpp) ───────────────────────────

#include "CodeCoverageIntegration.h"
