// AutoUpdatePolicyV4.h — Auto-Update Policy v4
// Copyright (c) 2026 ExplorerLens Project
//
// Per-plugin and global update policies with channel selection and enterprise lock support.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

enum class UpdateMode : uint8_t {
    Silent = 0,
    Notify = 1,
    Defer = 2,
    EnterpriseLocked = 3,
    Manual = 4,
};

enum class UpdateChannel : uint8_t {
    Stable = 0,
    Beta = 1,
    Nightly = 2,
    LTS = 3,
};

struct PolicyEntry
{
    std::string pluginId;
    UpdateMode mode = UpdateMode::Silent;
    UpdateChannel channel = UpdateChannel::Stable;
    uint32_t deferDays = 0;
    std::chrono::system_clock::time_point lastCheck{};

    [[nodiscard]] bool IsDeferPeriodElapsed() const noexcept
    {
        using namespace std::chrono;
        const auto elapsed = duration_cast<hours>(system_clock::now() - lastCheck).count();
        return elapsed >= static_cast<int64_t>(deferDays) * 24;
    }
};

struct UpdateAvailable
{
    std::string pluginId;
    std::string currentVersion;
    std::string availableVersion;
    UpdateChannel channel = UpdateChannel::Stable;
    uint64_t packageSize = 0;
};

using UpdateCallback = std::function<void(const UpdateAvailable&)>;

class AutoUpdatePolicyV4
{
  public:
    AutoUpdatePolicyV4() = default;
    ~AutoUpdatePolicyV4() = default;

    AutoUpdatePolicyV4(const AutoUpdatePolicyV4&) = delete;
    AutoUpdatePolicyV4& operator=(const AutoUpdatePolicyV4&) = delete;
    AutoUpdatePolicyV4(AutoUpdatePolicyV4&&) = default;
    AutoUpdatePolicyV4& operator=(AutoUpdatePolicyV4&&) = default;

    // Set or replace the policy for a single plugin.
    void SetPolicy(const PolicyEntry& entry);

    // Retrieve the effective policy for a plugin (falls back to global default).
    [[nodiscard]] PolicyEntry GetPolicy(const std::string& pluginId) const;

    // Poll the marketplace for updates respecting each plugin's channel and mode.
    // Returns all plugins for which a newer version was found.
    [[nodiscard]] std::vector<UpdateAvailable> CheckForUpdates(const std::vector<std::string>& pluginIds,
                                                               UpdateCallback onFound = nullptr) const;

    // Apply pending updates; returns the list of plugins successfully updated.
    std::vector<std::string> ApplyUpdates(const std::vector<UpdateAvailable>& updates,
                                          UpdateCallback onComplete = nullptr);

    // Override the mode for all plugins (overridden by EnterpriseLocked entries).
    void SetGlobalMode(UpdateMode mode) noexcept;
    [[nodiscard]] UpdateMode GetGlobalMode() const noexcept
    {
        return m_globalMode;
    }

    void SetGlobalChannel(UpdateChannel channel) noexcept;
    [[nodiscard]] UpdateChannel GetGlobalChannel() const noexcept
    {
        return m_globalChannel;
    }

    // Remove the per-plugin policy override and revert to global default.
    bool ClearPolicy(const std::string& pluginId) noexcept;

    [[nodiscard]] std::vector<PolicyEntry> GetAllPolicies() const;

    // Persist policies to a JSON file; returns false on I/O error.
    bool SavePolicies(const std::string& filePath) const;
    bool LoadPolicies(const std::string& filePath);

  private:
    UpdateMode m_globalMode = UpdateMode::Silent;
    UpdateChannel m_globalChannel = UpdateChannel::Stable;

    struct Impl
    {};
    std::unique_ptr<Impl> m_impl;

    [[nodiscard]] bool ShouldApplyUpdate(const PolicyEntry& policy, const UpdateAvailable& update) const noexcept;
};

}  // namespace ExplorerLens::Engine
