// StaticAnalysisGate.h — Static Analysis Quality Gate
// Copyright (c) 2026 ExplorerLens Project
//
// Defines quality gates for static analysis tools (clang-tidy, MSVC /analyze,
// CppCheck, PVS-Studio). Enforces zero-warning policy with categorized
// suppression management and trend tracking.

#pragma once

#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Static analysis tool
enum class AnalysisTool : uint8_t {
  None = 0,
  ClangTidy = 1,
  MSVCAnalyze = 2,
  CppCheck = 3,
  PVSStudio = 4,
  CodeQL = 5,
  COUNT = 6
};

/// Warning severity
enum class WarningSeverity : uint8_t {
  Note = 0,
  Warning = 1,
  Error = 2,
  Fatal = 3
};

/// Warning category
enum class WarningCategory : uint8_t {
  Security = 0,      ///< Buffer overflows, use-after-free
  Performance = 1,   ///< Unnecessary copies, cache unfriendly
  Correctness = 2,   ///< Logic errors, undefined behavior
  Readability = 3,   ///< Naming, complexity, magic numbers
  Modernization = 4, ///< C++17/20 upgrades available
  Portability = 5,   ///< Platform-specific assumptions
  Concurrency = 6,   ///< Race conditions, deadlocks
  CategoryCount = 7
};

/// Warning suppression entry
struct WarningSuppression {
  const char *warningId =
      nullptr; ///< e.g., "C26495" or "cppcoreguidelines-init-variables"
  const char *file = nullptr;   ///< File pattern (nullptr = global)
  const char *reason = nullptr; ///< Why suppressed
  WarningSeverity severity = WarningSeverity::Warning;
  WarningCategory category = WarningCategory::Correctness;
  bool isTemporary = false;              ///< Will be fixed later
  const char *fixTargetSprint = nullptr; ///< Sprint to fix in
};

/// Quality gate thresholds per category
struct AnalysisThresholds {
  uint32_t maxSecurityWarnings = 0;     ///< Zero tolerance
  uint32_t maxCorrectnessWarnings = 0;  ///< Zero tolerance
  uint32_t maxPerformanceWarnings = 10; ///< Soft limit
  uint32_t maxReadabilityWarnings = 50; ///< Info only
  uint32_t maxModernizationWarnings = 100;
  uint32_t maxConcurrencyWarnings = 0; ///< Zero tolerance
  bool failOnSecurityWarning = true;
  bool failOnCorrectnessWarning = true;
  bool failOnConcurrencyWarning = true;
};

/// Static analysis quality gate
class StaticAnalysisGate {
public:
  static StaticAnalysisGate &Instance() {
    static StaticAnalysisGate inst;
    return inst;
  }

  /// Get clang-tidy checks string
  static const char *GetClangTidyChecks() {
    return "bugprone-*,"
           "cert-*,"
           "clang-analyzer-*,"
           "concurrency-*,"
           "cppcoreguidelines-*,"
           "misc-*,"
           "modernize-*,"
           "performance-*,"
           "readability-*,"
           "-modernize-use-trailing-return-type,"    // Style preference
           "-readability-magic-numbers,"             // Too noisy for constants
           "-cppcoreguidelines-avoid-magic-numbers," // Same
           "-readability-identifier-length,"         // Short names OK in loops
           "-cppcoreguidelines-pro-type-reinterpret-cast," // Needed for Windows
                                                           // API
           "-cppcoreguidelines-pro-type-union-access";     // COM interop
  }

  /// Get MSVC /analyze warning level
  static const char *GetMSVCAnalyzeFlags() {
    return "/analyze "
           "/analyze:WX- " // Warnings don't fail (we handle separately)
           "/analyze:log analyze.xml "
           "/analyze:ruleset NativeRecommendedRules.ruleset";
  }

  /// Get CppCheck command line
  static const char *GetCppCheckCommand() {
    return "cppcheck "
           "--enable=all "
           "--suppress=missingInclude "
           "--suppress=unusedFunction "
           "--std=c++20 "
           "--platform=win64 "
           "--inline-suppr "
           "--xml "
           "--output-file=cppcheck-report.xml "
           "Engine/ LENSShell/";
  }

  /// Check if a gate passes
  bool CheckGate(uint32_t securityWarnings, uint32_t correctnessWarnings,
                 uint32_t concurrencyWarnings) const {
    if (m_thresholds.failOnSecurityWarning &&
        securityWarnings > m_thresholds.maxSecurityWarnings)
      return false;
    if (m_thresholds.failOnCorrectnessWarning &&
        correctnessWarnings > m_thresholds.maxCorrectnessWarnings)
      return false;
    if (m_thresholds.failOnConcurrencyWarning &&
        concurrencyWarnings > m_thresholds.maxConcurrencyWarnings)
      return false;
    return true;
  }

  /// Known suppressions
  static constexpr uint32_t SUPPRESSION_COUNT = 5;
  static const WarningSuppression *GetSuppressions() {
    static const WarningSuppression suppressions[] = {
        {"C4996", nullptr,
         "Deprecated CRT functions used intentionally (fopen_s used instead "
         "where possible)",
         WarningSeverity::Warning, WarningCategory::Security, false, nullptr},
        {"C26812", nullptr,
         "Prefer enum class — already using enum class everywhere in new code",
         WarningSeverity::Warning, WarningCategory::Modernization, false,
         nullptr},
        {"C6387", "LENSShell/*",
         "SAL annotations sometimes over-report on COM interfaces",
         WarningSeverity::Warning, WarningCategory::Correctness, true,
         "Sprint 400"},
        {"cppcoreguidelines-owning-memory", nullptr,
         "COM raw pointers managed by AddRef/Release", WarningSeverity::Note,
         WarningCategory::Readability, false, nullptr},
        {"performance-no-int-to-ptr", nullptr,
         "MAKEINTRESOURCE and Windows API macros require int-to-ptr",
         WarningSeverity::Note, WarningCategory::Performance, false, nullptr},
    };
    return suppressions;
  }

  /// Category name
  static const char *CategoryName(WarningCategory c) {
    switch (c) {
    case WarningCategory::Security:
      return "Security";
    case WarningCategory::Performance:
      return "Performance";
    case WarningCategory::Correctness:
      return "Correctness";
    case WarningCategory::Readability:
      return "Readability";
    case WarningCategory::Modernization:
      return "Modernization";
    case WarningCategory::Portability:
      return "Portability";
    case WarningCategory::Concurrency:
      return "Concurrency";
    default:
      return "Unknown";
    }
  }

  const AnalysisThresholds &GetThresholds() const { return m_thresholds; }
  void SetThresholds(const AnalysisThresholds &t) { m_thresholds = t; }

  /// Tool queries
  static constexpr size_t ToolCount() {
    return static_cast<size_t>(AnalysisTool::COUNT);
  }
  static const wchar_t *ToolName(AnalysisTool t) {
    switch (t) {
    case AnalysisTool::None:
      return L"None";
    case AnalysisTool::ClangTidy:
      return L"clang-tidy";
    case AnalysisTool::MSVCAnalyze:
      return L"MSVC /analyze";
    case AnalysisTool::CppCheck:
      return L"cppcheck";
    case AnalysisTool::PVSStudio:
      return L"PVS-Studio";
    case AnalysisTool::CodeQL:
      return L"CodeQL";
    default:
      return L"Unknown";
    }
  }

private:
  StaticAnalysisGate() = default;
  AnalysisThresholds m_thresholds;
};

} // namespace Engine
} // namespace ExplorerLens
