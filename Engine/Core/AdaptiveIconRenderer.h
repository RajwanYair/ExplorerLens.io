// AdaptiveIconRenderer.h — DPI-Aware Icon & Badge Renderer
// Copyright (c) 2026 ExplorerLens Project
//
// Renders format-type badges, codec icons, and status indicators on
// thumbnails at the correct DPI scale. Adapts icon size and placement
// based on thumbnail dimensions and display scaling factor.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class IconBadgeType : uint8_t {
    FormatType, CodecInfo, PlayButton, PageCount,
    Duration, FileSize, Warning, Encrypted, COUNT
};

struct IconBadgeConfig {
    IconBadgeType type = IconBadgeType::FormatType;
    uint8_t sizePx = 16;
    float opacity = 0.9f;
    bool useShadow = true;
    uint32_t tintColor = 0xFFFFFFFF;
};

struct IconRenderResult {
    bool rendered = false;
    int badgeCount = 0;
    int totalPixels = 0;
};

class AdaptiveIconRenderer {
public:
    void SetDPIScale(float scale) { m_dpiScale = scale; }
    float GetDPIScale() const { return m_dpiScale; }

    uint8_t ScaledSize(uint8_t baseSizePx) const {
        return static_cast<uint8_t>(baseSizePx * m_dpiScale);
    }

    IconRenderResult RenderBadge(const IconBadgeConfig& cfg,
        uint32_t thumbW, uint32_t thumbH) const {
        IconRenderResult r;
        r.rendered = (thumbW >= 32 && thumbH >= 32);
        r.badgeCount = r.rendered ? 1 : 0;
        r.totalPixels = r.badgeCount * ScaledSize(cfg.sizePx) * ScaledSize(cfg.sizePx);
        return r;
    }

    static const wchar_t* BadgeTypeName(IconBadgeType t) {
        switch (t) {
        case IconBadgeType::FormatType: return L"FormatType";
        case IconBadgeType::CodecInfo:  return L"CodecInfo";
        case IconBadgeType::PlayButton: return L"PlayButton";
        case IconBadgeType::PageCount:  return L"PageCount";
        case IconBadgeType::Duration:   return L"Duration";
        case IconBadgeType::FileSize:   return L"FileSize";
        case IconBadgeType::Warning:    return L"Warning";
        case IconBadgeType::Encrypted:  return L"Encrypted";
        default: return L"Unknown";
        }
    }
    static size_t BadgeTypeCount() { return static_cast<size_t>(IconBadgeType::COUNT); }

private:
    float m_dpiScale = 1.0f;
};

} // namespace Engine
} // namespace ExplorerLens
