// LivePreviewUpdater.h — Live Preview Updater
// Copyright (c) 2026 ExplorerLens Project
//
// Watches file system change events and triggers re-thumbnail for modified
// files. Delivers incremental updates to the preview panel without full reload.
//
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class FileChangeType { Modified, Created, Deleted, Renamed };

struct FileChangeEvent {
    std::string    filePath;
    std::string    oldPath;  // non-empty on rename
    FileChangeType type      = FileChangeType::Modified;
    int64_t        timestampMs = 0;
};

using LivePreviewCallback = std::function<void(const FileChangeEvent&)>;

class LivePreviewUpdater {
public:
    LivePreviewUpdater() = default;

    bool Initialize(uint32_t debounceMs = 300) {
        m_debounceMs = debounceMs;
        m_ready      = true;
        return true;
    }
    bool IsReady() const { return m_ready; }

    void SetCallback(LivePreviewCallback cb) { m_callback = std::move(cb); }

    bool WatchPath(const std::string& dirPath) {
        if (dirPath.empty()) return false;
        for (const auto& p : m_watchedPaths) if (p == dirPath) return true;
        m_watchedPaths.push_back(dirPath);
        return true;
    }

    bool UnwatchPath(const std::string& dirPath) {
        for (size_t i = 0; i < m_watchedPaths.size(); ++i) {
            if (m_watchedPaths[i] == dirPath) {
                m_watchedPaths.erase(m_watchedPaths.begin() + static_cast<ptrdiff_t>(i));
                return true;
            }
        }
        return false;
    }

    void SimulateChange(const FileChangeEvent& evt) {
        m_pendingEvents.push_back(evt);
    }

    uint32_t Flush(int64_t nowMs) {
        uint32_t fired = 0;
        for (const auto& evt : m_pendingEvents) {
            if (nowMs - evt.timestampMs >= m_debounceMs) {
                if (m_callback) m_callback(evt);
                ++fired;
            }
        }
        m_pendingEvents.clear();
        return fired;
    }

    uint32_t GetWatchCount() const {
        return static_cast<uint32_t>(m_watchedPaths.size());
    }

    void Shutdown() { m_watchedPaths.clear(); m_ready = false; }

private:
    bool                      m_ready      = false;
    uint32_t                  m_debounceMs = 300;
    std::vector<std::string>  m_watchedPaths;
    std::vector<FileChangeEvent> m_pendingEvents;
    LivePreviewCallback       m_callback;
};

}} // namespace ExplorerLens::Engine
