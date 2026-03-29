// OcclusionAwareRenderer.h — Occlusion-Aware AR Renderer
// Copyright (c) 2026 ExplorerLens Project
//
// Uses device depth API to composite AR thumbnail overlays with correct
// occlusion by physical objects in the camera feed.
//
#pragma once
#include <vector>
#include <cstdint>
#include <array>

namespace ExplorerLens { namespace Engine {

struct OcclusionConfig {
    bool    enableDepthOcclusion  = true;
    float   depthThresholdM       = 0.02f;
    uint32_t downsampleFactor     = 2;
    bool    enableSoftEdges       = true;
};

struct CompositeResult {
    bool    success        = false;
    uint32_t width         = 0;
    uint32_t height        = 0;
    uint32_t occludedPixels= 0;
    float   occlusionRatio = 0.0f;
    std::vector<uint8_t> rgbaPixels;
};

class OcclusionAwareRenderer {
public:
    explicit OcclusionAwareRenderer(const OcclusionConfig& cfg = {}) : m_cfg(cfg) {}

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    CompositeResult Composite(
        const std::vector<uint8_t>& cameraRGBA,
        const std::vector<float>&   depthBuffer,
        const std::vector<uint8_t>& thumbnailRGBA,
        uint32_t width, uint32_t height,
        float thumbnailDepthM = 0.5f)
    {
        CompositeResult result;
        if (!m_ready) return result;
        result.width  = width;
        result.height = height;
        result.success = true;
        result.rgbaPixels = cameraRGBA;

        uint32_t occluded = 0;
        uint32_t pixels = width * height;
        for (uint32_t i = 0; i < pixels && i < depthBuffer.size(); ++i) {
            if (depthBuffer[i] < thumbnailDepthM) {
                ++occluded;
            } else if (i * 4 + 3 < thumbnailRGBA.size()) {
                if (result.rgbaPixels.size() == thumbnailRGBA.size()) {
                    result.rgbaPixels[i*4+0] = thumbnailRGBA[i*4+0];
                    result.rgbaPixels[i*4+1] = thumbnailRGBA[i*4+1];
                    result.rgbaPixels[i*4+2] = thumbnailRGBA[i*4+2];
                    result.rgbaPixels[i*4+3] = thumbnailRGBA[i*4+3];
                }
            }
        }
        result.occludedPixels = occluded;
        result.occlusionRatio = pixels > 0 ? static_cast<float>(occluded) / pixels : 0.0f;
        return result;
    }

    const OcclusionConfig& GetConfig() const { return m_cfg; }

private:
    OcclusionConfig m_cfg;
    bool m_ready = false;
};

}} // namespace ExplorerLens::Engine
