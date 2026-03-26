// MonitorConfigWatcher.h — WM_DPICHANGED / WM_DISPLAYCHANGE Event Watcher
// Copyright (c) 2026 ExplorerLens Project
//
// Creates a hidden message-only HWND to receive WM_DISPLAYCHANGE and
// WM_DPICHANGED notifications, then fires registered callbacks to update
// MultiMonitorContext and invalidate per-DPI thumbnail caches.
//
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Event Types ------------------------------------------------------------

enum class MonitorEvent : uint8_t {
    DPIChanged         = 0,  // WM_DPICHANGED — DPI on a specific HMONITOR changed
    DisplayChanged     = 1,  // WM_DISPLAYCHANGE — resolution/color depth changed
    MonitorConnected   = 2,  // New monitor attached (WM_DISPLAYCHANGE superset)
    MonitorDisconnected = 3, // Monitor removed
};

struct MonitorEventData {
    MonitorEvent type;
    void*        hmonitor    = nullptr;  // Relevant HMONITOR (null = all monitors)
    uint32_t     newDPI      = 0;        // Only valid for DPIChanged
    uint32_t     newWidth    = 0;        // Only valid for DisplayChanged
    uint32_t     newHeight   = 0;
    uint32_t     newBitDepth = 0;
};

using MonitorEventCallback = std::function<void(const MonitorEventData&)>;

// ---- MonitorConfigWatcher ---------------------------------------------------

class MonitorConfigWatcher {
public:
    MonitorConfigWatcher();
    ~MonitorConfigWatcher();

    // Create the hidden message-only HWND and start the pump thread.
    bool Start();

    // Stop the pump thread and destroy the HWND.
    void Stop();

    bool IsRunning() const;

    // Register a callback — called on the pump thread; keep callbacks fast.
    void AddCallback(MonitorEventCallback cb);
    void ClearCallbacks();

    // Singleton — shared by MultiMonitorContext, HiDPIThumbnailCache, etc.
    static MonitorConfigWatcher& Instance();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    MonitorConfigWatcher(const MonitorConfigWatcher&) = delete;
    MonitorConfigWatcher& operator=(const MonitorConfigWatcher&) = delete;
};

} // namespace Engine
} // namespace ExplorerLens
