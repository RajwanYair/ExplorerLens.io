#pragma once
//==============================================================================
// ExplorerLens — Archive Content Grid Preview
// 2x2 grid thumbnail for archives, grid layout engine, page-count badge,
// CBZ/CBR cover layout, archive format detection, configuration toggle.
//==============================================================================

#ifndef EXPLORERLENS_ARCHIVE_GRID_PREVIEW_H
#define EXPLORERLENS_ARCHIVE_GRID_PREVIEW_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <array>

namespace ExplorerLens {
namespace Engine {
namespace Decoders {

//==============================================================================
// Archive Format Classification
//==============================================================================

enum class ArchiveFormat
{
    Unknown,
    ZIP, // Standard ZIP
    RAR, // RAR archive
    SevenZip, // 7z archive
    TAR, // TAR (optionally compressed)
    CBZ, // Comic Book ZIP
    CBR, // Comic Book RAR
    CB7, // Comic Book 7z
    CBT // Comic Book TAR
};

inline const char* ArchiveFormatName(ArchiveFormat f) {
    switch (f) {
    case ArchiveFormat::Unknown: return "Unknown";
    case ArchiveFormat::ZIP: return "ZIP";
    case ArchiveFormat::RAR: return "RAR";
    case ArchiveFormat::SevenZip: return "7z";
    case ArchiveFormat::TAR: return "TAR";
    case ArchiveFormat::CBZ: return "CBZ";
    case ArchiveFormat::CBR: return "CBR";
    case ArchiveFormat::CB7: return "CB7";
    case ArchiveFormat::CBT: return "CBT";
    }
    return "Unknown";
}

inline bool IsComicBookFormat(ArchiveFormat f) {
    return f == ArchiveFormat::CBZ || f == ArchiveFormat::CBR ||
        f == ArchiveFormat::CB7 || f == ArchiveFormat::CBT;
}

// Detect archive format from file extension
inline ArchiveFormat DetectArchiveFormat(const std::string& ext) {
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(),
        [](char c) { return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });

    if (lower == ".zip") return ArchiveFormat::ZIP;
    if (lower == ".rar") return ArchiveFormat::RAR;
    if (lower == ".7z") return ArchiveFormat::SevenZip;
    if (lower == ".tar" || lower == ".tar.gz" || lower == ".tgz")
        return ArchiveFormat::TAR;
    if (lower == ".cbz") return ArchiveFormat::CBZ;
    if (lower == ".cbr") return ArchiveFormat::CBR;
    if (lower == ".cb7") return ArchiveFormat::CB7;
    if (lower == ".cbt") return ArchiveFormat::CBT;

    return ArchiveFormat::Unknown;
}

// All supported archive extensions
inline std::vector<std::string> SupportedArchiveExtensions() {
    return { ".zip", ".rar", ".7z", ".tar", ".tar.gz", ".tgz",
    ".cbz", ".cbr", ".cb7", ".cbt" };
}

//==============================================================================
// Archive Image Entry — represents an image found inside an archive
//==============================================================================

struct ArchiveImageEntry
{
    std::string filename;
    std::string archivePath; // Internal path within archive
    uint64_t compressedSize = 0;
    uint64_t uncompressedSize = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    bool isImage = true;

    // Sort key for natural ordering (comic pages)
    std::string SortKey() const { return filename; }

    double CompressionRatio() const {
        if (uncompressedSize == 0) return 1.0;
        return static_cast<double>(compressedSize) / uncompressedSize;
    }
};

//==============================================================================
// Grid Layout Mode — determines thumbnail composition
//==============================================================================

enum class GridLayoutMode
{
    Standard2x2, // Even 2x2 grid (all cells equal size)
    CoverPlusThree, // Large cover + 3 small interior images (comic books)
    SingleCover, // Just the first image (fallback)
    StripHorizontal // 4 images in horizontal strip
};

inline const char* GridLayoutModeName(GridLayoutMode m) {
    switch (m) {
    case GridLayoutMode::Standard2x2: return "Standard 2x2";
    case GridLayoutMode::CoverPlusThree: return "Cover + 3";
    case GridLayoutMode::SingleCover: return "Single Cover";
    case GridLayoutMode::StripHorizontal: return "Horizontal Strip";
    }
    return "Unknown";
}

// Auto-select layout mode based on format and image count
inline GridLayoutMode RecommendedGridLayout(ArchiveFormat format, uint32_t imageCount) {
    if (imageCount == 0)
        return GridLayoutMode::SingleCover;
    if (imageCount == 1)
        return GridLayoutMode::SingleCover;
    if (IsComicBookFormat(format))
        return GridLayoutMode::CoverPlusThree;
    if (imageCount < 4)
        return GridLayoutMode::SingleCover;
    return GridLayoutMode::Standard2x2;
}

//==============================================================================
// Grid Cell — position and size for a grid cell
//==============================================================================

struct GridCell
{
    float x, y;
    float width, height;
    uint32_t imageIndex;
    bool isCover = false; // Larger cell for comic book cover

    float CenterX() const { return x + width / 2.0f; }
    float CenterY() const { return y + height / 2.0f; }
    float Area() const { return width * height; }
};

//==============================================================================
// Grid Layout Engine — computes cell positions for archive grid
//==============================================================================

class GridLayoutEngine
{
public:
    explicit GridLayoutEngine(uint32_t canvasSize = 256)
        : canvasSize_(canvasSize) {
    }

    uint32_t CanvasSize() const { return canvasSize_; }

    // Generate cell layout for Standard 2x2 grid
    std::vector<GridCell> Layout2x2() {
        float half = canvasSize_ / 2.0f;
        float gap = gapPixels_ / 2.0f;
        float cellW = half - gap;
        float cellH = half - gap;

        return {
        {0, 0, cellW, cellH, 0, false},
        {half + gap, 0, cellW, cellH, 1, false},
        {0, half + gap, cellW, cellH, 2, false},
        {half + gap, half + gap, cellW, cellH, 3, false}
        };
    }

    // Generate cell layout for Cover + 3 small (comic book mode)
    std::vector<GridCell> LayoutCoverPlusThree() {
        float gap = gapPixels_ / 2.0f;
        float coverW = canvasSize_ * 0.6f;
        float smallW = canvasSize_ - coverW - gap;
        float thirdH = (canvasSize_ - 2 * gap) / 3.0f;

        return {
        {0, 0, coverW, static_cast<float>(canvasSize_), 0, true},
        {coverW + gap, 0, smallW, thirdH, 1, false},
        {coverW + gap, thirdH + gap, smallW, thirdH, 2, false},
        {coverW + gap, 2 * (thirdH + gap),smallW, thirdH, 3, false}
        };
    }

    // Generate layout based on mode
    std::vector<GridCell> GenerateLayout(GridLayoutMode mode) {
        switch (mode) {
        case GridLayoutMode::Standard2x2:
            return Layout2x2();
        case GridLayoutMode::CoverPlusThree:
            return LayoutCoverPlusThree();
        case GridLayoutMode::SingleCover:
            return { {0, 0, static_cast<float>(canvasSize_), static_cast<float>(canvasSize_), 0, true} };
        case GridLayoutMode::StripHorizontal: {
            float w = canvasSize_ / 4.0f;
            return {
            {0, 0, w, static_cast<float>(canvasSize_), 0, false},
            {w, 0, w, static_cast<float>(canvasSize_), 1, false},
            {2 * w, 0, w, static_cast<float>(canvasSize_), 2, false},
            {3 * w, 0, w, static_cast<float>(canvasSize_), 3, false}
            };
        }
        }
        return {};
    }

    void SetGap(float gap) { gapPixels_ = gap; }
    float Gap() const { return gapPixels_; }

private:
    uint32_t canvasSize_;
    float gapPixels_ = 2.0f;
};

//==============================================================================
// Page Count Badge — for archive thumbnails
//==============================================================================

struct ArchiveBadge
{
    uint32_t imageCount = 0;
    uint32_t totalFiles = 0;
    ArchiveFormat format = ArchiveFormat::Unknown;

    std::string BadgeText() const {
        if (imageCount == 0) return "";
        return std::to_string(imageCount) + " images";
    }

    std::string FormatBadge() const {
        return ArchiveFormatName(format);
    }

    bool ShouldShow() const { return imageCount > 0; }
};

//==============================================================================
// Archive Grid Config — user-configurable settings
//==============================================================================

struct ArchiveGridConfig
{
    bool enabled = true; // Grid mode on/off toggle
    uint32_t canvasSize = 256;
    uint32_t maxImagesToScan = 20; // Scan first N entries for images
    uint32_t maxImagesToDecode = 4; // Decode at most 4 for grid
    bool showPageCountBadge = true;
    bool useCoverLayoutForComics = true;
    float borderWidth = 1.0f;
    float shadowSize = 2.0f;
    uint32_t borderColor = 0xFF404040; // Dark gray (ARGB)
    uint32_t backgroundColor = 0xFF1E1E1E; // Dark background (ARGB)

    static ArchiveGridConfig Default() { return ArchiveGridConfig{}; }

    static ArchiveGridConfig Disabled() {
        ArchiveGridConfig c;
        c.enabled = false;
        return c;
    }

    static ArchiveGridConfig HighQuality() {
        ArchiveGridConfig c;
        c.canvasSize = 512;
        c.maxImagesToScan = 50;
        c.shadowSize = 4.0f;
        return c;
    }
};

//==============================================================================
// Archive Grid Preview Result
//==============================================================================

struct ArchiveGridResult
{
    bool success = false;
    ArchiveFormat format = ArchiveFormat::Unknown;
    GridLayoutMode layoutMode = GridLayoutMode::Standard2x2;
    uint32_t imagesFound = 0;
    uint32_t imagesDecoded = 0;
    uint32_t totalFiles = 0;
    double decodeTimeMs = 0.0;
    std::string errorMessage;

    bool HasContent() const { return imagesFound > 0 && success; }

    std::string Summary() const {
        std::ostringstream ss;
        ss << ArchiveFormatName(format)
            << ": " << imagesDecoded << "/" << imagesFound << " images"
            << " (" << GridLayoutModeName(layoutMode) << ")"
            << " in " << static_cast<int>(decodeTimeMs) << "ms";
        return ss.str();
    }
};

}
}
} // namespace ExplorerLens::Engine::Decoders

#endif // EXPLORERLENS_ARCHIVE_GRID_PREVIEW_H
