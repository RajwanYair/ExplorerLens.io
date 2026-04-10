// CrossDeviceCacheSync.h — Cloud-Backed Device Cache Synchronisation
// Copyright (c) 2026 ExplorerLens Project
//
// Uploads and downloads DeviceSyncManifest objects via an abstract cloud
// storage provider so thumbnails generated on the desktop appear on other
// devices without re-decoding.
//
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Abstract result of a single cloud sync operation.
enum class SyncOpResult : uint8_t {
    OK,
    UPLOAD_FAILED,
    DOWNLOAD_FAILED,
    NOT_FOUND,
    QUOTA_EXCEEDED,
    CONFLICT
};

// Direction of a cross-device sync pass.
enum class DeviceSyncDirection : uint8_t {
    Upload,
    Download,
    Bidirectional
};

// Metadata returned after a completed sync pass.
struct SyncPassStats {
    size_t entriesUploaded  = 0;
    size_t entriesDownloaded = 0;
    size_t conflicts         = 0;
    size_t bytesTransferred  = 0;
    double durationMs        = 0.0;
};

// Progress callback invoked periodically during a sync pass.
using SyncProgressCallback = std::function<void(size_t done, size_t total)>;

// Syncs the local DeviceSyncManifest with the cloud storage endpoint.
class CrossDeviceCacheSync {
public:
    CrossDeviceCacheSync() = default;

    // Configure the endpoint and local device identifier.
    void Configure(const std::string& cloudEndpoint, const std::string& deviceId);

    // Execute a sync pass in the given direction.
    SyncOpResult Sync(DeviceSyncDirection direction = DeviceSyncDirection::Bidirectional,
                      SyncProgressCallback progress = nullptr);

    // Upload a raw manifest blob (caller serialises).
    SyncOpResult UploadManifest(const std::vector<uint8_t>& blob);

    // Download remote manifest blob; returns empty vector on miss.
    std::vector<uint8_t> DownloadManifest();

    // Stats from the most recent Sync() call.
    const SyncPassStats& LastStats() const { return m_lastStats; }
    size_t SyncCount() const { return m_syncCount; }
    bool IsConfigured() const { return !m_deviceId.empty(); }

private:
    std::string   m_cloudEndpoint;
    std::string   m_deviceId;
    SyncPassStats m_lastStats;
    size_t        m_syncCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
