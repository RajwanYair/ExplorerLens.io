// DeadCodeAudit.h — Dead Code Detection and Cleanup Tracker
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks deprecated files, stale references, and dead code paths
// identified during the Zenith cleanup phase. Provides automated
// detection of common dead code patterns in the codebase.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Classification of dead code findings
enum class DeadCodeType : uint8_t {
    ObsoleteFile       = 0,   ///< Entire file superseded by newer version
    CommentedOutCode   = 1,   ///< Large blocks of commented-out code
    UnreachablePath    = 2,   ///< Code behind always-false conditions
    UnusedInclude      = 3,   ///< Header included but no symbols used
    DeprecatedAPI      = 4,   ///< Function declared deprecated, no callers
    StaleBackup        = 5,   ///< Files ending in _old, _backup, _bak
    OrphanedTest       = 6,   ///< Test for deleted functionality
    UnusedVariable     = 7    ///< Declared but never read
};

/// Severity of a dead code finding
enum class DeadCodeSeverity : uint8_t {
    Info     = 0,   ///< Informational only
    Low      = 1,   ///< Minor cleanup opportunity
    Medium   = 2,   ///< Should be cleaned up
    High     = 3,   ///< Actively confusing or maintenance burden
    Critical = 4    ///< May cause build issues or bugs
};

/// Status of a dead code finding
enum class DeadCodeStatus : uint8_t {
    Identified  = 0,   ///< Found but not yet addressed
    Confirmed   = 1,   ///< Verified as dead code
    Cleaned     = 2,   ///< Removed/fixed
    Deferred    = 3,   ///< Intentionally kept for now
    FalseAlarm  = 4    ///< Not actually dead code
};

/// A single dead code finding
struct DeadCodeFinding {
    const char* filePath = nullptr;
    const char* description = nullptr;
    DeadCodeType type = DeadCodeType::ObsoleteFile;
    DeadCodeSeverity severity = DeadCodeSeverity::Medium;
    DeadCodeStatus status = DeadCodeStatus::Identified;
    uint32_t lineStart = 0;
    uint32_t lineEnd = 0;
};

/// Dead code audit engine — tracks and validates cleanup
class DeadCodeAudit {
public:
    static DeadCodeAudit& Instance() {
        static DeadCodeAudit instance;
        return instance;
    }

    /// Get all findings from the cleanup audit
    const std::vector<DeadCodeFinding>& GetFindings() const { return m_findings; }

    /// Count findings by status
    uint32_t CountByStatus(DeadCodeStatus status) const {
        uint32_t count = 0;
        for (const auto& f : m_findings)
            if (f.status == status) ++count;
        return count;
    }

    /// Count findings by severity
    uint32_t CountBySeverity(DeadCodeSeverity severity) const {
        uint32_t count = 0;
        for (const auto& f : m_findings)
            if (f.severity == severity) ++count;
        return count;
    }

    /// Get cleanup completion percentage
    float GetCleanupProgress() const {
        if (m_findings.empty()) return 100.0f;
        uint32_t resolved = 0;
        for (const auto& f : m_findings)
            if (f.status == DeadCodeStatus::Cleaned || f.status == DeadCodeStatus::FalseAlarm)
                ++resolved;
        return 100.0f * static_cast<float>(resolved) / static_cast<float>(m_findings.size());
    }

    /// Check if all critical/high findings are resolved
    bool AllCriticalResolved() const {
        for (const auto& f : m_findings) {
            if (f.severity >= DeadCodeSeverity::High &&
                f.status != DeadCodeStatus::Cleaned &&
                f.status != DeadCodeStatus::FalseAlarm) {
                return false;
            }
        }
        return true;
    }

    /// Get summary string
    std::string GetSummary() const {
        std::string summary = "Dead Code Audit: ";
        summary += std::to_string(m_findings.size()) + " findings, ";
        summary += std::to_string(CountByStatus(DeadCodeStatus::Cleaned)) + " cleaned, ";
        summary += std::to_string(CountByStatus(DeadCodeStatus::Identified)) + " remaining";
        return summary;
    }

    /// Type name lookup
    static const char* TypeName(DeadCodeType t) {
        switch (t) {
            case DeadCodeType::ObsoleteFile:     return "ObsoleteFile";
            case DeadCodeType::CommentedOutCode:  return "CommentedOut";
            case DeadCodeType::UnreachablePath:   return "Unreachable";
            case DeadCodeType::UnusedInclude:     return "UnusedInclude";
            case DeadCodeType::DeprecatedAPI:     return "DeprecatedAPI";
            case DeadCodeType::StaleBackup:       return "StaleBackup";
            case DeadCodeType::OrphanedTest:      return "OrphanedTest";
            case DeadCodeType::UnusedVariable:    return "UnusedVar";
            default:                              return "Unknown";
        }
    }

    /// Severity name lookup
    static const char* SeverityName(DeadCodeSeverity s) {
        switch (s) {
            case DeadCodeSeverity::Info:     return "Info";
            case DeadCodeSeverity::Low:      return "Low";
            case DeadCodeSeverity::Medium:   return "Medium";
            case DeadCodeSeverity::High:     return "High";
            case DeadCodeSeverity::Critical: return "Critical";
            default:                         return "Unknown";
        }
    }

private:
    DeadCodeAudit() {
        // Populate with known findings from dead code audit
        m_findings = {
            { "Engine/Decoders/WMFDecoder_old.cpp",
              "Obsolete WMF decoder superseded by WMFDecoder.cpp",
              DeadCodeType::ObsoleteFile, DeadCodeSeverity::High,
              DeadCodeStatus::Cleaned },

            { "Engine/Decoders/unzip.cpp",
              "Legacy unzip implementation replaced by minizip-ng",
              DeadCodeType::ObsoleteFile, DeadCodeSeverity::High,
              DeadCodeStatus::Cleaned },

            { "LENSShell/LENSArchive.h",
              "Commented-out CBuffer template class (~30 lines)",
              DeadCodeType::CommentedOutCode, DeadCodeSeverity::Medium,
              DeadCodeStatus::Cleaned },

            { "Engine/CMakeLists.txt",
              "Commented-out add_subdirectory(PluginHost)",
              DeadCodeType::CommentedOutCode, DeadCodeSeverity::Medium,
              DeadCodeStatus::Confirmed },

            { "LENSShell/LENSArchive.h",
              "LENSTYPE constants duplicated in LENSTypes.h (Sprint 354 extraction)",
              DeadCodeType::DeprecatedAPI, DeadCodeSeverity::Low,
              DeadCodeStatus::Deferred },

            { "external/compression-libs/unrar/UnRAR.dll",
              "Binary blob in source tree — should be built from source",
              DeadCodeType::StaleBackup, DeadCodeSeverity::Medium,
              DeadCodeStatus::Confirmed },
        };
    }

    std::vector<DeadCodeFinding> m_findings;
};

} // namespace Engine
} // namespace ExplorerLens
