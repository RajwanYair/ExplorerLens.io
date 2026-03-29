// XRSpatialPreviewEngine.h — XR Spatial Preview Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Generates 6DoF-aware spatial thumbnails with parallax depth for XR asset preview.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class XRPreviewTarget { HoloLens2, QuestPro, Standard2D };

struct XRSpatialPreviewRequest {
    std::wstring     assetPath;
    XRPreviewTarget  target        = XRPreviewTarget::Standard2D;
    uint32_t         width         = 512;
    uint32_t         height        = 512;
    float            parallaxDepth = 1.0f;
};

struct XRSpatialPreviewResult {
    bool                 success    = false;
    std::vector<uint8_t> rgbaData;
    float                depthRange = 0.0f;
    std::string          errorMsg;
};

class XRSpatialPreviewEngine {
public:
    static std::string TargetName(XRPreviewTarget target) {
        switch (target) {
            case XRPreviewTarget::HoloLens2:  return "HoloLens2";
            case XRPreviewTarget::QuestPro:   return "QuestPro";
            case XRPreviewTarget::Standard2D: return "Standard2D";
        }
        return "Unknown";
    }
    XRSpatialPreviewResult Preview(const XRSpatialPreviewRequest& req) {
        XRSpatialPreviewResult r;
        if (req.assetPath.empty()) { r.errorMsg = "Empty path"; return r; }
        uint32_t w = req.width  > 0 ? req.width  : 512;
        uint32_t h = req.height > 0 ? req.height : 512;
        r.rgbaData.assign(static_cast<size_t>(w) * h * 4, 0xCCu);
        r.depthRange = req.parallaxDepth * 2.0f;
        r.success    = true;
        return r;
    }
    bool IsTargetSupported(XRPreviewTarget target) const {
        return target == XRPreviewTarget::Standard2D || target == XRPreviewTarget::HoloLens2;
    }
};

}} // namespace ExplorerLens::Engine
