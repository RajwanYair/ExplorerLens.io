// ThumbnailSizeNegotiator.h — Optimal Thumbnail Size Selection
// Copyright (c) 2026 ExplorerLens Project
//
// Determines the optimal decode resolution based on the requested
// thumbnail size, source image dimensions, DPI scaling, and GPU
// memory constraints to minimize unnecessary work.
//
#pragma once

#include <cstdint>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct SizeNegotiationResult {
    uint32_t decodeWidth = 0;
    uint32_t decodeHeight = 0;
    uint32_t outputWidth = 0;
    uint32_t outputHeight = 0;
    float scaleFactor = 1.0f;
    bool needsResize = false;
};

class ThumbnailSizeNegotiator {
public:
    SizeNegotiationResult Negotiate(
        uint32_t srcWidth, uint32_t srcHeight,
        uint32_t requestedSize, float dpiScale = 1.0f) const {
        SizeNegotiationResult r;
        uint32_t effectiveSize = static_cast<uint32_t>(requestedSize * dpiScale);
        effectiveSize = std::clamp(effectiveSize, 32u, 2048u);

        r.outputWidth = effectiveSize;
        r.outputHeight = effectiveSize;

        if (srcWidth == 0 || srcHeight == 0) {
            r.decodeWidth = effectiveSize;
            r.decodeHeight = effectiveSize;
            return r;
        }

        // Maintain aspect ratio
        float aspect = static_cast<float>(srcWidth) / srcHeight;
        if (aspect > 1.0f) {
            r.outputWidth = effectiveSize;
            r.outputHeight = static_cast<uint32_t>(effectiveSize / aspect);
        }
        else {
            r.outputHeight = effectiveSize;
            r.outputWidth = static_cast<uint32_t>(effectiveSize * aspect);
        }

        // Decode at 2x for quality if source is large enough
        r.scaleFactor = static_cast<float>(effectiveSize) /
            std::max(srcWidth, srcHeight);
        if (r.scaleFactor < 0.5f) {
            r.decodeWidth = r.outputWidth * 2;
            r.decodeHeight = r.outputHeight * 2;
            r.needsResize = true;
        }
        else {
            r.decodeWidth = srcWidth;
            r.decodeHeight = srcHeight;
            r.needsResize = (srcWidth != r.outputWidth ||
                srcHeight != r.outputHeight);
        }

        return r;
    }

    static uint32_t ClampToTier(uint32_t size) {
        if (size <= 48) return 48;
        if (size <= 96) return 96;
        if (size <= 128) return 128;
        if (size <= 256) return 256;
        if (size <= 512) return 512;
        return 1024;
    }
};

} // namespace Engine
} // namespace ExplorerLens
