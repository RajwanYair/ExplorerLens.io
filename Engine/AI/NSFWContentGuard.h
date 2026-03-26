// NSFWContentGuard.h — NSFW / Explicit Content Detection Gate
// Copyright (c) 2026 ExplorerLens Project
//
// Runs a lightweight binary classifier to detect explicit/adult content in
// thumbnail pixels and optionally blurs or replaces the thumbnail for safe
// display in enterprise environments. Requires an enterprise license key.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Guard Modes ------------------------------------------------------------

enum class NSFWGuardMode : uint8_t {
    Disabled         = 0,   // Feature not active
    Monitor          = 1,   // Log detections; don't alter thumbnails
    BlurOnDetect     = 2,   // Apply heavy Gaussian blur overlay
    ReplaceOnDetect  = 3,   // Replace with a neutral placeholder icon
    BlockOnDetect    = 4,   // Return S_FALSE to Explorer (no thumbnail)
};

// ---- Detection Result -------------------------------------------------------

struct NSFWDetectionResult {
    bool   flagged           = false;
    float  confidence        = 0.0f;   // Classifier confidence [0.0-1.0]
    float  inferenceMs       = 0.0f;
    bool   thumbnailAltered  = false;  // Blurred or replaced
    NSFWGuardMode appliedMode = NSFWGuardMode::Disabled;
};

// ---- NSFWContentGuard -------------------------------------------------------

class NSFWContentGuard {
public:
    NSFWContentGuard();
    ~NSFWContentGuard();

    // Activate (requires valid enterprise license key).
    bool Activate(const std::string& licenseKey);
    void Deactivate();
    bool IsActive() const;

    // Load the NSFW classifier model.
    bool LoadModel(const std::string& modelPath = "");
    void UnloadModel();

    // Classify + optionally alter thumbnail pixels.
    NSFWDetectionResult Process(
        std::vector<uint8_t>& pixels,   // Modified in-place if flagged
        uint32_t              width,
        uint32_t              height,
        NSFWGuardMode         mode = NSFWGuardMode::BlurOnDetect) const;

    // Detection confidence threshold — detections below this are ignored.
    void  SetThreshold(float threshold);
    float GetThreshold() const;

    static NSFWContentGuard& Instance();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    static void ApplyGaussianBlur(
        std::vector<uint8_t>& pixels,
        uint32_t w, uint32_t h,
        float sigma = 12.0f);
};

} // namespace Engine
} // namespace ExplorerLens
