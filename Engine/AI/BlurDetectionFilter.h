// BlurDetectionFilter.h — Blur Detection and Deblur Sharpening Filter
// Copyright (c) 2026 ExplorerLens Project
//
// Detects motion blur and out-of-focus blur in thumbnail pixels using a
// Laplacian variance heuristic, and optionally applies an unsharp mask or
// DeblurGAN-lite AI deblur to improve thumbnail clarity.
//
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---- Blur Metrics -----------------------------------------------------------

enum class BlurType : uint8_t {
    None = 0,
    Motion = 1,    // Directional linear smear
    Defocus = 2,   // Gaussian circular blur
    Combined = 3,  // Both present
};

struct BlurReport
{
    BlurType blurType = BlurType::None;
    float laplacianVariance = 0.0f;  // Higher = sharper
    float blurScore = 0.0f;          // [0.0 (sharp) – 1.0 (heavily blurred)]
    bool isBlurry = false;           // score > threshold (default 0.35)
    float motionAngleDeg = 0.0f;     // Dominant motion blur angle (if Motion)
};

// ---- Deblur Options ---------------------------------------------------------

enum class DeblurMethod : uint8_t {
    UnsharpMask = 0,      // CPU: fast, light sharpening
    FrequencyDomain = 1,  // Wiener deconvolution (moderate quality)
    AIDeblur = 2,         // GAN-lite model (best quality, GPU preferred)
};

struct DeblurOptions
{
    DeblurMethod method = DeblurMethod::UnsharpMask;
    float strength = 0.5f;     // 0.0 (none) – 1.0 (maximum)
    bool onlyIfBlurry = true;  // Skip deblur if blurScore < threshold
    float blurThreshold = 0.35f;
};

struct DeblurResult
{
    bool success = false;
    std::vector<uint8_t> pixels;  // BGRA
    uint32_t width = 0;
    uint32_t height = 0;
    DeblurMethod methodUsed = DeblurMethod::UnsharpMask;
    bool deblurApplied = false;
};

// ---- BlurDetectionFilter ----------------------------------------------------

class BlurDetectionFilter
{
  public:
    BlurDetectionFilter();
    ~BlurDetectionFilter();

    // Analyse a BGRA thumbnail for blur (fast Laplacian variance, no model needed).
    BlurReport Detect(const uint8_t* pixels, uint32_t width, uint32_t height) const;

    // Detect and optionally deblur.
    DeblurResult Process(const uint8_t* pixels, uint32_t width, uint32_t height, const DeblurOptions& opts = {}) const;

    // Load AI deblur model (required only for DeblurMethod::AIDeblur).
    bool LoadAIModel(const std::string& modelPath = "");
    void UnloadAIModel();
    bool IsAIModelLoaded() const;

  private:
    struct Impl
    {};
    std::unique_ptr<Impl> m_impl;

    static float LaplacianVariance(const uint8_t* gray, uint32_t w, uint32_t h);
};

}  // namespace Engine
}  // namespace ExplorerLens
