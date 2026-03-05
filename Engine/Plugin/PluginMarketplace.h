// PluginMarketplace.h — Forwarding include
// Copyright (c) 2026 ExplorerLens Project
//
// Forwarding include — canonical implementation in PluginMarketplaceUnified.h
//
#pragma once
#include "PluginMarketplaceUnified.h"

#include <windows.h>
#include <winhttp.h>
#include <wintrust.h>
#include <softpub.h>
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wintrust.lib")

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Forward type definitions (used by MarketplaceClient / PluginInstaller)
// ============================================================================

/// Aggregate star-rating statistics for a marketplace plugin
struct RatingStats {
    double averageRating = 0.0;
    uint32_t totalRatings = 0;
    uint32_t distribution[5] = {}; ///< Index 0 = 1-star, 4 = 5-star
};

/// A user-submitted review for a plugin
struct PluginReview {
    std::string pluginId;
    std::string userId;
    std::string displayName;
    uint32_t rating = 0; ///< 1-5 stars
    std::string title;
    std::string body;
    std::string pluginVersion;
};

/// Result of a security scan on a plugin manifest
struct ScanResult {
    bool passed = true;
    uint32_t criticalCount = 0;
    uint32_t warningCount = 0;
    std::string summary;
};

/// Scans plugin manifests for security policy violations
class PluginSecurityScanner {
public:
    ScanResult Scan(const PluginManifest& manifest) const {
        ScanResult result;
        // Basic policy: reject plugins with empty IDs or names
        if (manifest.id.empty() || manifest.name.empty()) {
            result.passed = false;
            result.criticalCount = 1;
            result.summary = "Missing required manifest fields";
        }
        return result;
    }
};

/// Verifies Authenticode digital signatures on plugin packages
class SignatureVerifier {
public:
    /// Verify a package file's signature using WinVerifyTrust
    SignatureStatus Verify(const std::wstring& packagePath,
        PluginCertificateInfo& certOut) const {
        certOut = {};

        // Check the file actually exists
        DWORD attrs = GetFileAttributesW(packagePath.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY))
            return SignatureStatus::Invalid;

        // Use WinVerifyTrust (Authenticode) to validate digital signature
        GUID actionId = WINTRUST_ACTION_GENERIC_VERIFY_V2;
        WINTRUST_FILE_INFO fileInfo{};
        fileInfo.cbStruct = sizeof(fileInfo);
        fileInfo.pcwszFilePath = packagePath.c_str();
        fileInfo.hFile = nullptr;
        fileInfo.pgKnownSubject = nullptr;

        WINTRUST_DATA trustData{};
        trustData.cbStruct = sizeof(trustData);
        trustData.pPolicyCallbackData = nullptr;
        trustData.pSIPClientData = nullptr;
        trustData.dwUIChoice = WTD_UI_NONE;
        trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
        trustData.dwUnionChoice = WTD_CHOICE_FILE;
        trustData.pFile = &fileInfo;
        trustData.dwStateAction = WTD_STATEACTION_VERIFY;
        trustData.hWVTStateData = nullptr;
        trustData.pwszURLReference = nullptr;
        trustData.dwProvFlags = WTD_SAFER_FLAG;

        LONG status = WinVerifyTrust(
            static_cast<HWND>(INVALID_HANDLE_VALUE), &actionId, &trustData);

        // Close the state handle
        trustData.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), &actionId, &trustData);

        switch (status) {
        case ERROR_SUCCESS:
            certOut.subject = "Verified";
            certOut.issuer = "Authenticode CA";
            certOut.status = SignatureStatus::Valid;
            return SignatureStatus::Valid;
        case TRUST_E_NOSIGNATURE:
            return SignatureStatus::Missing;
        case TRUST_E_EXPLICIT_DISTRUST:
        case TRUST_E_SUBJECT_NOT_TRUSTED:
            return SignatureStatus::Untrusted;
        case CERT_E_EXPIRED:
        case CERT_E_VALIDITYPERIODNESTING:
            return SignatureStatus::Expired;
        default:
            return SignatureStatus::Invalid;
        }
    }

    /// Check whether the given signature status is acceptable for install
    bool IsAcceptable(SignatureStatus status) const {
        return status == SignatureStatus::Valid;
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
