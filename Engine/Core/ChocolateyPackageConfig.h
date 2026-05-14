// Engine/Core/ChocolateyPackageConfig.h
// ExplorerLens Engine — S385 (Phase 4, Sprint 5)
//
// Purpose:
//   Chocolatey package manifest configuration and validation.
//   Phase 4 exit criterion: "Chocolatey package published"
//
//   Generates and validates the Chocolatey package:
//   - explorerlens.nuspec       — package metadata
//   - tools/chocolateyInstall.ps1  — runs MSI installer
//   - tools/chocolateyUninstall.ps1 — runs MSI uninstall
//
//   Chocolatey package URL pattern:
//     https://github.com/RajwanYair/ExplorerLens.io/releases/download/vX.Y.Z/
//       ExplorerLens-Setup-X.Y.Z.msi
//
//   CI workflow: choco-publish.yml
//   - Runs on release tag push
//   - Builds package, installs locally, smoke-tests, pushes to community.chocolatey.org

#pragma once
#ifndef EXPLORERLENS_ENGINE_CHOCOLATEYPACKAGECONFIG_H
#define EXPLORERLENS_ENGINE_CHOCOLATEYPACKAGECONFIG_H

#include <cstdint>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── Package status ───────────────────────────────────────────────────────────

enum class ChocoPackageStatus : uint8_t {
    OK                  = 0,
    NUSPEC_INVALID      = 1,
    CHECKSUM_MISSING    = 2,
    URL_EMPTY           = 3,
    INSTALL_SCRIPT_FAIL = 4,
    PUSH_FAILED         = 5,
    NOT_WIN32           = 6,
};

// ─── Checksum algorithm ───────────────────────────────────────────────────────

enum class ChocoChecksumAlgo : uint8_t {
    SHA256 = 0,
    SHA512 = 1,
};

// ─── Installer entry ──────────────────────────────────────────────────────────

struct ChocoInstallerEntry final {
    const char*       msiUrl          = nullptr;  // release asset URL
    char              sha256[65]      = {};        // 64 hex + NUL
    ChocoChecksumAlgo algo            = ChocoChecksumAlgo::SHA256;
    const char*       silentArgs      = "/quiet /norestart";
    const char*       uninstallArgs   = "/quiet /norestart";

    bool IsValid() const noexcept {
        if (!msiUrl || msiUrl[0] == '\0') return false;
        size_t len = 0; while (sha256[len]) ++len;
        return len == 64;
    }
};

// ─── Package config ────────────────────────────────────────────────────────────

struct ChocolateyPackageConfig final {
    const char*        packageId       = "explorerlens";
    const char*        title           = "ExplorerLens";
    const char*        version         = nullptr;
    const char*        authors         = "Rajwan Yair";
    const char*        projectUrl      = "https://github.com/RajwanYair/ExplorerLens.io";
    const char*        licenseUrl      = "https://github.com/RajwanYair/ExplorerLens.io/blob/main/LICENSE";
    const char*        iconUrl         = "https://raw.githubusercontent.com/RajwanYair/ExplorerLens.io/main/docs/assets/icon.png";
    const char*        description     = "GPU-accelerated Windows Shell Extension thumbnail provider for 200+ file formats";
    const char*        tags            = "thumbnail shell-extension windows explorer";
    ChocoInstallerEntry installer;

    static ChocolateyPackageConfig Default() noexcept {
        return ChocolateyPackageConfig{};
    }

    static ChocolateyPackageConfig ForRelease(const char* version) noexcept {
        ChocolateyPackageConfig c{};
        c.version = version;
        return c;
    }
};

// ─── Publish result ────────────────────────────────────────────────────────────

struct ChocoPublishResult final {
    ChocoPackageStatus status        = ChocoPackageStatus::OK;
    bool               nuspecBuilt   = false;
    bool               localInstallOk= false;
    bool               publishedOk   = false;
    const char*        packagePath   = nullptr;

    bool IsOk() const noexcept { return status == ChocoPackageStatus::OK; }
    bool IsPublished() const noexcept { return IsOk() && publishedOk; }
};

// ─── Manager ─────────────────────────────────────────────────────────────────

class ChocolateyPackageManager final {
public:
    ChocolateyPackageManager() = default;
    ~ChocolateyPackageManager() = default;

    ChocolateyPackageManager(const ChocolateyPackageManager&) = delete;
    ChocolateyPackageManager& operator=(const ChocolateyPackageManager&) = delete;

    static ChocolateyPackageManager& Global() noexcept {
        static ChocolateyPackageManager s_instance;
        return s_instance;
    }

    void Configure(const ChocolateyPackageConfig& config) noexcept { m_config = config; }

    // Build .nuspec + install scripts in outputDir
    ChocoPackageStatus Build(const char* outputDir) noexcept;

    // Validate using `choco pack --validate`
    ChocoPackageStatus Validate() noexcept;

    // Push to community.chocolatey.org (requires CHOCO_API_KEY env var)
    ChocoPublishResult Publish() noexcept;

    bool IsBuilt()     const noexcept { return m_isBuilt; }
    uint32_t BuildCount() const noexcept { return m_buildCount; }

    const ChocolateyPackageConfig& Config() const noexcept { return m_config; }

private:
    ChocolateyPackageConfig m_config{};
    bool                    m_isBuilt   = false;
    uint32_t                m_buildCount= 0;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline ChocoPackageStatus ChocolateyPackageManager::Build(const char* outputDir) noexcept {
    if (!m_config.version || m_config.version[0] == '\0')
        return ChocoPackageStatus::NUSPEC_INVALID;
    if (!m_config.installer.IsValid())
        return ChocoPackageStatus::CHECKSUM_MISSING;
    (void)outputDir;
    m_isBuilt = true;
    ++m_buildCount;
    return ChocoPackageStatus::OK;
}

inline ChocoPackageStatus ChocolateyPackageManager::Validate() noexcept {
    if (!m_isBuilt) return ChocoPackageStatus::NUSPEC_INVALID;
    return ChocoPackageStatus::OK;
}

inline ChocoPublishResult ChocolateyPackageManager::Publish() noexcept {
    ChocoPublishResult r{};
    r.nuspecBuilt    = m_isBuilt;
    r.localInstallOk = true;
    r.publishedOk    = false;  // stub: real impl calls choco push
    r.status         = ChocoPackageStatus::OK;
    return r;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr const char* kChocoPackageId         = "explorerlens";
static constexpr const char* kChocoCommunityFeedUrl  = "https://push.chocolatey.org/";
static constexpr const char* kChocoApiKeyEnvVar      = "CHOCO_API_KEY";
static constexpr const char* kChocoMinVersion        = "2.0.0";

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_CHOCOLATEYPACKAGECONFIG_H
