// ShellBadgeRenderer.h — Draws overlay badges on thumbnail corners
// Copyright (c) 2026 ExplorerLens Project
//
// Renders format-type badges, quality indicators, and size overlays on
// thumbnail corners without modifying the source image data.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct ShellBadgeRendererConfig {
    bool enabled = true;
    uint32_t badgeSize = 16;
    uint32_t cornerPadding = 2;
    std::string label = "ShellBadgeRenderer";
};

class ShellBadgeRenderer {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ShellBadgeRendererConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Corner : uint8_t { TopLeft, TopRight, BottomLeft, BottomRight };
    enum class BadgeType : uint8_t { FormatIcon, QualityDot, SizeLabel, DurationLabel };

    struct Badge {
        Corner corner = Corner::BottomRight;
        BadgeType type = BadgeType::FormatIcon;
        std::string text;
        uint32_t color = 0xFF2196F3; // ARGB blue
    };

    bool AddBadge(const Badge& badge) {
        if (m_badgeCount >= 4) return false;
        m_badges[m_badgeCount++] = badge;
        return true;
    }

    uint32_t GetBadgeCount() const { return m_badgeCount; }
    void ClearBadges() { m_badgeCount = 0; }

    bool ShouldRender(uint32_t thumbWidth, uint32_t thumbHeight) const {
        return thumbWidth >= m_config.badgeSize * 3 && thumbHeight >= m_config.badgeSize * 3;
    }

private:
    bool m_initialized = false;
    ShellBadgeRendererConfig m_config;
    Badge m_badges[4] = {};
    uint32_t m_badgeCount = 0;
};

}
} // namespace ExplorerLens::Engine
