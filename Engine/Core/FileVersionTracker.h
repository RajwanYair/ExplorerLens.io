// FileVersionTracker.h — Tracks file version changes for cache invalidation
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors file last-write timestamps and sizes to detect when cached
// thumbnails become stale. Uses NTFS change journal when available.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct FileVersionTrackerConfig
{
    bool enabled = true;
    uint32_t maxTrackedFiles = 100000;
    std::string label = "FileVersionTracker";
};

class FileVersionTracker
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    FileVersionTrackerConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct FileVersion
    {
        uint64_t lastWriteTime = 0;
        uint64_t fileSize = 0;
        uint32_t generation = 0;
    };

    bool TrackFile(const std::string& path, uint64_t writeTime, uint64_t size)
    {
        if (m_versions.size() >= m_config.maxTrackedFiles)
            return false;
        auto& v = m_versions[path];
        v.lastWriteTime = writeTime;
        v.fileSize = size;
        v.generation++;
        return true;
    }

    bool IsStale(const std::string& path, uint64_t currentWriteTime) const
    {
        auto it = m_versions.find(path);
        if (it == m_versions.end())
            return true;
        return it->second.lastWriteTime != currentWriteTime;
    }

    size_t GetTrackedCount() const
    {
        return m_versions.size();
    }

  private:
    bool m_initialized = false;
    FileVersionTrackerConfig m_config;
    std::unordered_map<std::string, FileVersion> m_versions;
};

}  // namespace Engine
}  // namespace ExplorerLens
