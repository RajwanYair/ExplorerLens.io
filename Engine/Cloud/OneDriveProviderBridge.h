// OneDriveProviderBridge.h — Windows CfApi / StorageProvider Shell Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges the Windows Cloud Files API (CfApi) to ExplorerLens, allowing
// thumbnails to be generated from on-demand cloud placeholders without
// triggering a full file hydration.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Placeholder State ------------------------------------------------------

enum class PlaceholderState : uint8_t {
    FullyLocal     = 0,   // CF_PLACEHOLDER_STATE_COMPLETE — cached offline
    CloudOnly      = 1,   // CF_PLACEHOLDER_STATE_PLACEHOLDER — needs download
    InSync         = 2,   // CF_PLACEHOLDER_STATE_IN_SYNC — local copy up to date
    PendingUpload  = 3,   // CF_PLACEHOLDER_STATE_PENDING_HYDRATION
    Dehydrating    = 4,   // Transition: local → cloud
    Unknown        = 0xFF,
};

struct OneDrivePlaceholderInfo {
    std::string      localPath;
    PlaceholderState state        = PlaceholderState::Unknown;
    uint64_t         fileSize     = 0;
    std::string      synchRootId; // StorageProvider providerId from CfApi
    bool             hasEmbeddedThumb = false;  // Thumbnail in EA or EXIF
};

// ---- Range Hydration --------------------------------------------------------

struct HydrationRequest {
    std::string localPath;
    uint64_t    offset   = 0;
    uint64_t    length   = 0;    // Bytes to hydrate; 0 = full file
    uint32_t    timeoutMs = 5000;
};

struct HydrationResult {
    bool    success        = false;
    uint64_t bytesHydrated = 0;
    std::string error;
};

// ---- OneDriveProviderBridge -------------------------------------------------

class OneDriveProviderBridge {
public:
    OneDriveProviderBridge();
    ~OneDriveProviderBridge();

    // Query CfApi placeholder state for a local path.
    bool QueryPlaceholder(const std::string& localPath,
                          OneDrivePlaceholderInfo& outInfo) const;

    // Hydrate a byte range without waiting for the full file (CfOpenFileWithOplock).
    HydrationResult HydrateRange(const HydrationRequest& req) const;

    // Check if CfApi is available on this Windows build (Win10 1709+).
    static bool IsCfApiAvailable();

    // Register the LENSShell thumbnail provider as a StorageProvider thumbnail handler
    // so Explorer calls us for cloud-placeholder files.
    bool RegisterThumbnailHandler(const std::string& syncRootId) const;

    // Unregister.
    bool UnregisterThumbnailHandler(const std::string& syncRootId) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
