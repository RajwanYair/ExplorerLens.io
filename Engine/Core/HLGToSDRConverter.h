// HLGToSDRConverter.h — ITU-R BT.2100 HLG to sRGB Converter
// Copyright (c) 2026 ExplorerLens Project
//
// Converts Hybrid Log-Gamma (HLG, ITU-R BT.2100) encoded pixels to
// BT.709 sRGB for standard-display thumbnail rendering. Supports the
// HLG OOTF (scene adaptive) and the simplified system-gamma adaptation.
// P50 target: < 0.5 ms. Used by HEIFDecoder and VideoDecoder.
//
#pragma once
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct HLGConvertParams {
    float systemGamma  = 1.2f;    // BT.2100 nominal gamma (1.0 at 1000 cd/m²)
    float peakLuminance = 1000.0f; // Display peak luminance in nits for OOTF
    bool  sceneAdaptive = true;    // Use OOTF (scene-referred) vs display-referred
};

struct HLGConvertResult {
    uint8_t* pixelsBGRA = nullptr;  // Caller-owned; width × height × 4 bytes
    uint32_t width      = 0;
    uint32_t height     = 0;
    bool     success    = false;
    float    processMs  = 0.0f;
};

class HLGToSDRConverter {
public:
    HLGToSDRConverter()  = default;
    ~HLGToSDRConverter() = default;

    // Convert `srcBGRA16F` (16-bit float BGRA, HLG-encoded) to BGRA8 sRGB.
    // srcBGRA16F must be width × height × 8 bytes.
    HLGConvertResult Convert(
        const uint16_t*         srcBGRA16F,
        uint32_t                width,
        uint32_t                height,
        const HLGConvertParams& params) const noexcept;

    // Convert from 10-bit HLG BGRA packed (BT.2100) to BGRA8 sRGB.
    HLGConvertResult ConvertBGRA10(
        const uint32_t*         srcBGRA10,
        uint32_t                width,
        uint32_t                height,
        const HLGConvertParams& params) const noexcept;

    // Inverse HLG OETF (signal → scene light). Input range [0, 1].
    static float HLGtoLinear(float signal) noexcept;

    // HLG OOTF: scene-linear → display-linear, applying system gamma.
    static float HLGOOTF(float sceneLinear, float systemGamma,
                          float peakLuminance) noexcept;

    // Linear BT.2020 → BT.709 primaries (3×3 matrix multiply, one channel).
    static void BT2020ToBT709(float& r, float& g, float& b) noexcept;
};

}} // namespace ExplorerLens::Engine
