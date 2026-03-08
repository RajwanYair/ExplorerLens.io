// ContentClassifier.h — Image content type classification
// Copyright (c) 2026 ExplorerLens Project
//
// Classifies image content into categories (landscape, portrait, document,
// diagram, screenshot) using aspect ratio and color distribution heuristics.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct ContentClassifierConfig {
    bool enabled = true;
    float documentThreshold = 0.8f;
    std::string label = "ContentClassifier";
};

class ContentClassifier {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ContentClassifierConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Category : uint8_t {
        Photo, Landscape, Portrait, Document, Diagram, Screenshot, Icon, Unknown
    };

    Category Classify(uint32_t width, uint32_t height, float avgSaturation,
        float whiteFraction) const {
        float aspect = static_cast<float>(width) / height;
        if (whiteFraction > m_config.documentThreshold) return Category::Document;
        if (width <= 128 && height <= 128) return Category::Icon;
        if (avgSaturation < 0.1f && aspect > 1.2f) return Category::Screenshot;
        if (aspect > 1.5f) return Category::Landscape;
        if (aspect < 0.75f) return Category::Portrait;
        return Category::Photo;
    }

    std::string CategoryToString(Category cat) const {
        switch (cat) {
        case Category::Photo: return "photo";
        case Category::Landscape: return "landscape";
        case Category::Portrait: return "portrait";
        case Category::Document: return "document";
        case Category::Diagram: return "diagram";
        case Category::Screenshot: return "screenshot";
        case Category::Icon: return "icon";
        default: return "unknown";
        }
    }

private:
    bool m_initialized = false;
    ContentClassifierConfig m_config;
};

}
} // namespace ExplorerLens::Engine
