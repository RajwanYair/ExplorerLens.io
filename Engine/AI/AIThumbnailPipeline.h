// AIThumbnailPipeline.h — AI Module Orchestration for Thumbnail Post-Processing
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates the AI post-processing pipeline: content classification → optional
// smart crop → blur detection/deblur → synthesis fallback → final BGRA output.
// All AI steps are opt-in and gate on model availability.
//
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---- Pipeline Config --------------------------------------------------------

struct AIPipelineConfig
{
    bool enableClassification = true;
    bool enableSmartCrop = true;  // Uses SmartCropAnalyzer (existing)
    bool enableBlurDetection = true;
    bool enableDeblur = true;
    bool enableSynthesisFallback = true;  // Generate if decode failed wholly
    bool enableNSFWGuard = false;         // Requires enterprise license
    bool preferGPU = true;
    float blurDeblurThreshold = 0.35f;
};

// ---- Per-Step Diagnostics ---------------------------------------------------

struct AIPipelineDiagnostics
{
    bool classificationRan = false;
    bool smartCropRan = false;
    bool blurFilterRan = false;
    bool deblurApplied = false;
    bool synthesisFallbackUsed = false;
    bool nsfwBlocked = false;
    float totalAIMs = 0.0f;  // Accumulated inference time
    std::string contentCategoryName;
    float blurScore = 0.0f;
};

// ---- Pipeline Result --------------------------------------------------------

struct AIPipelineResult
{
    bool success = false;
    std::vector<uint8_t> pixels;  // Final BGRA
    uint32_t width = 0;
    uint32_t height = 0;
    AIPipelineDiagnostics diagnostics;
};

// ---- AIThumbnailPipeline ----------------------------------------------------

class AIThumbnailPipeline
{
  public:
    explicit AIThumbnailPipeline(AIPipelineConfig config = {});
    ~AIThumbnailPipeline();

    // Run all enabled AI stages on a decoded BGRA thumbnail.
    AIPipelineResult Process(const uint8_t* pixels, uint32_t width, uint32_t height,
                             const std::string& filePath = "") const;

    // Synthesis-only path for files that couldn't be decoded at all.
    AIPipelineResult Synthesize(const std::string& filePath) const;

    // Load all AI models (call once at startup after GPU init).
    bool LoadModels(const std::string& modelDir = "");
    void UnloadModels();
    bool AreModelsLoaded() const;

    void SetConfig(AIPipelineConfig config);
    const AIPipelineConfig& Config() const;

    static AIThumbnailPipeline& Instance();

  private:
    struct Impl
    {};
    std::unique_ptr<Impl> m_impl;
};

}  // namespace Engine
}  // namespace ExplorerLens
