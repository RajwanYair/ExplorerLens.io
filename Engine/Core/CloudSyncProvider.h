#pragma once
//==============================================================================
// CloudSyncProvider
// Cloud storage integration for thumbnail syncing.
// Supports OneDrive, SharePoint, S3, Azure Blob, Google Drive.
//==============================================================================

#include "CloudStorageIntegration.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ProviderSyncStatus : uint8_t {
 Idle = 0,
 Syncing = 1,
 Completed = 2,
 Error = 3,
 Conflict = 4,
 Offline = 5
};

struct CloudSyncResult {
 bool success = false;
 ProviderSyncStatus status = ProviderSyncStatus::Idle;
 uint32_t filesProcessed = 0;
 uint32_t thumbnailsCached = 0;
 uint64_t bytesTransferred = 0;
 double syncTimeMs = 0.0;
 std::wstring errorMessage;
};

class CloudSyncProvider {
public:
 CloudSyncProvider();

 CloudSyncResult SyncThumbnails(StorageCloudProvider provider,
 const std::wstring &path);
 StorageCloudFileInfo GetFileInfo(const std::wstring &path) const;

 bool IsCloudPath(const std::wstring &path) const;
 bool IsPlaceholderFile(const std::wstring &path) const;

 void SetMaxCacheSize(uint64_t bytes) { m_maxCacheBytes = bytes; }
 uint64_t GetMaxCacheSize() const { return m_maxCacheBytes; }

 static StorageCloudProvider DetectProvider(const std::wstring &path);
 static const wchar_t *GetProviderName(StorageCloudProvider provider);
 static const wchar_t *GetStatusName(ProviderSyncStatus status);
 static uint32_t GetProviderCount() {
 return static_cast<uint32_t>(StorageCloudProvider::COUNT);
 }

private:
 uint64_t m_maxCacheBytes = 1024 * 1024 * 1024; // 1 GB
};

} // namespace Engine
} // namespace ExplorerLens
