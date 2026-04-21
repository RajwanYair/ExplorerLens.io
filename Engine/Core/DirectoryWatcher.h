// DirectoryWatcher.h — RAII ReadDirectoryChangesW Wrapper
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors a directory tree for file creation, modification, or deletion
// events and calls a registered callback on a dedicated background thread.
// Used by the cache layer to invalidate stale thumbnail entries when source
// files change.
//
// Thread safety: Watch/Stop are thread-safe. The callback is invoked from a
// background thread — callers must synchronise shared state.
//
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <filesystem>

namespace ExplorerLens { namespace Engine {

// ─── Change Event ─────────────────────────────────────────────────────────────

enum class FileChangeAction : uint32_t {
    ADDED    = FILE_ACTION_ADDED,
    REMOVED  = FILE_ACTION_REMOVED,
    MODIFIED = FILE_ACTION_MODIFIED,
    RENAMED  = FILE_ACTION_RENAMED_OLD_NAME,  // old name in path field
    RENAMED_NEW = FILE_ACTION_RENAMED_NEW_NAME,
};

struct FileChangeEvent {
    FileChangeAction          action;
    std::filesystem::path     path;    // full absolute path
};

using ChangeCallback = std::function<void(const FileChangeEvent&)>;

// ─── DirectoryWatcher ─────────────────────────────────────────────────────────

class DirectoryWatcher {
public:
    DirectoryWatcher() = default;
    ~DirectoryWatcher();

    // Non-copyable, movable
    DirectoryWatcher(const DirectoryWatcher&) = delete;
    DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;
    DirectoryWatcher(DirectoryWatcher&&) noexcept;
    DirectoryWatcher& operator=(DirectoryWatcher&&) noexcept;

    // Start watching 'directory'.
    // 'recursive' — if true, monitors the entire subtree.
    // 'filter'    — FILE_NOTIFY_CHANGE_* flags (default: FILE_NAME|DIR_NAME|SIZE|LAST_WRITE).
    // 'callback'  — invoked on a background thread for each event.
    // Returns true on success; false if the directory cannot be opened.
    bool Watch(const std::filesystem::path& directory,
               bool recursive,
               ChangeCallback callback,
               DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME  |
                              FILE_NOTIFY_CHANGE_DIR_NAME   |
                              FILE_NOTIFY_CHANGE_SIZE       |
                              FILE_NOTIFY_CHANGE_LAST_WRITE);

    // Stop watching and join the background thread.
    void Stop();

    bool IsWatching() const noexcept { return m_running.load(); }
    const std::filesystem::path& Directory() const noexcept { return m_directory; }

private:
    void WatchLoop(HANDLE hDir, DWORD filter, bool recursive);

    std::filesystem::path m_directory;
    std::thread           m_thread;
    std::atomic<bool>     m_running{ false };
    HANDLE                m_stopEvent = INVALID_HANDLE_VALUE;
};

}}  // namespace ExplorerLens::Engine
