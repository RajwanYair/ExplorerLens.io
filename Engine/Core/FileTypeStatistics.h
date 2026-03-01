#pragma once
// ============================================================================
// FileTypeStatistics.h — File type distribution statistics collection
//
// Purpose:   File type distribution statistics collection
// Provides:  StatPeriod enum, FormatStatEntry struct,
//            FileTypeStatistics class
// Used by:   Analytics dashboard
// ============================================================================

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Enums ────────────────────────────────────────────────────────────────────

enum class StatCategory : uint8_t {
    DecodeCount = 0,
    AvgTime = 1,
    ErrorRate = 2,
    CacheHitRate = 3,
    FileSizeAvg = 4
};

inline const char* StatCategoryName(StatCategory c) {
    switch (c) {
    case StatCategory::DecodeCount:  return "DecodeCount";
    case StatCategory::AvgTime:      return "AvgTime";
    case StatCategory::ErrorRate:    return "ErrorRate";
    case StatCategory::CacheHitRate: return "CacheHitRate";
    case StatCategory::FileSizeAvg:  return "FileSizeAvg";
    default:                         return "Unknown";
    }
}

enum class StatTimeRange : uint8_t {
    LastHour = 0,
    Today = 1,
    ThisWeek = 2,
    ThisMonth = 3,
    AllTime = 4
};

inline const char* StatTimeRangeName(StatTimeRange r) {
    switch (r) {
    case StatTimeRange::LastHour:  return "LastHour";
    case StatTimeRange::Today:     return "Today";
    case StatTimeRange::ThisWeek:  return "ThisWeek";
    case StatTimeRange::ThisMonth: return "ThisMonth";
    case StatTimeRange::AllTime:   return "AllTime";
    default:                       return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct FileTypeStat {
    std::string   extension;
    StatCategory  category = StatCategory::DecodeCount;
    double        value = 0.0;
    uint64_t      sampleCount = 0;
    StatTimeRange timeRange = StatTimeRange::AllTime;
};

// ── Class ────────────────────────────────────────────────────────────────────

class FileTypeStatistics {
public:
    FileTypeStatistics() = default;
    ~FileTypeStatistics() = default;

    // Record a decode event for a file type
    void RecordDecode(const std::string& extension, double timeMs,
        uint64_t sizeBytes, bool cacheHit, bool error) {
        auto& entry = m_data[extension];
        entry.totalDecodes++;
        entry.totalTimeMs += timeMs;
        entry.totalSizeBytes += sizeBytes;
        if (cacheHit) entry.cacheHits++;
        if (error)    entry.errors++;
        m_totalRecords++;
    }

    // Get stats for a specific extension and category
    FileTypeStat GetStats(const std::string& extension, StatCategory category) const {
        FileTypeStat stat;
        stat.extension = extension;
        stat.category = category;
        stat.timeRange = StatTimeRange::AllTime;

        auto it = m_data.find(extension);
        if (it == m_data.end())
            return stat;

        const auto& e = it->second;
        stat.sampleCount = e.totalDecodes;
        switch (category) {
        case StatCategory::DecodeCount:
            stat.value = static_cast<double>(e.totalDecodes);
            break;
        case StatCategory::AvgTime:
            stat.value = (e.totalDecodes > 0) ? e.totalTimeMs / e.totalDecodes : 0.0;
            break;
        case StatCategory::ErrorRate:
            stat.value = (e.totalDecodes > 0) ? 100.0 * e.errors / e.totalDecodes : 0.0;
            break;
        case StatCategory::CacheHitRate:
            stat.value = (e.totalDecodes > 0) ? 100.0 * e.cacheHits / e.totalDecodes : 0.0;
            break;
        case StatCategory::FileSizeAvg:
            stat.value = (e.totalDecodes > 0)
                ? static_cast<double>(e.totalSizeBytes) / e.totalDecodes : 0.0;
            break;
        }
        return stat;
    }

    // Get top N formats by decode count
    std::vector<std::string> GetTopFormats(size_t topN = 10) const {
        std::vector<std::pair<std::string, uint64_t>> sorted;
        for (const auto& [ext, data] : m_data)
            sorted.push_back({ ext, data.totalDecodes });
        std::sort(sorted.begin(), sorted.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        std::vector<std::string> result;
        for (size_t i = 0; i < topN && i < sorted.size(); i++)
            result.push_back(sorted[i].first);
        return result;
    }

    void ResetStats() {
        m_data.clear();
        m_totalRecords = 0;
    }

    size_t GetTrackedFormatCount() const { return m_data.size(); }
    uint64_t GetTotalRecords() const { return m_totalRecords; }

private:
    struct ExtensionData {
        uint64_t totalDecodes = 0;
        double   totalTimeMs = 0.0;
        uint64_t totalSizeBytes = 0;
        uint64_t cacheHits = 0;
        uint64_t errors = 0;
    };

    std::unordered_map<std::string, ExtensionData> m_data;
    uint64_t m_totalRecords = 0;
};

} // namespace Engine
} // namespace ExplorerLens
