//==============================================================================
// CloudSyncProvider
//==============================================================================

#include "CloudSyncProvider.h"
#include <algorithm>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

CloudSyncProvider::CloudSyncProvider() {}

CloudSyncResult CloudSyncProvider::SyncThumbnails(StorageCloudProvider provider,
 const std::wstring &path) {
 (void)provider;
 CloudSyncResult result;
 auto start = std::chrono::high_resolution_clock::now();

 // Validate path
 if (path.empty()) {
 result.status = ProviderSyncStatus::Error;
 result.errorMessage = L"Empty path";
 return result;
 }

 result.status = ProviderSyncStatus::Syncing;

 // In production: enumerate cloud files and generate thumbnails
 result.filesProcessed = 0;
 result.thumbnailsCached = 0;
 result.status = ProviderSyncStatus::Completed;
 result.success = true;

 auto end = std::chrono::high_resolution_clock::now();
 result.syncTimeMs =
 std::chrono::duration<double, std::milli>(end - start).count();
 return result;
}

StorageCloudFileInfo CloudSyncProvider::GetFileInfo(const std::wstring &path) const {
 StorageCloudFileInfo info;
 info.cloudPath = path;
 info.provider = DetectProvider(path);
 return info;
}

bool CloudSyncProvider::IsCloudPath(const std::wstring &path) const {
 // Check for known cloud sync folders
 auto lower = path;
 for (auto &c : lower)
 c = towlower(c);

 return lower.find(L"onedrive") != std::wstring::npos ||
 lower.find(L"sharepoint") != std::wstring::npos ||
 lower.find(L"dropbox") != std::wstring::npos ||
 lower.find(L"google drive") != std::wstring::npos;
}

bool CloudSyncProvider::IsPlaceholderFile(const std::wstring & /*path*/) const {
 // In production: check FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
 return false;
}

StorageCloudProvider CloudSyncProvider::DetectProvider(const std::wstring &path) {
 auto lower = path;
 for (auto &c : lower)
 c = towlower(c);

 if (lower.find(L"onedrive - ") != std::wstring::npos)
 return StorageCloudProvider::OneDriveBusiness;
 if (lower.find(L"onedrive") != std::wstring::npos)
 return StorageCloudProvider::OneDrive;
 if (lower.find(L"sharepoint") != std::wstring::npos)
 return StorageCloudProvider::SharePoint;
 if (lower.find(L"google drive") != std::wstring::npos)
 return StorageCloudProvider::GoogleDrive;
 if (lower.find(L"dropbox") != std::wstring::npos)
 return StorageCloudProvider::Dropbox;
 if (lower.find(L"icloud") != std::wstring::npos)
 return StorageCloudProvider::iCloudDrive;
 return StorageCloudProvider::OneDrive;
}

const wchar_t *CloudSyncProvider::GetProviderName(StorageCloudProvider provider) {
 switch (provider) {
 case StorageCloudProvider::OneDrive:
 return L"OneDrive";
 case StorageCloudProvider::OneDriveBusiness:
 return L"OneDrive for Business";
 case StorageCloudProvider::GoogleDrive:
 return L"Google Drive";
 case StorageCloudProvider::SharePoint:
 return L"SharePoint";
 case StorageCloudProvider::Dropbox:
 return L"Dropbox";
 case StorageCloudProvider::iCloudDrive:
 return L"iCloud Drive";
 default:
 return L"Unknown";
 }
}

const wchar_t *CloudSyncProvider::GetStatusName(ProviderSyncStatus status) {
 switch (status) {
 case ProviderSyncStatus::Idle:
 return L"Idle";
 case ProviderSyncStatus::Syncing:
 return L"Syncing";
 case ProviderSyncStatus::Completed:
 return L"Completed";
 case ProviderSyncStatus::Error:
 return L"Error";
 case ProviderSyncStatus::Conflict:
 return L"Conflict";
 case ProviderSyncStatus::Offline:
 return L"Offline";
 default:
 return L"Unknown";
 }
}

} // namespace Engine
} // namespace ExplorerLens
