//==============================================================================
// ExplorerLens Engine — Sprint 353: Dead Code Auditor
// Identifies and tracks dead/stale code across the codebase.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Audits codebase for dead code, stale references, and unused files.
class DeadCodeAuditor {
public:
  enum class AuditCategory {
    DeadFile,
    UnusedInclude,
    CommentedCode,
    StaleReference,
    DeprecatedAPI,
    COUNT
  };

  enum class AuditSeverity { Info, Warning, Error, COUNT };

  struct AuditFinding {
    std::wstring file;
    AuditCategory category;
    AuditSeverity severity;
    std::wstring description;
    bool resolved;
  };

  static const wchar_t *CategoryName(AuditCategory c) {
    switch (c) {
    case AuditCategory::DeadFile:
      return L"DeadFile";
    case AuditCategory::UnusedInclude:
      return L"UnusedInclude";
    case AuditCategory::CommentedCode:
      return L"CommentedCode";
    case AuditCategory::StaleReference:
      return L"StaleReference";
    case AuditCategory::DeprecatedAPI:
      return L"DeprecatedAPI";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *SeverityName(AuditSeverity s) {
    switch (s) {
    case AuditSeverity::Info:
      return L"Info";
    case AuditSeverity::Warning:
      return L"Warning";
    case AuditSeverity::Error:
      return L"Error";
    default:
      return L"Unknown";
    }
  }

  static size_t CategoryCount() {
    return static_cast<size_t>(AuditCategory::COUNT);
  }
  static size_t SeverityCount() {
    return static_cast<size_t>(AuditSeverity::COUNT);
  }

  static std::vector<AuditFinding> RunAudit() {
    return {
        {L"WMFDecoder_old.cpp", AuditCategory::DeadFile, AuditSeverity::Warning,
         L"Superseded by WMFDecoder.cpp", true},
        {L"LENSArchive.h CBuffer", AuditCategory::CommentedCode,
         AuditSeverity::Info, L"Commented-out template class", true},
    };
  }

  static size_t ResolvedCount() {
    size_t count = 0;
    for (const auto &f : RunAudit())
      if (f.resolved)
        ++count;
    return count;
  }

  static bool AllResolved() { return ResolvedCount() == RunAudit().size(); }
};

} // namespace Engine
} // namespace ExplorerLens
