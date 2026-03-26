// ContentCategoryClassifier.h — File Content Category Classifier
// Copyright (c) 2026 ExplorerLens Project
//
// Classifies decoded thumbnail pixels into semantic content categories
// (Photo / Document / Diagram / Code / Art / Medical / ...) to enable
// category-specific post-processing and Explorer overlay icons.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Categories -------------------------------------------------------------

enum class ContentCategory : uint8_t {
    Unknown         = 0,
    Photo           = 1,   // Natural scene / portrait / landscape
    Document        = 2,   // Text-heavy, white background
    Spreadsheet     = 3,   // Grid / tabular data
    Presentation    = 4,   // Slide-like layout with title/body
    Diagram         = 5,   // Flowchart / UML / wireframe
    Code            = 6,   // Syntax-highlighted code screenshot
    Art             = 7,   // Illustration / graphic design / vector art
    Medical         = 8,   // DICOM-style grayscale clinical image
    Map             = 9,   // Cartographic / satellite imagery
    Video           = 10,  // Single frame from video (motion blur present)
    Icon            = 11,  // Small icon / logo (<64 px dominant content)
    Screenshot      = 12,  // OS screenshot with UI chrome
};

// ---- Classification Result --------------------------------------------------

struct CategoryPrediction {
    ContentCategory category    = ContentCategory::Unknown;
    float           confidence  = 0.0f;  // [0.0 - 1.0]
};

struct ClassificationResult {
    bool   success              = false;
    ContentCategory primary     = ContentCategory::Unknown;
    float  primaryConfidence    = 0.0f;
    std::vector<CategoryPrediction> topK;  // Top-3 predictions
    std::string backendUsed;
    float  inferenceMs          = 0.0f;
};

// ---- ContentCategoryClassifier ----------------------------------------------

class ContentCategoryClassifier {
public:
    ContentCategoryClassifier();
    ~ContentCategoryClassifier();

    bool LoadModel(const std::string& modelPath = "");
    void UnloadModel();
    bool IsModelLoaded() const;

    // Classify a BGRA thumbnail pixel buffer.
    ClassificationResult Classify(
        const uint8_t* pixels,
        uint32_t       width,
        uint32_t       height) const;

    // Human-readable category name for display.
    static const char* CategoryName(ContentCategory cat);

    // Retrieve the recommended post-processing hint for a category.
    // e.g. ContentCategory::Document → "sharpen_text"
    static const char* PostProcessHint(ContentCategory cat);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
