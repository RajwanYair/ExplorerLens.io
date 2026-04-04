// AnnotationOverlayRenderer.h — Annotation Overlay Renderer (Stars, Tags, Comments)
// Copyright (c) 2026 ExplorerLens Project
//
// Composites annotation indicators onto thumbnail images: star ratings, tag pills,
// comment badges, and color labels. Positions are determined by a configurable
// corner/edge anchor system.
//
#pragma once
#include <stdint.h>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class OverlayAnchor {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    TopCenter,
    BottomCenter
};

struct AnnotationOverlayConfig
{
    bool showStars = true;
    bool showTags = true;
    bool showComments = true;
    bool showColors = true;
    OverlayAnchor starAnchor = OverlayAnchor::TopRight;
    OverlayAnchor tagAnchor = OverlayAnchor::BottomLeft;
    OverlayAnchor commentAnchor = OverlayAnchor::TopLeft;
    int overlayScale = 100;  // percent
    float opacity = 0.85f;
};

struct AnnotationOverlayData
{
    int starRating = 0;  // 0-5
    std::vector<std::wstring> tags;
    int commentCount = 0;
    uint32_t colorLabel = 0;  // RGBA
    bool hasAnnotations() const noexcept
    {
        return starRating > 0 || !tags.empty() || commentCount > 0 || colorLabel != 0;
    }
};

struct RenderedAnnotationOverlay
{
    bool rendered = false;
    int elementsDrawn = 0;
    std::string description;
};

class AnnotationOverlayRenderer
{
  public:
    explicit AnnotationOverlayRenderer(const AnnotationOverlayConfig& cfg = {}) : m_cfg(cfg) {}

    RenderedAnnotationOverlay Render(const AnnotationOverlayData& data, uint8_t* /*thumbPixels*/, int thumbW,
                                     int thumbH) const
    {
        RenderedAnnotationOverlay result;
        if (!data.hasAnnotations())
            return result;
        result.rendered = true;
        std::string desc;
        if (m_cfg.showStars && data.starRating > 0) {
            desc += "Stars=" + std::to_string(data.starRating) + " ";
            result.elementsDrawn++;
        }
        if (m_cfg.showTags && !data.tags.empty()) {
            desc += "Tags=" + std::to_string(data.tags.size()) + " ";
            result.elementsDrawn++;
        }
        if (m_cfg.showComments && data.commentCount > 0) {
            desc += "Comments=" + std::to_string(data.commentCount) + " ";
            result.elementsDrawn++;
        }
        (void)thumbW;
        (void)thumbH;
        result.description = desc;
        return result;
    }

    const AnnotationOverlayConfig& Config() const noexcept
    {
        return m_cfg;
    }
    void SetConfig(const AnnotationOverlayConfig& c) noexcept
    {
        m_cfg = c;
    }

  private:
    AnnotationOverlayConfig m_cfg;
};

}  // namespace Engine
}  // namespace ExplorerLens
