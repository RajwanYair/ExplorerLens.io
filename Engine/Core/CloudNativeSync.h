// CloudNativeSync.h — Cloud-Native Thumbnail Sync & Distribution
// Copyright (c) 2026 ExplorerLens Project
//
// Synchronizes thumbnail cache across devices via cloud storage providers
// (OneDrive, SharePoint, Google Drive, Dropbox). Enables pre-generated
// thumbnails to follow users across machines, eliminating cold-start latency.

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Supported cloud storage providers
enum class NativeCloudProvider : uint8_t {
 None = 0,
 OneDrive = 1,
 OneDriveBusiness = 2,
 SharePoint = 3,
 GoogleDrive = 4,
 Dropbox = 5,
 iCloudDrive = 6,
 Box = 7,
 AmazonS3 = 8,
 AzureBlob = 9,
 Custom = 10,
 COUNT
};

/// Sync direction
enum class SyncDirection : uint8_t {
 Upload = 0, ///< Local → Cloud
 Download = 1, ///< Cloud → Local
 Bidirectional = 2,
 COUNT
};

/// Sync status for a thumbnail
enum class NativeSyncStatus : uint8_t {
 NotSynced = 0,
 Pending = 1,
 Syncing = 2,
 Synced = 3,
 Conflict = 4,
 Error = 5,
 COUNT
};

/// Cloud thumbnail manifest entry
struct CloudThumbnailEntry {
 std::wstring fileHash; ///< SHA-256 of source file
 std::wstring thumbnailKey; ///< Cache key in cloud storage
 uint32_t width = 0;
 uint32_t height = 0;
 uint64_t sizeBytes = 0;
 uint64_t lastModifiedMs = 0;
 NativeSyncStatus status = NativeSyncStatus::NotSynced;
 NativeCloudProvider provider = NativeCloudProvider::None;
 std::wstring machineId; ///< Source machine identifier
};

/// Cloud sync configuration
struct CloudSyncConfig {
 NativeCloudProvider provider = NativeCloudProvider::None;
 SyncDirection direction = SyncDirection::Bidirectional;
 uint64_t maxSyncSizeBytes = 100 * 1024 * 1024; ///< 100MB max sync
 uint32_t maxConcurrentSyncs = 4;
 bool syncOnMeteredNetwork = false;
 bool encryptThumbnails = true;
 std::wstring syncFolderPath; ///< Cloud folder for thumb cache
 uint32_t syncIntervalMinutes = 15;
 bool compressBeforeSync = true;
};

/// Sync operation result
struct NativeSyncResult {
 bool success = false;
 uint32_t thumbnailsSynced = 0;
 uint32_t thumbnailsSkipped = 0;
 uint32_t conflicts = 0;
 uint64_t bytesTransferred = 0;
 double durationMs = 0.0;
 std::wstring errorMessage;
};

/// Cloud Native Sync
class CloudNativeSync {
public:
 static const wchar_t *ProviderName(NativeCloudProvider p) {
 switch (p) {
 case NativeCloudProvider::None:
 return L"None";
 case NativeCloudProvider::OneDrive:
 return L"OneDrive";
 case NativeCloudProvider::OneDriveBusiness:
 return L"OneDrive for Business";
 case NativeCloudProvider::SharePoint:
 return L"SharePoint";
 case NativeCloudProvider::GoogleDrive:
 return L"Google Drive";
 case NativeCloudProvider::Dropbox:
 return L"Dropbox";
 case NativeCloudProvider::iCloudDrive:
 return L"iCloud Drive";
 case NativeCloudProvider::Box:
 return L"Box";
 case NativeCloudProvider::AmazonS3:
 return L"Amazon S3";
 case NativeCloudProvider::AzureBlob:
 return L"Azure Blob Storage";
 case NativeCloudProvider::Custom:
 return L"Custom";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *SyncStatusName(NativeSyncStatus s) {
 switch (s) {
 case NativeSyncStatus::NotSynced:
 return L"Not Synced";
 case NativeSyncStatus::Pending:
 return L"Pending";
 case NativeSyncStatus::Syncing:
 return L"Syncing";
 case NativeSyncStatus::Synced:
 return L"Synced";
 case NativeSyncStatus::Conflict:
 return L"Conflict";
 case NativeSyncStatus::Error:
 return L"Error";
 default:
 return L"Unknown";
 }
 }

 static constexpr size_t ProviderCount() {
 return static_cast<size_t>(NativeCloudProvider::COUNT);
 }

 static constexpr size_t StatusCount() {
 return static_cast<size_t>(NativeSyncStatus::COUNT);
 }

 /// Detect available cloud providers on this system
 static std::vector<NativeCloudProvider> DetectProviders() {
 std::vector<NativeCloudProvider> providers;
#pragma warning(push)
#pragma warning(disable : 4996) // _wgetenv deprecation
 // Check for OneDrive
 const wchar_t *onedrive = _wgetenv(L"OneDrive");
 if (onedrive && onedrive[0] != L'\0')
 providers.push_back(NativeCloudProvider::OneDrive);
 // Check for OneDrive for Business
 const wchar_t *onedriveBiz = _wgetenv(L"OneDriveCommercial");
 if (onedriveBiz && onedriveBiz[0] != L'\0')
 providers.push_back(NativeCloudProvider::OneDriveBusiness);
#pragma warning(pop)
 return providers;
 }
};

} // namespace Engine
} // namespace ExplorerLens
