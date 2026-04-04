// SpreadsheetCellPreview.h — Spreadsheet Cell Grid Layout
// Copyright (c) 2026 ExplorerLens Project
//
// Generates spreadsheet-style cell grid layouts for CSV/XLSX thumbnail
// previews. Manages cell values and computes column widths and cell
// rectangles — pure layout and data structure, no rendering.
//
#pragma once

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// A single cell value in the spreadsheet preview
struct SpreadsheetCell
{
    int row = 0;
    int col = 0;
    std::wstring value;
};

// Rectangle of a visible cell
struct CellLayoutRect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

class SpreadsheetCellPreview
{
  public:
    SpreadsheetCellPreview() = default;
    ~SpreadsheetCellPreview() = default;

    // Sets the spreadsheet grid dimensions. Returns false if invalid.
    bool SetDimensions(int rows, int cols)
    {
        if (rows <= 0 || cols <= 0 || rows > 1024 || cols > 256)
            return false;
        m_rows = rows;
        m_cols = cols;
        m_grid.assign(static_cast<size_t>(rows) * cols, L"");
        return true;
    }

    // Adds a cell value at the given position. Returns false if out of range.
    bool AddCellValue(int row, int col, const std::wstring& value)
    {
        if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
            return false;
        m_grid[static_cast<size_t>(row) * m_cols + col] = value;
        return true;
    }

    // Returns the value at (row, col), or empty string if invalid.
    std::wstring GetCellValue(int row, int col) const
    {
        if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
            return {};
        return m_grid[static_cast<size_t>(row) * m_cols + col];
    }

    // Computes cell layout for the given canvas size. Returns false if not configured.
    bool CalculateCellLayout(int canvasW, int canvasH)
    {
        if (m_rows <= 0 || m_cols <= 0 || canvasW <= 0 || canvasH <= 0)
            return false;

        m_canvasW = canvasW;
        m_canvasH = canvasH;

        // Estimate column widths based on content length
        m_colWidths = EstimateColumnWidths();

        // Compute total width units
        int totalUnits = 0;
        for (int w : m_colWidths)
            totalUnits += w;
        if (totalUnits <= 0)
            totalUnits = m_cols;

        // Row height: uniform distribution with header
        m_rowHeight = (std::max)(1, (canvasH - m_headerHeight) / m_rows);

        // Scale column widths to fit canvas
        m_scaledColWidths.resize(m_cols);
        const int usableW = canvasW - m_borderWidth * (m_cols + 1);
        for (int c = 0; c < m_cols; ++c) {
            m_scaledColWidths[c] = (std::max)(1, (m_colWidths[c] * usableW) / totalUnits);
        }

        m_layoutComputed = true;
        return true;
    }

    // Returns the number of cells visible in the current layout.
    int GetVisibleCellCount() const
    {
        if (!m_layoutComputed || m_rowHeight <= 0)
            return 0;
        const int visibleRows = (std::min)(m_rows, (std::max)(1, (m_canvasH - m_headerHeight) / m_rowHeight));
        return visibleRows * m_cols;
    }

    // Estimates column widths based on max content length per column.
    // Returns a vector of width-units (one per column).
    std::vector<int> EstimateColumnWidths() const
    {
        std::vector<int> widths(m_cols, 1);
        if (m_cols <= 0 || m_rows <= 0)
            return widths;

        for (int c = 0; c < m_cols; ++c) {
            int maxLen = 1;
            for (int r = 0; r < m_rows; ++r) {
                const auto& val = m_grid[static_cast<size_t>(r) * m_cols + c];
                maxLen = (std::max)(maxLen, static_cast<int>(val.size()));
            }
            widths[c] = (std::min)(maxLen, 40);  // cap at 40 chars
        }
        return widths;
    }

    // Gets the layout rect for a cell. Returns zero rect if layout not computed.
    CellLayoutRect GetCellRect(int row, int col) const
    {
        if (!m_layoutComputed || row < 0 || row >= m_rows || col < 0 || col >= m_cols)
            return {};

        CellLayoutRect rect;
        rect.x = m_borderWidth;
        for (int c = 0; c < col; ++c)
            rect.x += m_scaledColWidths[c] + m_borderWidth;

        rect.y = m_headerHeight + row * m_rowHeight;
        rect.width = m_scaledColWidths[col];
        rect.height = m_rowHeight - m_borderWidth;
        return rect;
    }

    // Gets total rows.
    int GetRows() const
    {
        return m_rows;
    }

    // Gets total columns.
    int GetCols() const
    {
        return m_cols;
    }

    // Sets the header row height in pixels. Returns false if negative.
    bool SetHeaderHeight(int h)
    {
        if (h < 0)
            return false;
        m_headerHeight = h;
        m_layoutComputed = false;
        return true;
    }

    // Returns true if layout has been computed.
    bool IsLayoutComputed() const
    {
        return m_layoutComputed;
    }

  private:
    int m_rows = 0;
    int m_cols = 0;
    int m_canvasW = 0;
    int m_canvasH = 0;
    int m_rowHeight = 0;
    int m_headerHeight = 20;
    int m_borderWidth = 1;
    bool m_layoutComputed = false;
    std::vector<std::wstring> m_grid;
    std::vector<int> m_colWidths;
    std::vector<int> m_scaledColWidths;
};

}  // namespace Engine
}  // namespace ExplorerLens
