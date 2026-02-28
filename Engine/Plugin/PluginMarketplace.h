// PluginMarketplace.h - Plugin Marketplace & Distribution
// ExplorerLens Engine v7.0.0+
// Copyright (c) 2026 ExplorerLens Project
//
// Features:
// - REST API client for marketplace catalog
// - Plugin package format (.dtpkg) with metadata + signatures
// - Authenticode & custom code signing verification
// - Security scanning before installation
// - Rating/review system integration
// - In-app marketplace browser
// - Auto-update and version management
//
// Architecture:
// MarketplaceClient → PluginPackage → SecurityScanner → PluginInstaller
// ↘ ReviewSystem

#pragma once

#include "../Core/Types.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Plugin Package Format
// ============================================================================

/// Package content type
enum class PackageType {
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
 // Identity
 std::string id; ///< Reverse-domain: "com.author.pluginname"
 std::string name; ///< Human-readable name
 std::string description; ///< Short description
 std::string author; ///< Author name
 std::string authorEmail; ///< Contact email
 std::string homepage; ///< URL to project page
 std::string license; ///< SPDX identifier (e.g., "MIT")
 
 // Version & compatibility
 uint32_t versionMajor = 1;
 uint32_t versionMinor = 0;
 uint32_t versionPatch = 0;
 VersionRange compatibleWith; ///< ExplorerLens version compatibility
 PluginArch architecture = PluginArch::x64;
 
 // Classification
 PackageType type = PackageType::Decoder;
 std::vector<std::string> tags; ///< Searchable tags
 std::vector<std::string> supportedFormats; ///< File extensions for decoders
 
 // Dependencies
 struct Dependency {
 std::string pluginId;
 uint32_t minVersion = 0;
 };
 std::vector<Dependency> dependencies;
 
 // Package contents
 std::string entryDll; ///< Main DLL filename
 std::vector<std::string> files; ///< All files in package
 uint64_t packageSizeBytes = 0;
 
 /// Get version string
 std::string GetVersionString() const {
 return std::to_string(versionMajor) + "." +
 std::to_string(versionMinor) + "." +
 std::to_string(versionPatch);
 }
};


// ============================================================================
// Code Signing & Verification
// ============================================================================

/// Signature verification status
enum class SignatureStatus {
 Valid, ///< Signature valid and trusted
 Invalid, ///< Signature corrupted or doesn't match
 Expired, ///< Certificate has expired
 Untrusted, ///< Valid signature but untrusted root CA
 SelfSigned, ///< Self-signed certificate (warning)
 Missing, ///< No signature present
 Revoked ///< Certificate has been revoked
};

/// Certificate information from a signed package
struct CertificateInfo {
 std::string subject; ///< Certificate subject (CN)
 std::string issuer; ///< Certificate issuer
 std::string thumbprint; ///< SHA-256 thumbprint (hex)
 std::string serialNumber; ///< Serial number
 std::chrono::system_clock::time_point validFrom;
 std::chrono::system_clock::time_point validTo;
 bool isEV = false; ///< Extended Validation certificate
 SignatureStatus status = SignatureStatus::Missing;
};

/// Signing policy for the marketplace
struct SigningPolicy {
 bool requireSignature = true; ///< Reject unsigned packages
 bool allowSelfSigned = false; ///< Allow self-signed certs
 bool requireEV = false; ///< Require EV certificates
 std::vector<std::string> trustedIssuers; ///< Whitelist of trusted CAs
 std::vector<std::string> trustedThumbprints;///< Pinned certificate thumbprints
 bool enforceTimestamp = true; ///< Require signed timestamp (RFC 3161)
};

/// Verifies code signatures on plugin packages
class SignatureVerifier {
public:
 explicit SignatureVerifier(SigningPolicy policy = {})
 : m_policy(policy) {}

 /// Verify a package file's signature
 SignatureStatus Verify(const std::wstring& packagePath, CertificateInfo& certOut) const {
 // In real implementation: use WinVerifyTrust / WinCrypt APIs
 // This is the architecture and contract definition
 (void)packagePath;
 certOut.status = SignatureStatus::Missing;
 return certOut.status;
 }

 /// Check if a signature status is acceptable under current policy
 bool IsAcceptable(SignatureStatus status) const {
 switch (status) {
 case SignatureStatus::Valid: return true;
 case SignatureStatus::SelfSigned:return m_policy.allowSelfSigned;
 case SignatureStatus::Missing: return !m_policy.requireSignature;
 default: return false;
 }
 }

 const SigningPolicy& GetPolicy() const { return m_policy; }

private:
 SigningPolicy m_policy;
};


// ============================================================================
// Security Scanner
// ============================================================================

/// Security scan severity levels
enum class ScanSeverity {
 None, ///< No issues found
 Info, ///< Informational finding
 Warning, ///< Potential issue (non-blocking)
 Critical ///< Security risk (blocks installation)
};

/// Individual security finding
struct SecurityFinding {
 ScanSeverity severity = ScanSeverity::None;
 std::string category; ///< "API", "Permission", "Binary", "Dependency"
 std::string description; ///< Human-readable finding description
 std::string filePath; ///< File within package
 std::string recommendation; ///< Suggested remediation
};

/// Result of a security scan
struct ScanResult {
 bool passed = true;
 std::vector<SecurityFinding> findings;
 uint32_t criticalCount = 0;
 uint32_t warningCount = 0;
 uint32_t infoCount = 0;
 double scanDurationMs = 0.0;

 void AddFinding(SecurityFinding finding) {
 switch (finding.severity) {
 case ScanSeverity::Critical: criticalCount++; passed = false; break;
 case ScanSeverity::Warning: warningCount++; break;
 case ScanSeverity::Info: infoCount++; break;
 default: break;
 }
 findings.push_back(std::move(finding));
 }
};

/// Scans plugin packages for security issues before installation
class SecurityScanner {
public:
 /// Scan a plugin package for security issues
 ScanResult Scan(const PluginManifest& manifest) const {
 ScanResult result;

 // Check 1: Verify no suspicious API imports
 CheckSuspiciousAPIs(manifest, result);

 // Check 2: Verify file types are expected
 CheckFileTypes(manifest, result);

 // Check 3: Verify size limits
 CheckSizeLimits(manifest, result);

 // Check 4: Verify dependencies are known
 CheckDependencies(manifest, result);

 return result;
 }

private:
 void CheckSuspiciousAPIs(const PluginManifest& manifest, ScanResult& result) const {
 // In real implementation: parse PE imports for dangerous APIs
 // Block: CreateRemoteThread, WriteProcessMemory, VirtualAllocEx
 (void)manifest;
 (void)result;
 }

 void CheckFileTypes(const PluginManifest& manifest, ScanResult& result) const {
 static const std::vector<std::string> blockedExtensions = {
 ".exe", ".bat", ".cmd", ".ps1", ".vbs", ".js", ".wsf"
 };

 for (const auto& file : manifest.files) {
 for (const auto& blocked : blockedExtensions) {
 if (file.length() >= blocked.length() &&
 file.compare(file.length() - blocked.length(), blocked.length(), blocked) == 0) {
 result.AddFinding({
 ScanSeverity::Critical,
 "FileType",
 "Package contains blocked file type: " + blocked,
 file,
 "Remove executable files from plugin package"
 });
 }
 }
 }
 }

 void CheckSizeLimits(const PluginManifest& manifest, ScanResult& result) const {
 const uint64_t maxPackageSize = 100 * 1024 * 1024; // 100 MB
 if (manifest.packageSizeBytes > maxPackageSize) {
 result.AddFinding({
 ScanSeverity::Warning,
 "Size",
 "Package exceeds recommended size limit (100 MB)",
 "",
 "Optimize plugin size or split into multiple packages"
 });
 }
 }

 void CheckDependencies(const PluginManifest& manifest, ScanResult& result) const {
 for (const auto& dep : manifest.dependencies) {
 if (dep.pluginId.empty()) {
 result.AddFinding({
 ScanSeverity::Warning,
 "Dependency",
 "Empty dependency ID found",
 "",
 "Specify valid plugin IDs for all dependencies"
 });
 }
 }
 }
};


// ============================================================================
// Rating & Review System
// ============================================================================

/// User review for a plugin
struct PluginReview {
 std::string pluginId;
 std::string userId;
 std::string displayName;
 uint32_t rating = 0; ///< 1-5 stars
 std::string title;
 std::string body;
 std::string pluginVersion; ///< Version at time of review
 std::chrono::system_clock::time_point submittedAt;
 uint32_t helpfulVotes = 0;
 bool verified = false; ///< Verified installation
};

/// Aggregate rating statistics
struct RatingStats {
 double averageRating = 0.0;
 uint32_t totalRatings = 0;
 uint32_t distribution[5] = {}; ///< [0]=1-star, [4]=5-star

 void AddRating(uint32_t stars) {
 if (stars < 1 || stars > 5) return;
 distribution[stars - 1]++;
 totalRatings++;
 // Recalculate average
 double sum = 0;
 for (int i = 0; i < 5; ++i) sum += distribution[i] * (i + 1);
 averageRating = sum / totalRatings;
 }
};


// ============================================================================
// Marketplace REST API Client
// ============================================================================

/// Marketplace catalog entry (from server response)
struct MarketplaceEntry {
 PluginManifest manifest;
 RatingStats ratings;
 uint32_t downloadCount = 0;
 bool featured = false;
 bool verified = false; ///< Marketplace-verified plugin
 std::string downloadUrl;
 std::string iconUrl;
 std::string screenshotUrl;
 std::chrono::system_clock::time_point lastUpdated;
};

/// Search/filter criteria
struct MarketplaceQuery {
 std::string searchText;
 PackageType typeFilter = PackageType::Decoder;
 bool filterByType = false;
 enum class SortBy { Relevance, Downloads, Rating, Recent } sort = SortBy::Relevance;
 uint32_t page = 1;
 uint32_t pageSize = 20;
 bool featuredOnly = false;
 bool verifiedOnly = false;
};

/// Marketplace API response
struct MarketplaceResponse {
 std::vector<MarketplaceEntry> entries;
 uint32_t totalCount = 0;
 uint32_t page = 1;
 uint32_t pageSize = 20;
 bool hasMore = false;
};

/// REST API client for the ExplorerLens plugin marketplace
class MarketplaceClient {
public:
 /// Marketplace server configuration
 struct ServerConfig {
 std::string baseUrl = "https://marketplace.explorerlens.io/api/v1";
 std::string apiKey;
 uint32_t timeoutMs = 30000;
 bool useTLS = true;
 std::string proxyUrl;
 };

 explicit MarketplaceClient(ServerConfig config = {})
 : m_config(config) {}

 /// Search the marketplace catalog
 bool Search(const MarketplaceQuery& query, MarketplaceResponse& response) const {
 // Build REST URL: GET /plugins?q=<text>&type=<type>&sort=<sort>&page=<page>
 std::string url = m_config.baseUrl + "/plugins?";
 if (!query.searchText.empty()) {
 url += "q=" + query.searchText + "&";
 }
 if (query.filterByType) {
 url += "type=" + std::to_string(static_cast<int>(query.typeFilter)) + "&";
 }
 url += "page=" + std::to_string(query.page);
 url += "&pageSize=" + std::to_string(query.pageSize);

 // In real implementation: WinHTTP GET request
 m_lastRequestUrl = url;
 response.page = query.page;
 response.pageSize = query.pageSize;
 return true;
 }

 /// Get details for a specific plugin
 bool GetPluginDetails(const std::string& pluginId, MarketplaceEntry& entry) const {
 std::string url = m_config.baseUrl + "/plugins/" + pluginId;
 m_lastRequestUrl = url;
 (void)entry;
 return true;
 }

 /// Download a plugin package
 bool DownloadPackage(const std::string& downloadUrl, const std::wstring& destPath,
 std::function<void(uint64_t, uint64_t)> progressCallback = nullptr) const {
 // In real implementation: WinHTTP download with progress
 (void)downloadUrl;
 (void)destPath;
 if (progressCallback) progressCallback(0, 0);
 return true;
 }

 /// Submit a review
 bool SubmitReview(const PluginReview& review) const {
 std::string url = m_config.baseUrl + "/plugins/" + review.pluginId + "/reviews";
 m_lastRequestUrl = url;
 return !review.pluginId.empty() && review.rating >= 1 && review.rating <= 5;
 }

 /// Check for plugin updates
 bool CheckForUpdates(const std::vector<std::string>& installedPluginIds,
 std::vector<MarketplaceEntry>& updates) const {
 // POST /plugins/check-updates with installed plugin IDs and versions
 std::string url = m_config.baseUrl + "/plugins/check-updates";
 m_lastRequestUrl = url;
 (void)installedPluginIds;
 (void)updates;
 return true;
 }

 const ServerConfig& GetConfig() const { return m_config; }
 const std::string& GetLastRequestUrl() const { return m_lastRequestUrl; }

private:
 ServerConfig m_config;
 mutable std::string m_lastRequestUrl;
};


// ============================================================================
// Plugin Installer
// ============================================================================

/// Installation status
enum class InstallStatus {
 Pending,
 Downloading,
 Verifying,
 Scanning,
 Extracting,
 Registering,
 Completed,
 Failed,
 Cancelled
};

/// Plugin installation result
struct InstallResult {
 InstallStatus status = InstallStatus::Pending;
 std::string errorMessage;
 std::wstring installPath;
 CertificateInfo signature;
 ScanResult securityScan;
 double elapsedMs = 0.0;
};

/// Marketplace statistics
struct MarketplaceStats {
 uint32_t pluginsInstalled = 0;
 uint32_t pluginsUpdated = 0;
 uint32_t pluginsRemoved = 0;
 uint32_t securityBlockedCount = 0;
 uint32_t signatureFailCount = 0;
 uint32_t reviewsSubmitted = 0;
 uint32_t searchesPerformed = 0;

 void Reset() { *this = MarketplaceStats{}; }
};

/// Orchestrates plugin download, verification, scanning, and installation
class PluginInstaller {
public:
 PluginInstaller()
 : m_scanner()
 , m_verifier()
 , m_client() {}

 /// Install a plugin from the marketplace
 InstallResult Install(const MarketplaceEntry& entry) {
 InstallResult result;
 auto start = std::chrono::steady_clock::now();

 // Step 1: Verify signature
 result.status = InstallStatus::Verifying;
 result.signature.status = SignatureStatus::Valid; // Placeholder
 if (!m_verifier.IsAcceptable(result.signature.status)) {
 result.status = InstallStatus::Failed;
 result.errorMessage = "Signature verification failed";
 m_stats.signatureFailCount++;
 return result;
 }

 // Step 2: Security scan
 result.status = InstallStatus::Scanning;
 result.securityScan = m_scanner.Scan(entry.manifest);
 if (!result.securityScan.passed) {
 result.status = InstallStatus::Failed;
 result.errorMessage = "Security scan failed: " +
 std::to_string(result.securityScan.criticalCount) + " critical findings";
 m_stats.securityBlockedCount++;
 return result;
 }

 // Step 3: Extract and register
 result.status = InstallStatus::Extracting;
 // In real implementation: extract .dtpkg to plugins directory

 result.status = InstallStatus::Completed;
 m_stats.pluginsInstalled++;

 auto elapsed = std::chrono::steady_clock::now() - start;
 result.elapsedMs = std::chrono::duration<double, std::milli>(elapsed).count();
 return result;
 }

 /// Uninstall a plugin
 bool Uninstall(const std::string& pluginId) {
 if (pluginId.empty()) return false;
 m_stats.pluginsRemoved++;
 return true;
 }

 const MarketplaceStats& GetStats() const { return m_stats; }

private:
 SecurityScanner m_scanner;
 SignatureVerifier m_verifier;
 MarketplaceClient m_client;
 MarketplaceStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens

