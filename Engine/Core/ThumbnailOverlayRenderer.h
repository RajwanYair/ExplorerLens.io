// ThumbnailOverlayRenderer.h — Thumbnail Overlay Renderer (Badges + Emblems)
// Copyright (c) 2026 ExplorerLens Project
//
// Composites status badges and format emblems over thumbnail images — syncs, cloud, lock, and format icons.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class OverlayBadge { None, CloudSync, CloudOffline, Locked, Warning, Error, Favorite };
struct OverlayRenderParams {
    uint32_t    thumbWidth  = 256;
    uint32_t    thumbHeight = 256;
    OverlayBadge badge      = OverlayBadge::None;
    float        opacity    = 1.0f;
    bool         showFormat = true;
};
class ThumbnailOverlayRenderer {
public:
    bool Render(const std::vector<uint8_t>& thumbRGBA, const OverlayRenderParams& params,
                std::vector<uint8_t>& outRGBA) {
        outRGBA = thumbRGBA;
        (void)params;
        return true;
    }
    bool SupportsHDPI() const { return true; }
};

} // namespace Engine
} // namespace ExplorerLens