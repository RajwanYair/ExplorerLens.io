// =============================================================================
// ExplorerLens Engine — Cloud Thumbnail Provider
// Cloud Integration & Sync
// Copyright (c) 2026 ExplorerLens Project
//
// Provides cloud-aware thumbnail resolution for files stored in
// OneDrive, Google Drive, and Dropbox via their respective APIs.
// Falls back to local decode when cloud previews are unavailable.
// =============================================================================

#pragma once

#include <windows.h>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <cstdint>

namespace ExplorerLens {
namespace Cloud {

// ============================================================================
// Enums
// ============================================================================

enum class CloudProvider {
    None = 0,
    OneDrive,
    GoogleDrive,
    Dropbox,
    SharePoint,
    iCloudDrive
};

enum class SyncState {
    Unknown = 0,
    Local, // Fully downloaded, file on disk
    CloudOnly, // Placeholder, not hydrated
    Syncing, // Currently downloading
    PinnedLocal, // Always-on-device
    Error // Sync error
};

enum class AuthStatus {
    NotAuthenticated = 0,
    Authenticated,
    TokenExpired,
    AuthenticationFailed
};

// ============================================================================
// Cloud File Metadata — returned by cloud API queries
// ============================================================================

struct CloudFileInfo {
    std::wstring fileId; // Provider-specific file ID
    std::wstring displayName;
    std::wstring remotePath;
    std::wstring mimeType;
    uint64_t sizeBytes = 0;
    uint64_t lastModifiedUtc = 0; // Unix epoch seconds
    std::wstring thumbnailUrl; // Provider preview URL (if available)
    uint32_t thumbnailWidth = 0;
    uint32_t thumbnailHeight = 0;
    SyncState syncState = SyncState::Unknown;
    CloudProvider provider = CloudProvider::None;
    bool hasCloudPreview = false; // True if provider can serve a thumbnail
};

// ============================================================================
// OAuth 2.0 Token — reusable across providers
// ============================================================================

struct OAuthToken {
    std::wstring accessToken;
    std::wstring refreshToken;
    std::wstring tokenType; // "Bearer"
    std::chrono::system_clock::time_point expiresAt;

    bool IsExpired() const {
        return std::chrono::system_clock::now() >= expiresAt;
    }

    bool IsValid() const {
        return !accessToken.empty() && !IsExpired();
    }
};

// ============================================================================
// ICloudProvider — abstract interface for cloud services
// ============================================================================

class ICloudProvider {
public:
    virtual ~ICloudProvider() = default;

    /// Human-readable provider name (e.g., "OneDrive", "Google Drive")
    virtual const wchar_t* GetName() const = 0;

    /// Provider enum
    virtual CloudProvider GetType() const = 0;

    /// Returns true if this provider handles the given local path
    /// (e.g., OneDrive detects paths under ~/OneDrive)
    virtual bool HandlesPath(const wchar_t* localPath) const = 0;

    // --- Authentication ---

    /// Check current token status
    virtual AuthStatus GetAuthStatus() const = 0;

    /// Start OAuth 2.0 authorization code flow (opens browser)
    virtual HRESULT BeginAuthentication() = 0;

    /// Complete OAuth flow with authorization code
    virtual HRESULT CompleteAuthentication(const wchar_t* authorizationCode) = 0;

    /// Refresh access token using stored refresh token
    virtual HRESULT RefreshToken() = 0;

    /// Sign out and clear stored credentials
    virtual HRESULT SignOut() = 0;

    // --- Cloud File Operations ---

    /// Resolve local file path to cloud file info
    virtual HRESULT GetCloudFileInfo(const wchar_t* localPath, CloudFileInfo& outInfo) = 0;

    /// Download provider-generated thumbnail for a cloud file
    /// Returns S_OK with bitmap data if provider has preview, S_FALSE otherwise
    virtual HRESULT DownloadCloudThumbnail(const CloudFileInfo& fileInfo,
        uint32_t desiredWidth,
        uint32_t desiredHeight,
        std::vector<uint8_t>& outImageData) = 0;

    /// Check if the file has been modified since a given time
    virtual HRESULT HasBeenModified(const wchar_t* localPath,
        uint64_t sinceUtcEpoch,
        bool& outModified) = 0;
};

// ============================================================================
// OneDrive Provider — Microsoft Graph API integration
// ============================================================================

class OneDriveProvider : public ICloudProvider {
public:
    OneDriveProvider();
    ~OneDriveProvider() override;

    const wchar_t* GetName() const override { return L"OneDrive"; }
    CloudProvider GetType() const override { return CloudProvider::OneDrive; }

    bool HandlesPath(const wchar_t* localPath) const override;

    AuthStatus GetAuthStatus() const override;
    HRESULT BeginAuthentication() override;
    HRESULT CompleteAuthentication(const wchar_t* authorizationCode) override;
    HRESULT RefreshToken() override;
    HRESULT SignOut() override;

    HRESULT GetCloudFileInfo(const wchar_t* localPath, CloudFileInfo& outInfo) override;
    HRESULT DownloadCloudThumbnail(const CloudFileInfo& fileInfo,
        uint32_t desiredWidth, uint32_t desiredHeight,
        std::vector<uint8_t>& outImageData) override;
    HRESULT HasBeenModified(const wchar_t* localPath, uint64_t sinceUtcEpoch,
        bool& outModified) override;

    // OneDrive-specific: detect sync root from NTFS reparse points
    static bool DetectOneDriveRoot(std::wstring& outRoot);

    // Microsoft Graph API endpoints
    static constexpr const wchar_t* kGraphApiBase = L"https://graph.microsoft.com/v1.0";
    static constexpr const wchar_t* kAuthEndpoint =
        L"https://login.microsoftonline.com/common/oauth2/v2.0/authorize";
    static constexpr const wchar_t* kTokenEndpoint =
        L"https://login.microsoftonline.com/common/oauth2/v2.0/token";

private:
    OAuthToken m_token;
    std::wstring m_oneDriveRoot;
    mutable std::mutex m_mutex;
};

// ============================================================================
// Google Drive Provider
// ============================================================================

class GoogleDriveProvider : public ICloudProvider {
public:
    GoogleDriveProvider();
    ~GoogleDriveProvider() override;

    const wchar_t* GetName() const override { return L"Google Drive"; }
    CloudProvider GetType() const override { return CloudProvider::GoogleDrive; }

    bool HandlesPath(const wchar_t* localPath) const override;

    AuthStatus GetAuthStatus() const override;
    HRESULT BeginAuthentication() override;
    HRESULT CompleteAuthentication(const wchar_t* authorizationCode) override;
    HRESULT RefreshToken() override;
    HRESULT SignOut() override;

    HRESULT GetCloudFileInfo(const wchar_t* localPath, CloudFileInfo& outInfo) override;
    HRESULT DownloadCloudThumbnail(const CloudFileInfo& fileInfo,
        uint32_t desiredWidth, uint32_t desiredHeight,
        std::vector<uint8_t>& outImageData) override;
    HRESULT HasBeenModified(const wchar_t* localPath, uint64_t sinceUtcEpoch,
        bool& outModified) override;

    static constexpr const wchar_t* kDriveApiBase = L"https://www.googleapis.com/drive/v3";
    static constexpr const wchar_t* kAuthEndpoint =
        L"https://accounts.google.com/o/oauth2/v2/auth";

private:
    OAuthToken m_token;
    std::wstring m_driveRoot;
    mutable std::mutex m_mutex;
};

// ============================================================================
// Dropbox Provider
// ============================================================================

class DropboxProvider : public ICloudProvider {
public:
    DropboxProvider();
    ~DropboxProvider() override;

    const wchar_t* GetName() const override { return L"Dropbox"; }
    CloudProvider GetType() const override { return CloudProvider::Dropbox; }

    bool HandlesPath(const wchar_t* localPath) const override;

    AuthStatus GetAuthStatus() const override;
    HRESULT BeginAuthentication() override;
    HRESULT CompleteAuthentication(const wchar_t* authorizationCode) override;
    HRESULT RefreshToken() override;
    HRESULT SignOut() override;

    HRESULT GetCloudFileInfo(const wchar_t* localPath, CloudFileInfo& outInfo) override;
    HRESULT DownloadCloudThumbnail(const CloudFileInfo& fileInfo,
        uint32_t desiredWidth, uint32_t desiredHeight,
        std::vector<uint8_t>& outImageData) override;
    HRESULT HasBeenModified(const wchar_t* localPath, uint64_t sinceUtcEpoch,
        bool& outModified) override;

    static constexpr const wchar_t* kApiBase = L"https://api.dropboxapi.com/2";

private:
    OAuthToken m_token;
    std::wstring m_dropboxRoot;
    mutable std::mutex m_mutex;
};

// ============================================================================
// CloudThumbnailResolver — orchestrates cloud providers
// ============================================================================

class CloudThumbnailResolver {
public:
    CloudThumbnailResolver();
    ~CloudThumbnailResolver();

    /// Register a cloud provider
    void RegisterProvider(std::unique_ptr<ICloudProvider> provider);

    /// Detect which cloud provider (if any) handles a local path
    ICloudProvider* FindProvider(const wchar_t* localPath) const;

    /// Resolve cloud thumbnail: returns cloud-generated preview bitmap data
    /// Falls back gracefully: cloud preview → local decode (caller handles)
    HRESULT ResolveThumbnail(const wchar_t* localPath,
        uint32_t width, uint32_t height,
        std::vector<uint8_t>& outImageData,
        CloudFileInfo& outFileInfo);

    /// Check if local file has been modified on the cloud since last cache
    HRESULT CheckCloudModification(const wchar_t* localPath,
        uint64_t cachedTimestamp,
        bool& outNeedsRefresh);

    /// Get cloud sync state for a file (placeholder, synced, etc.)
    SyncState GetSyncState(const wchar_t* localPath) const;

    /// Enable/disable cloud preview usage
    void SetCloudPreviewEnabled(bool enabled) { m_cloudPreviewEnabled = enabled; }
    bool IsCloudPreviewEnabled() const { return m_cloudPreviewEnabled; }

    /// Stats
    struct CloudStats {
        uint64_t cloudHits = 0; // Times cloud preview was used
        uint64_t cloudMisses = 0; // Times fell back to local decode
        uint64_t networkErrors = 0; // Times cloud API failed
        uint64_t bandwidthSavedBytes = 0; // Bytes not downloaded due to cloud preview
    };
    CloudStats GetStats() const;

private:
    std::vector<std::unique_ptr<ICloudProvider>> m_providers;
    bool m_cloudPreviewEnabled = true;
    mutable std::mutex m_mutex;

    // File identity cache to avoid repeated cloud queries
    struct CachedCloudInfo {
        CloudFileInfo info;
        std::chrono::system_clock::time_point fetchTime;
    };
    std::unordered_map<std::wstring, CachedCloudInfo> m_cloudInfoCache;
    static constexpr auto kCloudInfoCacheTTL = std::chrono::minutes(5);

    // Stats
    mutable CloudStats m_stats;
};

// ============================================================================
// Helper: Detect cloud sync state via NTFS reparse points / file attributes
// ============================================================================

/// Check if a file is a cloud placeholder (FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS)
bool IsCloudPlaceholder(const wchar_t* filePath);

/// Check if a file is pinned for offline access
bool IsCloudPinned(const wchar_t* filePath);

/// Get the NTFS reparse tag for cloud file identification
DWORD GetCloudReparseTag(const wchar_t* filePath);

// Known reparse tags (guarded — may already be defined by Windows SDK)
#ifndef IO_REPARSE_TAG_CLOUD
constexpr DWORD IO_REPARSE_TAG_CLOUD = 0x9000001A;
#endif
#ifndef IO_REPARSE_TAG_CLOUD_1
constexpr DWORD IO_REPARSE_TAG_CLOUD_1 = 0x9000101A;
#endif
#ifndef IO_REPARSE_TAG_CLOUD_2
constexpr DWORD IO_REPARSE_TAG_CLOUD_2 = 0x9000201A;
#endif
#ifndef IO_REPARSE_TAG_ONEDRIVE
constexpr DWORD IO_REPARSE_TAG_ONEDRIVE = 0x9000301A;
#endif

} // namespace Cloud
} // namespace ExplorerLens
