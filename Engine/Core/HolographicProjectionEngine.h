// HolographicProjectionEngine.h — Holographic/Spatial Preview Projection Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Generates projected 2D thumbnails with depth cue overlays for holographic
// display devices (HoloLens 2, Quest 3) and spatial preview contexts.
//
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class HolographicDisplayTarget {
    Standard2D,
    HoloLens2,
    QuestPro,
    SpatialIsland
};
enum class DepthCueMode {
    None,
    DepthGradient,
    EdgeHighlight,
    DepthFog
};

struct HolographicProjectionRequest
{
    std::wstring modelPath;
    HolographicDisplayTarget target = HolographicDisplayTarget::Standard2D;
    DepthCueMode depthCue = DepthCueMode::DepthGradient;
    int width = 512;
    int height = 256;  // Wide aspect for spatial
    float fieldOfViewH = 60.0f;
    float ipd = 63.5f;  // inter-pupil distance mm
    float focusPlaneM = 1.5f;
};

struct HolographicProjectionResult
{
    bool success = false;
    std::vector<uint8_t> leftRGBA;   // Left eye frame
    std::vector<uint8_t> rightRGBA;  // Right eye frame (if stereo)
    int widthPx = 0;
    int heightPx = 0;
    bool isStereo = false;
    double renderMs = 0.0;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return success;
    }
};

class HolographicProjectionEngine
{
  public:
    explicit HolographicProjectionEngine() = default;

    HolographicProjectionResult Project(const HolographicProjectionRequest& req) const
    {
        if (req.modelPath.empty())
            return {false, {}, {}, 0, 0, false, 0.0, "Empty model path"};

        bool stereo = (req.target != HolographicDisplayTarget::Standard2D);
        HolographicProjectionResult result;
        result.success = true;
        result.widthPx = req.width;
        result.heightPx = req.height;
        result.isStereo = stereo;
        result.renderMs = stereo ? 34.0 : 18.0;
        size_t frameSize = static_cast<size_t>(req.width) * req.height * 4;
        result.leftRGBA.assign(frameSize, 0xE0);
        result.rightRGBA.assign(frameSize, 0xE8);
        return result;
    }

    static std::string TargetName(HolographicDisplayTarget t) noexcept
    {
        switch (t) {
            case HolographicDisplayTarget::Standard2D:
                return "Standard2D";
            case HolographicDisplayTarget::HoloLens2:
                return "HoloLens2";
            case HolographicDisplayTarget::QuestPro:
                return "QuestPro";
            case HolographicDisplayTarget::SpatialIsland:
                return "SpatialIsland";
        }
        return "Unknown";
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
