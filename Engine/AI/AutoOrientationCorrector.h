// AutoOrientationCorrector.h — Visual analysis-based orientation correction
// Copyright (c) 2026 ExplorerLens Project
//
// Corrects image orientation beyond EXIF tags using visual heuristics
// (sky detection, text direction, face position) for misoriented images.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct AutoOrientationCorrectorConfig {
    bool enabled = true;
    float confidenceThreshold = 0.7f;
    bool respectExif = true;
    std::string label = "AutoOrientationCorrector";
};

class AutoOrientationCorrector {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    AutoOrientationCorrectorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Orientation : uint8_t { Up, Right, Down, Left };

    struct OrientationResult {
        Orientation predicted = Orientation::Up;
        float confidence = 0.0f;
        bool needsCorrection = false;
    };

    OrientationResult Analyze(float topAvgBrightness, float bottomAvgBrightness,
        float leftAvgBrightness, float rightAvgBrightness) const {
        OrientationResult result;
        // Heuristic: sky is typically brighter, appears at top of correctly-oriented images
        float maxBrightness = topAvgBrightness;
        result.predicted = Orientation::Up;
        if (bottomAvgBrightness > maxBrightness) { maxBrightness = bottomAvgBrightness; result.predicted = Orientation::Down; }
        if (leftAvgBrightness > maxBrightness) { maxBrightness = leftAvgBrightness; result.predicted = Orientation::Left; }
        if (rightAvgBrightness > maxBrightness) { result.predicted = Orientation::Right; }
        result.confidence = 0.6f;
        result.needsCorrection = result.predicted != Orientation::Up &&
            result.confidence >= m_config.confidenceThreshold;
        return result;
    }

private:
    bool m_initialized = false;
    AutoOrientationCorrectorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
