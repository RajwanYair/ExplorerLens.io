//==============================================================================
// ExplorerLens Engine — Cloud Storage Integration
// OneDrive, Google Drive, and SharePoint thumbnail integration pipeline.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Cloud storage provider
enum class StorageCloudProvider : uint8_t {
    OneDrive,
    OneDriveBusiness,
    GoogleDrive,
    SharePoint,
    Dropbox,
    iCloudDrive,
    COUNT
};

/// Cloud file state
enum class CloudFileState : uint8_t {
    Available,    // Fully downloaded locally
    OnlineOnly,   // Cloud-only placeholder
    Syncing,      // Currently downloading
    PinnedLocal,  // Pinned for offline
    Conflict,     // Sync conflict
    COUNT
};

/// Cloud file hydration strategy for thumbnails
enum class HydrationStrategy : uint8_t {
    AlwaysHydrate,   // Download for thumbnail generation
    HydrateIfSmall,  // Download only if < size limit
    UseCloudThumb,   // Request cloud-side thumbnail
    Skip,            // Don't generate for cloud files
    COUNT
};

/// Cloud file info
struct StorageCloudFileInfo
{
    StorageCloudProvider provider = StorageCloudProvider::OneDrive;
    CloudFileState state = CloudFileState::Available;
    uint64_t fileSize = 0;
    std::wstring cloudPath;
    std::wstring localPath;
    bool hasCloudThumb = false;
};

/// Cloud integration config
struct CloudIntegrationConfig
{
    HydrationStrategy strategy = HydrationStrategy::HydrateIfSmall;
    uint64_t maxHydrationSize = 50 * 1024 * 1024;  // 50MB
    bool respectMeterConn = true;                  // Don't hydrate on metered
    bool cacheCloudThumbs = true;
    uint32_t timeoutSeconds = 30;
};

/// Cloud storage integration
class CloudStorageIntegration
{
  public:
    static const wchar_t* ProviderName(StorageCloudProvider p)
    {
        switch (p) {
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

    static const wchar_t* FileStateName(CloudFileState s)
    {
        switch (s) {
            case CloudFileState::Available:
                return L"Available";
            case CloudFileState::OnlineOnly:
                return L"Online Only";
            case CloudFileState::Syncing:
                return L"Syncing";
            case CloudFileState::PinnedLocal:
                return L"Pinned Local";
            case CloudFileState::Conflict:
                return L"Conflict";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* HydrationName(HydrationStrategy h)
    {
        switch (h) {
            case HydrationStrategy::AlwaysHydrate:
                return L"Always Hydrate";
            case HydrationStrategy::HydrateIfSmall:
                return L"Hydrate If Small";
            case HydrationStrategy::UseCloudThumb:
                return L"Use Cloud Thumbnail";
            case HydrationStrategy::Skip:
                return L"Skip";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t ProviderCount()
    {
        return static_cast<size_t>(StorageCloudProvider::COUNT);
    }
    static constexpr size_t FileStateCount()
    {
        return static_cast<size_t>(CloudFileState::COUNT);
    }
    static constexpr size_t HydrationCount()
    {
        return static_cast<size_t>(HydrationStrategy::COUNT);
    }

    static bool ShouldHydrate(const StorageCloudFileInfo& info, const CloudIntegrationConfig& cfg)
    {
        if (info.state == CloudFileState::Available)
            return false;  // Already local
        switch (cfg.strategy) {
            case HydrationStrategy::AlwaysHydrate:
                return true;
            case HydrationStrategy::HydrateIfSmall:
                return info.fileSize <= cfg.maxHydrationSize;
            case HydrationStrategy::UseCloudThumb:
                return !info.hasCloudThumb;
            case HydrationStrategy::Skip:
                return false;
            default:
                return false;
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
