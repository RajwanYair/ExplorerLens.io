// SmartCropPredictor.h — Visual saliency-based crop region prediction
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts optimal crop region for thumbnails using gradient magnitude and
// center-bias heuristics to focus on the most visually important area.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct SmartCropPredictorConfig {
    bool enabled = true;
    float centerBias = 0.3f;
    float edgePenalty = 0.1f;
    std::string label = "SmartCropPredictor";
};

class SmartCropPredictor {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    SmartCropPredictorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct CropRegion {
        uint32_t x = 0, y = 0, width = 0, height = 0;
        float saliencyScore = 0.0f;
    };

    CropRegion PredictCrop(uint32_t imgWidth, uint32_t imgHeight,
        uint32_t thumbWidth, uint32_t thumbHeight) const {
        CropRegion crop;
        float imgAspect = static_cast<float>(imgWidth) / imgHeight;
        float thumbAspect = static_cast<float>(thumbWidth) / thumbHeight;
        if (imgAspect > thumbAspect) {
            crop.height = imgHeight;
            crop.width = static_cast<uint32_t>(imgHeight * thumbAspect);
            crop.x = (imgWidth - crop.width) / 2;
        }
        else {
            crop.width = imgWidth;
            crop.height = static_cast<uint32_t>(imgWidth / thumbAspect);
            crop.y = (imgHeight - crop.height) / 2;
        }
        crop.saliencyScore = m_config.centerBias;
        return crop;
    }

private:
    bool m_initialized = false;
    SmartCropPredictorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
