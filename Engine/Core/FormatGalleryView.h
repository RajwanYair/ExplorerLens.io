// FormatGalleryView.h — Gallery-style format browsing configuration
// Copyright (c) 2026 ExplorerLens Project
//
// Singleton that manages gallery view settings including tile sizes,
// sort ordering, and initialization for the format browser UI.
//
#pragma once

#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class GalleryTileSize : uint32_t {
    Small      = 64,
    Medium     = 128,
    Large      = 256,
    ExtraLarge = 512
};

enum class GallerySortOrder : uint8_t {
    ByCategory   = 0,
    ByName       = 1,
    ByDate       = 2,
    BySize       = 3,
    ByPopularity = 4
};

struct GalleryViewConfig {
    GalleryTileSize tileSize = GalleryTileSize::Medium;
    GallerySortOrder sortOrder = GallerySortOrder::ByCategory;
    bool showLabels = true;
    bool enableAnimations = true;
};

class FormatGalleryView {
public:
    static FormatGalleryView& Instance() noexcept {
        static FormatGalleryView inst;
        return inst;
    }

    void Initialize(const GalleryViewConfig& config) { m_config = config; }
    GalleryViewConfig GetConfig() const noexcept { return m_config; }

private:
    FormatGalleryView() = default;
    GalleryViewConfig m_config{};
};

// Lightweight in-process preview engine for thumbnail zoom/pan
class ThumbnailPreviewEngine {
public:
    struct State {
        float zoomLevel = 1.0f;
        bool imageLoaded = false;
    };

    bool LoadImage(const uint8_t* rgba, uint32_t w, uint32_t h) {
        if (!rgba || w == 0 || h == 0) return false;
        m_state.imageLoaded = true;
        return true;
    }

    void SetZoom(float z) { m_state.zoomLevel = (z < 0.1f) ? 0.1f : ((z > 10.0f) ? 10.0f : z); }
    State GetState() const noexcept { return m_state; }

private:
    State m_state{};
};

} // namespace Engine
} // namespace ExplorerLens
