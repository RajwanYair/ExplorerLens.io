// PluginMarketplaceUnified.h — Canonical Plugin Marketplace (V1+V2+V3 consolidated)
// Copyright (c) 2026 ExplorerLens Project
//
// Unified marketplace header consolidating V1 key types, V2 production API,
// and V3 trust/sandbox utilities.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

// ════════════════════════════════════════════════════════════════════════════
// Section 1: V1 key types (consolidated from PluginMarketplace.h)
// ════════════════════════════════════════════════════════════════════════════

/// Package content type
enum class PluginPackageType {
    Decoder, ///< Format decoder plugin (IThumbnailDecoder)
    Theme, ///< UI theme/skin
    LanguagePack, ///< Localization pack
    GPUFilter, ///< GPU post-processing filter
    CacheProvider, ///< Custom cache backend
    Utility ///< General utility plugin
};

/// Target architecture
enum class PluginArch {
    x64,
    ARM64,
    Universal ///< Contains both x64 and ARM64
};

/// Plugin compatibility version range
struct VersionRange {
    uint32_t minMajor = 7, minMinor = 0, minPatch = 0;
    uint32_t maxMajor = 99, maxMinor = 99, maxPatch = 99;

    bool Contains(uint32_t major, uint32_t minor, uint32_t patch) const {
        uint32_t ver = major * 10000 + minor * 100 + patch;
        uint32_t minVer = minMajor * 10000 + minMinor * 100 + minPatch;
        uint32_t maxVer = maxMajor * 10000 + maxMinor * 100 + maxPatch;
        return ver >= minVer && ver <= maxVer;
    }
};

/// Metadata embedded in a .dtpkg package
struct PluginManifest {
    std::string id;
    std::string name;
    std::string description;
    std::string author;
    std::string authorEmail;
    std::string homepage;
    std::string license;

    uint32_t versionMajor = 1;
    uint32_t versionMinor = 0;
    uint32_t versionPatch = 0;
    VersionRange compatibleWith;
    PluginArch architecture = PluginArch::x64;

    PluginPackageType type = PluginPackageType::Decoder;
    std::vector<std::string> tags;
    std::vector<std::string> supportedFormats;

    struct Dependency {
        std::string pluginId;
        uint32_t minVersion = 0;
    };
    std::vector<Dependency> dependencies;

    std::string entryDll;
    std::vector<std::string> files;
    uint64_t packageSizeBytes = 0;

    std::string GetVersionString() const {
        return std::to_string(versionMajor) + "." +
            std::to_string(versionMinor) + "." +
            std::to_string(versionPatch);
    }
};

/// Signature verification status
enum class SignatureStatus {
    Valid,
    Invalid,
    Expired,
    Untrusted,
    SelfSigned,
    Missing,
    Revoked
};

/// Certificate information from a signed package
struct PluginCertificateInfo {
    std::string subject;
    std::string issuer;
    std::string thumbprint;
    std::string serialNumber;
    std::chrono::system_clock::time_point validFrom;
    std::chrono::system_clock::time_point validTo;
    bool isEV = false;
    SignatureStatus status = SignatureStatus::Missing;
};

/// Signing policy for the marketplace
struct SigningPolicy {
    bool requireSignature = true;
    bool allowSelfSigned = false;
    bool requireEV = false;
    std::vector<std::string> trustedIssuers;
    std::vector<std::string> trustedThumbprints;
    bool enforceTimestamp = true;
};

// ════════════════════════════════════════════════════════════════════════════
// Section 2: V2 production API (consolidated from PluginMarketplaceV2.h)
// ════════════════════════════════════════════════════════════════════════════

/// Plugin category
enum class PluginCategory : uint8_t {
    Decoder, ///< Format decoder plugin
    Renderer, ///< Custom rendering plugin
    PostProcessor, ///< Post-processing effect
    CacheProvider, ///< Alternative cache backend
    Integration, ///< Third-party integration
    Utility ///< Utility/helper plugin
};

/// Plugin version (semver)
struct PluginSemVer {
    uint16_t major = 0;
    uint16_t minor = 0;
    uint16_t patch = 0;
    std::wstring prerelease;

    bool operator>=(const PluginSemVer& other) const {
        if (major != other.major) return major > other.major;
        if (minor != other.minor) return minor > other.minor;
        return patch >= other.patch;
    }
    bool operator==(const PluginSemVer& other) const {
        return major == other.major && minor == other.minor && patch == other.patch;
    }
    std::wstring ToString() const {
        return std::to_wstring(major) + L"." + std::to_wstring(minor) + L"." + std::to_wstring(patch);
    }
    static PluginSemVer Parse(const std::wstring& str);
};

/// Plugin dependency
struct PluginDependency {
    std::wstring pluginId;
    PluginSemVer minVersion;
    bool optional = false;
};

/// Plugin listing in marketplace
struct PluginListing {
    std::wstring id; ///< Unique identifier (reverse-domain)
    std::wstring name;
    std::wstring description;
    std::wstring author;
    std::wstring authorUrl;
    PluginSemVer version;
    PluginCategory category = PluginCategory::Decoder;
    std::vector<std::wstring> extensions; ///< File extensions this plugin handles
    std::vector<PluginDependency> dependencies;
    uint64_t downloadSize = 0; ///< Bytes
    std::wstring sha256Hash;
    uint32_t downloads = 0;
    double rating = 0.0; ///< 0-5 stars
    bool isVerified = false; ///< Authenticode signed
    bool isCompatible = true; ///< Compatible with current version
    std::wstring engineMinVersion; ///< Minimum engine version required
};

/// Installation state
enum class PluginInstallState : uint8_t {
    NotInstalled,
    Downloading,
    Verifying,
    Installing,
    Installed,
    UpdateAvailable,
    Failed,
    Disabled
};

/// Installed plugin info
struct InstalledPlugin {
    PluginListing listing;
    PluginInstallState state = PluginInstallState::NotInstalled;
    std::wstring installPath;
    std::wstring installDate;
    PluginSemVer installedVersion;
    bool autoUpdate = true;
};

/// Marketplace search filter
struct MarketplaceFilter {
    std::wstring query;
    PluginCategory category = PluginCategory::Decoder;
    bool verifiedOnly = false;
    bool compatibleOnly = true;
    uint32_t maxResults = 50;
    enum SortBy { Relevance, Downloads, Rating, Recent } sortBy = Relevance;
};

/// Marketplace result
struct MarketplaceResult {
    std::vector<PluginListing> plugins;
    uint32_t totalCount = 0;
    uint32_t page = 1;
    uint32_t pageSize = 50;
    bool hasMore = false;
    std::wstring error;
};

/// Plugin Marketplace V2 — production API
class PluginMarketplaceV2 {
public:
    PluginMarketplaceV2();
    explicit PluginMarketplaceV2(const std::wstring& catalogUrl);

    MarketplaceResult Search(const MarketplaceFilter& filter) const;
    bool Install(const PluginListing& plugin);
    bool Uninstall(const std::wstring& pluginId);
    std::vector<PluginListing> CheckUpdates() const;
    const std::vector<InstalledPlugin>& GetInstalled() const { return m_installed; }
    void AddToCatalog(const PluginListing& plugin);
    PluginInstallState GetState(const std::wstring& pluginId) const;
    static bool VerifyHash(const std::wstring& filePath, const std::wstring& expectedHash);
    static const wchar_t* GetCategoryName(PluginCategory cat);
    static const wchar_t* GetStateName(PluginInstallState state);
    const std::wstring& GetCatalogUrl() const { return m_catalogUrl; }

private:
    std::wstring m_catalogUrl;
    std::vector<PluginListing> m_catalog;
    std::vector<InstalledPlugin> m_installed;
};

// ════════════════════════════════════════════════════════════════════════════
// Section 3: V3 trust/sandbox utilities (consolidated from PluginMarketplaceV3.h)
// ════════════════════════════════════════════════════════════════════════════

/// Plugin marketplace category V3
enum class PluginCategoryV3 : uint8_t {
    ImageDecoder,
    ArchiveHandler,
    DocumentViewer,
    ModelRenderer,
    ScientificData,
    AudioVisualizer,
    ThemeProvider,
    Utility,
    COUNT
};

/// Plugin trust level V3
enum class PluginTrustLevelV3 : uint8_t {
    Untrusted,
    CommunityReviewed,
    Verified,
    Official,
    COUNT
};

/// Plugin sandbox policy
enum class SandboxPolicy : uint8_t {
    None,
    FileSystem,
    Network,
    Full,
    COUNT
};

/// Marketplace entry V3
struct MarketplaceEntryV3 {
    std::wstring pluginId;
    std::wstring displayName;
    std::wstring publisher;
    std::wstring version;
    PluginCategoryV3 category = PluginCategoryV3::Utility;
    PluginTrustLevelV3 trustLevel = PluginTrustLevelV3::Untrusted;
    SandboxPolicy sandbox = SandboxPolicy::Full;
    uint32_t downloads = 0;
    double rating = 0.0;
    uint32_t ratingCount = 0;
    bool autoUpdate = true;
};

/// Plugin Marketplace V3 — static utilities
class PluginMarketplaceV3 {
public:
    static const wchar_t* CategoryName(PluginCategoryV3 c) {
        switch (c) {
        case PluginCategoryV3::ImageDecoder: return L"Image Decoder";
        case PluginCategoryV3::ArchiveHandler: return L"Archive Handler";
        case PluginCategoryV3::DocumentViewer: return L"Document Viewer";
        case PluginCategoryV3::ModelRenderer: return L"Model Renderer";
        case PluginCategoryV3::ScientificData: return L"Scientific Data";
        case PluginCategoryV3::AudioVisualizer: return L"Audio Visualizer";
        case PluginCategoryV3::ThemeProvider: return L"Theme Provider";
        case PluginCategoryV3::Utility: return L"Utility";
        default: return L"Unknown";
        }
    }

    static const wchar_t* TrustName(PluginTrustLevelV3 t) {
        switch (t) {
        case PluginTrustLevelV3::Untrusted: return L"Untrusted";
        case PluginTrustLevelV3::CommunityReviewed: return L"Community Reviewed";
        case PluginTrustLevelV3::Verified: return L"Verified";
        case PluginTrustLevelV3::Official: return L"Official";
        default: return L"Unknown";
        }
    }

    static const wchar_t* SandboxName(SandboxPolicy s) {
        switch (s) {
        case SandboxPolicy::None: return L"None";
        case SandboxPolicy::FileSystem: return L"File System";
        case SandboxPolicy::Network: return L"Network";
        case SandboxPolicy::Full: return L"Full";
        default: return L"Unknown";
        }
    }

    static constexpr size_t CategoryCount() { return static_cast<size_t>(PluginCategoryV3::COUNT); }
    static constexpr size_t TrustLevelCount() { return static_cast<size_t>(PluginTrustLevelV3::COUNT); }
    static constexpr size_t SandboxPolicyCount() { return static_cast<size_t>(SandboxPolicy::COUNT); }
};

// ════════════════════════════════════════════════════════════════════════════
// Section 4: Unified helpers
// ════════════════════════════════════════════════════════════════════════════

/// Helper to select the appropriate marketplace version at compile time
enum class MarketplaceVersion : uint32_t {
    V2 = 2,   ///< Production implementation with SHA-256 (recommended)
    V3 = 3    ///< Static utility enums only
};

/// Returns the recommended marketplace version for new code
inline constexpr MarketplaceVersion RecommendedMarketplaceVersion() {
    return MarketplaceVersion::V2;
}

/// Returns the number of active (non-legacy) marketplace versions
inline constexpr uint32_t ActiveMarketplaceVersionCount() {
    return 2;
}

} // namespace Engine
} // namespace ExplorerLens
