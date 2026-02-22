//==============================================================================
// DarkThumbs Engine — Sprint 331: SharePoint & Teams Integration
// Thumbnail generation for SharePoint document libraries and Teams file cards
// with Graph API auth, delta sync, and adaptive thumbnail sizes.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class CloudFileSource : uint8_t { SharePoint=0,OneDrive,Teams,Yammer,COUNT };
enum class GraphAuthMethod : uint8_t { DeviceCode=0,ClientCredential,ManagedIdentity,InteractiveMSAL,COUNT };
enum class CloudSyncState  : uint8_t { Idle=0,Syncing,Error,Throttled,COUNT };

struct SharePointSite {
    std::wstring    siteId;
    std::wstring    displayName;
    std::wstring    webUrl;
    CloudFileSource source      = CloudFileSource::SharePoint;
    bool            deltaEnabled = false;
};

struct TeamsFileCardResult {
    std::wstring  fileId;
    uint32_t      thumbnailWidthPx  = 0;
    uint32_t      thumbnailHeightPx = 0;
    CloudSyncState syncState        = CloudSyncState::Idle;
};

class SharePointTeamsIntegration {
public:
    static const wchar_t* CloudSourceName(CloudFileSource s) {
        switch(s) {
            case CloudFileSource::SharePoint: return L"SharePoint";
            case CloudFileSource::OneDrive:   return L"OneDrive";
            case CloudFileSource::Teams:      return L"Microsoft Teams";
            case CloudFileSource::Yammer:     return L"Viva Engage";
            default: return L"Unknown";
        }
    }
    static const wchar_t* AuthMethodName(GraphAuthMethod a) {
        switch(a) {
            case GraphAuthMethod::DeviceCode:        return L"Device Code";
            case GraphAuthMethod::ClientCredential:  return L"Client Credential";
            case GraphAuthMethod::ManagedIdentity:   return L"Managed Identity";
            case GraphAuthMethod::InteractiveMSAL:   return L"Interactive MSAL";
            default: return L"Unknown";
        }
    }
    static const wchar_t* SyncStateName(CloudSyncState s) {
        switch(s) {
            case CloudSyncState::Idle:      return L"Idle";
            case CloudSyncState::Syncing:   return L"Syncing";
            case CloudSyncState::Error:     return L"Error";
            case CloudSyncState::Throttled: return L"Throttled";
            default: return L"Unknown";
        }
    }
    static constexpr size_t CloudSourceCount() { return static_cast<size_t>(CloudFileSource::COUNT); }
    static constexpr size_t AuthMethodCount()  { return static_cast<size_t>(GraphAuthMethod::COUNT); }
    static constexpr size_t SyncStateCount()   { return static_cast<size_t>(CloudSyncState::COUNT); }
};

}} // namespace DarkThumbs::Engine
