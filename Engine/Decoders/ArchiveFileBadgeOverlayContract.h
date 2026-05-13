// ArchiveFileBadgeOverlayContract.h -- S294 / ROADMAP v6.0 H5 Phase 2
// File-count badge overlay policy for multi-image archive thumbnails.
// Inspired by SageThumbs: show first image as cover, overlay file count.
//
// Design:
//   - First valid image in archive becomes the cover thumbnail.
//   - A small badge in a configurable corner shows total file count.
//   - Badge is rendered in GDI+ (Phase 2) or Direct2D (Phase 3+).
//   - Badge is suppressed if archive has only 1 image.
//
// Rule: contract header only — no implementation, no Win32 headers.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Badge Position ────────────────────────────────────────────────────────────

enum class ArchiveFileBadgePosition : uint8_t {
    BOTTOM_RIGHT = 0,  // Default — matches Windows badge convention
    BOTTOM_LEFT  = 1,
    TOP_RIGHT    = 2,
    TOP_LEFT     = 3,
};

// ── Badge Style ───────────────────────────────────────────────────────────────

enum class ArchiveFileBadgeStyle : uint8_t {
    ROUNDED_PILL      = 0,  // Dark pill with white digit(s) — default
    CIRCLE            = 1,  // Circle badge (single-digit counts)
    BADGE_TRANSPARENT = 2,  // Translucent overlay
    NONE              = 3,  // Suppress badge entirely
};

// ── Badge Policy ──────────────────────────────────────────────────────────────

struct ArchiveFileBadgeOverlayPolicy {
    ArchiveFileBadgePosition position    = ArchiveFileBadgePosition::BOTTOM_RIGHT;
    ArchiveFileBadgeStyle    style       = ArchiveFileBadgeStyle::ROUNDED_PILL;
    uint8_t                  badgeSizePx = 16;     // Badge height in pixels
    uint8_t                  paddingPx   = 2;      // Inset from thumbnail edge
    uint32_t                 maxBadgeNum = 999;    // Cap display at "999+" above this
    bool                     suppressOnSingle = true;  // Hide if only 1 file
};

// ── Cover Extraction Policy ────────────────────────────────────────────────────

enum class ArchiveFileBadgeCoverStrategy : uint8_t {
    FIRST_IMAGE     = 0,  // First image in archive by natural order (default)
    LARGEST_IMAGE   = 1,  // Largest file by byte size
    FILENAME_SORT   = 2,  // First by alphabetic filename
};

struct ArchiveFileBadgeCoverPolicy {
    ArchiveFileBadgeCoverStrategy strategy  = ArchiveFileBadgeCoverStrategy::FIRST_IMAGE;
    uint32_t                      scanLimitFiles = 64;  // Scan at most N entries to find cover
    uint32_t                      hardTimeoutMs  = 500; // Abort cover search after this
};

// ── Probe ─────────────────────────────────────────────────────────────────────

struct ArchiveFileBadgeOverlayProbe {
    uint32_t archivesProcessed = 0;
    uint32_t badgesRendered    = 0;
    uint32_t badgesSuppressed  = 0;
    uint32_t coverSearchAborts = 0;  // Timed out before finding cover
};

// ── Constants ─────────────────────────────────────────────────────────────────

static constexpr uint8_t  kArchiveFileBadgeMinSizePx      = 10;
static constexpr uint8_t  kArchiveFileBadgeMaxSizePx       = 32;
static constexpr uint32_t kArchiveFileBadgeScanHardLimit   = 256;  // Never scan > 256 entries
static constexpr uint32_t kArchiveFileBadgeHardTimeoutMs   = 2000;

} // namespace Engine
} // namespace ExplorerLens
