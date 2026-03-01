// StaticAnalysisCIGate.h — Static Analysis CI Gate
// Copyright (c) 2026 ExplorerLens Project
//
// CI gate that integrates multiple static analysis tools (clang-tidy,
// MSVC /analyze, cppcheck, PVS-Studio) with configurable severity
// thresholds and diff-aware filtering for pull request checks.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Enums
// ============================================================================

/// Static analysis tool identifier
enum class SAToolId : uint8_t {
    ClangTidy = 0,
    MSVCAnalyze,
    CppCheck,
    PVSStudio,
    Coverity,
    COUNT
};

/// Finding severity level
enum class SAFindingSeverity : uint8_t {
    Note = 0,
    Warning,
    Error,
    Critical,
    COUNT
};

/// Gate verdict for CI pass/fail
enum class SAGateVerdict : uint8_t {
    Pass = 0,       ///< All checks within threshold
    Warn,           ///< Below threshold but findings exist
    Fail,           ///< Threshold exceeded
    Error,          ///< Tool execution error
    COUNT
};

// ============================================================================
// String conversions
// ============================================================================

inline const char* SAToolIdToString(SAToolId t) {
    static const char* names[] = {
        "clang-tidy", "MSVC /analyze", "cppcheck", "PVS-Studio", "Coverity"
    };
    auto idx = static_cast<uint8_t>(t);
    return (idx < static_cast<uint8_t>(SAToolId::COUNT)) ? names[idx] : "Unknown";
}

inline const char* SAFindingSeverityToString(SAFindingSeverity s) {
    static const char* names[] = { "Note", "Warning", "Error", "Critical" };
    auto idx = static_cast<uint8_t>(s);
    return (idx < static_cast<uint8_t>(SAFindingSeverity::COUNT)) ? names[idx] : "Unknown";
}

inline const char* SAGateVerdictToString(SAGateVerdict v) {
    static const char* names[] = { "Pass", "Warn", "Fail", "Error" };
    auto idx = static_cast<uint8_t>(v);
    return (idx < static_cast<uint8_t>(SAGateVerdict::COUNT)) ? names[idx] : "Unknown";
}

// ============================================================================
// Structs
// ============================================================================

/// A single static analysis finding
struct SAFinding {
    SAToolId            tool = SAToolId::ClangTidy;
    SAFindingSeverity   severity = SAFindingSeverity::Note;
    std::string         checkName;      ///< e.g., "bugprone-use-after-move"
    std::string         filePath;
    uint32_t            line = 0;
    uint32_t            column = 0;
    std::string         message;
    bool                isNewInDiff = false;  ///< Only in changed lines
};

/// Configuration thresholds for pass/fail
struct SAGateThresholds {
    uint32_t maxErrors = 0;         ///< Max errors before fail
    uint32_t maxWarnings = 10;      ///< Max warnings before fail
    uint32_t maxCriticals = 0;      ///< Max criticals before fail
    bool     diffAwareOnly = false; ///< Only count findings in changed files
    bool     failOnNewErrors = true;
};

/// Per-tool execution result
struct SAToolResult {
    SAToolId    tool = SAToolId::ClangTidy;
    bool        executed = false;
    double      durationSec = 0.0;
    uint32_t    filesScanned = 0;
    uint32_t    findingsCount = 0;
    std::string errorOutput;
};

/// Aggregate gate result
struct SAGateResult {
    SAGateVerdict verdict = SAGateVerdict::Pass;
    uint32_t    totalFindings = 0;
    uint32_t    errors = 0;
    uint32_t    warnings = 0;
    uint32_t    criticals = 0;
    uint32_t    notes = 0;
    uint32_t    newInDiff = 0;
    std::vector<SAToolResult> toolResults;

    double TotalDurationSec() const {
        double total = 0.0;
        for (const auto& t : toolResults) total += t.durationSec;
        return total;
    }
};

// ============================================================================
// StaticAnalysisCIGate class
// ============================================================================

class StaticAnalysisCIGate {
public:
    /// Configure gate thresholds
    void SetThresholds(const SAGateThresholds& t) { m_thresholds = t; }
    const SAGateThresholds& GetThresholds() const { return m_thresholds; }

    /// Enable/disable a specific tool
    void EnableTool(SAToolId tool) {
        auto idx = static_cast<uint8_t>(tool);
        if (idx < static_cast<uint8_t>(SAToolId::COUNT))
            m_enabledTools[idx] = true;
    }

    void DisableTool(SAToolId tool) {
        auto idx = static_cast<uint8_t>(tool);
        if (idx < static_cast<uint8_t>(SAToolId::COUNT))
            m_enabledTools[idx] = false;
    }

    bool IsToolEnabled(SAToolId tool) const {
        auto idx = static_cast<uint8_t>(tool);
        return (idx < static_cast<uint8_t>(SAToolId::COUNT)) ? m_enabledTools[idx] : false;
    }

    /// Add a finding (simulates tool output parsing)
    void AddFinding(const SAFinding& finding) {
        m_findings.push_back(finding);
    }

    /// Record a tool execution result
    void RecordToolRun(SAToolId tool, double durationSec, uint32_t filesScanned) {
        SAToolResult result;
        result.tool = tool;
        result.executed = true;
        result.durationSec = durationSec;
        result.filesScanned = filesScanned;
        result.findingsCount = 0;
        for (const auto& f : m_findings)
            if (f.tool == tool) result.findingsCount++;
        m_toolResults.push_back(std::move(result));
    }

    /// Evaluate gate verdict based on findings + thresholds
    SAGateResult Evaluate() const {
        SAGateResult result;
        result.toolResults = m_toolResults;

        for (const auto& f : m_findings) {
            if (m_thresholds.diffAwareOnly && !f.isNewInDiff) continue;
            result.totalFindings++;
            if (f.isNewInDiff) result.newInDiff++;

            switch (f.severity) {
            case SAFindingSeverity::Critical: result.criticals++; break;
            case SAFindingSeverity::Error:    result.errors++; break;
            case SAFindingSeverity::Warning:  result.warnings++; break;
            case SAFindingSeverity::Note:     result.notes++; break;
            }
        }

        // Determine verdict
        if (result.criticals > m_thresholds.maxCriticals ||
            result.errors > m_thresholds.maxErrors) {
            result.verdict = SAGateVerdict::Fail;
        }
        else if (result.warnings > m_thresholds.maxWarnings) {
            result.verdict = SAGateVerdict::Fail;
        }
        else if (result.totalFindings > 0) {
            result.verdict = SAGateVerdict::Warn;
        }
        else {
            result.verdict = SAGateVerdict::Pass;
        }

        return result;
    }

    /// Get all findings
    const std::vector<SAFinding>& GetFindings() const { return m_findings; }

    /// Get findings for a specific tool
    std::vector<SAFinding> GetFindingsForTool(SAToolId tool) const {
        std::vector<SAFinding> filtered;
        for (const auto& f : m_findings)
            if (f.tool == tool) filtered.push_back(f);
        return filtered;
    }

    /// Clear all findings and results
    void Clear() { m_findings.clear(); m_toolResults.clear(); }

private:
    SAGateThresholds                m_thresholds;
    bool                            m_enabledTools[static_cast<uint8_t>(SAToolId::COUNT)] = { true, true, true, false, false };
    std::vector<SAFinding>          m_findings;
    std::vector<SAToolResult>       m_toolResults;
};

} // namespace Engine
} // namespace ExplorerLens
