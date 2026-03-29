// PluginUpdateScheduler.h — Background Auto-Update with Rollback Support
// Copyright (c) 2026 ExplorerLens Project
//
// Polls the plugin marketplace for updates on a configurable schedule,
// downloads updates in the background, and applies them at idle time with
// automatic rollback on repeated load failures.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Update Policy ----------------------------------------------------------

enum class UpdatePolicy : uint8_t {
    Disabled         = 0,
    CheckOnly        = 1,   // Notify user but don't download
    AutoDownload     = 2,   // Download but require user confirmation to apply
    AutoApply        = 3,   // Download and apply silently at idle
};

struct UpdateSchedule {
    UpdatePolicy policy          = UpdatePolicy::AutoDownload;
    uint32_t     checkIntervalHrs = 24;   // How often to poll marketplace
    bool         skipOnMetered   = true;  // Don't download on metered network
    bool         suppressNotifications = false;
};

// ---- Update Event -----------------------------------------------------------

struct PluginUpdateEvent {
    std::string pluginId;
    std::string currentVersion;
    std::string availableVersion;
    bool        downloaded      = false;
    bool        applied         = false;
    bool        rolledBack      = false;
    std::string error;
};

using UpdateEventCallback = std::function<void(const PluginUpdateEvent&)>;

// ---- PluginUpdateScheduler --------------------------------------------------

class PluginUpdateScheduler {
public:
    explicit PluginUpdateScheduler(UpdateSchedule schedule = {}) {  (void)schedule; }
    ~PluginUpdateScheduler() {}

    void Start();
    void Stop();
    bool IsRunning() const { return false; }

    // Trigger an immediate check (runs asynchronously).
    void CheckNow();

    // Force-apply a staged download for a specific plugin.
    bool ApplyUpdate(const std::string& pluginId, std::string& outError);

    // Roll back the last applied update for a plugin.
    bool RollbackUpdate(const std::string& pluginId, std::string& outError);

    // Number of consecutive load failures before auto-rollback triggers.
    void SetRollbackThreshold(uint32_t failureCount);

    void SetSchedule(UpdateSchedule schedule);
    UpdateSchedule GetSchedule() const;

    void SetCallback(UpdateEventCallback cb);

    static PluginUpdateScheduler& Instance();

private:
    struct Impl;
    Impl* m_impl{nullptr};
};

} // namespace Engine
} // namespace ExplorerLens
