// IconBadgeRenderer.h — Format and Status Icon Badge Overlay
// Copyright (c) 2026 ExplorerLens Project
//
// Renders small icon badges on thumbnails to indicate file format,
// decoder type (GPU/CPU), error status, or metadata properties.
// Uses scalable vector-style rendering with DPI awareness.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class BadgeType : uint8_t {
    FormatIcon,
    DecoderType,
    ErrorStatus,
    ProtectedContent,
    HDRContent,
    AnimatedContent,
    MultiPage,
    COUNT
};

enum class BadgeShape : uint8_t {
    Circle,
    RoundedRect,
    Pill,
    Diamond,
    COUNT
};

struct BadgeStyle
{
    BadgeShape shape = BadgeShape::RoundedRect;
    uint32_t backgroundColor = 0xCC000000;
    uint32_t foregroundColor = 0xFFFFFFFF;
    uint32_t borderColor = 0x40FFFFFF;
    float borderWidth = 1.0f;
    float cornerRadius = 4.0f;
    float opacity = 0.9f;
};

struct BadgeConfig
{
    BadgeType type = BadgeType::FormatIcon;
    BadgeStyle style;
    std::wstring label;
    uint32_t sizePx = 16;
    uint32_t marginPx = 2;
    bool dpiAware = true;
    float dpiScale = 1.0f;
};

struct BadgePlacement
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    float effectiveDPI = 96.0f;
    bool visible = false;
};

class IconBadgeRenderer
{
  public:
    void SetStyle(const BadgeStyle& style)
    {
        m_style = style;
    }
    const BadgeStyle& GetStyle() const
    {
        return m_style;
    }

    BadgePlacement ComputePlacement(const BadgeConfig& cfg, uint32_t thumbWidth, uint32_t thumbHeight) const
    {
        BadgePlacement p;
        uint32_t size = cfg.dpiAware ? static_cast<uint32_t>(cfg.sizePx * cfg.dpiScale) : cfg.sizePx;
        uint32_t margin = cfg.dpiAware ? static_cast<uint32_t>(cfg.marginPx * cfg.dpiScale) : cfg.marginPx;

        p.width = size;
        p.height = size;
        p.x = thumbWidth - size - margin;
        p.y = thumbHeight - size - margin;
        p.effectiveDPI = 96.0f * cfg.dpiScale;
        p.visible = (thumbWidth >= size * 2 && thumbHeight >= size * 2);
        return p;
    }

    bool ShouldRenderBadge(uint32_t thumbWidth, uint32_t thumbHeight, uint32_t minThumbSize = 48) const
    {
        return (thumbWidth >= minThumbSize && thumbHeight >= minThumbSize);
    }

    uint32_t ScaledSize(uint32_t basePx, float dpiScale) const
    {
        return static_cast<uint32_t>(basePx * dpiScale);
    }

    static size_t TypeCount()
    {
        return static_cast<size_t>(BadgeType::COUNT);
    }
    static size_t ShapeCount()
    {
        return static_cast<size_t>(BadgeShape::COUNT);
    }

  private:
    BadgeStyle m_style;
};

}  // namespace Engine
}  // namespace ExplorerLens
