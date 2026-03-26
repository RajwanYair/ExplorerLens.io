// CollaborationMarker.h — Co-Authoring and Sharing Overlay Badges
// Copyright (c) 2026 ExplorerLens Project
//
// Renders sharing/co-authoring overlay badges on thumbnails using SharePoint
// and OneDrive Graph API presence data (shared link, live co-author count).
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Sharing State ----------------------------------------------------------

enum class SharingScope : uint8_t {
    Private         = 0,
    SharedWithPeople = 1,  // Shared with 1-N specific users
    SharedWithOrg   = 2,   // Anyone in the org via link
    SharedPublic    = 3,   // Public link (anyone with link)
};

struct CollaborationInfo {
    SharingScope scope           = SharingScope::Private;
    uint32_t     activeEditors   = 0;  // Currently editing (live co-author count)
    uint32_t     totalSharedWith = 0;  // Total recipients
    bool         hasShareLink    = false;
    bool         isFolder        = false;
    std::string  ownerDisplayName;
};

// ---- Overlay Options --------------------------------------------------------

struct CollabOverlayOptions {
    uint32_t thumbnailWidth  = 256;
    uint32_t thumbnailHeight = 256;
    uint32_t badgeSizePx     = 28;
    bool     showAvatarCount = true;  // Show "3" disc if 3+ co-authors
    float    opacity         = 0.92f;
};

// ---- Result -----------------------------------------------------------------

struct CollabOverlayResult {
    bool                 success = false;
    std::vector<uint8_t> pixels; // Composited BGRA
    uint32_t             width  = 0;
    uint32_t             height = 0;
};

// ---- CollaborationMarker ----------------------------------------------------

class CollaborationMarker {
public:
    CollaborationMarker()  = default;
    ~CollaborationMarker() = default;

    // Composite collaboration badges onto a thumbnail.
    CollabOverlayResult Composite(
        const uint8_t*           thumbPixels,
        uint32_t                 thumbWidth,
        uint32_t                 thumbHeight,
        const CollaborationInfo& info,
        const CollabOverlayOptions& opts = {}) const;

    // Fetch collaboration info from Graph API for a driveItem path.
    // Returns false if not authenticated or item not found.
    bool FetchInfo(
        const std::string&  localPath,
        const std::string&  driveItemId,
        CollaborationInfo&  outInfo) const;

private:
    static bool GetSharingIcon(SharingScope scope,
                                std::vector<uint8_t>& outBGRA,
                                uint32_t size);
};

} // namespace Engine
} // namespace ExplorerLens
