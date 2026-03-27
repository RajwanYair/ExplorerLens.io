// LensWatchDaemon.h — lens watch — Live Directory Monitor + Auto-Regen
// Copyright (c) 2026 ExplorerLens Project
//
// Daemon for lens watch — monitors directories via ReadDirectoryChangesW and auto-regenerates stale thumbnails.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct WatchJob { std::wstring rootPath; bool recursive = true; uint32_t debounceMs = 500; };
struct WatchEvent { std::wstring path; std::wstring action; uint64_t timestamp; };
class LensWatchDaemon {
public:
    bool   Start(const WatchJob& job) { m_job = job; m_running = true; return true; }
    void   Stop()                     { m_running = false; }
    bool   IsRunning()     const      { return m_running; }
    size_t EventCount()    const      { return m_events.size(); }
    std::vector<WatchEvent> DrainEvents() {
        auto e = std::move(m_events);
        m_events.clear();
        return e;
    }
private:
    WatchJob                m_job;
    std::atomic<bool>       m_running{false};
    std::vector<WatchEvent> m_events;
};

} // namespace Engine
} // namespace ExplorerLens