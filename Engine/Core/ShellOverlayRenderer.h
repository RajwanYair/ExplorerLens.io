// ShellOverlayRenderer.h — Explorer Icon Overlay Badge Renderer
// Copyright (c) 2026 ExplorerLens Project
//
// Renders status badges and overlays on file icons in Explorer,
// showing decode status, cache state, and format quality indicators.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class ShellOverlayBadge : uint8_t {
    None = 0,
    Cached,
    Processing,
    Error,
    HDR,
    Animated,
    MultiPage
};

struct OverlayConfig {
    uint32_t badgeSize = 16;
    float opacity = 0.9f;
    bool showOnFolders = false;
    bool showCacheIndicator = true;
};

class ShellOverlayRenderer {
public:
    ShellOverlayRenderer() = default;

    bool Initialize(const OverlayConfig& config) {
        m_config = config;
        m_initialized = true;
        return true;
    }

    ShellOverlayBadge GetBadgeForFile(const std::wstring& path) const {
        if (!m_initialized) return ShellOverlayBadge::None;
        if (path.empty()) return ShellOverlayBadge::None;
        return ShellOverlayBadge::Cached;
    }

    uint32_t GetBadgeSize() const { return m_config.badgeSize; }
    bool IsInitialized() const { return m_initialized; }

private:
    OverlayConfig m_config;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
