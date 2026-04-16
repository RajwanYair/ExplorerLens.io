// ThumbnailSizeGrid.h — Standard thumbnail size presets and grid layout utilities
// Copyright (c) 2026 ExplorerLens Project
//
// Defines canonical thumbnail sizes matching Windows Shell (SHIL_*) and
// Explorer's view modes, plus utilities for computing grid layouts for
// thumbnail strips (comic book reader, batch preview, etc.).
//
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// ThumbnailSize — standard preset sizes
// ---------------------------------------------------------------------------

enum class ThumbnailSize : uint16_t {
    ICON_SMALL    =  16,   // SHIL_SMALL
    ICON_MEDIUM   =  32,
    ICON_LARGE    =  48,   // SHIL_LARGE (default Explorer List view)
    ICON_XLARGE   =  64,
    THUMBNAIL_96  =  96,   // Explorer "medium icons"
    THUMBNAIL_128 = 128,   // Edge of Shell ExtractImage territory
    THUMBNAIL_256 = 256,   // Explorer "large icons" (default thumbnail request)
    THUMBNAIL_512 = 512,   // Explorer "extra large icons"
    PREVIEW_1024  = 1024,  // Full preview pane
    PREVIEW_2048  = 2048,  // High-DPI 4K display
};

// ---------------------------------------------------------------------------
// ThumbnailSizeInfo — metadata for a preset
// ---------------------------------------------------------------------------

struct ThumbnailSizeInfo {
    ThumbnailSize size;
    uint32_t      pixels;       // Square edge length
    const char*   name;         // Human-readable label
    bool          isShell;      // True if Windows Shell icon list uses this size
    uint8_t       shilIndex;    // SHIL_* index (0 if isShell == false)
};

// ---------------------------------------------------------------------------
// All known presets (constexpr table)
// ---------------------------------------------------------------------------

inline constexpr std::array<ThumbnailSizeInfo, 10> THUMBNAIL_SIZE_TABLE {{
    { ThumbnailSize::ICON_SMALL,    16,  "16 (Small Icon)",       true,  0 },
    { ThumbnailSize::ICON_MEDIUM,   32,  "32 (Medium Icon)",      true,  1 },
    { ThumbnailSize::ICON_LARGE,    48,  "48 (Large Icon)",       true,  2 },
    { ThumbnailSize::ICON_XLARGE,   64,  "64 (Extra-Large Icon)", false, 0 },
    { ThumbnailSize::THUMBNAIL_96,  96,  "96 (Medium Thumbs)",    false, 0 },
    { ThumbnailSize::THUMBNAIL_128, 128, "128 (Large List)",      false, 0 },
    { ThumbnailSize::THUMBNAIL_256, 256, "256 (Large Icons)",     true,  3 },
    { ThumbnailSize::THUMBNAIL_512, 512, "512 (Extra-Large)",     true,  4 },
    { ThumbnailSize::PREVIEW_1024,  1024,"1024 (Preview Pane)",   false, 0 },
    { ThumbnailSize::PREVIEW_2048,  2048,"2048 (HiDPI Preview)",  false, 0 },
}};

// ---------------------------------------------------------------------------
// Helper: resolve a pixel count to the nearest standard preset
// ---------------------------------------------------------------------------

inline constexpr ThumbnailSize NearestPreset(uint32_t px) noexcept
{
    ThumbnailSize best = ThumbnailSize::THUMBNAIL_256;
    uint32_t bestDiff  = 0xFFFFFFFFu;
    for (auto& s : THUMBNAIL_SIZE_TABLE) {
        uint32_t diff = (px > s.pixels) ? (px - s.pixels) : (s.pixels - px);
        if (diff < bestDiff) { bestDiff = diff; best = s.size; }
    }
    return best;
}

// ---------------------------------------------------------------------------
// GridLayout — compute a thumbnail strip or grid arrangement
// ---------------------------------------------------------------------------

struct GridCell {
    uint32_t x, y;         // Top-left pixel position in the composite bitmap
    uint32_t width, height;
    uint32_t itemIndex;
};

struct GridLayout {
    uint32_t totalWidth;
    uint32_t totalHeight;
    uint32_t columns;
    uint32_t rows;
    std::array<GridCell, 64> cells;  // Max 64 items in a single grid
    uint32_t cellCount;
};

// Compute a grid layout for `count` thumbnails of `cellSize` × `cellSize` pixels,
// arranged in `maxColumns` columns with `gap` pixel padding between cells.
inline GridLayout ComputeGrid(
    uint32_t count,
    uint32_t cellSize,
    uint32_t maxColumns = 4,
    uint32_t gap        = 4) noexcept
{
    GridLayout gl{};
    if (count == 0 || count > 64) return gl;

    gl.columns   = (count < maxColumns) ? count : maxColumns;
    gl.rows      = (count + gl.columns - 1) / gl.columns;
    gl.cellCount = count;

    for (uint32_t i = 0; i < count; ++i) {
        uint32_t col = i % gl.columns;
        uint32_t row = i / gl.columns;
        gl.cells[i] = {
            col * (cellSize + gap),
            row * (cellSize + gap),
            cellSize,
            cellSize,
            i
        };
    }

    gl.totalWidth  = gl.columns * cellSize + (gl.columns - 1) * gap;
    gl.totalHeight = gl.rows    * cellSize + (gl.rows    - 1) * gap;
    return gl;
}

} // namespace Engine
} // namespace ExplorerLens
