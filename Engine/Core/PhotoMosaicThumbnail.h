// PhotoMosaicThumbnail.h — Photo Mosaic Layout Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Creates photo mosaic thumbnails from multiple images by computing
// grid-based tile layouts. Pure geometry and layout calculations with
// no pixel manipulation — produces tile rects and color metadata.
//
#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// Average color of a mosaic tile stored as packed ARGB
struct MosaicTileColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

// Rectangle describing a tile's position and size within the mosaic
struct MosaicTileRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// Describes one tile in the mosaic grid
struct MosaicTile {
    int index = -1;
    MosaicTileColor avgColor{};
    MosaicTileRect rect{};
    bool occupied = false;
};

class PhotoMosaicThumbnail {
public:
    PhotoMosaicThumbnail() = default;
    ~PhotoMosaicThumbnail() = default;

    // Sets the grid dimensions (columns x rows). Returns false if either is <= 0 or > 256.
    bool SetGridSize(int cols, int rows) {
        if (cols <= 0 || rows <= 0 || cols > 256 || rows > 256)
            return false;
        m_cols = cols;
        m_rows = rows;
        m_tiles.clear();
        m_tiles.resize(static_cast<size_t>(cols) * rows);
        for (int i = 0; i < cols * rows; ++i)
            m_tiles[i].index = i;
        m_layoutDirty = true;
        return true;
    }

    // Assigns an average color to a tile at the given index. Returns false if out of range.
    bool AddTile(int index, MosaicTileColor color) {
        if (index < 0 || index >= static_cast<int>(m_tiles.size()))
            return false;
        m_tiles[index].avgColor = color;
        m_tiles[index].occupied = true;
        m_layoutDirty = true;
        return true;
    }

    // Computes mosaic layout for the given target canvas size.
    // Returns false if grid is not configured or dimensions are invalid.
    bool ComputeMosaicLayout(int targetWidth, int targetHeight) {
        if (m_cols <= 0 || m_rows <= 0 || targetWidth <= 0 || targetHeight <= 0)
            return false;

        const int gapTotal = m_gap * (m_cols + 1);
        const int vGapTotal = m_gap * (m_rows + 1);

        const int availW = (std::max)(1, targetWidth - gapTotal);
        const int availH = (std::max)(1, targetHeight - vGapTotal);

        const int cellW = availW / m_cols;
        const int cellH = availH / m_rows;

        if (cellW <= 0 || cellH <= 0)
            return false;

        for (int r = 0; r < m_rows; ++r) {
            for (int c = 0; c < m_cols; ++c) {
                const int idx = r * m_cols + c;
                auto& tile = m_tiles[idx];
                tile.rect.x = m_gap + c * (cellW + m_gap);
                tile.rect.y = m_gap + r * (cellH + m_gap);
                tile.rect.width = cellW;
                tile.rect.height = cellH;
            }
        }

        m_canvasWidth = targetWidth;
        m_canvasHeight = targetHeight;
        m_layoutDirty = false;
        return true;
    }

    // Returns the rect for a tile at the given index. Returns zero rect if invalid.
    MosaicTileRect GetTileRect(int index) const {
        if (index < 0 || index >= static_cast<int>(m_tiles.size()))
            return {};
        return m_tiles[index].rect;
    }

    // Returns the total number of tiles in the grid (cols * rows).
    int GetTileCount() const {
        return static_cast<int>(m_tiles.size());
    }

    // Returns the number of tiles that have been assigned a color.
    int GetOccupiedTileCount() const {
        int count = 0;
        for (const auto& t : m_tiles) {
            if (t.occupied) ++count;
        }
        return count;
    }

    // Gets the current grid columns.
    int GetCols() const { return m_cols; }

    // Gets the current grid rows.
    int GetRows() const { return m_rows; }

    // Sets the gap (in pixels) between tiles. Returns false if negative.
    bool SetGap(int gap) {
        if (gap < 0) return false;
        m_gap = gap;
        m_layoutDirty = true;
        return true;
    }

    // Returns true if layout needs recomputation.
    bool IsLayoutDirty() const { return m_layoutDirty; }

private:
    int m_cols = 0;
    int m_rows = 0;
    int m_gap = 2;
    int m_canvasWidth = 0;
    int m_canvasHeight = 0;
    bool m_layoutDirty = true;
    std::vector<MosaicTile> m_tiles;
};

} // namespace Engine
} // namespace ExplorerLens
