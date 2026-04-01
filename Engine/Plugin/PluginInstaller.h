// PluginInstaller.h — Plugin Package Install / Uninstall / Update
// Copyright (c) 2026 ExplorerLens Project
//
// Installs and manages .lenspkg plugin packages: extracts the ZIP, verifies
// the manifest signature, registers the extension mapping, and maintains
// the installed-plugins registry in %LOCALAPPDATA%\ExplorerLens\plugins\.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "PluginPackageManifest.h"

namespace ExplorerLens {
namespace Engine {

// ---- Install Record ---------------------------------------------------------

struct PluginInstallRecord {
    PluginPackageManifest manifest;
    std::string  installDir;        // Full path to extracted plugin directory
    std::string  installedDate;     // ISO 8601
    bool         enabled     = true;
    bool         updateAvail = false;
    std::string  availableVersion;  // Latest version from marketplace
};

// ---- Result Codes -----------------------------------------------------------

enum class PluginInstallStatus {
    Success             = 0,
    AlreadyInstalled    = 1,
    SignatureInvalid    = 2,
    ManifestInvalid     = 3,
    EngineTooOld        = 4,
    DiskError           = 5,
    DependencyMissing   = 6,
    AccessDenied        = 7,
    InternalError       = 99,
};

struct PluginInstallResult {
    PluginInstallStatus status = PluginInstallStatus::InternalError;
    PluginInstallRecord     plugin;
    std::string         error;
};

using InstallProgressCallback = std::function<void(const std::string& step, float fraction)>;

// ---- PluginInstaller --------------------------------------------------------

class PluginInstaller {
public:
    explicit PluginInstaller(std::string pluginsRoot = "");   // Default = %LOCALAPPDATA%\ExplorerLens\plugins
    ~PluginInstaller();

    // Install a .lenspkg file. Verifies signature; does NOT require admin.
    PluginInstallResult Install(
        const std::string&       pkgPath,
        InstallProgressCallback  progress = nullptr);

    // Update an already-installed plugin (installs new over old; keeps config).
    PluginInstallResult Update(
        const std::string&       pluginId,
        const std::string&       newPkgPath,
        InstallProgressCallback  progress = nullptr);

    // Uninstall by plugin ID. Removes directory + registry entries.
    bool Uninstall(const std::string& pluginId, std::string& outError);

    // Enable / disable a plugin without uninstalling it.
    bool SetEnabled(const std::string& pluginId, bool enabled);

    // List all installed plugins.
    std::vector<PluginInstallRecord> ListInstalled() const;

    // Look up by plugin ID.
    bool FindInstalled(const std::string& pluginId, PluginInstallRecord& out) const;

    // Rollback the last install/update (delete new, restore backup if present).
    bool Rollback(const std::string& pluginId, std::string& outError);

private:
    struct Impl {};
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
