// CrossDeviceCacheSync.cpp — Cloud-Backed Device Cache Synchronisation
// Copyright (c) 2026 ExplorerLens Project
//
#include "CrossDeviceCacheSync.h"
#include <chrono>

namespace ExplorerLens {
namespace Engine {

void CrossDeviceCacheSync::Configure(const std::string& cloudEndpoint,
                                     const std::string& deviceId)
{
    m_cloudEndpoint = cloudEndpoint;
    m_deviceId      = deviceId;
}

SyncOpResult CrossDeviceCacheSync::Sync(DeviceSyncDirection direction,
                                        SyncProgressCallback progress)
{
    if (!IsConfigured()) return SyncOpResult::UPLOAD_FAILED;

    m_lastStats = {};
    const auto start = std::chrono::steady_clock::now();

    if (direction == DeviceSyncDirection::Upload ||
        direction == DeviceSyncDirection::Bidirectional)
    {
        // Stub: simulate 1 uploaded entry
        ++m_lastStats.entriesUploaded;
        m_lastStats.bytesTransferred += 1024;
        if (progress) progress(1, 1);
    }

    if (direction == DeviceSyncDirection::Download ||
        direction == DeviceSyncDirection::Bidirectional)
    {
        // Stub: simulate 1 downloaded entry
        ++m_lastStats.entriesDownloaded;
        m_lastStats.bytesTransferred += 1024;
        if (progress) progress(1, 1);
    }

    const auto end = std::chrono::steady_clock::now();
    m_lastStats.durationMs =
        std::chrono::duration<double, std::milli>(end - start).count();
    ++m_syncCount;
    return SyncOpResult::OK;
}

SyncOpResult CrossDeviceCacheSync::UploadManifest(const std::vector<uint8_t>& blob)
{
    if (blob.empty()) return SyncOpResult::UPLOAD_FAILED;
    m_lastStats.bytesTransferred += blob.size();
    ++m_syncCount;
    return SyncOpResult::OK;
}

std::vector<uint8_t> CrossDeviceCacheSync::DownloadManifest()
{
    // Stub: return an empty manifest placeholder
    return { 0x01 }; // version byte only
}

} // namespace Engine
} // namespace ExplorerLens
