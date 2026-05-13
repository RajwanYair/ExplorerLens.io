// =============================================================================
// ExplorerLens Engine — ChocolateyPackageSchema.h
// Sprint S359 | ROADMAP v8.0 Phase 4 (Chocolatey package published)
// Chocolatey .nuspec package manifest schema types for ExplorerLens.
//
// Phase 4 exit criterion: "Chocolatey package published"
// This header defines the data contract types used by the packaging script
// that generates ExplorerLens.nuspec for submission to chocolatey.org.
//
// Chocolatey .nuspec format is NuGet XML-based:
//   <package><metadata>
//     <id>explorerlens</id>
//     <version>39.7.0</version>
//     <authors>RajwanYair</authors>
//     ...
//   </metadata></package>
//
// The ChocolateyPackageSchema::Validate() method checks all required fields
// against the Chocolatey submission requirements before packaging.
// =============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#ifndef EXPLORERLENS_ENGINE_CHOCOLATEYPACKAGESCHEMA_H
#define EXPLORERLENS_ENGINE_CHOCOLATEYPACKAGESCHEMA_H

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// ChocolateyPackageVersion — NuGet semantic version (4-part)
// ---------------------------------------------------------------------------
struct ChocolateyPackageVersion final {
    uint16_t major{0};
    uint16_t minor{0};
    uint16_t patch{0};
    uint16_t revision{0};  ///< Chocolatey uses 4-part NuGet versioning

    [[nodiscard]] std::wstring ToString() const {
        return std::to_wstring(major) + L"." +
               std::to_wstring(minor) + L"." +
               std::to_wstring(patch) + L"." +
               std::to_wstring(revision);
    }

    [[nodiscard]] std::wstring ToStringThreePart() const {
        return std::to_wstring(major) + L"." +
               std::to_wstring(minor) + L"." +
               std::to_wstring(patch);
    }
};

// ---------------------------------------------------------------------------
// ChocolateyInstallType — installer type for chocolateyInstall.ps1
// ---------------------------------------------------------------------------
enum class ChocolateyInstallType : uint8_t {
    MSI      = 0,  ///< Install-ChocolateyPackage with fileType msi
    MSIX     = 1,  ///< Install-ChocolateyWindowsApp (MSIX)
    EXE      = 2,  ///< Install-ChocolateyPackage with fileType exe
    ZIP      = 3,  ///< Install-ChocolateyZipPackage
    PORTABLE = 4,  ///< Portable install (copy to tools\)
};

// ---------------------------------------------------------------------------
// ChocolateyLicenseType — SPDX license identifier
// ---------------------------------------------------------------------------
enum class ChocolateyLicenseType : uint8_t {
    MIT         = 0,
    APACHE_2    = 1,
    GPL_2       = 2,
    GPL_3       = 3,
    BSD_2       = 4,
    BSD_3       = 5,
    PROPRIETARY = 6,
    OTHER       = 7,
};

// ---------------------------------------------------------------------------
// ChocolateyValidationStatus — nuspec validation result
// ---------------------------------------------------------------------------
enum class ChocolateyValidationStatus : uint8_t {
    OK                      = 0,
    MISSING_ID              = 1,
    ID_CONTAINS_SPACES      = 2,
    MISSING_VERSION         = 3,
    MISSING_TITLE           = 4,
    MISSING_AUTHORS         = 5,
    MISSING_DESCRIPTION     = 6,
    DESCRIPTION_TOO_SHORT   = 7,
    MISSING_TAGS            = 8,
    MISSING_PROJECT_URL     = 9,
    MISSING_LICENSE         = 10,
    CHECKSUM_WRONG_LENGTH   = 11,
    INSTALL_URL_NOT_HTTPS   = 12,
};

// ---------------------------------------------------------------------------
// ChocolateyInstallerSpec — download + verification details for chocolateyInstall.ps1
// ---------------------------------------------------------------------------
struct ChocolateyInstallerSpec final {
    std::wstring           downloadUrl;       ///< Must be HTTPS
    std::wstring           checksumSha256;    ///< 64 hex chars
    ChocolateyInstallType  installType{ChocolateyInstallType::MSI};
    std::wstring           silentArgs{L"/qn /norestart"}; ///< MSI silent flags
    std::wstring           validExitCodes{L"0, 3010"};    ///< Acceptable exit codes

    [[nodiscard]] bool HasValidChecksum() const noexcept {
        return checksumSha256.length() == 64u;
    }
    [[nodiscard]] bool IsHttps() const noexcept {
        return downloadUrl.substr(0, 8) == L"https://";
    }
};

// ---------------------------------------------------------------------------
// ChocolateyPackageMetadata — .nuspec metadata section
// ---------------------------------------------------------------------------
struct ChocolateyPackageMetadata final {
    std::wstring              packageId;           ///< e.g. "explorerlens" (no spaces)
    ChocolateyPackageVersion  version;
    std::wstring              title;               ///< Human-readable name
    std::wstring              authors;             ///< Comma-separated
    std::wstring              owners;              ///< Chocolatey.org profile name
    std::wstring              projectUrl;          ///< Must be a valid HTTPS URL
    std::wstring              licenseUrl;
    ChocolateyLicenseType     licenseType{ChocolateyLicenseType::MIT};
    std::wstring              iconUrl;             ///< CDN-hosted package icon
    std::wstring              description;         ///< Min 4 sentences recommended
    std::wstring              summary;             ///< One-line summary
    std::wstring              releaseNotes;
    std::vector<std::wstring> tags;                ///< Searchable tags
    bool                      requireLicenseAcceptance{false};
};

// ---------------------------------------------------------------------------
// ChocolateyPackageSchema — full package spec (metadata + installer)
// ---------------------------------------------------------------------------
struct ChocolateyPackageSchema final {
    ChocolateyPackageMetadata  metadata;
    ChocolateyInstallerSpec    installer;

    /// Validate all fields against Chocolatey submission requirements.
    [[nodiscard]] ChocolateyValidationStatus Validate() const noexcept {
        if (metadata.packageId.empty())
            return ChocolateyValidationStatus::MISSING_ID;
        if (metadata.packageId.find(L' ') != std::wstring::npos)
            return ChocolateyValidationStatus::ID_CONTAINS_SPACES;
        if (metadata.version.major == 0 && metadata.version.minor == 0
            && metadata.version.patch == 0)
            return ChocolateyValidationStatus::MISSING_VERSION;
        if (metadata.title.empty())
            return ChocolateyValidationStatus::MISSING_TITLE;
        if (metadata.authors.empty())
            return ChocolateyValidationStatus::MISSING_AUTHORS;
        if (metadata.description.empty())
            return ChocolateyValidationStatus::MISSING_DESCRIPTION;
        if (metadata.description.length() < 50u)
            return ChocolateyValidationStatus::DESCRIPTION_TOO_SHORT;
        if (metadata.tags.empty())
            return ChocolateyValidationStatus::MISSING_TAGS;
        if (metadata.projectUrl.empty())
            return ChocolateyValidationStatus::MISSING_PROJECT_URL;
        if (metadata.licenseUrl.empty())
            return ChocolateyValidationStatus::MISSING_LICENSE;
        if (!installer.HasValidChecksum())
            return ChocolateyValidationStatus::CHECKSUM_WRONG_LENGTH;
        if (!installer.IsHttps())
            return ChocolateyValidationStatus::INSTALL_URL_NOT_HTTPS;
        return ChocolateyValidationStatus::OK;
    }

    [[nodiscard]] bool IsValid() const noexcept {
        return Validate() == ChocolateyValidationStatus::OK;
    }

    /// Factory: build the standard ExplorerLens Chocolatey package spec.
    [[nodiscard]] static ChocolateyPackageSchema MakeExplorerLens(
        uint16_t major, uint16_t minor, uint16_t patch,
        const std::wstring& installerUrl,
        const std::wstring& sha256) noexcept
    {
        ChocolateyPackageSchema s;
        s.metadata.packageId            = L"explorerlens";
        s.metadata.version              = {major, minor, patch, 0u};
        s.metadata.title                = L"ExplorerLens";
        s.metadata.authors              = L"RajwanYair";
        s.metadata.owners               = L"RajwanYair";
        s.metadata.projectUrl           = L"https://github.com/RajwanYair/ExplorerLens.io";
        s.metadata.licenseUrl           = L"https://github.com/RajwanYair/ExplorerLens.io/blob/main/LICENSE";
        s.metadata.licenseType          = ChocolateyLicenseType::MIT;
        s.metadata.summary              = L"GPU-accelerated Windows Shell thumbnail provider for 200+ formats";
        s.metadata.description          =
            L"ExplorerLens is a Windows Shell Extension (IThumbnailProvider) that "
            L"generates thumbnails for 200+ file formats including HEIC, AVIF, JPEG XL, "
            L"camera RAW, PDF, archives, 3D models, and scientific formats. "
            L"It integrates directly into Windows Explorer with no restart required. "
            L"Zero-config, lightweight, and SSIM-validated for quality.";
        s.metadata.tags = {L"thumbnail", L"shell-extension", L"heic", L"avif", L"jpeg-xl",
                           L"raw", L"pdf", L"explorer", L"windows"};
        s.installer.downloadUrl         = installerUrl;
        s.installer.checksumSha256      = sha256;
        s.installer.installType         = ChocolateyInstallType::MSI;
        s.installer.silentArgs          = L"/qn /norestart";
        s.installer.validExitCodes      = L"0, 3010";
        return s;
    }
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kChocolateyMinDescriptionLength = 50u;
static constexpr uint32_t kChocolateyMaxIdLength          = 100u;
static constexpr uint32_t kChocolateySha256HexLength      = 64u;
static constexpr wchar_t  kChocolateyPackageId[]          = L"explorerlens";
static constexpr wchar_t  kChocolateyRegistryUrl[]        = L"https://community.chocolatey.org/packages/explorerlens";

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_CHOCOLATEYPACKAGESCHEMA_H
