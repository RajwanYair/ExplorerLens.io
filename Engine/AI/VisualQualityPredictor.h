// VisualQualityPredictor.h — Decode Quality Prediction Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts the visual quality of a thumbnail before full decode based on
// file metadata, compression parameters, and historical decode data.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class VisualQualityGrade : uint8_t {
    Excellent,
    Good,
    Acceptable,
    Poor,
    Unacceptable
};

struct VisualQualityPrediction {
    VisualQualityGrade expectedGrade = VisualQualityGrade::Good;
    float confidenceScore = 0.0f;
    float estimatedSSIM = 0.0f;
    float estimatedPSNR = 0.0f;
    uint32_t suggestedThumbnailSize = 256;
    bool needsEnhancement = false;
};

struct FormatQualityProfile {
    std::string extension;
    float avgSSIM = 0.0f;
    uint64_t sampleCount = 0;
    VisualQualityGrade typicalGrade = VisualQualityGrade::Good;
};

class VisualQualityPredictor {
public:
    VisualQualityPredictor() { InitializeProfiles(); }

    VisualQualityPrediction Predict(const std::string& extension, uint64_t fileSize,
        uint32_t width, uint32_t height) const {
        VisualQualityPrediction prediction;
        auto it = m_profiles.find(extension);
        if (it != m_profiles.end()) {
            prediction.estimatedSSIM = it->second.avgSSIM;
            prediction.expectedGrade = it->second.typicalGrade;
            prediction.confidenceScore = 0.8f;
        }
        else {
            prediction.estimatedSSIM = 0.85f;
            prediction.expectedGrade = VisualQualityGrade::Good;
            prediction.confidenceScore = 0.3f;
        }

        // Adjust based on resolution vs thumbnail ratio
        uint64_t pixels = static_cast<uint64_t>(width) * height;
        if (pixels > 25000000) {
            prediction.suggestedThumbnailSize = 512;
        }
        else if (pixels < 65536) {
            prediction.needsEnhancement = true;
            prediction.estimatedSSIM -= 0.1f;
        }

        float bpp = (fileSize > 0 && pixels > 0)
            ? static_cast<float>(fileSize * 8) / pixels : 0.0f;
        if (bpp < 0.5f) {
            prediction.expectedGrade = VisualQualityGrade::Poor;
            prediction.needsEnhancement = true;
        }

        return prediction;
    }

    void UpdateProfile(const std::string& extension, float measuredSSIM) {
        auto& profile = m_profiles[extension];
        profile.extension = extension;
        profile.avgSSIM = (profile.avgSSIM * profile.sampleCount + measuredSSIM)
            / (profile.sampleCount + 1);
        profile.sampleCount++;
    }

    size_t GetProfileCount() const { return m_profiles.size(); }

private:
    void InitializeProfiles() {
        m_profiles[".jpg"] = { ".jpg", 0.92f, 1000, VisualQualityGrade::Good };
        m_profiles[".png"] = { ".png", 0.98f, 1000, VisualQualityGrade::Excellent };
        m_profiles[".webp"] = { ".webp", 0.94f, 500, VisualQualityGrade::Good };
        m_profiles[".bmp"] = { ".bmp", 1.0f, 200, VisualQualityGrade::Excellent };
        m_profiles[".gif"] = { ".gif", 0.80f, 300, VisualQualityGrade::Acceptable };
    }

    std::unordered_map<std::string, FormatQualityProfile> m_profiles;
};

} // namespace Engine
} // namespace ExplorerLens
