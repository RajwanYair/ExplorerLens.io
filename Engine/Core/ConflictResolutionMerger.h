// ConflictResolutionMerger.h — Annotation Conflict Resolution Merger
// Copyright (c) 2026 ExplorerLens Project
//
// Resolves editing conflicts in collaborative annotation sessions using
// last-write-wins, three-way merge, and user-prompt resolution strategies.
//
#pragma once
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ConflictResolutionStrategy {
    LastWriteWins,
    ThreeWayMerge,
    PromptUser,
    OwnerPriority
};
enum class ConflictType {
    StarRating,
    ColorLabel,
    Tag,
    Comment,
    Deletion
};
enum class MergeOutcome {
    Resolved,
    Conflicted,
    Deferred
};

struct ConflictEntry
{
    ConflictType type = ConflictType::StarRating;
    std::wstring filePath;
    std::string field;
    std::string localValue;
    std::string remoteValue;
    std::string baseValue;  // common ancestor (3-way)
    std::string localAuthor;
    std::string remoteAuthor;
    int64_t localTimestamp = 0;
    int64_t remoteTimestamp = 0;
};

struct CRMergeResult
{
    MergeOutcome outcome = MergeOutcome::Resolved;
    std::string resolvedValue;
    std::vector<ConflictEntry> remainingConflicts;
    int resolvedCount = 0;
    bool Ok() const noexcept
    {
        return outcome == MergeOutcome::Resolved;
    }
};

class ConflictResolutionMerger
{
  public:
    explicit ConflictResolutionMerger(ConflictResolutionStrategy strategy = ConflictResolutionStrategy::LastWriteWins)
        : m_strategy(strategy)
    {}

    CRMergeResult Resolve(const std::vector<ConflictEntry>& conflicts) const
    {
        CRMergeResult result;
        for (const auto& c : conflicts) {
            std::string resolved = ResolveOne(c);
            if (!resolved.empty()) {
                result.resolvedCount++;
                result.resolvedValue = resolved;
            } else {
                result.remainingConflicts.push_back(c);
            }
        }
        result.outcome = result.remainingConflicts.empty() ? MergeOutcome::Resolved : MergeOutcome::Conflicted;
        return result;
    }

    ConflictResolutionStrategy GetStrategy() const noexcept
    {
        return m_strategy;
    }
    void SetStrategy(ConflictResolutionStrategy s) noexcept
    {
        m_strategy = s;
    }

    static std::string StrategyName(ConflictResolutionStrategy s) noexcept
    {
        switch (s) {
            case ConflictResolutionStrategy::LastWriteWins:
                return "LastWriteWins";
            case ConflictResolutionStrategy::ThreeWayMerge:
                return "ThreeWayMerge";
            case ConflictResolutionStrategy::PromptUser:
                return "PromptUser";
            case ConflictResolutionStrategy::OwnerPriority:
                return "OwnerPriority";
        }
        return "Unknown";
    }

  private:
    std::string ResolveOne(const ConflictEntry& c) const
    {
        switch (m_strategy) {
            case ConflictResolutionStrategy::LastWriteWins:
                return (c.remoteTimestamp >= c.localTimestamp) ? c.remoteValue : c.localValue;
            case ConflictResolutionStrategy::ThreeWayMerge:
                if (c.localValue == c.baseValue)
                    return c.remoteValue;
                if (c.remoteValue == c.baseValue)
                    return c.localValue;
                return {};  // genuine conflict → defer
            case ConflictResolutionStrategy::OwnerPriority:
                return c.localValue;  // owner wins
            case ConflictResolutionStrategy::PromptUser:
                return {};  // deferred to UI
        }
        return {};
    }

    ConflictResolutionStrategy m_strategy;
};

}  // namespace Engine
}  // namespace ExplorerLens
