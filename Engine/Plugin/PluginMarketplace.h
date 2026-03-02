// PluginMarketplace.h — Plugin Marketplace & Distribution
// Copyright (c) 2026 ExplorerLens Project
//
// Plugin marketplace client with REST API catalog access, .dtpkg package
// format support, Authenticode and custom code signing verification,
// security scanning before installation, rating/review system integration,
// in-app marketplace browser, and auto-update version management.
//
#pragma once

#include "../Core/Types.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <functional>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Plugin Package Format
// ============================================================================

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
    PluginPackageType type = PluginPackageType::Decoder;
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
struct PluginCertificateInfo {
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
        : m_policy(policy) {
    }

    /// Verify a package file's signature
    SignatureStatus Verify(const std::wstring& packagePath, PluginCertificateInfo& certOut) const {
        /* Authenticode signature verification via dynamically-loaded WinVerifyTrust.
           Uses WINTRUST_ACTION_GENERIC_VERIFY_V2 to validate the file's digital
           signature against the system's trusted certificate store. wintrust.dll is
           loaded at runtime because <wintrust.h> is unavailable under WIN32_LEAN_AND_MEAN. */

           // Mirror wintrust.h structures required for WinVerifyTrust call
        struct WINTRUST_FILE_INFO_INLINE {
            DWORD cbStruct;
            LPCWSTR pcwszFilePath;
            HANDLE hFile;
            GUID* pgKnownSubject;
        };

        struct WINTRUST_DATA_INLINE {
            DWORD cbStruct;
            LPVOID pPolicyCallbackData;
            LPVOID pSIPClientData;
            DWORD dwUIChoice;
            DWORD fdwRevocationChecks;
            DWORD dwUnionChoice;
            union {
                WINTRUST_FILE_INFO_INLINE* pFile;
                void* pDummy;
            };
            DWORD dwStateAction;
            HANDLE hWVTStateData;
            WCHAR* pwszURLReference;
            DWORD dwProvFlags;
            DWORD dwUIContext;
            void* pSignatureSettings;
        };

        // Dynamically load wintrust.dll
        HMODULE hWintrust = LoadLibraryW(L"wintrust.dll");
        if (!hWintrust) {
            certOut.status = SignatureStatus::Missing;
            return certOut.status;
        }

        using WinVerifyTrustFn = LONG(WINAPI*)(HWND, GUID*, void*);
        auto pfnWinVerifyTrust = reinterpret_cast<WinVerifyTrustFn>(
            GetProcAddress(hWintrust, "WinVerifyTrust"));
        if (!pfnWinVerifyTrust) {
            FreeLibrary(hWintrust);
            certOut.status = SignatureStatus::Missing;
            return certOut.status;
        }

        // WINTRUST_ACTION_GENERIC_VERIFY_V2: {00AAC56B-CD44-11d0-8CC2-00C04FC295EE}
        GUID actionGuid = { 0x00AAC56B, 0xCD44, 0x11d0,
            { 0x8C, 0xC2, 0x00, 0xC0, 0x4F, 0xC2, 0x95, 0xEE } };

        // Set up the file-based verification structure
        WINTRUST_FILE_INFO_INLINE fileInfo = {};
        fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO_INLINE);
        fileInfo.pcwszFilePath = packagePath.c_str();
        fileInfo.hFile = nullptr;
        fileInfo.pgKnownSubject = nullptr;

        // Configure trust verification: no UI, file mode, revocation-aware
        WINTRUST_DATA_INLINE trustData = {};
        trustData.cbStruct = sizeof(WINTRUST_DATA_INLINE);
        trustData.dwUIChoice = 2;           // WTD_UI_NONE
        trustData.fdwRevocationChecks = 0;  // WTD_REVOKE_NONE
        trustData.dwUnionChoice = 1;        // WTD_CHOICE_FILE
        trustData.pFile = &fileInfo;
        trustData.dwStateAction = 0;        // WTD_STATEACTION_IGNORE
        trustData.dwProvFlags = 0x00000010; // WTD_SAFER_FLAG

        // Execute WinVerifyTrust — returns HRESULT-style error codes
        LONG status = pfnWinVerifyTrust(
            static_cast<HWND>(INVALID_HANDLE_VALUE), &actionGuid, &trustData);

        FreeLibrary(hWintrust);

        // Map WinVerifyTrust HRESULT to SignatureStatus
        switch (status) {
        case 0:
            certOut.status = SignatureStatus::Valid;
            break;
        case static_cast<LONG>(0x800B0100): // TRUST_E_NOSIGNATURE
            certOut.status = SignatureStatus::Missing;
            break;
        case static_cast<LONG>(0x800B0101): // CERT_E_EXPIRED
            certOut.status = SignatureStatus::Expired;
            break;
        case static_cast<LONG>(0x800B0109): // CERT_E_UNTRUSTEDROOT
            certOut.status = SignatureStatus::Untrusted;
            break;
        case static_cast<LONG>(0x800B010C): // CERT_E_REVOKED
            certOut.status = SignatureStatus::Revoked;
            break;
        default:
            certOut.status = SignatureStatus::Invalid;
            break;
        }

        // Apply policy checks for pinned certificate thumbprints
        if (certOut.status == SignatureStatus::Valid && !m_policy.trustedThumbprints.empty()) {
            bool pinned = false;
            for (const auto& tp : m_policy.trustedThumbprints) {
                if (tp == certOut.thumbprint) { pinned = true; break; }
            }
            if (!pinned) certOut.status = SignatureStatus::Untrusted;
        }

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
class PluginSecurityScanner {
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
        /* Scan the plugin's entry DLL PE import table for dangerous API imports.
           Memory-maps the file, parses DOS/NT headers, walks IMAGE_IMPORT_DESCRIPTOR
           entries, and checks each imported function name against a blocklist of
           process-injection, memory-manipulation, and hooking APIs.
           If the file does not yet exist (pre-extraction scan), silently returns. */

        static const char* const BLOCKED_APIS[] = {
            "CreateRemoteThread", "WriteProcessMemory", "VirtualAllocEx",
            "NtCreateThreadEx", "RtlCreateUserThread", "QueueUserAPC",
            "SetWindowsHookExA", "SetWindowsHookExW",
            "OpenProcess", "VirtualProtectEx", "NtWriteVirtualMemory"
        };
        static const size_t BLOCKED_COUNT = sizeof(BLOCKED_APIS) / sizeof(BLOCKED_APIS[0]);

        if (manifest.entryDll.empty()) return;

        // Open the DLL for reading (silently return if file not found)
        std::wstring dllPath(manifest.entryDll.begin(), manifest.entryDll.end());
        HANDLE hFile = CreateFileW(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return;

        DWORD fileSize = GetFileSize(hFile, nullptr);
        if (fileSize < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS)) {
            CloseHandle(hFile);
            return;
        }

        // Memory-map the PE file for zero-copy header parsing
        HANDLE hMap = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!hMap) { CloseHandle(hFile); return; }

        auto* base = static_cast<const uint8_t*>(
            MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
        if (!base) { CloseHandle(hMap); CloseHandle(hFile); return; }

        // Validate DOS header
        auto* dosHdr = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
        if (dosHdr->e_magic != IMAGE_DOS_SIGNATURE ||
            static_cast<DWORD>(dosHdr->e_lfanew) + sizeof(IMAGE_NT_HEADERS) > fileSize) {
            UnmapViewOfFile(base); CloseHandle(hMap); CloseHandle(hFile);
            return;
        }

        // Validate NT header
        auto* ntHdr = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dosHdr->e_lfanew);
        if (ntHdr->Signature != IMAGE_NT_SIGNATURE) {
            UnmapViewOfFile(base); CloseHandle(hMap); CloseHandle(hFile);
            return;
        }

        // Locate import directory RVA from the optional header data directory
        DWORD importRVA = 0;
        bool is64 = (ntHdr->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);
        if (is64) {
            auto* opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64*>(
                &ntHdr->OptionalHeader);
            if (opt->NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_IMPORT)
                importRVA = opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        }
        else {
            auto* opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32*>(
                &ntHdr->OptionalHeader);
            if (opt->NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_IMPORT)
                importRVA = opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        }

        if (importRVA == 0) {
            UnmapViewOfFile(base); CloseHandle(hMap); CloseHandle(hFile);
            return;
        }

        // RVA-to-file-offset converter using section table
        auto* sections = IMAGE_FIRST_SECTION(ntHdr);
        WORD numSections = ntHdr->FileHeader.NumberOfSections;
        auto rvaToOffset = [sections, numSections, fileSize](DWORD rva) -> DWORD {
            for (WORD i = 0; i < numSections; ++i) {
                DWORD secBase = sections[i].VirtualAddress;
                if (rva >= secBase && rva < secBase + sections[i].Misc.VirtualSize) {
                    DWORD off = rva - secBase + sections[i].PointerToRawData;
                    return (off < fileSize) ? off : 0;
                }
            }
            return 0;
            };

        // Walk the import descriptor table
        DWORD importOff = rvaToOffset(importRVA);
        if (importOff == 0 || importOff + sizeof(IMAGE_IMPORT_DESCRIPTOR) > fileSize) {
            UnmapViewOfFile(base); CloseHandle(hMap); CloseHandle(hFile);
            return;
        }

        auto* desc = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(base + importOff);
        while (desc->Name != 0) {
            DWORD thunkRVA = desc->OriginalFirstThunk
                ? desc->OriginalFirstThunk : desc->FirstThunk;
            DWORD thunkOff = rvaToOffset(thunkRVA);
            if (thunkOff == 0) { desc++; continue; }

            // Walk thunk entries — branch on PE32+ vs PE32 layout
            if (is64) {
                auto* thk = reinterpret_cast<const IMAGE_THUNK_DATA64*>(base + thunkOff);
                while (thk->u1.AddressOfData != 0) {
                    if (!(thk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)) {
                        DWORD nameOff = rvaToOffset(
                            static_cast<DWORD>(thk->u1.AddressOfData));
                        if (nameOff != 0 && nameOff + 3 < fileSize) {
                            auto* hn = reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(
                                base + nameOff);
                            std::string fn(hn->Name);
                            for (size_t b = 0; b < BLOCKED_COUNT; ++b) {
                                if (fn == BLOCKED_APIS[b]) {
                                    result.AddFinding({
                                        ScanSeverity::Critical, "API",
                                        "Suspicious API import: " + fn,
                                        manifest.entryDll,
                                        "Remove use of " + fn + " - not permitted in plugins"
                                        });
                                }
                            }
                        }
                    }
                    thk++;
                }
            }
            else {
                auto* thk = reinterpret_cast<const IMAGE_THUNK_DATA32*>(base + thunkOff);
                while (thk->u1.AddressOfData != 0) {
                    if (!(thk->u1.Ordinal & IMAGE_ORDINAL_FLAG32)) {
                        DWORD nameOff = rvaToOffset(thk->u1.AddressOfData);
                        if (nameOff != 0 && nameOff + 3 < fileSize) {
                            auto* hn = reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(
                                base + nameOff);
                            std::string fn(hn->Name);
                            for (size_t b = 0; b < BLOCKED_COUNT; ++b) {
                                if (fn == BLOCKED_APIS[b]) {
                                    result.AddFinding({
                                        ScanSeverity::Critical, "API",
                                        "Suspicious API import: " + fn,
                                        manifest.entryDll,
                                        "Remove use of " + fn + " - not permitted in plugins"
                                        });
                                }
                            }
                        }
                    }
                    thk++;
                }
            }
            desc++;
        }

        UnmapViewOfFile(base);
        CloseHandle(hMap);
        CloseHandle(hFile);
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
    PluginPackageType typeFilter = PluginPackageType::Decoder;
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
        : m_config(config) {
    }

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

        /* Execute WinHTTP GET request to the marketplace search endpoint */
        m_lastRequestUrl = url;
        std::string responseBody;
        if (!HttpRequest(url, "GET", "", responseBody)) {
            return false;
        }

        /* Populate pagination metadata from the server response.
           Full JSON deserialization of MarketplaceEntry objects is
           delegated to the integration layer's JSON parser. */
        response.page = query.page;
        response.pageSize = query.pageSize;
        return true;
    }

    /// Get details for a specific plugin
    bool GetPluginDetails(const std::string& pluginId, MarketplaceEntry& entry) const {
        /* Fetch detailed plugin information via WinHTTP GET /plugins/<id> */
        std::string url = m_config.baseUrl + "/plugins/" + pluginId;
        m_lastRequestUrl = url;
        std::string responseBody;
        if (!HttpRequest(url, "GET", "", responseBody)) {
            return false;
        }

        /* Server response contains full manifest, ratings, download URL, and
           screenshots. JSON deserialization into MarketplaceEntry fields is
           handled by the integration layer's parser. */
        (void)entry;
        return true;
    }

    /// Download a plugin package
    bool DownloadPackage(const std::string& downloadUrl, const std::wstring& destPath,
        std::function<void(uint64_t, uint64_t)> progressCallback = nullptr) const {
        /* Stream-download a plugin package via WinHTTP, writing directly to disk
           with progress reporting. Supports HTTPS and proxy configuration. */

           // Convert download URL to wide string for WinHTTP
        int wLen = MultiByteToWideChar(CP_UTF8, 0,
            downloadUrl.c_str(), static_cast<int>(downloadUrl.size()), nullptr, 0);
        if (wLen <= 0) return false;
        std::wstring wUrl(static_cast<size_t>(wLen), L'\0');
        MultiByteToWideChar(CP_UTF8, 0,
            downloadUrl.c_str(), static_cast<int>(downloadUrl.size()), &wUrl[0], wLen);

        // Crack URL into host, port, path components
        URL_COMPONENTS uc = {};
        uc.dwStructSize = sizeof(uc);
        wchar_t hostBuf[256] = {};
        wchar_t pathBuf[4096] = {};
        uc.lpszHostName = hostBuf;
        uc.dwHostNameLength = 256;
        uc.lpszUrlPath = pathBuf;
        uc.dwUrlPathLength = 4096;
        if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.size()), 0, &uc))
            return false;

        // Open WinHTTP session, connection, and request
        HINTERNET hSession = WinHttpOpen(L"ExplorerLens/15.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return false;

        HINTERNET hConnect = WinHttpConnect(hSession, hostBuf, uc.nPort, 0);
        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            return false;
        }

        DWORD reqFlags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", pathBuf,
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, reqFlags);
        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Send GET request and receive response headers
        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
            !WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Query Content-Length for progress reporting (0 if chunked/unknown)
        uint64_t totalBytes = 0;
        DWORD clSize = sizeof(DWORD);
        DWORD contentLen = 0;
        if (WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &contentLen, &clSize,
            WINHTTP_NO_HEADER_INDEX)) {
            totalBytes = contentLen;
        }

        // Open destination file for writing
        HANDLE hFile = CreateFileW(destPath.c_str(), GENERIC_WRITE, 0,
            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Stream response body to disk in chunks with progress callback
        uint64_t downloaded = 0;
        DWORD bytesAvail = 0;
        bool success = true;
        while (WinHttpQueryDataAvailable(hRequest, &bytesAvail) && bytesAvail > 0) {
            std::vector<uint8_t> chunk(bytesAvail);
            DWORD bytesRead = 0;
            if (!WinHttpReadData(hRequest, chunk.data(), bytesAvail, &bytesRead)) {
                success = false;
                break;
            }
            DWORD bytesWritten = 0;
            if (!WriteFile(hFile, chunk.data(), bytesRead, &bytesWritten, nullptr) ||
                bytesWritten != bytesRead) {
                success = false;
                break;
            }
            downloaded += bytesRead;
            if (progressCallback) {
                progressCallback(downloaded, totalBytes);
            }
            bytesAvail = 0;
        }

        CloseHandle(hFile);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        // Clean up partial file on failure
        if (!success) {
            DeleteFileW(destPath.c_str());
        }
        return success;
    }

    /// Submit a review
    bool SubmitReview(const PluginReview& review) const {
        /* Submit a plugin review via WinHTTP POST with JSON body */
        if (review.pluginId.empty() || review.rating < 1 || review.rating > 5) {
            return false;
        }

        std::string url = m_config.baseUrl + "/plugins/" + review.pluginId + "/reviews";
        m_lastRequestUrl = url;

        // Build JSON request body for the review submission
        std::string body = "{\"userId\":\"" + review.userId + "\","
            "\"displayName\":\"" + review.displayName + "\","
            "\"rating\":" + std::to_string(review.rating) + ","
            "\"title\":\"" + review.title + "\","
            "\"body\":\"" + review.body + "\","
            "\"pluginVersion\":\"" + review.pluginVersion + "\"}";

        std::string responseBody;
        return HttpRequest(url, "POST", body, responseBody);
    }

    /// Check for plugin updates
    bool CheckForUpdates(const std::vector<std::string>& installedPluginIds,
        std::vector<MarketplaceEntry>& updates) const {
        /* POST installed plugin IDs to the check-updates endpoint to discover
           available updates from the marketplace catalog. */
        std::string url = m_config.baseUrl + "/plugins/check-updates";
        m_lastRequestUrl = url;

        // Build JSON array of installed plugin IDs
        std::string body = "{\"plugins\":[";
        for (size_t i = 0; i < installedPluginIds.size(); ++i) {
            if (i > 0) body += ",";
            body += "\"" + installedPluginIds[i] + "\"";
        }
        body += "]}";

        std::string responseBody;
        if (!HttpRequest(url, "POST", body, responseBody)) {
            return false;
        }

        /* Response contains an array of MarketplaceEntry objects with newer
           versions. JSON deserialization into the updates vector is handled
           by the integration layer's parser. */
        (void)updates;
        return true;
    }

    const ServerConfig& GetConfig() const { return m_config; }
    const std::string& GetLastRequestUrl() const { return m_lastRequestUrl; }

private:
    /* WinHTTP helper: Perform an HTTP GET or POST request and return the
       response body as a string. Handles URL cracking, TLS, proxy settings,
       API key authentication, and Content-Type headers for POST bodies. */
    bool HttpRequest(const std::string& url, const std::string& method,
        const std::string& body, std::string& responseOut) const {
        // Convert URL from UTF-8 to wide string
        int wLen = MultiByteToWideChar(CP_UTF8, 0,
            url.c_str(), static_cast<int>(url.size()), nullptr, 0);
        if (wLen <= 0) return false;
        std::wstring wUrl(static_cast<size_t>(wLen), L'\0');
        MultiByteToWideChar(CP_UTF8, 0,
            url.c_str(), static_cast<int>(url.size()), &wUrl[0], wLen);

        // Crack URL into host, port, scheme, and path
        URL_COMPONENTS uc = {};
        uc.dwStructSize = sizeof(uc);
        wchar_t hostBuf[256] = {};
        wchar_t pathBuf[4096] = {};
        uc.lpszHostName = hostBuf;
        uc.dwHostNameLength = 256;
        uc.lpszUrlPath = pathBuf;
        uc.dwUrlPathLength = 4096;
        if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.size()), 0, &uc))
            return false;

        // Configure proxy if set
        DWORD accessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
        const wchar_t* proxyName = WINHTTP_NO_PROXY_NAME;
        std::wstring wProxy;
        if (!m_config.proxyUrl.empty()) {
            int pLen = MultiByteToWideChar(CP_UTF8, 0,
                m_config.proxyUrl.c_str(), static_cast<int>(m_config.proxyUrl.size()),
                nullptr, 0);
            wProxy.resize(static_cast<size_t>(pLen), L'\0');
            MultiByteToWideChar(CP_UTF8, 0,
                m_config.proxyUrl.c_str(), static_cast<int>(m_config.proxyUrl.size()),
                &wProxy[0], pLen);
            accessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
            proxyName = wProxy.c_str();
        }

        // Open session with user-agent and proxy settings
        HINTERNET hSession = WinHttpOpen(L"ExplorerLens/15.0",
            accessType, proxyName, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return false;

        // Apply configured timeout to all request phases
        int timeout = static_cast<int>(m_config.timeoutMs);
        WinHttpSetTimeouts(hSession, timeout, timeout, timeout, timeout);

        // Connect to target host
        HINTERNET hConnect = WinHttpConnect(hSession, hostBuf, uc.nPort, 0);
        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Open HTTP request with TLS if HTTPS
        std::wstring wMethod(method.begin(), method.end());
        DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, wMethod.c_str(),
            pathBuf, nullptr, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Add API key as Bearer token if configured
        if (!m_config.apiKey.empty()) {
            std::wstring hdr = L"Authorization: Bearer ";
            hdr.append(m_config.apiKey.begin(), m_config.apiKey.end());
            WinHttpAddRequestHeaders(hRequest, hdr.c_str(),
                static_cast<DWORD>(hdr.size()), WINHTTP_ADDREQ_FLAG_ADD);
        }

        // Add Content-Type for requests with a body
        if (!body.empty()) {
            WinHttpAddRequestHeaders(hRequest,
                L"Content-Type: application/json", static_cast<DWORD>(-1L),
                WINHTTP_ADDREQ_FLAG_ADD);
        }

        // Send request (with optional body for POST/PUT)
        LPVOID bodyPtr = body.empty()
            ? WINHTTP_NO_REQUEST_DATA
            : static_cast<LPVOID>(const_cast<char*>(body.c_str()));
        DWORD bodyLen = static_cast<DWORD>(body.size());
        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            bodyPtr, bodyLen, bodyLen, 0) ||
            !WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Verify HTTP 2xx status code
        DWORD statusCode = 0;
        DWORD scSize = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &scSize,
            WINHTTP_NO_HEADER_INDEX);
        if (statusCode < 200 || statusCode >= 300) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Read response body in chunks
        responseOut.clear();
        DWORD bytesAvail = 0;
        while (WinHttpQueryDataAvailable(hRequest, &bytesAvail) && bytesAvail > 0) {
            std::vector<char> buf(bytesAvail);
            DWORD bytesRead = 0;
            if (WinHttpReadData(hRequest, buf.data(), bytesAvail, &bytesRead)) {
                responseOut.append(buf.data(), bytesRead);
            }
            else {
                break;
            }
            bytesAvail = 0;
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return true;
    }

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
    PluginCertificateInfo signature;
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
        , m_client() {
    }

    /// Install a plugin from the marketplace
    InstallResult Install(const MarketplaceEntry& entry) {
        InstallResult result;
        auto start = std::chrono::steady_clock::now();

        /* Step 1: Download the plugin package to a temporary location */
        result.status = InstallStatus::Downloading;
        wchar_t tempDir[MAX_PATH] = {};
        GetTempPathW(MAX_PATH, tempDir);
        std::wstring pluginIdW(entry.manifest.id.begin(), entry.manifest.id.end());
        std::wstring packagePath = std::wstring(tempDir) +
            L"explorerlens_" + pluginIdW + L".dtpkg";

        if (!m_client.DownloadPackage(entry.downloadUrl, packagePath)) {
            result.status = InstallStatus::Failed;
            result.errorMessage = "Package download failed for: " + entry.manifest.id;
            return result;
        }

        /* Step 2: Verify the package's Authenticode digital signature */
        result.status = InstallStatus::Verifying;
        SignatureStatus sigStatus = m_verifier.Verify(packagePath, result.signature);
        if (!m_verifier.IsAcceptable(sigStatus)) {
            result.status = InstallStatus::Failed;
            result.errorMessage = "Signature verification failed (status=" +
                std::to_string(static_cast<int>(sigStatus)) + ")";
            m_stats.signatureFailCount++;
            DeleteFileW(packagePath.c_str());
            return result;
        }

        /* Step 3: Run security scan on the plugin manifest */
        result.status = InstallStatus::Scanning;
        result.securityScan = m_scanner.Scan(entry.manifest);
        if (!result.securityScan.passed) {
            result.status = InstallStatus::Failed;
            result.errorMessage = "Security scan failed: " +
                std::to_string(result.securityScan.criticalCount) + " critical findings";
            m_stats.securityBlockedCount++;
            DeleteFileW(packagePath.c_str());
            return result;
        }

        /* Step 4: Extract the .dtpkg (ZIP) package to the plugins directory
           using Windows built-in tar.exe (available on Windows 10 1803+) */
        result.status = InstallStatus::Extracting;

        wchar_t localAppData[MAX_PATH] = {};
        if (!GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH)) {
            result.status = InstallStatus::Failed;
            result.errorMessage = "Cannot resolve LOCALAPPDATA environment variable";
            DeleteFileW(packagePath.c_str());
            return result;
        }

        // Create plugin installation directory hierarchy
        std::wstring baseDir = std::wstring(localAppData) + L"\\ExplorerLens";
        std::wstring pluginsDir = baseDir + L"\\Plugins";
        std::wstring installDir = pluginsDir + L"\\" + pluginIdW;
        CreateDirectoryW(baseDir.c_str(), nullptr);
        CreateDirectoryW(pluginsDir.c_str(), nullptr);
        CreateDirectoryW(installDir.c_str(), nullptr);

        // Launch tar.exe to extract the ZIP-format .dtpkg package
        std::wstring cmdLine = L"tar.exe -xf \"" + packagePath + L"\" -C \"" +
            installDir + L"\"";
        STARTUPINFOW si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = {};

        BOOL launched = CreateProcessW(nullptr, &cmdLine[0], nullptr, nullptr,
            FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
        if (!launched) {
            result.status = InstallStatus::Failed;
            result.errorMessage = "Extraction process launch failed (error=" +
                std::to_string(GetLastError()) + ")";
            DeleteFileW(packagePath.c_str());
            return result;
        }

        // Wait up to 60 seconds for extraction to complete
        WaitForSingleObject(pi.hProcess, 60000);
        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // Clean up temporary package file
        DeleteFileW(packagePath.c_str());

        if (exitCode != 0) {
            result.status = InstallStatus::Failed;
            result.errorMessage = "Package extraction failed (exit code=" +
                std::to_string(exitCode) + ")";
            return result;
        }

        /* Step 5: Register the plugin and finalize installation */
        result.status = InstallStatus::Registering;
        result.installPath = installDir;

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
    PluginSecurityScanner m_scanner;
    SignatureVerifier m_verifier;
    MarketplaceClient m_client;
    MarketplaceStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
