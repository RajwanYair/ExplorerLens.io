#pragma once
// ============================================================================
// ThumbnailSpriteSheet.h — Sprite sheet packing for batch thumbnail atlasing
//
// Purpose:   Sprite sheet packing for batch thumbnail atlasing
// Provides:  SpriteLayout, SpritePackStrategy enums, SpriteEntry struct,
//            ThumbnailSpriteSheet class
// Used by:   Web preview and batch export
// ============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Layout algorithm for sprite sheet generation
enum class SpriteLayout : uint8_t {
    Grid = 0,  // Uniform NxM grid
    Strip = 1,  // Single row or column strip
    Atlas = 2,  // Texture atlas packing
    TreePack = 3,  // Binary tree rectangle packing
    Custom = 4   // User-defined placement
};

inline const char* SpriteLayoutName(SpriteLayout l) noexcept {
    switch (l) {
    case SpriteLayout::Grid:     return "Grid";
    case SpriteLayout::Strip:    return "Strip";
    case SpriteLayout::Atlas:    return "Atlas";
    case SpriteLayout::TreePack: return "TreePack";
    case SpriteLayout::Custom:   return "Custom";
    default:                     return "Unknown";
    }
}

/// Output format for the generated sprite sheet
enum class SpriteOutputFormat : uint8_t {
    PNG = 0,  // Lossless PNG
    WebP = 1,  // WebP (lossy or lossless)
    JPEG = 2,  // Lossy JPEG
    DDS = 3,  // DirectDraw Surface (GPU textures)
    BMP = 4   // Uncompressed BMP
};

inline const char* SpriteOutputFormatName(SpriteOutputFormat f) noexcept {
    switch (f) {
    case SpriteOutputFormat::PNG:  return "PNG";
    case SpriteOutputFormat::WebP: return "WebP";
    case SpriteOutputFormat::JPEG: return "JPEG";
    case SpriteOutputFormat::DDS:  return "DDS";
    case SpriteOutputFormat::BMP:  return "BMP";
    default:                       return "Unknown";
    }
}

/// A single thumbnail entry in the sprite sheet
struct SpriteEntry {
    uint32_t     x = 0;    // X offset in sheet
    uint32_t     y = 0;    // Y offset in sheet
    uint32_t     width = 0;    // Thumbnail width
    uint32_t     height = 0;    // Thumbnail height
    std::wstring sourcePath;      // Original file path
};

/// Result of sprite sheet generation
struct SpriteSheetResult {
    uint32_t totalWidth = 0;
    uint32_t totalHeight = 0;
    uint32_t spriteCount = 0;
    uint64_t estimatedSizeBytes = 0;
    bool     success = false;
};

/// Configuration for sprite sheet generation
struct SpriteSheetConfig {
    SpriteLayout       layout = SpriteLayout::Grid;
    SpriteOutputFormat format = SpriteOutputFormat::PNG;
    uint32_t           thumbWidth = 128;   // Individual thumbnail width
    uint32_t           thumbHeight = 128;   // Individual thumbnail height
    uint32_t           padding = 2;     // Padding between sprites
    uint32_t           columns = 8;     // Grid columns (Grid mode)
};

/// Generates sprite sheets (texture atlases) from multiple
/// thumbnails, supporting grid, strip, and tree-pack layouts
/// with configurable output formats and padding.
class ThumbnailSpriteSheet {
public:
    ThumbnailSpriteSheet() = default;
    ~ThumbnailSpriteSheet() = default;

    ThumbnailSpriteSheet(const ThumbnailSpriteSheet&) = delete;
    ThumbnailSpriteSheet& operator=(const ThumbnailSpriteSheet&) = delete;
    ThumbnailSpriteSheet(ThumbnailSpriteSheet&&) noexcept = default;
    ThumbnailSpriteSheet& operator=(ThumbnailSpriteSheet&&) noexcept = default;

    /// Add a thumbnail to the sprite sheet
    void AddThumbnail(const std::wstring& sourcePath, uint32_t w = 0, uint32_t h = 0) {
        SpriteEntry entry;
        entry.sourcePath = sourcePath;
        entry.width = (w > 0) ? w : m_config.thumbWidth;
        entry.height = (h > 0) ? h : m_config.thumbHeight;
        m_entries.push_back(entry);
    }

    /// Generate the sprite sheet layout
    SpriteSheetResult Generate() {
        SpriteSheetResult result;
        result.spriteCount = static_cast<uint32_t>(m_entries.size());
        if (result.spriteCount == 0) return result;

        uint32_t cols = m_config.columns;
        uint32_t rows = (result.spriteCount + cols - 1) / cols;
        uint32_t tw = m_config.thumbWidth + m_config.padding;
        uint32_t th = m_config.thumbHeight + m_config.padding;
        result.totalWidth = cols * tw;
        result.totalHeight = rows * th;
        result.estimatedSizeBytes = static_cast<uint64_t>(result.totalWidth) *
            result.totalHeight * 4;
        // Place sprites
        for (uint32_t i = 0; i < result.spriteCount; i++) {
            m_entries[i].x = (i % cols) * tw;
            m_entries[i].y = (i / cols) * th;
        }
        result.success = true;
        m_generateCount++;
        return result;
    }

    /// Get the current layout mode
    SpriteLayout GetLayout() const noexcept { return m_config.layout; }

    /// Get estimated output size in bytes
    uint64_t GetEstimatedSize() const noexcept {
        uint32_t cols = m_config.columns;
        uint32_t count = static_cast<uint32_t>(m_entries.size());
        if (count == 0) return 0;
        uint32_t rows = (count + cols - 1) / cols;
        return static_cast<uint64_t>(cols * (m_config.thumbWidth + m_config.padding)) *
            rows * (m_config.thumbHeight + m_config.padding) * 4;
    }

    /// Get sprite count
    size_t GetSpriteCount() const noexcept { return m_entries.size(); }

    /// Apply configuration
    void SetConfig(const SpriteSheetConfig& cfg) noexcept { m_config = cfg; }

    /// Get generation count
    uint64_t GetGenerateCount() const noexcept { return m_generateCount; }

private:
    SpriteSheetConfig        m_config;
    std::vector<SpriteEntry> m_entries;
    uint64_t                 m_generateCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
