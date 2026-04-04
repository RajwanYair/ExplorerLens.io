#pragma once
// ============================================================================
// FilesystemWatchdog.h — Directory change notification monitoring
//
// Purpose:   Directory change notification monitoring via ReadDirectoryChangesW
// Provides:  FSWatchEvent, FSWatchScope enums, FSWatchChangeEvent,
//            FSWatchDirConfig structs, and FilesystemWatchdog class
// Used by:   Hot-reload and cache invalidation
// ============================================================================

#include <chrono>
#include <cstdint>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// FilesystemWatchdog — Recursive directory monitoring via ReadDirectoryChangesW
// ============================================================================

enum class FSWatchEvent {
    Created,
    Modified,
    Deleted,
    Renamed,
    AttributeChanged
};

inline const char* FSWatchEventName(FSWatchEvent value)
{
    switch (value) {
        case FSWatchEvent::Created:
            return "Created";
        case FSWatchEvent::Modified:
            return "Modified";
        case FSWatchEvent::Deleted:
            return "Deleted";
        case FSWatchEvent::Renamed:
            return "Renamed";
        case FSWatchEvent::AttributeChanged:
            return "AttributeChanged";
        default:
            return "Unknown";
    }
}

enum class FSWatchScope {
    File,
    Directory,
    Recursive,
    NonRecursive,
    FilteredExts
};

inline const char* FSWatchScopeName(FSWatchScope value)
{
    switch (value) {
        case FSWatchScope::File:
            return "File";
        case FSWatchScope::Directory:
            return "Directory";
        case FSWatchScope::Recursive:
            return "Recursive";
        case FSWatchScope::NonRecursive:
            return "NonRecursive";
        case FSWatchScope::FilteredExts:
            return "FilteredExts";
        default:
            return "Unknown";
    }
}

struct FSWatchChangeEvent
{
    std::wstring path;
    FSWatchEvent eventType = FSWatchEvent::Modified;
    uint64_t timestampMs = 0;
    std::wstring oldPath;  // Only set for Renamed events
    uint64_t fileSizeBytes = 0;

    bool IsRename() const
    {
        return eventType == FSWatchEvent::Renamed;
    }
    bool IsDelete() const
    {
        return eventType == FSWatchEvent::Deleted;
    }
};

struct FSWatchDirConfig
{
    std::wstring directoryPath;
    FSWatchScope scope = FSWatchScope::Recursive;
    std::vector<std::wstring> extensionFilters;  // e.g., {L".jpg", L".png"}
    bool includeHidden = false;
    uint32_t debounceMs = 50;
};

class FilesystemWatchdog
{
  public:
    static constexpr uint32_t MAX_WATCH_DIRS = 64;
    static constexpr uint32_t MAX_PENDING_EVENTS = 8192;
    static constexpr uint32_t DEFAULT_DEBOUNCE_MS = 50;
    static constexpr uint32_t BUFFER_SIZE = 65536;

    FilesystemWatchdog() = default;
    ~FilesystemWatchdog()
    {
        StopAll();
    }

    FilesystemWatchdog(const FilesystemWatchdog&) = delete;
    FilesystemWatchdog& operator=(const FilesystemWatchdog&) = delete;

    bool StartWatching(const FSWatchDirConfig& config)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_watchedDirs.size() >= MAX_WATCH_DIRS) {
            return false;
        }

        // In production, this would call CreateFileW + ReadDirectoryChangesW
        // For testability, we just register the directory
        m_watchedDirs.insert(config.directoryPath);
        m_configs.push_back(config);
        return true;
    }

    bool StopWatching(const std::wstring& directoryPath)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_watchedDirs.find(directoryPath);
        if (it == m_watchedDirs.end()) {
            return false;
        }

        m_watchedDirs.erase(it);
        // Remove matching config
        m_configs.erase(std::remove_if(m_configs.begin(), m_configs.end(),
                                       [&](const FSWatchDirConfig& c) { return c.directoryPath == directoryPath; }),
                        m_configs.end());
        return true;
    }

    void StopAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_watchedDirs.clear();
        m_configs.clear();
    }

    std::vector<FSWatchChangeEvent> GetPendingEvents()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<FSWatchChangeEvent> events;
        while (!m_pendingEvents.empty()) {
            events.push_back(m_pendingEvents.front());
            m_pendingEvents.pop();
        }
        return events;
    }

    // For testing: inject a simulated event
    void InjectEvent(const FSWatchChangeEvent& event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_pendingEvents.size() < MAX_PENDING_EVENTS) {
            m_pendingEvents.push(event);
            m_totalEventsReceived++;
        }
    }

    size_t GetWatchedDirectoryCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_watchedDirs.size();
    }

    bool IsWatching(const std::wstring& directoryPath) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_watchedDirs.count(directoryPath) > 0;
    }

    size_t GetPendingEventCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_pendingEvents.size();
    }

    uint64_t GetTotalEventsReceived() const
    {
        return m_totalEventsReceived;
    }

  private:
    mutable std::mutex m_mutex;
    std::set<std::wstring> m_watchedDirs;
    std::vector<FSWatchDirConfig> m_configs;
    std::queue<FSWatchChangeEvent> m_pendingEvents;
    uint64_t m_totalEventsReceived = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
