// Engine/Core/WinGetManifestSchema.h
// ExplorerLens — WinGet package manifest schema types (ROADMAP v8.0 Phase 3)
// Sprint S350.
//
// Purpose:
//   Phase 3 exit criterion: "WinGet manifest published" (ADR A26).
//   WinGetManifestSchema provides:
//     1. Typed enums for WinGet manifest schema version and installer type.
//     2. Typed structs for the three manifest files required by winget-pkgs:
//          - WinGetPackageLocale  (ExplorerLens.installer.yaml fields)
//          - WinGetInstallerEntry (installer URL + SHA-256 + architecture)
//          - WinGetDefaultLocale  (display name, description, tags)
//     3. A Validate() method that checks required fields before submission.
//
//   The actual YAML files live under packaging/winget/.  This header provides
//   the data contract that build-scripts/Bump-Version.ps1 can reference, and
//   that CI uses to verify the manifest before a PR to microsoft/winget-pkgs.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_WIN_GET_MANIFEST_SCHEMA_H
#define EXPLORERLENS_ENGINE_WIN_GET_MANIFEST_SCHEMA_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// WinGetManifestVersion — schema version tag
// ---------------------------------------------------------------------------

enum class WinGetManifestVersion : std::uint8_t {
    V1_0 = 0,
    V1_1 = 1,
    V1_2 = 2,
    V1_4 = 3,  ///< Current stable schema used for submission
    V1_6 = 4,  ///< Preview schema (experimental features)
};

// ---------------------------------------------------------------------------
// WinGetInstallerType — installer mechanism
// ---------------------------------------------------------------------------

enum class WinGetInstallerType : std::uint8_t {
    MSI      = 0,
    MSIX     = 1,
    EXE      = 2,
    ZIP      = 3,
    NULLSOFT = 4,
    INNO     = 5,
    WIX      = 6,  ///< ExplorerLens uses WiX MSI
};

// ---------------------------------------------------------------------------
// WinGetPackageArch — target architecture
// ---------------------------------------------------------------------------

enum class WinGetPackageArch : std::uint8_t {
    X86   = 0,
    X64   = 1,  ///< Primary ExplorerLens target
    ARM64 = 2,  ///< Phase 6 target
    ARM   = 3,
    NEUTRAL = 4,
};

// ---------------------------------------------------------------------------
// WinGetManifestValidationStatus
// ---------------------------------------------------------------------------

enum class WinGetManifestValidationStatus : std::uint8_t {
    OK                        = 0,
    MISSING_PACKAGE_ID        = 1,  ///< PackageIdentifier is empty
    INVALID_PACKAGE_ID        = 2,  ///< Not in Publisher.AppName format
    MISSING_VERSION           = 3,
    MISSING_INSTALLER_URL     = 4,
    MISSING_INSTALLER_SHA256  = 5,
    INVALID_SHA256_LENGTH     = 6,  ///< SHA-256 must be 64 hex chars
    MISSING_DISPLAY_NAME      = 7,
    MISSING_PUBLISHER         = 8,
    MISSING_LICENSE           = 9,
    MISSING_SHORT_DESCRIPTION = 10,
};

// ---------------------------------------------------------------------------
// WinGetInstallerEntry — one installer in the manifest
// ---------------------------------------------------------------------------

struct WinGetInstallerEntry final {
    WinGetPackageArch   arch              = WinGetPackageArch::X64;
    WinGetInstallerType installerType     = WinGetInstallerType::WIX;
    std::string         installerUrl{};          ///< Direct download URL
    std::string         installerSha256{};       ///< 64-char lowercase hex SHA-256
    std::string         productCode{};           ///< MSI ProductCode GUID (optional)
    std::string         scope{};                 ///< "user" or "machine"

    [[nodiscard]] bool HasValidSha256() const noexcept
    {
        return installerSha256.size() == 64u;
    }
};

// ---------------------------------------------------------------------------
// WinGetDefaultLocale — locale-en-us section fields
// ---------------------------------------------------------------------------

struct WinGetDefaultLocale final {
    std::string packageLocale      = "en-US";
    std::string publisher{};                      ///< e.g. "Rajwan Yair"
    std::string publisherUrl{};                   ///< e.g. "https://github.com/RajwanYair"
    std::string packageName{};                    ///< e.g. "ExplorerLens"
    std::string license{};                        ///< e.g. "MIT"
    std::string licenseUrl{};
    std::string shortDescription{};               ///< ≤ 256 characters
    std::string description{};                    ///< Full markdown description
    std::string moniker{};                        ///< e.g. "explorerlens"
    std::vector<std::string> tags{};              ///< e.g. ["thumbnail","windows","shell"]
};

// ---------------------------------------------------------------------------
// WinGetManifestPackage — top-level package manifest
// ---------------------------------------------------------------------------

struct WinGetManifestPackage final {
    WinGetManifestVersion schemaVersion = WinGetManifestVersion::V1_4;
    std::string packageIdentifier{};              ///< e.g. "RajwanYair.ExplorerLens"
    std::string packageVersion{};                 ///< e.g. "39.9.0"
    std::vector<WinGetInstallerEntry> installers{};
    WinGetDefaultLocale locale{};

    // ------------------------------------------------------------------
    // Validate
    // ------------------------------------------------------------------

    [[nodiscard]] WinGetManifestValidationStatus Validate() const noexcept
    {
        if (packageIdentifier.empty())
            return WinGetManifestValidationStatus::MISSING_PACKAGE_ID;

        // Must contain at least one '.' (Publisher.AppName)
        if (packageIdentifier.find('.') == std::string::npos)
            return WinGetManifestValidationStatus::INVALID_PACKAGE_ID;

        if (packageVersion.empty())
            return WinGetManifestValidationStatus::MISSING_VERSION;

        for (const auto& inst : installers) {
            if (inst.installerUrl.empty())
                return WinGetManifestValidationStatus::MISSING_INSTALLER_URL;
            if (inst.installerSha256.empty())
                return WinGetManifestValidationStatus::MISSING_INSTALLER_SHA256;
            if (inst.installerSha256.size() != 64u)
                return WinGetManifestValidationStatus::INVALID_SHA256_LENGTH;
        }

        if (locale.packageName.empty())
            return WinGetManifestValidationStatus::MISSING_DISPLAY_NAME;
        if (locale.publisher.empty())
            return WinGetManifestValidationStatus::MISSING_PUBLISHER;
        if (locale.license.empty())
            return WinGetManifestValidationStatus::MISSING_LICENSE;
        if (locale.shortDescription.empty())
            return WinGetManifestValidationStatus::MISSING_SHORT_DESCRIPTION;

        return WinGetManifestValidationStatus::OK;
    }

    // ------------------------------------------------------------------
    // Factory — minimal ExplorerLens v39.x manifest
    // ------------------------------------------------------------------

    [[nodiscard]] static WinGetManifestPackage MakeExplorerLens(
        const std::string& version,
        const std::string& installerUrl,
        const std::string& sha256) noexcept
    {
        WinGetManifestPackage m;
        m.packageIdentifier = "RajwanYair.ExplorerLens";
        m.packageVersion    = version;

        WinGetInstallerEntry inst;
        inst.arch             = WinGetPackageArch::X64;
        inst.installerType    = WinGetInstallerType::WIX;
        inst.installerUrl     = installerUrl;
        inst.installerSha256  = sha256;
        inst.scope            = "machine";
        m.installers.push_back(inst);

        m.locale.publisher         = "Rajwan Yair";
        m.locale.publisherUrl      = "https://github.com/RajwanYair";
        m.locale.packageName       = "ExplorerLens";
        m.locale.license           = "MIT";
        m.locale.licenseUrl        = "https://github.com/RajwanYair/ExplorerLens.io/blob/main/LICENSE";
        m.locale.shortDescription  =
            "Windows Shell Extension providing GPU-accelerated thumbnails for 200+ file formats.";
        m.locale.moniker           = "explorerlens";
        m.locale.tags              = { "thumbnail", "windows", "shell-extension", "explorer", "file-preview" };

        return m;
    }
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

/// Required SHA-256 hex string length (64 chars).
inline constexpr std::size_t kWinGetSha256HexLength = 64u;

/// Maximum short description length per WinGet spec.
inline constexpr std::size_t kWinGetMaxShortDescriptionLength = 256u;

/// WinGet package identifier for ExplorerLens.
inline constexpr const char* kWinGetPackageIdentifier = "RajwanYair.ExplorerLens";

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_WIN_GET_MANIFEST_SCHEMA_H
