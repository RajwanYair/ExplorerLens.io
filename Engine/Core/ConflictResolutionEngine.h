// ConflictResolutionEngine.h — Concurrent Edit Conflict Arbitration Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Resolves thumbnail cache conflicts that arise when two or more users save the
// same cloud file concurrently.  The engine selects the canonical version using
// configurable strategies (latest ETag, largest mtime, or explicit user override)
// and appends an audit record for SIEM ingestion.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

/// Strategy used to resolve competing thumbnail versions.
enum class ConflictStrategy : uint8_t {
    LATEST_ETAG   = 0,  ///< Pick lexicographically largest ETag
    LATEST_MTIME  = 1,  ///< Pick most recent modification time
    LARGEST_SIZE  = 2,  ///< Pick version with the largest file size
    MANUAL        = 3,  ///< Defer to caller's explicit choice
};

/// One candidate version in a conflict set.
struct VersionCandidate {
    std::string  etag;
    uint64_t     mtimeMs   = 0;
    uint64_t     fileSize  = 0;
    uint32_t     userId    = 0;
};

/// Resolution outcome.
struct ConflictResolution {
    VersionCandidate winner;
    uint32_t         candidateCount  = 0;
    ConflictStrategy strategyApplied = ConflictStrategy::LATEST_MTIME;
    bool             wasConflict     = false; ///< false if only one candidate
};

/// Audit record written for each resolved conflict.
struct ConflictAuditEntry {
    std::wstring     filePath;
    ConflictResolution resolution;
    uint64_t         resolvedAtMs = 0;
};

/// Conflict resolution engine.
class ConflictResolutionEngine {
public:
    explicit ConflictResolutionEngine(ConflictStrategy strategy = ConflictStrategy::LATEST_MTIME);

    /// Resolve a set of competing candidates; returns the winning version.
    ConflictResolution Resolve(
        const std::wstring&              filePath,
        const std::vector<VersionCandidate>& candidates);

    /// Override the active strategy.
    void SetStrategy(ConflictStrategy strategy);

    ConflictStrategy GetStrategy() const;

    const std::vector<ConflictAuditEntry>& AuditLog() const;

    void ClearAuditLog();

private:
    ConflictStrategy                  m_strategy;
    std::vector<ConflictAuditEntry>   m_auditLog;

    static uint64_t NowMs();
};

}} // namespace ExplorerLens::Engine
