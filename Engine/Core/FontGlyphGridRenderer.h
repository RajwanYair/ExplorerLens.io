// FontGlyphGridRenderer.h — Font Glyph Grid Layout Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Computes grid layouts for displaying font glyphs in thumbnails of
// .ttf/.otf files. Returns cell rectangles and display character sets
// with no GDI or DirectWrite calls — pure layout calculations.
//
#pragma once

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Rectangle for a single glyph cell in the grid
struct GlyphCellRect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

class FontGlyphGridRenderer
{
  public:
    FontGlyphGridRenderer() = default;
    ~FontGlyphGridRenderer() = default;

    // Sets the grid dimensions (columns x rows). Returns false if invalid.
    bool SetGridDimensions(int cols, int rows)
    {
        if (cols <= 0 || rows <= 0 || cols > 64 || rows > 64)
            return false;
        m_cols = cols;
        m_rows = rows;
        m_cells.clear();
        m_cells.resize(static_cast<size_t>(cols) * rows);
        return true;
    }

    // Returns a representative set of display characters for font preview.
    // Includes uppercase, lowercase, digits, and common punctuation.
    std::wstring GetDisplayCharacters() const
    {
        // Standard character set for font preview thumbnails
        std::wstring chars;
        // Uppercase A-Z
        for (wchar_t c = L'A'; c <= L'Z'; ++c)
            chars += c;
        // Lowercase a-z
        for (wchar_t c = L'a'; c <= L'z'; ++c)
            chars += c;
        // Digits 0-9
        for (wchar_t c = L'0'; c <= L'9'; ++c)
            chars += c;
        // Common punctuation
        chars += L"!@#$%&*()";

        const int capacity = m_cols * m_rows;
        if (capacity > 0 && static_cast<int>(chars.size()) > capacity)
            chars.resize(static_cast<size_t>(capacity));

        return chars;
    }

    // Calculates the cell size for a given canvas. Returns {0,0} if invalid.
    SIZE CalculateCellSize(int canvasW, int canvasH) const
    {
        SIZE result = {0, 0};
        if (m_cols <= 0 || m_rows <= 0 || canvasW <= 0 || canvasH <= 0)
            return result;

        const int totalPadX = m_padding * (m_cols + 1);
        const int totalPadY = m_padding * (m_rows + 1);
        const int availW = (std::max)(1, canvasW - totalPadX);
        const int availH = (std::max)(1, canvasH - totalPadY);

        result.cx = availW / m_cols;
        result.cy = availH / m_rows;
        return result;
    }

    // Returns the bounding rect for a character at the given flat index.
    GlyphCellRect GetCellRect(int charIndex) const
    {
        if (charIndex < 0 || charIndex >= m_cols * m_rows)
            return {};

        SIZE cellSize = CalculateCellSize(m_canvasW, m_canvasH);
        if (cellSize.cx <= 0 || cellSize.cy <= 0)
            return {};

        const int col = charIndex % m_cols;
        const int row = charIndex / m_cols;

        GlyphCellRect rect;
        rect.x = m_padding + col * (static_cast<int>(cellSize.cx) + m_padding);
        rect.y = m_padding + row * (static_cast<int>(cellSize.cy) + m_padding);
        rect.width = static_cast<int>(cellSize.cx);
        rect.height = static_cast<int>(cellSize.cy);
        return rect;
    }

    // Checks if the given file extension is a supported font format.
    // Supports: .ttf, .otf, .ttc, .woff, .woff2, .fon
    static bool IsSupportedFontFormat(const std::wstring& ext)
    {
        if (ext.empty())
            return false;

        // Normalize: ensure leading dot and lowercase
        std::wstring normalized = ext;
        if (normalized[0] != L'.')
            normalized = L"." + normalized;
        for (auto& ch : normalized)
            ch = static_cast<wchar_t>(towlower(ch));

        return (normalized == L".ttf" || normalized == L".otf" || normalized == L".ttc" || normalized == L".woff"
                || normalized == L".woff2" || normalized == L".fon");
    }

    // Sets the canvas size for layout calculations. Returns false if invalid.
    bool SetCanvasSize(int w, int h)
    {
        if (w <= 0 || h <= 0)
            return false;
        m_canvasW = w;
        m_canvasH = h;
        return true;
    }

    // Sets padding between cells. Returns false if negative.
    bool SetPadding(int padding)
    {
        if (padding < 0)
            return false;
        m_padding = padding;
        return true;
    }

    // Returns the total number of cells in the grid.
    int GetCellCount() const
    {
        return m_cols * m_rows;
    }

    int GetCols() const
    {
        return m_cols;
    }
    int GetRows() const
    {
        return m_rows;
    }

  private:
    int m_cols = 0;
    int m_rows = 0;
    int m_padding = 2;
    int m_canvasW = 256;
    int m_canvasH = 256;
    std::vector<GlyphCellRect> m_cells;
};

}  // namespace Engine
}  // namespace ExplorerLens
