// Engine/Platform/WinGetPublishConfig.h
// ExplorerLens Engine — S376
//
// Purpose:
//   WinGet publish configuration and manifest schema builder.
//   ADR A26: WinGet manifest published in Phase 3.
//   Complements WinGetManifestSchema.h (S350) which defines the raw YAML schema;
//   this header defines the CI pipeline that builds and submits the manifest.
//
//   Publishing flow:
//   1. WinGetPublishConfig::Build()     — assemble manifest fields
//   2. WinGetPublishConfig::Validate()  — run winget-pkgs YAML validation
//   3. WinGetPublishConfig::Submit()    — fork + PR to microsoft/winget-pkgs
//   4. CI checks manifest via wingetcreate / WinGet validator

#pragma once
#ifndef EXPLORERLENS_ENGINE_WINGETPUBLISHCONFIG_H
#define EXPLORERLENS_ENGINE_WINGETPUBLISHCONFIG_H

#include <cstdint>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── Publish status ──────────────────────────────────────────────────────────

enum class WinGetPublishStatus : uint8_t {
    OK                  = 0,
    VERSION_MISSING     = 1,
    INSTALLER_URL_EMPTY = 2,
    SHA256_INVALID      = 3,   // must be exactly 64 hex chars
    VALIDATION_FAILED   = 4,
    PR_SUBMISSION_FAILED= 5,
    TOOL_NOT_FOUND      = 6,   // wingetcreate.exe not found
    NOT_WIN32           = 7,
};

// ─── Manifest tier ───────────────────────────────────────────────────────────

enum class WinGetManifestTier : uint8_t {
    SINGLETON   = 0,   // single-file manifest (small packages)
    MULTI_FILE  = 1,   // version + installer + locale YAML (recommended)
};

// ─── Installer type ──────────────────────────────────────────────────────────

enum class WinGetInstallerType : uint8_t {
    MSI         = 0,
    MSIX        = 1,
    INNO        = 2,
    NULLSOFT    = 3,
    EXE         = 4,
    ZIP         = 5,
};

// ─── Installer entry ─────────────────────────────────────────────────────────

struct WinGetInstallerEntry final {
    WinGetInstallerType type          = WinGetInstallerType::MSI;
    const char*         arch          = "x64";   // "x86", "x64", "arm64"
    const char*         url           = nullptr;
    char                sha256[65]    = {};       // 64 hex chars + NUL
    const char*         locale        = "en-US";
    bool                silentAllowed = true;

    bool IsValid() const noexcept {
        if (!url || url[0] == '\0') return false;
        size_t len = 0;
        while (sha256[len]) ++len;
        return len == 64;
    }
};

// ─── Publish config ──────────────────────────────────────────────────────────

struct WinGetPublishConfig final {
    const char*          packageId       = "RajwanYair.ExplorerLens";
    const char*          version         = nullptr;  // e.g. "39.8.0"
    WinGetManifestTier   tier            = WinGetManifestTier::MULTI_FILE;
    WinGetInstallerEntry installer;
    bool                 dryRun          = false;   // validate only, don't submit PR
    bool                 autoFork        = true;    // fork winget-pkgs if needed
    const char*          wingetCreatePath = nullptr; // path to wingetcreate.exe

    static WinGetPublishConfig Default() noexcept {
        WinGetPublishConfig c{};
        c.packageId = "RajwanYair.ExplorerLens";
        c.tier      = WinGetManifestTier::MULTI_FILE;
        c.dryRun    = false;
        c.autoFork  = true;
        return c;
    }

    static WinGetPublishConfig DryRun() noexcept {
        auto c      = Default();
        c.dryRun    = true;
        c.autoFork  = false;
        return c;
    }
};

// ─── Publish result ──────────────────────────────────────────────────────────

struct WinGetPublishResult final {
    WinGetPublishStatus status         = WinGetPublishStatus::OK;
    bool                manifestBuilt  = false;
    bool                validationOk   = false;
    bool                prSubmitted    = false;
    const char*         prUrl          = nullptr;
    uint32_t            warningCount   = 0;

    bool IsOk() const noexcept { return status == WinGetPublishStatus::OK; }
    bool IsPublished() const noexcept {
        return IsOk() && manifestBuilt && validationOk && prSubmitted;
    }
};

// ─── Main class ──────────────────────────────────────────────────────────────

class WinGetPublisher final {
public:
    WinGetPublisher() = default;
    ~WinGetPublisher() = default;

    WinGetPublisher(const WinGetPublisher&) = delete;
    WinGetPublisher& operator=(const WinGetPublisher&) = delete;

    static WinGetPublisher& Global() noexcept {
        static WinGetPublisher s_instance;
        return s_instance;
    }

    void Configure(const WinGetPublishConfig& config) noexcept { m_config = config; }

    // Build YAML manifest in memory; populate m_manifestYaml
    WinGetPublishStatus Build(const char* version, const WinGetInstallerEntry& installer) noexcept;

    // Validate using wingetcreate validate
    WinGetPublishStatus Validate() noexcept;

    // Submit PR to microsoft/winget-pkgs (requires GitHub token in env)
    WinGetPublishResult Submit() noexcept;

    // Full pipeline: Build → Validate → Submit (or DryRun)
    WinGetPublishResult Publish(const char* version, const WinGetInstallerEntry& installer) noexcept;

    bool IsManifestBuilt() const noexcept { return m_manifestBuilt; }
    uint32_t SubmissionsTotal() const noexcept { return m_submissionsTotal; }

    const WinGetPublishConfig& Config() const noexcept { return m_config; }

private:
    WinGetPublishConfig m_config{};
    bool                m_manifestBuilt    = false;
    uint32_t            m_submissionsTotal = 0;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline WinGetPublishStatus WinGetPublisher::Build(
    const char*                  version,
    const WinGetInstallerEntry&  installer) noexcept
{
    if (!version || version[0] == '\0') return WinGetPublishStatus::VERSION_MISSING;
    if (!installer.IsValid())           return WinGetPublishStatus::INSTALLER_URL_EMPTY;
    m_manifestBuilt = true;
    return WinGetPublishStatus::OK;
}

inline WinGetPublishStatus WinGetPublisher::Validate() noexcept {
    if (!m_manifestBuilt) return WinGetPublishStatus::VALIDATION_FAILED;
    return WinGetPublishStatus::OK;
}

inline WinGetPublishResult WinGetPublisher::Submit() noexcept {
    WinGetPublishResult r{};
    r.manifestBuilt = m_manifestBuilt;
    r.validationOk  = true;
    if (m_config.dryRun) {
        r.prSubmitted = false;
        r.status      = WinGetPublishStatus::OK;
        return r;
    }
    r.prSubmitted = true;
    r.status      = WinGetPublishStatus::OK;
    ++m_submissionsTotal;
    return r;
}

inline WinGetPublishResult WinGetPublisher::Publish(
    const char*                  version,
    const WinGetInstallerEntry&  installer) noexcept
{
    WinGetPublishResult r{};
    auto buildStatus = Build(version, installer);
    if (buildStatus != WinGetPublishStatus::OK) {
        r.status = buildStatus;
        return r;
    }
    r.manifestBuilt = true;
    auto valStatus  = Validate();
    if (valStatus != WinGetPublishStatus::OK) {
        r.status = valStatus;
        return r;
    }
    r.validationOk = true;
    return Submit();
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr const char* kWinGetPackageId          = "RajwanYair.ExplorerLens";
static constexpr const char* kWinGetPublisherName      = "Rajwan Yair";
static constexpr const char* kWinGetMoniker            = "explorerlens";
static constexpr const char* kWinGetDefaultLocale      = "en-US";
static constexpr const char* kWinGetPkgsRepo           = "microsoft/winget-pkgs";
static constexpr uint32_t    kWinGetManifestVersionMaj = 1u;
static constexpr uint32_t    kWinGetManifestVersionMin = 9u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_WINGETPUBLISHCONFIG_H
