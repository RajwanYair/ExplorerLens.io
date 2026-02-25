// FormatGalleryView.h — Visual Format Gallery with Live Previews
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 378
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a visual gallery view for the LENSManager showing sample
// thumbnails for each supported format. Uses the engine's own decode
// pipeline to render live preview tiles, organized by category.

#pragma once

#include <cstdint>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Gallery tile size presets
enum class GalleryTileSize : uint8_t {
    Small   = 64,    ///< 64x64 compact view
    Medium  = 128,   ///< 128x128 default
    Large   = 256,   ///< 256x256 detailed
    ExtraLarge = 512 ///< 512x512 full preview
};

/// Gallery sort order
enum class GallerySortOrder : uint8_t {
    ByCategory    = 0,   ///< Group by format category
    ByExtension   = 1,   ///< Alphabetical by extension
    ByDecoder     = 2,   ///< Group by decoder engine
    ByStatus      = 3,   ///< Group by health status
    ByPopularity  = 4    ///< Most-decoded formats first
};

/// Gallery tile representing one format
struct GalleryTile {
    const char* extension = nullptr;     ///< e.g., ".webp"
    const char* displayName = nullptr;   ///< e.g., "WebP Image"
    const char* decoderName = nullptr;   ///< e.g., "WebPDecoder"
    const char* categoryName = nullptr;  ///< e.g., "Images"
    HBITMAP hThumbnail = nullptr;        ///< Sample thumbnail bitmap
    uint32_t tileSize = 128;
    bool isEnabled = true;
    bool isSelected = false;
    bool isHovered = false;
    uint32_t decodeCount = 0;            ///< Usage statistics
    float avgDecodeMs = 0.0f;            ///< Average decode time
};

/// Gallery view configuration
struct GalleryConfig {
    GalleryTileSize tileSize = GalleryTileSize::Medium;
    GallerySortOrder sortOrder = GallerySortOrder::ByCategory;
    uint32_t tilesPerRow = 6;
    uint32_t tileSpacing = 8;
    bool showLabels = true;
    bool showStatus = true;              ///< Show green/yellow/red indicator
    bool showDecodeTime = false;         ///< Show avg decode time overlay
    bool enableDragDrop = false;         ///< Allow reordering
    bool animateHover = true;            ///< Scale animation on hover
    COLORREF backgroundColor = RGB(32, 32, 32);
    COLORREF selectionColor = RGB(0, 120, 215);
    COLORREF hoverColor = RGB(60, 60, 60);
    COLORREF labelColor = RGB(200, 200, 200);
};

/// Gallery view controller
class FormatGalleryView {
public:
    static FormatGalleryView& Instance() {
        static FormatGalleryView inst;
        return inst;
    }

    /// Initialize gallery with sample tiles
    void Initialize(const GalleryConfig& config = {}) {
        m_config = config;
        BuildDefaultTiles();
    }

    /// Get all tiles
    const GalleryTile* GetTiles() const { return m_tiles; }
    uint32_t GetTileCount() const { return m_tileCount; }

    /// Get configuration
    const GalleryConfig& GetConfig() const { return m_config; }
    void SetConfig(const GalleryConfig& config) { m_config = config; }

    /// Calculate layout dimensions
    uint32_t GetRowCount() const {
        if (m_config.tilesPerRow == 0) return 0;
        return (m_tileCount + m_config.tilesPerRow - 1) / m_config.tilesPerRow;
    }

    uint32_t GetTotalHeight() const {
        uint32_t tileH = static_cast<uint32_t>(m_config.tileSize) +
                         (m_config.showLabels ? 20 : 0);
        return GetRowCount() * (tileH + m_config.tileSpacing);
    }

    uint32_t GetTotalWidth() const {
        return m_config.tilesPerRow *
               (static_cast<uint32_t>(m_config.tileSize) + m_config.tileSpacing);
    }

    /// Hit test: which tile is at point (x,y) in client coordinates?
    int HitTest(int x, int y) const {
        uint32_t tileW = static_cast<uint32_t>(m_config.tileSize) + m_config.tileSpacing;
        uint32_t tileH = static_cast<uint32_t>(m_config.tileSize) +
                         (m_config.showLabels ? 20 : 0) + m_config.tileSpacing;

        if (tileW == 0 || tileH == 0 || m_config.tilesPerRow == 0) return -1;

        uint32_t col = static_cast<uint32_t>(x) / tileW;
        uint32_t row = static_cast<uint32_t>(y) / tileH;

        if (col >= m_config.tilesPerRow) return -1;
        uint32_t idx = row * m_config.tilesPerRow + col;
        return (idx < m_tileCount) ? static_cast<int>(idx) : -1;
    }

    /// Paint a single tile
    void PaintTile(HDC hdc, int index, int x, int y) const {
        if (index < 0 || static_cast<uint32_t>(index) >= m_tileCount) return;

        const auto& tile = m_tiles[index];
        uint32_t size = static_cast<uint32_t>(m_config.tileSize);

        // Background
        RECT tileRect = {
            static_cast<LONG>(x), static_cast<LONG>(y),
            static_cast<LONG>(x + size), static_cast<LONG>(y + size)
        };
        COLORREF bg = tile.isSelected ? m_config.selectionColor :
                      tile.isHovered ? m_config.hoverColor :
                      m_config.backgroundColor;
        HBRUSH brush = CreateSolidBrush(bg);
        FillRect(hdc, &tileRect, brush);
        DeleteObject(brush);

        // Border
        HPEN pen = CreatePen(PS_SOLID, 1,
                             tile.isSelected ? m_config.selectionColor : RGB(80, 80, 80));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, tileRect.left, tileRect.top, tileRect.right, tileRect.bottom);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        // Thumbnail bitmap
        if (tile.hThumbnail) {
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, tile.hThumbnail);
            // Center in tile
            int margin = 4;
            StretchBlt(hdc, x + margin, y + margin,
                      static_cast<int>(size) - margin * 2,
                      static_cast<int>(size) - margin * 2,
                      hdcMem, 0, 0, static_cast<int>(size), static_cast<int>(size),
                      SRCCOPY);
            SelectObject(hdcMem, oldBmp);
            DeleteDC(hdcMem);
        } else {
            // Placeholder: draw extension text centered
            if (tile.extension) {
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(100, 100, 100));
                wchar_t extW[16] = {};
                MultiByteToWideChar(CP_UTF8, 0, tile.extension, -1, extW, 16);
                ::DrawTextW(hdc, extW, -1, &tileRect,
                           DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }

        // Label below tile
        if (m_config.showLabels && tile.displayName) {
            RECT labelRect = {
                static_cast<LONG>(x),
                static_cast<LONG>(y + size + 2),
                static_cast<LONG>(x + size),
                static_cast<LONG>(y + size + 18)
            };
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, m_config.labelColor);
            wchar_t nameW[64] = {};
            MultiByteToWideChar(CP_UTF8, 0, tile.displayName, -1, nameW, 64);
            ::DrawTextW(hdc, nameW, -1, &labelRect,
                       DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
    }

    /// Set tile selection
    void SetSelection(int index) {
        for (uint32_t i = 0; i < m_tileCount; ++i)
            m_tiles[i].isSelected = (static_cast<int>(i) == index);
    }

    /// Set tile hover state
    void SetHover(int index) {
        for (uint32_t i = 0; i < m_tileCount; ++i)
            m_tiles[i].isHovered = (static_cast<int>(i) == index);
    }

    /// Cleanup bitmaps
    void Cleanup() {
        for (uint32_t i = 0; i < m_tileCount; ++i) {
            if (m_tiles[i].hThumbnail) {
                DeleteObject(m_tiles[i].hThumbnail);
                m_tiles[i].hThumbnail = nullptr;
            }
        }
        m_tileCount = 0;
    }

private:
    FormatGalleryView() = default;
    ~FormatGalleryView() { Cleanup(); }

    static constexpr uint32_t MAX_TILES = 256;

    void BuildDefaultTiles() {
        m_tileCount = 0;
        AddTile(".zip",  "ZIP Archive",     "ArchiveDecoder", "Archives");
        AddTile(".7z",   "7-Zip Archive",   "ArchiveDecoder", "Archives");
        AddTile(".rar",  "RAR Archive",     "ArchiveDecoder", "Archives");
        AddTile(".cbz",  "Comic Book ZIP",  "ArchiveDecoder", "Comics");
        AddTile(".cbr",  "Comic Book RAR",  "ArchiveDecoder", "Comics");
        AddTile(".epub", "EPUB eBook",      "EBookDecoder",   "eBooks");
        AddTile(".webp", "WebP Image",      "WebPDecoder",    "Images");
        AddTile(".avif", "AVIF Image",      "AVIFDecoder",    "Images");
        AddTile(".jxl",  "JPEG XL",         "JXLDecoder",     "Images");
        AddTile(".heif", "HEIF/HEIC",       "HEIFDecoder",    "Images");
        AddTile(".psd",  "Photoshop",       "PSDDecoder",     "Images");
        AddTile(".svg",  "SVG Vector",      "SVGDecoder",     "Images");
        AddTile(".cr2",  "Canon RAW",       "LibRawDecoder",  "RAW Photos");
        AddTile(".nef",  "Nikon RAW",       "LibRawDecoder",  "RAW Photos");
        AddTile(".arw",  "Sony RAW",        "LibRawDecoder",  "RAW Photos");
        AddTile(".dng",  "Adobe DNG",       "LibRawDecoder",  "RAW Photos");
        AddTile(".pdf",  "PDF Document",    "MuPDFDecoder",   "Documents");
        AddTile(".ttf",  "TrueType Font",   "FontDecoder",    "Fonts");
        AddTile(".otf",  "OpenType Font",   "FontDecoder",    "Fonts");
        AddTile(".stl",  "STL 3D Model",    "CADDecoder",     "3D Models");
        AddTile(".obj",  "Wavefront OBJ",   "CADDecoder",     "3D Models");
        AddTile(".gltf", "glTF Scene",      "glTFDecoder",    "3D Models");
        AddTile(".exr",  "OpenEXR HDR",     "HDRDecoder",     "Specialized");
        AddTile(".hdr",  "Radiance HDR",    "HDRDecoder",     "Specialized");
        AddTile(".dds",  "DirectDraw Srf",  "DDSDecoder",     "Specialized");
    }

    void AddTile(const char* ext, const char* name, const char* decoder, const char* cat) {
        if (m_tileCount >= MAX_TILES) return;
        auto& t = m_tiles[m_tileCount++];
        t.extension = ext;
        t.displayName = name;
        t.decoderName = decoder;
        t.categoryName = cat;
        t.hThumbnail = nullptr;
        t.tileSize = static_cast<uint32_t>(m_config.tileSize);
        t.isEnabled = true;
    }

    GalleryConfig m_config;
    GalleryTile m_tiles[MAX_TILES] = {};
    uint32_t m_tileCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
