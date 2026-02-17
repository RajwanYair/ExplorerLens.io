//==============================================================================
// DarkThumbs Engine — Sprint 49: Release Packaging & Distribution
//
// Provides MSI package validation, portable ZIP packaging, auto-update
// manifest generation, SBOM (Software Bill of Materials), signature
// verification chain, and release artifact management.
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <functional>

namespace DarkThumbs::Engine::Release {

//==============================================================================
// Package Type — Distribution format
//==============================================================================

enum class PackageType : uint8_t {
    MSI,           // Windows Installer (standard install)
    PortableZIP,   // Self-contained ZIP archive
    MSIX,          // Modern Windows package
    NuGet,         // Library package
    Symbols        // Debug symbols archive
};

inline const char* PackageTypeName(PackageType t) {
    switch (t) {
        case PackageType::MSI:         return "MSI";
        case PackageType::PortableZIP: return "PortableZIP";
        case PackageType::MSIX:        return "MSIX";
        case PackageType::NuGet:       return "NuGet";
        case PackageType::Symbols:     return "Symbols";
        default:                       return "Unknown";
    }
}

inline const char* PackageExtension(PackageType t) {
    switch (t) {
        case PackageType::MSI:         return ".msi";
        case PackageType::PortableZIP: return ".zip";
        case PackageType::MSIX:        return ".msix";
        case PackageType::NuGet:       return ".nupkg";
        case PackageType::Symbols:     return ".symbols.zip";
        default:                       return "";
    }
}

//==============================================================================
// Architecture — Target platform
//==============================================================================

enum class Architecture : uint8_t {
    x64,
    x86,
    ARM64
};

inline const char* ArchitectureName(Architecture a) {
    switch (a) {
        case Architecture::x64:   return "x64";
        case Architecture::x86:   return "x86";
        case Architecture::ARM64: return "ARM64";
        default:                  return "Unknown";
    }
}

//==============================================================================
// Version — Semantic versioning
//==============================================================================

struct Version {
    uint16_t major = 0;
    uint16_t minor = 0;
    uint16_t patch = 0;
    uint16_t build = 0;
    std::string preRelease; // "alpha", "beta.1", "rc.1", etc.

    std::string ToString() const {
        std::ostringstream ss;
        ss << major << "." << minor << "." << patch;
        if (build > 0) ss << "." << build;
        if (!preRelease.empty()) ss << "-" << preRelease;
        return ss.str();
    }

    std::string FileVersion() const {
        std::ostringstream ss;
        ss << major << "." << minor << "." << patch << "." << build;
        return ss.str();
    }

    bool IsPreRelease() const { return !preRelease.empty(); }

    bool IsNewerThan(const Version& other) const {
        if (major != other.major) return major > other.major;
        if (minor != other.minor) return minor > other.minor;
        if (patch != other.patch) return patch > other.patch;
        if (build != other.build) return build > other.build;
        // Pre-release < release
        if (IsPreRelease() && !other.IsPreRelease()) return false;
        if (!IsPreRelease() && other.IsPreRelease()) return true;
        return false;
    }

    static Version Parse(const std::string& s) {
        Version v;
        std::istringstream ss(s);
        char dot;
        ss >> v.major >> dot >> v.minor >> dot >> v.patch;
        if (ss.peek() == '.') {
            ss >> dot >> v.build;
        }
        if (ss.peek() == '-') {
            ss >> dot; // consume '-'
            std::getline(ss, v.preRelease);
        }
        return v;
    }

    static Version Current() {
        return {7, 0, 0, 0, ""};
    }
};

//==============================================================================
// Artifact — Single release file
//==============================================================================

struct Artifact {
    std::string  filename;
    PackageType  type     = PackageType::MSI;
    Architecture arch     = Architecture::x64;
    uint64_t     sizeBytes = 0;
    std::string  sha256;
    std::string  sha512;
    bool         isSigned = false;

    bool HasChecksum() const { return !sha256.empty(); }
    bool IsValid() const { return !filename.empty() && sizeBytes > 0; }

    std::string SizeHuman() const {
        if (sizeBytes >= 1024ULL * 1024 * 1024)
            return FormatSize(sizeBytes, 1024.0 * 1024.0 * 1024.0, "GB");
        if (sizeBytes >= 1024ULL * 1024)
            return FormatSize(sizeBytes, 1024.0 * 1024.0, "MB");
        if (sizeBytes >= 1024)
            return FormatSize(sizeBytes, 1024.0, "KB");
        return std::to_string(sizeBytes) + " B";
    }

private:
    static std::string FormatSize(uint64_t bytes, double divisor, const char* unit) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << (static_cast<double>(bytes) / divisor) << " " << unit;
        return ss.str();
    }
};

//==============================================================================
// MSI Validation — Windows Installer package checks
//==============================================================================

struct MSIValidationResult {
    bool hasProductCode   = false;
    bool hasUpgradeCode   = false;
    bool hasVersion       = false;
    bool hasManufacturer  = false;
    bool hasFiles         = false;
    bool hasUninstallInfo = false;

    bool IsValid() const {
        return hasProductCode && hasUpgradeCode
            && hasVersion && hasManufacturer
            && hasFiles && hasUninstallInfo;
    }

    size_t PassedChecks() const {
        size_t n = 0;
        if (hasProductCode)   ++n;
        if (hasUpgradeCode)   ++n;
        if (hasVersion)       ++n;
        if (hasManufacturer)  ++n;
        if (hasFiles)         ++n;
        if (hasUninstallInfo) ++n;
        return n;
    }

    size_t TotalChecks() const { return 6; }

    static MSIValidationResult AllPassed() {
        return {true, true, true, true, true, true};
    }

    static MSIValidationResult Empty() {
        return {};
    }
};

//==============================================================================
// SBOM Entry — Single dependency in the bill of materials
//==============================================================================

struct SBOMEntry {
    std::string name;
    std::string version;
    std::string license;
    std::string supplier;
    std::string url;
    bool        isDirectDependency = true;

    bool HasLicense() const { return !license.empty(); }
    bool HasURL()     const { return !url.empty(); }

    std::string FullName() const {
        if (version.empty()) return name;
        return name + "@" + version;
    }
};

//==============================================================================
// SBOM — Software Bill of Materials
//==============================================================================

struct SBOM {
    std::string             product;
    std::string             productVersion;
    std::string             format = "CycloneDX";
    std::vector<SBOMEntry>  entries;
    int64_t                 generatedTimestamp = 0;

    size_t DirectDependencyCount() const {
        return std::count_if(entries.begin(), entries.end(),
            [](const SBOMEntry& e) { return e.isDirectDependency; });
    }

    size_t TransitiveDependencyCount() const {
        return entries.size() - DirectDependencyCount();
    }

    bool AllHaveLicenses() const {
        return std::all_of(entries.begin(), entries.end(),
            [](const SBOMEntry& e) { return e.HasLicense(); });
    }

    static SBOM DarkThumbsSBOM() {
        SBOM sbom;
        sbom.product = "DarkThumbs";
        sbom.productVersion = "7.0.0";
        sbom.entries = {
            {"libjxl",      "0.11.1", "BSD-3-Clause", "Google",         "https://github.com/libjxl/libjxl", true},
            {"libheif",     "1.19.5", "LGPL-3.0",     "Dirk Farin",     "https://github.com/nicfit/libheif", true},
            {"libwebp",     "1.5.0",  "BSD-3-Clause", "Google",         "https://chromium.googlesource.com/webm/libwebp", true},
            {"LibRaw",      "0.21.3", "LGPL-2.1",     "LibRaw LLC",     "https://www.libraw.org/", true},
            {"libavif",     "1.3.0",  "BSD-2-Clause", "AOMedia",        "https://github.com/AOMediaCodec/libavif", true},
            {"zlib",        "1.3.1",  "Zlib",         "Jean-loup Gailly, Mark Adler", "https://zlib.net/", true},
            {"zstd",        "1.5.7",  "BSD-3-Clause", "Facebook",       "https://github.com/facebook/zstd", true},
            {"lz4",         "1.10.0", "BSD-2-Clause", "Yann Collet",    "https://github.com/lz4/lz4", true},
            {"lzma-sdk",    "26.00",  "Public Domain","Igor Pavlov",    "https://www.7-zip.org/sdk.html", true},
            {"minizip-ng",  "4.0.10", "Zlib",         "Nathan Moinvaziri", "https://github.com/zlib-ng/minizip-ng", true},
            {"unrar",       "7.2.2",  "Proprietary",  "Alexander L. Roshal", "https://www.rarlab.com/", true},
            {"dav1d",       "1.5.1",  "BSD-2-Clause", "VideoLAN",       "https://code.videolan.org/videolan/dav1d", true},
            {"brotli",      "1.1.0",  "MIT",          "Google",         "https://github.com/google/brotli", false},
            {"highway",     "1.2.0",  "Apache-2.0",   "Google",         "https://github.com/google/highway", false},
        };
        return sbom;
    }

    std::string ToText() const {
        std::ostringstream ss;
        ss << "=== Software Bill of Materials ===" << "\n"
           << "Product: " << product << " " << productVersion << "\n"
           << "Format: " << format << "\n"
           << "Dependencies: " << entries.size()
           << " (" << DirectDependencyCount() << " direct, "
           << TransitiveDependencyCount() << " transitive)" << "\n\n";

        for (const auto& e : entries) {
            ss << "  " << e.FullName()
               << " [" << e.license << "]"
               << (e.isDirectDependency ? "" : " (transitive)")
               << "\n";
        }
        return ss.str();
    }
};

//==============================================================================
// Update Channel — Release stability tier
//==============================================================================

enum class UpdateChannel : uint8_t {
    Stable,
    Beta,
    Nightly,
    Canary
};

inline const char* UpdateChannelName(UpdateChannel c) {
    switch (c) {
        case UpdateChannel::Stable:  return "Stable";
        case UpdateChannel::Beta:    return "Beta";
        case UpdateChannel::Nightly: return "Nightly";
        case UpdateChannel::Canary:  return "Canary";
        default:                     return "Unknown";
    }
}

//==============================================================================
// Update Manifest — Auto-update descriptor
//==============================================================================

struct UpdateManifest {
    Version       version;
    UpdateChannel channel      = UpdateChannel::Stable;
    std::string   downloadUrl;
    std::string   releaseNotes;
    uint64_t      sizeBytes    = 0;
    std::string   sha256;
    bool          isRequired   = false; // Security/critical update

    bool HasDownloadInfo() const { return !downloadUrl.empty() && sizeBytes > 0; }
    bool HasChecksum()     const { return !sha256.empty(); }

    std::string ToJSON() const {
        std::ostringstream ss;
        ss << "{\n"
           << "  \"version\": \"" << version.ToString() << "\",\n"
           << "  \"channel\": \"" << UpdateChannelName(channel) << "\",\n"
           << "  \"download_url\": \"" << downloadUrl << "\",\n"
           << "  \"size_bytes\": " << sizeBytes << ",\n"
           << "  \"sha256\": \"" << sha256 << "\",\n"
           << "  \"required\": " << (isRequired ? "true" : "false") << "\n"
           << "}\n";
        return ss.str();
    }
};

//==============================================================================
// Signature Info — Code signing verification
//==============================================================================

struct SignatureInfo {
    std::string  subject;
    std::string  issuer;
    std::string  thumbprint;
    std::string  algorithm; // "SHA256", "SHA384"
    int64_t      notBefore = 0;
    int64_t      notAfter  = 0;
    bool         isValid   = false;
    bool         isTrusted = false;
    bool         isExpired = false;

    bool IsFullyVerified() const {
        return isValid && isTrusted && !isExpired;
    }

    std::string StatusText() const {
        if (IsFullyVerified()) return "Verified";
        if (!isValid) return "Invalid Signature";
        if (!isTrusted) return "Untrusted Certificate";
        if (isExpired) return "Expired Certificate";
        return "Unknown";
    }
};

//==============================================================================
// Release Config — Package generation settings
//==============================================================================

struct ReleaseConfig {
    Version      version;
    Architecture arch         = Architecture::x64;
    bool         signBinaries = true;
    bool         generateSBOM = true;
    bool         generateMSI  = true;
    bool         generateZIP  = true;
    bool         generateMSIX = false;
    std::string  outputDir    = "dist";
    std::string  signingCert;

    std::vector<PackageType> PackagesToBuild() const {
        std::vector<PackageType> pkgs;
        if (generateMSI)  pkgs.push_back(PackageType::MSI);
        if (generateZIP)  pkgs.push_back(PackageType::PortableZIP);
        if (generateMSIX) pkgs.push_back(PackageType::MSIX);
        pkgs.push_back(PackageType::Symbols);
        return pkgs;
    }

    static ReleaseConfig Default() {
        ReleaseConfig c;
        c.version = Version::Current();
        return c;
    }

    static ReleaseConfig CI() {
        ReleaseConfig c;
        c.version = Version::Current();
        c.signBinaries = false;
        c.generateMSIX = false;
        return c;
    }

    static ReleaseConfig Full() {
        ReleaseConfig c;
        c.version = Version::Current();
        c.generateMSIX = true;
        return c;
    }
};

} // namespace DarkThumbs::Engine::Release
