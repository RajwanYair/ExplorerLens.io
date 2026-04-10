// SyncConflictResolver.h — Cross-Device Manifest Merge & Conflict Resolution
// Copyright (c) 2026 ExplorerLens Project
//
// Merges thumbnail manifests received from two devices with diverged caches.
// Uses latest-ETag-wins as the default strategy; caller-supplied comparators
// allow overrides for specific enterprise policies.
//
#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct SyncManifestEntry; // forward declare from DeviceSyncManifest.h

// How to resolve a conflict between two manifest entries for the same hash.
enum class ConflictResolutionMode : uint8_t {
    LATEST_ETAG,        // Pick the entry with the lexicographically greater ETag
    LATEST_GENERATED,   // Pick the entry with the newer generatedAt timestamp
    LOCAL_WINS,         // Always keep the local entry
    REMOTE_WINS,        // Always keep the remote entry
    CUSTOM              // Use the ConflictComparator callback
};

// Callback: return true if 'a' should be kept over 'b'.
using ConflictComparator =
    std::function<bool(const SyncManifestEntry& a, const SyncManifestEntry& b)>;

// Per-conflict record included in the merge result.
struct ConflictRecord {
    uint64_t pathHash = 0;
    bool     keptLocal = false; // true = kept local, false = kept remote
};

// Result of a merge operation.
struct MergeResult {
    size_t                    entriesMerged  = 0;
    size_t                    conflictsFound = 0;
    std::vector<ConflictRecord> conflicts;
};

// Merges two manifest entry lists, resolving entry-level conflicts.
class SyncConflictResolver {
public:
    explicit SyncConflictResolver(
        ConflictResolutionMode mode = ConflictResolutionMode::LATEST_ETAG,
        ConflictComparator     comparator = nullptr);

    // Merge remote entries into local. Returns the unified entry list
    // and a summary of conflicts encountered.
    MergeResult Merge(std::vector<SyncManifestEntry>& local,
                      const std::vector<SyncManifestEntry>& remote);

    ConflictResolutionMode Mode() const { return m_mode; }
    size_t TotalConflictsResolved() const { return m_totalConflicts; }

private:
    ConflictResolutionMode m_mode;
    ConflictComparator     m_comparator;
    size_t                 m_totalConflicts = 0;

    bool ShouldKeepLocal(const SyncManifestEntry& local,
                         const SyncManifestEntry& remote) const;
};

} // namespace Engine
} // namespace ExplorerLens
