// CacheInvalidationWatcher.h — File-system change watcher that drives cache invalidation
// Copyright (c) 2026 ExplorerLens Project
//
// Watches one or more directories for file modifications/deletions using
// ReadDirectoryChangesW (Windows) / inotify (Linux) / FSEvents (macOS).
// On change, fires a registered callback to invalidate matching cache entries.
//
// Typical use:
//   CacheInvalidationWatcher watcher(cacheManager);
//   watcher.Watch("C:\\Users\\Alice\\Pictures");
//   watcher.Start();   // spawns background thread
//   // ... later:
//   watcher.Stop();
//
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// ChangeType — the kind of filesystem event observed
// ---------------------------------------------------------------------------

enum class ChangeType : uint8_t {
    MODIFIED  = 0x01,
    DELETED   = 0x02,
    RENAMED   = 0x04,
    CREATED   = 0x08,
};

// ---------------------------------------------------------------------------
// ChangeEvent — a single filesystem event passed to the callback
// ---------------------------------------------------------------------------

struct ChangeEvent {
    ChangeType  type;
    std::string path;       // Absolute path of the affected file
    std::string oldPath;    // Non-empty on RENAMED (old name)
};

// ---------------------------------------------------------------------------
// CacheInvalidationCallback — signature for the event handler
// ---------------------------------------------------------------------------

using CacheInvalidationCallback = std::function<void(const ChangeEvent&)>;

// ---------------------------------------------------------------------------
// CacheInvalidationWatcher
// ---------------------------------------------------------------------------

class CacheInvalidationWatcher {
public:
    CacheInvalidationWatcher();
    ~CacheInvalidationWatcher();

    CacheInvalidationWatcher(const CacheInvalidationWatcher&) = delete;
    CacheInvalidationWatcher& operator=(const CacheInvalidationWatcher&) = delete;
    CacheInvalidationWatcher(CacheInvalidationWatcher&&) noexcept;
    CacheInvalidationWatcher& operator=(CacheInvalidationWatcher&&) noexcept;

    // Register a directory to watch (recursive). Returns false if path is invalid.
    bool Watch(std::string_view directoryPath, bool recursive = true);

    // Remove a previously registered directory.
    void Unwatch(std::string_view directoryPath);

    // Register the callback invoked on every relevant change event.
    // Replaces any previously registered callback.
    void SetCallback(CacheInvalidationCallback callback);

    // Start the background watcher thread.
    void Start();

    // Stop the background thread and release all OS handles. Blocks until done.
    void Stop();

    // True if the watcher is running.
    bool IsRunning() const noexcept;

    // Number of events processed since last Start().
    uint64_t EventsProcessed() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
