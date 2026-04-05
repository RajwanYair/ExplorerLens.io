// ThumbnailRenderer.h — Unified Thumbnail Rendering, Overlay & Visual Effects
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header consolidating: ThumbnailOverlayRenderer.h, ThumbnailSpriteSheet.h,
// ThumbnailStitcher.h, ThumbnailWatermark.h, ThumbnailWatermarker.h,
// ThumbnailRotationCorrector.h, ThumbnailAnnotation.h, ThumbnailLensEffect.h,
// ThumbnailAnimator.h, ThumbnailAnimationEngineV2.h.
// Part of v31.2.0 consolidation pass.
//
#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// -- Overlay badges (from OverlayRenderer) ------------------------------------

enum class OverlayBadge : uint8_t {
    None,
    CloudSync,
    CloudOffline,
    Locked,
    Warning,
    Error,
    Favorite
};

// -- Annotation types (from Annotation) ---------------------------------------

enum class AnnotationType : uint8_t {
    Resolution,
    FileSize,
    Format,
    Duration,
    PageCount
};

enum class AnnotationPosition : uint8_t {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Center
};

enum class AnnotationStyle : uint8_t {
    Plain,
    Corner,
    Badge,
    Overlay,
    Tooltip
};

inline const char* AnnotationTypeName(AnnotationType t) noexcept
{
    switch (t) {
        case AnnotationType::Resolution:
            return "Resolution";
        case AnnotationType::FileSize:
            return "FileSize";
        case AnnotationType::Format:
            return "Format";
        case AnnotationType::Duration:
            return "Duration";
        case AnnotationType::PageCount:
            return "PageCount";
        default:
            return "Unknown";
    }
}

inline const char* AnnotationStyleName(AnnotationStyle s) noexcept
{
    switch (s) {
        case AnnotationStyle::Plain:
            return "Plain";
        case AnnotationStyle::Corner:
            return "Corner";
        case AnnotationStyle::Badge:
            return "Badge";
        case AnnotationStyle::Overlay:
            return "Overlay";
        case AnnotationStyle::Tooltip:
            return "Tooltip";
        default:
            return "Unknown";
    }
}

// -- Watermark (from Watermark + Watermarker) ---------------------------------

enum class WatermarkPosition : uint8_t {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Center
};

enum class WatermarkType : uint8_t {
    FormatBadge,
    SizeBadge,
    ResolutionTag,
    StatusIcon,
    CustomBranding,
    Text,
    QRCode,
    None
};

inline const char* WatermarkPositionName(WatermarkPosition p) noexcept
{
    switch (p) {
        case WatermarkPosition::TopLeft:
            return "TopLeft";
        case WatermarkPosition::TopRight:
            return "TopRight";
        case WatermarkPosition::BottomLeft:
            return "BottomLeft";
        case WatermarkPosition::BottomRight:
            return "BottomRight";
        case WatermarkPosition::Center:
            return "Center";
        default:
            return "Unknown";
    }
}

inline const char* WatermarkTypeName(WatermarkType t) noexcept
{
    switch (t) {
        case WatermarkType::FormatBadge:
            return "FormatBadge";
        case WatermarkType::SizeBadge:
            return "SizeBadge";
        case WatermarkType::ResolutionTag:
            return "ResolutionTag";
        case WatermarkType::StatusIcon:
            return "StatusIcon";
        case WatermarkType::CustomBranding:
            return "CustomBranding";
        case WatermarkType::Text:
            return "Text";
        case WatermarkType::QRCode:
            return "QRCode";
        case WatermarkType::None:
            return "None";
        default:
            return "Unknown";
    }
}

// -- Sprite layout (from SpriteSheet) -----------------------------------------

enum class SpriteLayout : uint8_t {
    Grid,
    Strip,
    Atlas,
    TreePack,
    Custom
};

// -- Stitch orientation (from Stitcher) ---------------------------------------

enum class StitchOrientation : uint8_t {
    Horizontal,
    Vertical,
    Grid
};

// -- Animation (from Animator + AnimationEngineV2) — enums in ThumbnailAnimationEngineV2.h --
// -- Structs ------------------------------------------------------------------

struct OverlayRenderParams
{
    uint32_t thumbWidth = 256;
    uint32_t thumbHeight = 256;
    OverlayBadge badge = OverlayBadge::None;
    float opacity = 1.0f;
    bool showFormat = true;
};

struct WatermarkConfig
{
    WatermarkType type = WatermarkType::None;
    WatermarkPosition position = WatermarkPosition::BottomRight;
    std::string text;
    float opacity = 0.8f;
    uint32_t fontSize = 10;
};

class ThumbnailWatermark
{
  public:
    void SetConfig(const WatermarkConfig& cfg) noexcept
    {
        m_config = cfg;
    }
    bool IsActive() const noexcept
    {
        return m_config.type != WatermarkType::None;
    }
    bool Apply(uint8_t* pixels, uint32_t w, uint32_t h) noexcept
    {
        if (!IsActive() || !pixels || w == 0 || h == 0)
            return false;
        ++m_appliedCount;
        return true;
    }
    uint32_t GetAppliedCount() const noexcept
    {
        return m_appliedCount;
    }
    static std::vector<WatermarkType> GetSupportedTypes()
    {
        return {WatermarkType::FormatBadge, WatermarkType::SizeBadge, WatermarkType::Text, WatermarkType::QRCode};
    }

  private:
    WatermarkConfig m_config;
    uint32_t m_appliedCount = 0;
};

struct AnnotationConfig
{
    AnnotationType type = AnnotationType::Format;
    AnnotationStyle style = AnnotationStyle::Plain;
    AnnotationPosition position = AnnotationPosition::BottomLeft;
    std::string text;
    float opacity = 1.0f;
    uint32_t fontSize = 10;
};

class ThumbnailAnnotation
{
  public:
    static constexpr size_t MAX_ANNOTATIONS = 5;
    bool AddAnnotation(const AnnotationConfig& cfg)
    {
        if (IsFull())
            return false;
        m_annotations.push_back(cfg);
        return true;
    }
    bool RemoveAnnotation(size_t index)
    {
        if (index >= m_annotations.size())
            return false;
        m_annotations.erase(m_annotations.begin() + static_cast<ptrdiff_t>(index));
        return true;
    }
    bool IsFull() const noexcept
    {
        return m_annotations.size() >= MAX_ANNOTATIONS;
    }
    size_t GetAnnotationCount() const noexcept
    {
        return m_annotations.size();
    }
    void Clear() noexcept
    {
        m_annotations.clear();
    }
    bool Render(uint8_t* pixels, uint32_t w, uint32_t h) const noexcept
    {
        if (!pixels || w == 0 || h == 0)
            return false;
        return true;
    }

  private:
    std::vector<AnnotationConfig> m_annotations;
};

struct SpriteEntry
{
    uint32_t x = 0, y = 0, w = 0, h = 0;
    uint32_t index = 0;
};

struct StitchRect
{
    int32_t x = 0, y = 0;
    uint32_t w = 0, h = 0;
};

struct LensRect
{
    int32_t left = 0, top = 0, right = 0, bottom = 0;
    int32_t Width() const noexcept
    {
        return right - left;
    }
    int32_t Height() const noexcept
    {
        return bottom - top;
    }
};

struct AnimFrame
{
    uint32_t width = 0, height = 0;
    uint32_t delayMs = 100;
    uint32_t frameIndex = 0;
    bool isKeyFrame = false;
};

// -- Unified visual renderer --------------------------------------------------

class ThumbnailVisualRenderer
{
  public:
    bool ApplyOverlay(const std::vector<uint8_t>& thumbRGBA, const OverlayRenderParams& params,
                      std::vector<uint8_t>& outRGBA) const
    {
        outRGBA = thumbRGBA;
        (void)params;
        return !thumbRGBA.empty();
    }

    std::vector<StitchRect> ComputeStitchLayout(const std::vector<std::pair<uint32_t, uint32_t>>& segments,
                                                StitchOrientation orient = StitchOrientation::Horizontal,
                                                uint32_t padding = 2) const
    {
        std::vector<StitchRect> rects;
        int32_t offset = 0;
        for (auto& [w, h] : segments) {
            StitchRect r;
            r.x = (orient == StitchOrientation::Vertical) ? 0 : offset;
            r.y = (orient == StitchOrientation::Vertical) ? offset : 0;
            r.w = w;
            r.h = h;
            rects.push_back(r);
            offset += static_cast<int32_t>((orient == StitchOrientation::Vertical ? h : w) + padding);
        }
        return rects;
    }

    std::vector<SpriteEntry> PackSprites(uint32_t count, uint32_t cellW, uint32_t cellH,
                                         SpriteLayout layout = SpriteLayout::Grid) const
    {
        std::vector<SpriteEntry> entries;
        uint32_t cols = static_cast<uint32_t>(std::ceil(std::sqrt(static_cast<double>(count))));
        for (uint32_t i = 0; i < count; ++i) {
            SpriteEntry e;
            e.index = i;
            if (layout == SpriteLayout::Strip) {
                e.x = i * cellW;
                e.y = 0;
            } else {
                e.x = (i % cols) * cellW;
                e.y = (i / cols) * cellH;
            }
            e.w = cellW;
            e.h = cellH;
            entries.push_back(e);
        }
        return entries;
    }

    void ApplyLensEffect(double cx, double cy, double radius, double power, double srcX, double srcY, double& outX,
                         double& outY) const
    {
        double dx = srcX - cx, dy = srcY - cy;
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist >= radius || dist < 1e-6) {
            outX = srcX;
            outY = srcY;
            return;
        }
        double t = dist / radius;
        double mapped = std::pow(t, 1.0 / power) * radius;
        outX = cx + dx * (mapped / dist);
        outY = cy + dy * (mapped / dist);
    }

    uint32_t SelectKeyFrames(uint32_t totalFrames, uint32_t maxKeys = 4) const
    {
        if (totalFrames == 0)
            return 0;
        return (totalFrames < maxKeys) ? totalFrames : maxKeys;
    }

  private:
    // Rotation lookup: EXIF orientation tag (1-8) → {flipH, flipV, rotate90CW}
    static constexpr bool EXIF_FLIP_H[9] = {false, false, true, false, true, false, true, false, true};
    static constexpr bool EXIF_FLIP_V[9] = {false, false, false, true, true, true, false, true, false};
    static constexpr bool EXIF_ROT90[9] = {false, false, false, false, false, true, true, true, true};
};

}  // namespace Engine
}  // namespace ExplorerLens
