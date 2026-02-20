#pragma once
// Sprint 225: Watch Folder Engine — filesystem monitoring for auto-thumbnail regeneration
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace DarkThumbs { namespace Engine {

/// Type of filesystem change detected
enum class FileChangeType : uint32_t {
    Created   = 0,
    Modified  = 1,
    Deleted   = 2,
    Renamed   = 3,
    Attribute = 4,
    COUNT     = 5
};

/// Watch mode controlling notification granularity
enum class WatchMode : uint32_t {
    Polling    = 0,    ///< Timer-based poll
    Native     = 1,    ///< ReadDirectoryChangesW
    Hybrid     = 2,    ///< Native + polling fallback
    COUNT      = 3
};

/// Descriptor for a single watch folder
struct WatchFolder {
    std::wstring path;
    bool         recursive     = true;
    bool         enabled       = true;
    uint32_t     pollIntervalMs = 2000;
    WatchMode    mode          = WatchMode::Native;
    uint32_t     changeCount   = 0;
};

/// Notification when a file change is detected
struct FileChangeEvent {
    std::wstring   filePath;
    std::wstring   oldPath;       ///< For renames
    FileChangeType changeType = FileChangeType::Modified;
    uint64_t       timestamp  = 0;
};

/// Manages filesystem watchers for thumbnail regeneration
class WatchFolderEngine {
public:
    WatchFolderEngine();

    /// Human-readable name for a change type
    static const wchar_t* GetChangeTypeName(FileChangeType type);
    /// Human-readable name for a watch mode
    static const wchar_t* GetWatchModeName(WatchMode mode);

    /// Add a folder to the watch list
    bool AddFolder(const std::wstring& path, bool recursive = true, WatchMode mode = WatchMode::Native);
    /// Remove a folder from the watch list
    bool RemoveFolder(const std::wstring& path);
    /// Get current watch list
    const std::vector<WatchFolder>& GetWatchList() const { return m_watchFolders; }
    /// Total folders being watched
    size_t GetWatchCount() const { return m_watchFolders.size(); }

    using ChangeCallback = std::function<void(const FileChangeEvent&)>;
    /// Register callback for change notifications
    void SetChangeCallback(ChangeCallback cb) { m_callback = std::move(cb); }

    /// Simulate a change event (for testing)
    void SimulateChange(const std::wstring& path, FileChangeType type);

private:
    std::vector<WatchFolder> m_watchFolders;
    ChangeCallback           m_callback;
};

}} // namespace DarkThumbs::Engine
