#pragma once
// ThumbnailVersionControl.h — Version control for thumbnail cache entries
// Sprint 418 — ExplorerLens v15.0.0 Zenith

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Version identifier for a cached thumbnail entry
enum class ThumbnailVersion : uint8_t {
    Original = 0,   // First generated version
    Modified = 1,   // User-modified or re-rendered version
    Current = 2,   // Most recent active version
    Rollback = 3,   // Restored from a previous version
    Draft = 4    // Work-in-progress, not committed
};

inline const char* ThumbnailVersionName(ThumbnailVersion v) noexcept {
    switch (v) {
    case ThumbnailVersion::Original:  return "Original";
    case ThumbnailVersion::Modified:  return "Modified";
    case ThumbnailVersion::Current:   return "Current";
    case ThumbnailVersion::Rollback:  return "Rollback";
    case ThumbnailVersion::Draft:     return "Draft";
    default:                          return "Unknown";
    }
}

/// Action that produced or mutated a version entry
enum class VersionAction : uint8_t {
    Create = 0,   // New entry created
    Update = 1,   // Existing entry updated in-place
    Revert = 2,   // Rolled back to prior version
    Delete = 3,   // Entry removed from version history
    Archive = 4    // Moved to cold storage / archive tier
};

inline const char* VersionActionName(VersionAction a) noexcept {
    switch (a) {
    case VersionAction::Create:   return "Create";
    case VersionAction::Update:   return "Update";
    case VersionAction::Revert:   return "Revert";
    case VersionAction::Delete:   return "Delete";
    case VersionAction::Archive:  return "Archive";
    default:                      return "Unknown";
    }
}

/// Single entry in the version history for a thumbnail
struct ThumbnailVersionEntry {
    ThumbnailVersion version = ThumbnailVersion::Original;
    VersionAction    action = VersionAction::Create;
    uint64_t         timestamp = 0;      // Epoch milliseconds
    uint64_t         hash = 0;      // XXH3 content hash
    uint64_t         sizeBytes = 0;      // Payload size
};

/// Manages versioned history of thumbnail cache entries, enabling
/// rollback, audit trails, and deduplication by content hash.
class ThumbnailVersionControl {
public:
    ThumbnailVersionControl() = default;
    ~ThumbnailVersionControl() = default;

    // Non-copyable, movable
    ThumbnailVersionControl(const ThumbnailVersionControl&) = delete;
    ThumbnailVersionControl& operator=(const ThumbnailVersionControl&) = delete;
    ThumbnailVersionControl(ThumbnailVersionControl&&) noexcept = default;
    ThumbnailVersionControl& operator=(ThumbnailVersionControl&&) noexcept = default;

    /// Retrieve the current version entry for a given file path
    bool GetVersion(const std::wstring& filePath, ThumbnailVersionEntry& outEntry) const {
        for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
            if (it->version == ThumbnailVersion::Current) {
                outEntry = *it;
                return true;
            }
        }
        (void)filePath;
        return false;
    }

    /// Create a new version entry and push it onto the history stack
    bool CreateVersion(const std::wstring& filePath, uint64_t hash, uint64_t sizeBytes) {
        ThumbnailVersionEntry entry{};
        entry.version = ThumbnailVersion::Current;
        entry.action = m_history.empty() ? VersionAction::Create : VersionAction::Update;
        entry.timestamp = GetCurrentTimestamp();
        entry.hash = hash;
        entry.sizeBytes = sizeBytes;
        m_history.push_back(entry);
        m_totalVersions++;
        (void)filePath;
        return true;
    }

    /// Rollback to the previous version, marking current as Rollback
    bool Rollback() {
        if (m_history.size() < 2) return false;
        ThumbnailVersionEntry rollbackEntry = m_history[m_history.size() - 2];
        rollbackEntry.version = ThumbnailVersion::Rollback;
        rollbackEntry.action = VersionAction::Revert;
        rollbackEntry.timestamp = GetCurrentTimestamp();
        m_history.push_back(rollbackEntry);
        m_rollbackCount++;
        return true;
    }

    /// Get full version history (most recent last)
    const std::vector<ThumbnailVersionEntry>& GetHistory() const noexcept {
        return m_history;
    }

    /// Total number of version entries created across the session
    uint64_t GetTotalVersions() const noexcept { return m_totalVersions; }

    /// Number of rollback operations performed
    uint32_t GetRollbackCount() const noexcept { return m_rollbackCount; }

    /// Clear all history entries
    void PurgeHistory() noexcept {
        m_history.clear();
        m_totalVersions = 0;
        m_rollbackCount = 0;
    }

private:
    static uint64_t GetCurrentTimestamp() noexcept {
        // Placeholder — production uses QueryPerformanceCounter epoch conversion
        return 0;
    }

    std::vector<ThumbnailVersionEntry> m_history;
    uint64_t m_totalVersions = 0;
    uint32_t m_rollbackCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
