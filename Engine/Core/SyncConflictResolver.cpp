// SyncConflictResolver.cpp — Cross-Device Manifest Conflict Resolution
// Copyright (c) 2026 ExplorerLens Project
//
#include "SyncConflictResolver.h"
#include "DeviceSyncManifest.h"
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

SyncConflictResolver::SyncConflictResolver(ConflictResolutionMode mode,
                                           ConflictComparator comparator)
    : m_mode(mode)
    , m_comparator(std::move(comparator))
{}

bool SyncConflictResolver::ShouldKeepLocal(const SyncManifestEntry& local,
                                           const SyncManifestEntry& remote) const
{
    switch (m_mode) {
    case ConflictResolutionMode::LATEST_ETAG:
        return local.etag >= remote.etag;
    case ConflictResolutionMode::LATEST_GENERATED:
        return local.generatedAt >= remote.generatedAt;
    case ConflictResolutionMode::LOCAL_WINS:
        return true;
    case ConflictResolutionMode::REMOTE_WINS:
        return false;
    case ConflictResolutionMode::CUSTOM:
        return m_comparator ? m_comparator(local, remote) : true;
    }
    return true;
}

MergeResult SyncConflictResolver::Merge(std::vector<SyncManifestEntry>& local,
                                        const std::vector<SyncManifestEntry>& remote)
{
    MergeResult result;

    for (const auto& remoteEntry : remote) {
        auto it = std::find_if(local.begin(), local.end(),
            [&](const SyncManifestEntry& le){ return le.pathHash == remoteEntry.pathHash; });

        if (it == local.end()) {
            // No conflict — remote has an entry local doesn't.
            local.push_back(remoteEntry);
            ++result.entriesMerged;
        } else {
            // Conflict — same path hash exists on both sides.
            ++result.conflictsFound;
            ++m_totalConflicts;
            const bool keepLocal = ShouldKeepLocal(*it, remoteEntry);
            if (!keepLocal) {
                *it = remoteEntry;
            }
            result.conflicts.push_back({remoteEntry.pathHash, keepLocal});
            ++result.entriesMerged;
        }
    }
    return result;
}

} // namespace Engine
} // namespace ExplorerLens
