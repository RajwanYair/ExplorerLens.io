// ThumbnailWatermarker.h — Thumbnail Watermark Overlay
// Copyright (c) 2026 ExplorerLens Project
//
// Adds configurable watermark overlays to thumbnails including format
// badges, file-size indicators, resolution tags, and custom enterprise
// branding overlays using alpha-blended GDI+ rendering.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class ThumbWatermarkType : uint8_t {
    FormatBadge, SizeBadge, ResolutionTag, StatusIcon, CustomBranding, None, COUNT
};

enum class ThumbWatermarkPosition : uint8_t {
    TopLeft, TopRight, BottomLeft, BottomRight, Center, COUNT
};

struct ThumbWatermarkConfig {
    ThumbWatermarkType type = ThumbWatermarkType::FormatBadge;
    ThumbWatermarkPosition position = ThumbWatermarkPosition::BottomRight;
    std::wstring text;
    float opacity = 0.8f;
    uint32_t fontSize = 10;
    uint32_t backgroundColor = 0x80000000; // Semi-transparent black
    uint32_t textColor = 0xFFFFFFFF; // White
    uint32_t paddingPx = 2;
    uint32_t cornerRadius = 3;
};

struct WatermarkResult {
    ThumbWatermarkType type = ThumbWatermarkType::None;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    bool rendered = false;
};

class ThumbnailWatermarker {
public:
    void Configure(const ThumbWatermarkConfig& cfg) { m_config = cfg; }
    const ThumbWatermarkConfig& GetConfig() const { return m_config; }

    WatermarkResult ComputePlacement(uint32_t thumbWidth, uint32_t thumbHeight) const {
        WatermarkResult result;
        result.type = m_config.type;
        result.width = static_cast<uint32_t>(m_config.text.length() * m_config.fontSize * 0.6f) + m_config.paddingPx * 2;
        result.height = m_config.fontSize + m_config.paddingPx * 2;

        switch (m_config.position) {
        case ThumbWatermarkPosition::TopLeft:
            result.x = m_config.paddingPx;
            result.y = m_config.paddingPx;
            break;
        case ThumbWatermarkPosition::TopRight:
            result.x = thumbWidth - result.width - m_config.paddingPx;
            result.y = m_config.paddingPx;
            break;
        case ThumbWatermarkPosition::BottomLeft:
            result.x = m_config.paddingPx;
            result.y = thumbHeight - result.height - m_config.paddingPx;
            break;
        case ThumbWatermarkPosition::BottomRight:
            result.x = thumbWidth - result.width - m_config.paddingPx;
            result.y = thumbHeight - result.height - m_config.paddingPx;
            break;
        case ThumbWatermarkPosition::Center:
            result.x = (thumbWidth - result.width) / 2;
            result.y = (thumbHeight - result.height) / 2;
            break;
        default: break;
        }
        result.rendered = (m_config.type != ThumbWatermarkType::None);
        return result;
    }

    bool IsEnabled() const { return m_config.type != ThumbWatermarkType::None; }

    static size_t TypeCount() { return static_cast<size_t>(ThumbWatermarkType::COUNT); }
    static size_t PositionCount() { return static_cast<size_t>(ThumbWatermarkPosition::COUNT); }

private:
    ThumbWatermarkConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
