// ConflictResolutionEngine.cpp — Concurrent Edit Conflict Arbitration Engine
// Copyright (c) 2026 ExplorerLens Project
//
#include "ConflictResolutionEngine.h"
#include <chrono>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

uint64_t ConflictResolutionEngine::NowMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

ConflictResolutionEngine::ConflictResolutionEngine(ConflictStrategy strategy)
    : m_strategy(strategy)
{}

ConflictResolution ConflictResolutionEngine::Resolve(
    const std::wstring&              filePath,
    const std::vector<VersionCandidate>& candidates)
{
    ConflictResolution res;
    res.candidateCount  = static_cast<uint32_t>(candidates.size());
    res.strategyApplied = m_strategy;
    res.wasConflict     = candidates.size() > 1;

    if (candidates.empty()) return res;

    VersionCandidate winner = candidates[0];

    if (candidates.size() > 1) {
        switch (m_strategy) {
            case ConflictStrategy::LATEST_ETAG:
                for (const auto& c : candidates)
                    if (c.etag > winner.etag) winner = c;
                break;
            case ConflictStrategy::LARGEST_SIZE:
                for (const auto& c : candidates)
                    if (c.fileSize > winner.fileSize) winner = c;
                break;
            case ConflictStrategy::MANUAL:
                break;  // Caller's first candidate is treated as the chosen winner
            default:   // LATEST_MTIME
                for (const auto& c : candidates)
                    if (c.mtimeMs > winner.mtimeMs) winner = c;
                break;
        }
    }

    res.winner = winner;

    // Append audit record
    ConflictAuditEntry entry;
    entry.filePath       = filePath;
    entry.resolution     = res;
    entry.resolvedAtMs   = NowMs();
    m_auditLog.push_back(std::move(entry));

    return res;
}

void ConflictResolutionEngine::SetStrategy(ConflictStrategy strategy)
{
    m_strategy = strategy;
}

ConflictStrategy ConflictResolutionEngine::GetStrategy() const
{
    return m_strategy;
}

const std::vector<ConflictAuditEntry>& ConflictResolutionEngine::AuditLog() const
{
    return m_auditLog;
}

void ConflictResolutionEngine::ClearAuditLog()
{
    m_auditLog.clear();
}

}} // namespace ExplorerLens::Engine
