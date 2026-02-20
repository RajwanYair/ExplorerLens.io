//==============================================================================
// CloudSyncProvider — Sprint 221
//==============================================================================

#include "CloudSyncProvider.h"
#include <chrono>
#include <algorithm>

namespace DarkThumbs { namespace Engine {

CloudSyncProvider::CloudSyncProvider() {}

CloudSyncResult CloudSyncProvider::SyncThumbnails(
    CloudProvider provider, const std::wstring& path)
{
    CloudSyncResult result;
    auto start = std::chrono::high_resolution_clock::now();

    // Validate path
    if (path.empty()) {
        result.status = SyncStatus::Error;
        result.errorMessage = L"Empty path";
        return result;
    }

    result.status = SyncStatus::Syncing;

    // In production: enumerate cloud files and generate thumbnails
    result.filesProcessed = 0;
    result.thumbnailsCached = 0;
    result.status = SyncStatus::Completed;
    result.success = true;

    auto end = std::chrono::high_resolution_clock::now();
    result.syncTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

CloudFileInfo CloudSyncProvider::GetFileInfo(const std::wstring& path) const {
    CloudFileInfo info;
    info.remotePath = path;
    info.provider = DetectProvider(path);
    return info;
}

bool CloudSyncProvider::IsCloudPath(const std::wstring& path) const {
    // Check for known cloud sync folders
    auto lower = path;
    for (auto& c : lower) c = towlower(c);

    return lower.find(L"onedrive") != std::wstring::npos ||
           lower.find(L"sharepoint") != std::wstring::npos ||
           lower.find(L"dropbox") != std::wstring::npos ||
           lower.find(L"google drive") != std::wstring::npos;
}

bool CloudSyncProvider::IsPlaceholderFile(const std::wstring& /*path*/) const {
    // In production: check FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
    return false;
}

CloudProvider CloudSyncProvider::DetectProvider(const std::wstring& path) {
    auto lower = path;
    for (auto& c : lower) c = towlower(c);

    if (lower.find(L"onedrive") != std::wstring::npos) return CloudProvider::OneDrive;
    if (lower.find(L"sharepoint") != std::wstring::npos) return CloudProvider::SharePoint;
    if (lower.find(L"s3://") != std::wstring::npos) return CloudProvider::AmazonS3;
    if (lower.find(L"blob.core") != std::wstring::npos) return CloudProvider::AzureBlob;
    if (lower.find(L"google drive") != std::wstring::npos) return CloudProvider::GoogleDrive;
    if (lower.find(L"dropbox") != std::wstring::npos) return CloudProvider::Dropbox;
    return CloudProvider::OneDrive;
}

const wchar_t* CloudSyncProvider::GetProviderName(CloudProvider provider) {
    switch (provider) {
        case CloudProvider::OneDrive:    return L"OneDrive";
        case CloudProvider::SharePoint:  return L"SharePoint";
        case CloudProvider::AmazonS3:    return L"Amazon S3";
        case CloudProvider::AzureBlob:   return L"Azure Blob";
        case CloudProvider::GoogleDrive: return L"Google Drive";
        case CloudProvider::Dropbox:     return L"Dropbox";
        default: return L"Unknown";
    }
}

const wchar_t* CloudSyncProvider::GetStatusName(SyncStatus status) {
    switch (status) {
        case SyncStatus::Idle:      return L"Idle";
        case SyncStatus::Syncing:   return L"Syncing";
        case SyncStatus::Completed: return L"Completed";
        case SyncStatus::Error:     return L"Error";
        case SyncStatus::Conflict:  return L"Conflict";
        case SyncStatus::Offline:   return L"Offline";
        default: return L"Unknown";
    }
}

}} // namespace DarkThumbs::Engine
