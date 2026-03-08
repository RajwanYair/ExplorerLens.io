// ThumbnailCacheWarmer.h — Pre-generates thumbnails for frequent directories
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors directory access patterns and pre-generates thumbnails for
// frequently visited folders, reducing first-view latency.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct ThumbnailCacheWarmerConfig {
    bool enabled = true;
    uint32_t maxTrackedDirs = 128;
    uint32_t warmThreshold = 3;
    std::string label = "ThumbnailCacheWarmer";
};

class ThumbnailCacheWarmer {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ThumbnailCacheWarmerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    void RecordAccess(const std::string& dirPath) {
        if (m_accessCounts.size() >= m_config.maxTrackedDirs &&
            m_accessCounts.find(dirPath) == m_accessCounts.end())
            return;
        m_accessCounts[dirPath]++;
    }

    bool ShouldWarm(const std::string& dirPath) const {
        auto it = m_accessCounts.find(dirPath);
        return it != m_accessCounts.end() && it->second >= m_config.warmThreshold;
    }

    std::vector<std::string> GetHotDirectories() const {
        std::vector<std::string> hot;
        for (const auto& [path, count] : m_accessCounts)
            if (count >= m_config.warmThreshold)
                hot.push_back(path);
        return hot;
    }

    uint32_t GetTrackedCount() const {
        return static_cast<uint32_t>(m_accessCounts.size());
    }

private:
    bool m_initialized = false;
    ThumbnailCacheWarmerConfig m_config;
    std::unordered_map<std::string, uint32_t> m_accessCounts;
};

}
} // namespace ExplorerLens::Engine
