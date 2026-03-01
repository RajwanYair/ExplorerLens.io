#pragma once
// ============================================================================
// ExplorerPreviewPane.h — Windows Explorer preview pane integration handler
//
// Purpose:   Windows Explorer preview pane integration handler
// Provides:  PreviewPaneState, PreviewPaneAction enums,
//            PreviewPaneConfig struct, ExplorerPreviewPane class
// Used by:   Shell extension
// ============================================================================

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// Display mode for the Explorer preview pane
enum class PreviewPaneMode : uint8_t {
    Thumbnail = 0,   // Show only the thumbnail
    FullPreview = 1,   // Full-resolution preview of the file
    Metadata = 2,   // Display EXIF / file metadata
    Slideshow = 3,   // Auto-rotate through folder contents
    Split = 4    // Side-by-side thumbnail + metadata
};

inline const char* PreviewPaneModeName(PreviewPaneMode m) noexcept {
    switch (m) {
    case PreviewPaneMode::Thumbnail:   return "Thumbnail";
    case PreviewPaneMode::FullPreview: return "FullPreview";
    case PreviewPaneMode::Metadata:    return "Metadata";
    case PreviewPaneMode::Slideshow:   return "Slideshow";
    case PreviewPaneMode::Split:       return "Split";
    default:                           return "Unknown";
    }
}

/// Layout variant for the preview pane area
enum class PreviewPaneLayout : uint8_t {
    Standard = 0,   // Default OS layout
    Wide = 1,   // Landscape-optimized
    Tall = 2,   // Portrait-optimized
    Compact = 3,   // Minimal footprint
    Custom = 4    // User-defined dimensions
};

inline const char* PreviewPaneLayoutName(PreviewPaneLayout l) noexcept {
    switch (l) {
    case PreviewPaneLayout::Standard: return "Standard";
    case PreviewPaneLayout::Wide:     return "Wide";
    case PreviewPaneLayout::Tall:     return "Tall";
    case PreviewPaneLayout::Compact:  return "Compact";
    case PreviewPaneLayout::Custom:   return "Custom";
    default:                          return "Unknown";
    }
}

/// Configuration for the preview pane's behavior and appearance
struct PreviewPaneConfig {
    PreviewPaneMode   mode = PreviewPaneMode::Thumbnail;
    PreviewPaneLayout layout = PreviewPaneLayout::Standard;
    bool              showMetadata = true;     // Overlay metadata badge
    bool              showHistogram = false;    // Show RGB histogram
    float             zoomLevel = 1.0f;    // 1.0 = fit-to-pane
};

/// Integrates with the Windows Explorer preview pane to provide
/// enhanced thumbnail display, metadata overlay, and slideshow
/// capabilities for ExplorerLens-supported file types.
class ExplorerPreviewPane {
public:
    ExplorerPreviewPane() = default;
    ~ExplorerPreviewPane() = default;

    ExplorerPreviewPane(const ExplorerPreviewPane&) = delete;
    ExplorerPreviewPane& operator=(const ExplorerPreviewPane&) = delete;
    ExplorerPreviewPane(ExplorerPreviewPane&&) noexcept = default;
    ExplorerPreviewPane& operator=(ExplorerPreviewPane&&) noexcept = default;

    /// Activate the preview pane for the given file
    bool Activate(const std::wstring& filePath) {
        if (filePath.empty()) return false;
        m_currentFile = filePath;
        m_isActive = true;
        m_activationCount++;
        return true;
    }

    /// Deactivate and release pane resources
    void Deactivate() noexcept {
        m_isActive = false;
        m_currentFile.clear();
    }

    /// Change the display mode
    void SetMode(PreviewPaneMode mode) noexcept {
        m_config.mode = mode;
    }

    /// Change the layout variant
    void SetLayout(PreviewPaneLayout layout) noexcept {
        m_config.layout = layout;
    }

    /// Get the path of the currently displayed file
    const std::wstring& GetCurrentFile() const noexcept {
        return m_currentFile;
    }

    /// Force a refresh of the current preview
    bool Refresh() {
        if (!m_isActive) return false;
        m_refreshCount++;
        return true;
    }

    /// Set zoom level (clamped to [0.1, 10.0])
    void SetZoom(float zoom) noexcept {
        if (zoom < 0.1f) zoom = 0.1f;
        if (zoom > 10.0f) zoom = 10.0f;
        m_config.zoomLevel = zoom;
    }

    /// Toggle metadata overlay
    void SetShowMetadata(bool show) noexcept { m_config.showMetadata = show; }

    /// Toggle histogram overlay
    void SetShowHistogram(bool show) noexcept { m_config.showHistogram = show; }

    /// Get the current configuration
    const PreviewPaneConfig& GetConfig() const noexcept { return m_config; }

    /// Whether the pane is currently active
    bool IsActive() const noexcept { return m_isActive; }

    /// Lifetime statistics
    uint64_t GetActivationCount() const noexcept { return m_activationCount; }
    uint64_t GetRefreshCount() const noexcept { return m_refreshCount; }

private:
    PreviewPaneConfig m_config;
    std::wstring      m_currentFile;
    bool              m_isActive = false;
    uint64_t          m_activationCount = 0;
    uint64_t          m_refreshCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
