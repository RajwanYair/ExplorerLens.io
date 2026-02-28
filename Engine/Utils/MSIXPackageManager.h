#pragma once
//==============================================================================
// MSIXPackageManager
// Modern Windows MSIX packaging with auto-update and identity integration
//
// Architecture:
// 1. MSIX manifest generation from project metadata
// 2. Package signing with Authenticode/self-signed certificates
// 3. Auto-update channel management (stable/beta/dev)
// 4. App identity for Windows Store sideload
// 5. Sparse package registration for desktop bridge
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Package distribution channel
enum class PackageChannel : uint8_t {
 Stable, ///< Production release
 Beta, ///< Pre-release testing
 Dev, ///< Development builds
 Canary, ///< Nightly/experimental
 Internal ///< Internal-only distribution
};

/// Package signing mode
enum class SigningMode : uint8_t {
 None, ///< Unsigned (dev only)
 SelfSigned, ///< Self-signed certificate
 Authenticode, ///< CA-signed Authenticode
 StoreSigned, ///< Microsoft Store signing
 AzureTrusted ///< Azure Code Signing (EV)
};

/// Package type
enum class PackageType : uint8_t {
 MSIX, ///< Single architecture
 MSIXBundle, ///< Multi-architecture bundle
 AppX, ///< Legacy UWP package
 SparsePackage, ///< Desktop Bridge sparse package
 MSIX_Appinstaller ///< Auto-update manifest
};

/// Package capability
enum class PackageCapability : uint32_t {
 None = 0,
 RunFullTrust = 1 << 0, ///< Full trust for COM extension
 ShellExtension = 1 << 1, ///< Shell extension handler
 FileAssociation = 1 << 2, ///< File type associations
 COMServer = 1 << 3, ///< COM server registration
 BackgroundTasks = 1 << 4, ///< Background processing
 Notifications = 1 << 5, ///< Toast notifications (updates)
 InternetClient = 1 << 6, ///< Auto-update download
};

inline PackageCapability operator|(PackageCapability a, PackageCapability b) {
 return static_cast<PackageCapability>(static_cast<uint32_t>(a) |
 static_cast<uint32_t>(b));
}
inline bool HasCapability(PackageCapability set, PackageCapability cap) {
 return (static_cast<uint32_t>(set) & static_cast<uint32_t>(cap)) != 0;
}

/// Auto-update configuration for MSIX packages (distinct from
/// AutoUpdateEngineConfig)
struct MSIXAutoUpdateConfig {
 std::wstring updateUri; ///< .appinstaller URI
 uint32_t checkIntervalHours = 24; ///< Update check frequency
 bool showPrompt = true; ///< Prompt user before update
 bool allowBackgroundUpdate = true; ///< Download in background
 bool forceUpdateOnSecurity = true; ///< Force-update for security patches
 PackageChannel channel = PackageChannel::Stable;
};

/// MSIX package configuration
struct MSIXConfig {
 std::wstring packageName = L"ExplorerLens";
 std::wstring publisherName = L"CN=ExplorerLens";
 std::wstring publisherDisplayName = L"ExplorerLens Project";
 std::wstring version = L"9.2.0.0";
 std::wstring description =
 L"GPU-accelerated thumbnail provider for 200+ formats";
 std::wstring logoPath = L"Assets\\Logo.png";
 PackageType type = PackageType::MSIX;
 SigningMode signing = SigningMode::SelfSigned;
 PackageCapability capabilities = PackageCapability::RunFullTrust |
 PackageCapability::ShellExtension |
 PackageCapability::COMServer;
 MSIXAutoUpdateConfig autoUpdate;
 std::wstring minWindowsVersion = L"10.0.17763.0"; ///< Win10 1809+
 bool includeARM64 = true;
 bool includex64 = true;
};

/// Package build result
struct PackageBuildResult {
 bool success = false;
 std::wstring outputPath;
 uint64_t fileSizeBytes = 0;
 std::wstring errorMessage;
 std::wstring signatureInfo;
 PackageType type = PackageType::MSIX;
};

//==============================================================================
// MSIXPackageManager
//==============================================================================
class MSIXPackageManager {
public:
 MSIXPackageManager();
 explicit MSIXPackageManager(const MSIXConfig &config);

 /// Generate AppxManifest.xml content
 std::wstring GenerateManifest() const;

 /// Generate .appinstaller auto-update manifest
 std::wstring GenerateAppInstaller() const;

 /// Build MSIX package (shell out to makeappx.exe)
 PackageBuildResult BuildPackage(const std::wstring &outputDir) const;

 /// Sign package (shell out to signtool.exe)
 bool SignPackage(const std::wstring &packagePath,
 const std::wstring &certPath) const;

 /// Validate manifest against schema
 bool ValidateManifest(const std::wstring &manifestXml) const;

 /// Check if MSIX runtime is available
 static bool IsMSIXSupported();

 /// Check Windows version compatibility
 static bool MeetsMinVersion(const std::wstring &minVersion);

 /// Get config
 const MSIXConfig &GetConfig() const { return m_config; }

 /// Static name helpers
 static const wchar_t *GetChannelName(PackageChannel channel);
 static const wchar_t *GetSigningName(SigningMode mode);
 static const wchar_t *GetPackageTypeName(PackageType type);

private:
 MSIXConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
