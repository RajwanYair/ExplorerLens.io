// FileSystemWatchdog.h — Monitors directories for thumbnail invalidation
// Copyright (c) 2026 ExplorerLens Project
//
// Watches file system changes (create/modify/delete) in registered directories
// and triggers thumbnail cache invalidation for affected files.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct FileSystemWatchdogConfig {
    bool enabled = true;
    uint32_t maxWatchPaths = 64;
    uint32_t pollIntervalMs = 500;
    std::string label = "FileSystemWatchdog";
};

class FileSystemWatchdog {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    FileSystemWatchdogConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    bool AddWatchPath(const std::string& path) {
        if (m_watchPaths.size() >= m_config.maxWatchPaths) return false;
        m_watchPaths.push_back(path);
        return true;
    }

    bool RemoveWatchPath(const std::string& path) {
        for (auto it = m_watchPaths.begin(); it != m_watchPaths.end(); ++it) {
            if (*it == path) { m_watchPaths.erase(it); return true; }
        }
        return false;
    }

    uint32_t GetWatchCount() const { return static_cast<uint32_t>(m_watchPaths.size()); }

    struct ChangeEvent {
        enum Type { Created, Modified, Deleted, Renamed };
        Type type = Modified;
        std::string filePath;
    };

    std::vector<ChangeEvent> PollChanges() {
        // Platform-specific implementation would use ReadDirectoryChangesW
        return {};
    }

private:
    bool m_initialized = false;
    FileSystemWatchdogConfig m_config;
    std::vector<std::string> m_watchPaths;
};

}
} // namespace ExplorerLens::Engine
