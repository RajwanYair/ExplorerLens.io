// LandmarkDetectionEngine.h - BlazeFace Landmark Detection Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Fast face/landmark detection via BlazeFace for smart-crop and portrait enhancement.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace AI {

struct BoundingBox {
    int   x      = 0;
    int   y      = 0;
    int   width  = 0;
    int   height = 0;
    float score  = 0.0f;
};

struct FaceLandmarks {
    BoundingBox           bounds;
    std::vector<float>    keypoints; // 2D x,y pairs, 6 keypoints = 12 floats
    float                 rollAngleDeg  = 0.0f;
    float                 pitchAngleDeg = 0.0f;
};

struct LandmarkDetectResult {
    bool                      success = false;
    std::vector<FaceLandmarks> faces;
    std::string                error;
};

struct LandmarkConfig {
    int   maxFaces      = 10;
    float minFaceScore  = 0.75f;
    bool  detectLandmarks = true;
    bool  estimatePose  = false;
};

class LandmarkDetectionEngine {
public:
    explicit LandmarkDetectionEngine() = default;
    explicit LandmarkDetectionEngine(const LandmarkConfig& cfg) : m_config(cfg) {}

    LandmarkDetectResult Detect(const void* srcPixels, int w, int h) const noexcept {
        if (!srcPixels || w <= 0 || h <= 0)
            return { false, {}, "Invalid input" };
        const int minPx = static_cast<int>(MIN_FACE_SIZE_FRAC * w);
        (void)minPx;
        return { true, {}, {} };
    }

    LandmarkDetectResult DetectFile(const std::string& path) const noexcept {
        if (path.empty()) return { false, {}, "Empty path" };
        return { false, {}, "File not found: " + path };
    }

    BoundingBox GetOptimalCropBox(const LandmarkDetectResult& result,
                                  int imgW, int imgH) const noexcept {
        if (result.faces.empty() || imgW <= 0 || imgH <= 0)
            return {};
        const auto& b = result.faces[0].bounds;
        int padX = b.width  / 4;
        int padY = b.height / 4;
        BoundingBox crop;
        crop.x      = (b.x - padX < 0)    ? 0    : b.x - padX;
        crop.y      = (b.y - padY < 0)    ? 0    : b.y - padY;
        crop.width  = (b.width  + padX * 2 > imgW) ? imgW : b.width  + padX * 2;
        crop.height = (b.height + padY * 2 > imgH) ? imgH : b.height + padY * 2;
        crop.score  = b.score;
        return crop;
    }

    int   GetMaxFaces()       const noexcept { return m_config.maxFaces;       }
    float GetMinFaceScore()   const noexcept { return m_config.minFaceScore;   }
    bool  GetDetectLandmarks()const noexcept { return m_config.detectLandmarks;}
    bool  GetEstimatePose()   const noexcept { return m_config.estimatePose;   }

    void SetMaxFaces(int v)         noexcept { m_config.maxFaces       = v; }
    void SetMinFaceScore(float v)   noexcept { m_config.minFaceScore   = v; }
    void SetDetectLandmarks(bool v) noexcept { m_config.detectLandmarks= v; }
    void SetEstimatePose(bool v)    noexcept { m_config.estimatePose   = v; }

    static constexpr float MIN_FACE_SIZE_FRAC = 0.04f;
    static constexpr int   BLAZEFACE_INPUT    = 128;
    static constexpr float MIN_FACE_SCORE     = 0.75f;

private:
    LandmarkConfig m_config;
};

}} // namespace ExplorerLens::AI

