// CloudSyncStatusBadge.h — Cloud Sync Status Overlay Badge
// Copyright (c) 2026 ExplorerLens Project
//
// Renders an overlay badge on thumbnails indicating the cloud synchronization
// state of the underlying file (synced, pending upload, conflict, etc.).
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---- Sync Status Enum -------------------------------------------------------

enum class SyncStatus : uint8_t {
    Synced          = 0,   // Green checkmark — fully in-sync
    Uploading       = 1,   // Blue arrows — upload in progress
    Downloading     = 2,   // Blue arrows — download in progress
    Conflict        = 3,   // Red X — version conflict
    Error           = 4,   // Orange exclamation — sync error
    Paused          = 5,   // Grey pause icon
    CloudOnly       = 6,   // Cloud icon — not yet downloaded
    LocalOnly       = 7,   // Computer icon — not yet uploaded
    PinnedLocal     = 8,   // Pin icon — always keep on device
    Unknown         = 0xFF,
};

// ---- Badge Render Options ---------------------------------------------------

struct BadgeOptions {
    uint32_t thumbnailWidth  = 256;  // Full thumbnail width for placement calc
    uint32_t thumbnailHeight = 256;
    uint32_t badgeSizePx     = 32;   // Badge icon size (auto-scales with thumb)
    bool     cornerBadge     = true; // Bottom-right corner (false = bottom-left)
    float    opacity         = 0.90f;
};

// ---- Badge Result -----------------------------------------------------------

struct BadgeResult {
    bool success = false;
    // Composited BGRA output (thumbnail + badge overlaid).
    std::vector<uint8_t> pixels;
    uint32_t             width  = 0;
    uint32_t             height = 0;
};

// ---- CloudSyncStatusBadge ---------------------------------------------------

class CloudSyncStatusBadge {
public:
    CloudSyncStatusBadge()  = default;
    ~CloudSyncStatusBadge() = default;

    // Composite a sync status badge onto an existing thumbnail BGRA buffer.
    BadgeResult Composite(
        const uint8_t*     thumbPixels,
        uint32_t           thumbWidth,
        uint32_t           thumbHeight,
        SyncStatus         status,
        const BadgeOptions& opts = {}) const;

    // Query current sync status for a local file via Shell property PKEY_StorageProviderState.
    static SyncStatus QueryStatus(const std::string& localPath);

    // Return the display name for a sync status (e.g. "Synced", "Conflict").
    static const char* StatusName(SyncStatus status);

private:
    // Returns a 32x32 BGRA icon for the given SyncStatus (embedded PNG atlas).
    static bool GetBadgeIcon(SyncStatus status,
                              std::vector<uint8_t>& outBGRA,
                              uint32_t targetSize);

    static void AlphaBlend(
        uint8_t*       dst,
        const uint8_t* src,
        uint32_t       w,
        uint32_t       h,
        int            dstX,
        int            dstY,
        uint32_t       dstStride,
        float          opacity);
};

} // namespace Engine
} // namespace ExplorerLens
