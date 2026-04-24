// ============================================================================
// FolderCoverImageStage.h -- S269 / ROADMAP v6.0 F5 folder thumbnails
//
// Pipeline stage contract for synthesising a "folder cover" thumbnail from
// the best-ranked image inside a folder (album art, cover.jpg, first N files).
// Mirrors Explorer's Picture Folder icon behaviour but under our control.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class FolderCoverImageStrategy : uint8_t
{
    NONE                         = 0,
    EXPLICIT_COVER_FILE          = 1,  // cover.jpg / folder.jpg / poster.jpg
    FIRST_IMAGE_ALPHABETICAL     = 2,
    HIGHEST_RESOLUTION           = 3,
    LARGEST_FILE_SIZE            = 4,
    NEWEST_MODIFIED              = 5,
    GRID_OF_FIRST_FOUR           = 6,  // 2x2 grid
    AI_SALIENCY_BEST             = 7,  // needs Engine/AI saliency scorer
};

enum class FolderCoverImageStatus : uint8_t
{
    OK                     = 0,
    FOLDER_EMPTY           = 1,
    NO_INDEXABLE_IMAGES    = 2,
    ENUMERATION_DENIED     = 3,
    BUDGET_EXCEEDED        = 4,
    DECODER_FAILURE        = 5,
};

struct FolderCoverImageRequest
{
    FolderCoverImageStrategy strategy    = FolderCoverImageStrategy::EXPLICIT_COVER_FILE;
    uint32_t                 targetWidth  = 256;
    uint32_t                 targetHeight = 256;
    uint32_t                 scanLimit    = 256;   // max children enumerated
    bool                     respectHidden = false;
    bool                     followShortcuts = false;
    uint32_t                 budgetMs     = 60;
};

struct FolderCoverImageResult
{
    FolderCoverImageStatus status        = FolderCoverImageStatus::OK;
    uint32_t               scannedCount  = 0;
    uint32_t               imageCount    = 0;
    uint32_t               chosenIndex   = 0;      // 0-based into scanned list
    uint32_t               emittedWidth  = 0;
    uint32_t               emittedHeight = 0;
    uint32_t               latencyMs     = 0;
    bool                   usedFallbackIcon = false;
};

// Canonical cover-file names to look for, in priority order.
inline constexpr const char* kFolderCoverImageExplicitNames[] = {
    "cover.jpg",    "cover.jpeg",   "cover.png",
    "folder.jpg",   "folder.jpeg",  "folder.png",
    "poster.jpg",   "poster.png",
    "AlbumArt.jpg", "thumb.jpg",
};
inline constexpr size_t kFolderCoverImageExplicitNamesCount =
    sizeof(kFolderCoverImageExplicitNames) / sizeof(kFolderCoverImageExplicitNames[0]);

inline constexpr uint32_t kFolderCoverImageDefaultBudgetMs = 60;
inline constexpr uint32_t kFolderCoverImageHardBudgetMs    = 250;
inline constexpr uint32_t kFolderCoverImageMaxScanLimit    = 4096;

static_assert(std::is_trivially_copyable_v<FolderCoverImageRequest>,
              "FolderCoverImageRequest must be trivially copyable");
static_assert(std::is_trivially_copyable_v<FolderCoverImageResult>,
              "FolderCoverImageResult must be trivially copyable");
static_assert(kFolderCoverImageExplicitNamesCount == 10,
              "Cover-name list must have 10 entries");

} // namespace ExplorerLens::Engine
