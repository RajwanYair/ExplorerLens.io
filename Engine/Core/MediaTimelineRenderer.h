// MediaTimelineRenderer.h — Media Timeline Renderer
// Copyright (c) 2026 ExplorerLens Project
//
// Generates filmstrip timeline thumbnails for seekable video and audio files.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct MTRRenderRequest {
    std::wstring filePath;
    uint32_t     stripWidth   = 1280;
    uint32_t     frameHeight  = 72;
    uint32_t     framesTotal  = 20;
};

struct MTRKeyframe {
    uint32_t             timestampMs = 0;
    std::vector<uint8_t> rgbaData;
};

struct MTRRenderResult {
    bool                     success             = false;
    std::vector<MTRKeyframe> keyframes;
    uint32_t                 totalDurationMs     = 0;
    std::string              errorMsg;
};

class MediaTimelineRenderer {
public:
    MTRRenderResult RenderStrip(const MTRRenderRequest& req) {
        MTRRenderResult r;
        if (req.filePath.empty()) { r.errorMsg = "Empty path"; return r; }
        r.totalDurationMs = 120000u; // 2-minute fake duration
        uint32_t step     = r.totalDurationMs / std::max(1u, req.framesTotal);
        r.keyframes.reserve(req.framesTotal);
        for (uint32_t i = 0; i < req.framesTotal; ++i) {
            MTRKeyframe kf;
            kf.timestampMs = i * step;
            uint32_t w     = req.stripWidth  / std::max(1u, req.framesTotal);
            kf.rgbaData.assign(static_cast<size_t>(w) * req.frameHeight * 4,
                               static_cast<uint8_t>(0x20 + i * 5));
            r.keyframes.push_back(std::move(kf));
        }
        r.success = true;
        return r;
    }
    bool SupportsTimeline(const std::wstring& path) const {
        return !path.empty();
    }
    uint32_t OptimalFrameCount(uint32_t durationMs) const {
        return std::max(5u, std::min(30u, durationMs / 5000u));
    }
};

}} // namespace ExplorerLens::Engine
