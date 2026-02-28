#pragma once
//==============================================================================
// PluginMarketplaceV2
// Online plugin discovery, installation, and lifecycle management.
//
// Features:
// - Plugin catalog (REST API based) with search, categories, ratings
// - Semantic versioning with dependency resolution
// - Download with integrity verification (SHA-256 + Authenticode)
// - Auto-update checking and staged rollout
// - Plugin bundle format (.dtp — ExplorerLens Plugin archive)
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace ExplorerLens { namespace Engine {

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
struct PluginVersion {
 uint16_t major = 0;
 uint16_t minor = 0;
 uint16_t patch = 0;
 std::wstring prerelease;

 bool operator>=(const PluginVersion& other) const {
 if (major != other.major) return major > other.major;
 if (minor != other.minor) return minor > other.minor;
 return patch >= other.patch;
 }
 bool operator==(const PluginVersion& other) const {
 return major == other.major && minor == other.minor && patch == other.patch;
 }
 std::wstring ToString() const {
 return std::to_wstring(major) + L"." + std::to_wstring(minor) + L"." + std::to_wstring(patch);
 }
 static PluginVersion Parse(const std::wstring& str);
};

/// Plugin dependency
struct PluginDependency {
 std::wstring pluginId;
 PluginVersion minVersion;
 bool optional = false;
};

/// Plugin listing in marketplace
struct PluginListing {
 std::wstring id; ///< Unique identifier (reverse-domain)
 std::wstring name;
 std::wstring description;
 std::wstring author;
 std::wstring authorUrl;
 PluginVersion version;
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
 PluginVersion installedVersion;
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

//==============================================================================
// PluginMarketplaceV2
//==============================================================================
class PluginMarketplaceV2 {
public:
 PluginMarketplaceV2();
 explicit PluginMarketplaceV2(const std::wstring& catalogUrl);

 /// Search marketplace
 MarketplaceResult Search(const MarketplaceFilter& filter) const;

 /// Install a plugin
 bool Install(const PluginListing& plugin);

 /// Uninstall a plugin
 bool Uninstall(const std::wstring& pluginId);

 /// Check for updates
 std::vector<PluginListing> CheckUpdates() const;

 /// Get installed plugins
 const std::vector<InstalledPlugin>& GetInstalled() const { return m_installed; }

 /// Add a plugin to catalog (for testing)
 void AddToCatalog(const PluginListing& plugin);

 /// Get plugin install state
 PluginInstallState GetState(const std::wstring& pluginId) const;

 /// Verify download integrity
 static bool VerifyHash(const std::wstring& filePath, const std::wstring& expectedHash);

 /// Get category name
 static const wchar_t* GetCategoryName(PluginCategory cat);
 static const wchar_t* GetStateName(PluginInstallState state);

 /// Get catalog URL
 const std::wstring& GetCatalogUrl() const { return m_catalogUrl; }

private:
 std::wstring m_catalogUrl;
 std::vector<PluginListing> m_catalog;
 std::vector<InstalledPlugin> m_installed;
};

}} // namespace ExplorerLens::Engine

