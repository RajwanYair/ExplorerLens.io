// CloudStorageAdapter.h — Abstract Cloud File Accessor
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a provider-agnostic interface for reading file bytes from cloud
// storage backends (OneDrive, SharePoint, Azure Blob, S3-compatible APIs).
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Provider Identity ------------------------------------------------------

enum class CloudProvider : uint8_t {
    Unknown           = 0,
    OneDriveConsumer  = 1,   // Personal OneDrive via CFAPI
    OneDriveBusiness  = 2,   // OneDrive for Business / SharePoint Online
    SharePointServer  = 3,   // On-premises SharePoint (2016/2019/Subscription)
    AzureBlobStorage  = 4,   // Azure Blob via SAS URL
    S3Compatible      = 5,   // AWS S3 / MinIO / Backblaze B2
    GoogleDrive       = 6,
    Dropbox           = 7,
};

// ---- File Access ------------------------------------------------------------

struct CloudFileInfo {
    std::string      remotePath;      // Provider-specific path/URL
    std::string      localCachePath;  // Windows CfApi local placeholder path
    CloudProvider    provider        = CloudProvider::Unknown;
    bool             isPlaceholder   = false;  // Not yet hydrated
    bool             isLocal         = false;  // Fully available offline
    bool             isShared        = false;  // Shared with others
    uint64_t         sizeBytes       = 0;
    std::string      etag;
};

using CloudProgressCallback = std::function<void(uint64_t bytesReceived, uint64_t total)>;

struct CloudReadResult {
    bool                 success = false;
    std::vector<uint8_t> data;
    std::string          error;
    CloudProvider        provider = CloudProvider::Unknown;
};

// ---- Adapter Interface ------------------------------------------------------

class ICloudStorageAdapter {
public:
    virtual ~ICloudStorageAdapter() = default;

    virtual CloudProvider ProviderType() const = 0;

    // Read up to maxBytes starting at offset (range request).
    virtual CloudReadResult ReadRange(
        const CloudFileInfo&      fileInfo,
        uint64_t                  offset,
        uint64_t                  maxBytes,
        CloudProgressCallback     progress = nullptr) = 0;

    // Read the entire file (small files only; use ReadRange for large ones).
    virtual CloudReadResult ReadAll(
        const CloudFileInfo&      fileInfo,
        CloudProgressCallback     progress = nullptr) = 0;

    // Get file metadata without downloading content.
    virtual bool GetInfo(
        const std::string&        remotePath,
        CloudFileInfo&            outInfo) = 0;

    // Returns true if the adapter is authenticated and available.
    virtual bool IsAvailable() const = 0;
};

// ---- Factory ----------------------------------------------------------------

class CloudStorageAdapterFactory {
public:
    static std::unique_ptr<ICloudStorageAdapter> Create(CloudProvider provider);

    // Auto-detect provider from a local placeholder path (CfApi reparse tag).
    static CloudProvider DetectProvider(const std::string& localPath);
};

} // namespace Engine
} // namespace ExplorerLens
