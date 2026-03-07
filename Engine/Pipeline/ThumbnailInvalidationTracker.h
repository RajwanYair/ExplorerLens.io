// ThumbnailInvalidationTracker.h — Thumbnail Freshness Tracker
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks thumbnail cache validity by monitoring file modification times,
// content hashes, and invalidation events to ensure cache freshness.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class InvalidationReason : uint8_t {
    FileModified,
    FileDeleted,
    FileRenamed,
    ContentChanged,
    ManualPurge,
    TTLExpired,
    PolicyChange
};

struct InvalidationRecord {
    std::wstring filePath;
    InvalidationReason reason = InvalidationReason::FileModified;
    uint64_t timestamp = 0;
    uint64_t previousModTime = 0;
    uint64_t newModTime = 0;
};

struct TrackerStats {
    uint64_t totalTracked = 0;
    uint64_t totalInvalidated = 0;
    uint64_t totalRefreshed = 0;
    uint64_t staleEntries = 0;
};

class ThumbnailInvalidationTracker {
public:
    ThumbnailInvalidationTracker() = default;

    void Track(const std::wstring& filePath, uint64_t modTime) {
        m_entries[filePath] = modTime;
        m_stats.totalTracked++;
    }

    bool IsValid(const std::wstring& filePath, uint64_t currentModTime) const {
        auto it = m_entries.find(filePath);
        if (it == m_entries.end()) return false;
        return it->second == currentModTime;
    }

    void Invalidate(const std::wstring& filePath, InvalidationReason reason) {
        InvalidationRecord record;
        record.filePath = filePath;
        record.reason = reason;
        m_invalidations.push_back(record);
        m_entries.erase(filePath);
        m_stats.totalInvalidated++;
    }

    void Refresh(const std::wstring& filePath, uint64_t newModTime) {
        m_entries[filePath] = newModTime;
        m_stats.totalRefreshed++;
    }

    std::vector<InvalidationRecord> GetRecentInvalidations(size_t maxCount = 50) const {
        if (m_invalidations.size() <= maxCount) return m_invalidations;
        return std::vector<InvalidationRecord>(
            m_invalidations.end() - static_cast<ptrdiff_t>(maxCount),
            m_invalidations.end());
    }

    TrackerStats GetStats() const { return m_stats; }
    size_t GetTrackedCount() const { return m_entries.size(); }

    void Clear() {
        m_entries.clear();
        m_invalidations.clear();
    }

private:
    std::unordered_map<std::wstring, uint64_t> m_entries;
    std::vector<InvalidationRecord> m_invalidations;
    TrackerStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
