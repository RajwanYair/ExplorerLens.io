// PluginBundleInstaller.h — Bundle Installer for Plugin Collections
// Copyright (c) 2026 ExplorerLens Project
//
// Transactional installer for plugin bundles with staged commit and full rollback.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>

namespace ExplorerLens::Engine {

enum class InstallPhase : uint8_t {
    Verify   = 0,
    Download = 1,
    Stage    = 2,
    Commit   = 3,
    Rollback = 4,
};

struct BundleEntry {
    std::string pluginId;
    std::string version;
    std::string checksum;  // SHA-256 hex string
    uint64_t    size = 0;  // bytes

    [[nodiscard]] bool IsValid() const noexcept {
        return !pluginId.empty() && !version.empty()
               && checksum.size() == 64 && size > 0;
    }
};

struct InstallProgress {
    InstallPhase phase         = InstallPhase::Verify;
    uint32_t     current       = 0;
    uint32_t     total         = 0;
    std::string  currentPlugin;

    [[nodiscard]] float Percent() const noexcept {
        return total > 0 ? static_cast<float>(current) / static_cast<float>(total) * 100.0f : 0.0f;
    }
};

using ProgressCallback = std::function<void(const InstallProgress&)>;

struct BundleManifest {
    std::string              bundleId;
    std::string              displayName;
    std::vector<BundleEntry> entries;
    std::chrono::system_clock::time_point publishedAt{};
};

class PluginBundleInstaller {
public:
    explicit PluginBundleInstaller(std::string installPath = "");
    ~PluginBundleInstaller() noexcept;

    PluginBundleInstaller(const PluginBundleInstaller&)            = delete;
    PluginBundleInstaller& operator=(const PluginBundleInstaller&) = delete;
    PluginBundleInstaller(PluginBundleInstaller&&)                 = default;
    PluginBundleInstaller& operator=(PluginBundleInstaller&&)      = default;

    // Install all entries in the manifest atomically.
    // Returns true on full success; any failure triggers automatic rollback.
    bool InstallBundle(const BundleManifest& manifest,
                       ProgressCallback      callback = nullptr);

    // Explicitly roll back the most recently staged bundle.
    bool RollbackBundle(const std::string& bundleId);

    // Thread-safe snapshot of current installation progress.
    [[nodiscard]] InstallProgress GetProgress() const noexcept;

    // Verify checksums and signatures for all entries without installing.
    [[nodiscard]] bool VerifyBundle(const BundleManifest& manifest) const;

    void SetInstallPath(const std::string& path);
    [[nodiscard]] const std::string& GetInstallPath() const noexcept { return m_installPath; }

    // List bundle IDs that can still be rolled back.
    [[nodiscard]] std::vector<std::string> GetRollbackHistory() const;

private:
    std::string      m_installPath;
    InstallProgress  m_progress{};

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    bool RunVerifyPhase(const BundleManifest& manifest)  const;
    bool RunDownloadPhase(const BundleManifest& manifest, ProgressCallback& cb);
    bool RunStagePhase(const BundleManifest& manifest);
    bool RunCommitPhase(const BundleManifest& manifest);
    void SetProgress(InstallPhase phase, uint32_t current, uint32_t total,
                     const std::string& plugin) noexcept;
};

} // namespace ExplorerLens::Engine
