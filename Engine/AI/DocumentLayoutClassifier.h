// DocumentLayoutClassifier.h — Document Layout Classification
// Copyright (c) 2026 ExplorerLens Project
//
// Classifies document layouts (text-heavy, image-heavy, mixed, table, form)
// to select optimal thumbnail crop regions and rendering strategies.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DocumentLayout : uint8_t {
    Unknown,
    TextHeavy,
    ImageHeavy,
    Mixed,
    TableBased,
    FormBased,
    Presentation,
    Spreadsheet,
    Diagram
};

struct LayoutRegion {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    DocumentLayout type = DocumentLayout::Unknown;
    float confidence = 0.0f;
};

struct LayoutClassification {
    DocumentLayout primaryLayout = DocumentLayout::Unknown;
    float confidence = 0.0f;
    std::vector<LayoutRegion> regions;
    float textDensity = 0.0f;
    float imageDensity = 0.0f;
    uint32_t pageCount = 0;
};

struct CropSuggestion {
    float x = 0.0f;
    float y = 0.0f;
    float width = 1.0f;
    float height = 1.0f;
    float interestScore = 0.0f;
};

class DocumentLayoutClassifier {
public:
    DocumentLayoutClassifier() = default;

    LayoutClassification Classify(const uint8_t* pageData, uint32_t width, uint32_t height) {
        LayoutClassification result;
        if (!pageData || width == 0 || height == 0) return result;

        // Simple heuristic: analyze pixel variance
        uint64_t totalBrightness = 0;
        uint32_t darkPixels = 0;
        uint32_t pixelCount = width * height;
        for (uint32_t i = 0; i < pixelCount; i++) {
            totalBrightness += pageData[i];
            if (pageData[i] < 64) darkPixels++;
        }

        float avgBrightness = static_cast<float>(totalBrightness) / pixelCount;
        float darkRatio = static_cast<float>(darkPixels) / pixelCount;

        result.textDensity = darkRatio;
        result.imageDensity = 1.0f - darkRatio;

        if (darkRatio > 0.3f) result.primaryLayout = DocumentLayout::TextHeavy;
        else if (avgBrightness > 200.0f) result.primaryLayout = DocumentLayout::Presentation;
        else result.primaryLayout = DocumentLayout::Mixed;

        result.confidence = 0.7f;
        m_totalClassified++;
        return result;
    }

    CropSuggestion SuggestCrop(const LayoutClassification& layout) const {
        CropSuggestion suggestion;
        if (layout.primaryLayout == DocumentLayout::TextHeavy) {
            suggestion = { 0.05f, 0.05f, 0.9f, 0.5f, 0.8f };
        }
        else if (layout.primaryLayout == DocumentLayout::ImageHeavy) {
            suggestion = { 0.0f, 0.0f, 1.0f, 1.0f, 0.9f };
        }
        else {
            suggestion = { 0.0f, 0.0f, 1.0f, 0.6f, 0.6f };
        }
        return suggestion;
    }

    uint64_t GetTotalClassified() const { return m_totalClassified; }

private:
    uint64_t m_totalClassified = 0;
};

} // namespace Engine
} // namespace ExplorerLens
