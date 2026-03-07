// PluginUpdateChecker.h — Plugin Version and Update Management
// Copyright (c) 2026 ExplorerLens Project
//
// Checks loaded plugins against known versions and compatibility data
// to alert users about available updates and deprecations.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PluginVersion {
    uint16_t major = 0;
    uint16_t minor = 0;
    uint16_t patch = 0;

    bool operator<(const PluginVersion& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        return patch < o.patch;
    }
    bool operator==(const PluginVersion& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
    bool operator>(const PluginVersion& o) const { return o < *this; }
    std::string ToString() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
};

enum class UpdateStatus : uint8_t {
    UpToDate = 0,
    UpdateAvailable = 1,
    Deprecated = 2,
    Incompatible = 3,
    Unknown = 4
};

struct PluginUpdateInfo {
    std::string pluginId;
    std::string pluginName;
    PluginVersion currentVersion;
    PluginVersion latestVersion;
    UpdateStatus status = UpdateStatus::Unknown;
    std::string releaseNotes;
    bool securityUpdate = false;
};

class PluginUpdateChecker {
public:
    UpdateStatus CheckVersion(const PluginVersion& current,
        const PluginVersion& latest,
        const PluginVersion& minSupported) const {
        if (current < minSupported) return UpdateStatus::Incompatible;
        if (latest < current) return UpdateStatus::Unknown; // Current is newer?
        if (current == latest) return UpdateStatus::UpToDate;
        return UpdateStatus::UpdateAvailable;
    }

    void RegisterPlugin(const PluginUpdateInfo& info) {
        m_plugins.push_back(info);
    }

    std::vector<PluginUpdateInfo> GetUpdatablePlugins() const {
        std::vector<PluginUpdateInfo> result;
        for (const auto& p : m_plugins) {
            if (p.status == UpdateStatus::UpdateAvailable ||
                p.status == UpdateStatus::Deprecated) {
                result.push_back(p);
            }
        }
        return result;
    }

    std::vector<PluginUpdateInfo> GetSecurityUpdates() const {
        std::vector<PluginUpdateInfo> result;
        for (const auto& p : m_plugins) {
            if (p.securityUpdate && p.currentVersion < p.latestVersion)
                result.push_back(p);
        }
        return result;
    }

    size_t TotalPlugins() const { return m_plugins.size(); }

private:
    std::vector<PluginUpdateInfo> m_plugins;
};

} // namespace Engine
} // namespace ExplorerLens
