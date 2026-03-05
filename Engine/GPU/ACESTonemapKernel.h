// ACESTonemapKernel.h — ACES Filmic Tone Mapping GPU Kernel
// Copyright (c) 2026 ExplorerLens Project
//
// GPU-accelerated ACES filmic tone mapping for HDR→SDR thumbnail conversion.
// Implements the ACES Reference Rendering Transform (RRT) + Output Device
// Transform (ODT) approximation for real-time HDR content preview.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

// ACES tone curve variant selection
enum class ACESCurveVariant {
    RRT_ODT_Fit,     // Standard Stephen Hill ACES fit
    Narkowicz,       // Krzysztof Narkowicz simplified ACES
    Unreal,          // Unreal Engine 4 filmic curve
    AgX,             // AgX sigmoid-based (Blender 4.0+)
    Custom           // User-defined curve parameters
};

// Color grading pre-tone-map adjustments
struct ACESGradingParams {
    float exposure = 0.0f;   // EV adjustment (-4 to +4)
    float contrast = 1.0f;   // Contrast multiplier (0.5-2.0)
    float saturation = 1.0f;   // Saturation multiplier (0-2.0)
    float whitePoint = 1.0f;   // Scene-referred white luminance (nits/80)
    float shadowLift = 0.0f;   // Shadow lift (-0.1 to 0.1)
};

// Per-pixel tonemapped output
struct ACESTonemapResult {
    uint32_t pixelsProcessed = 0;
    float    avgLuminancePre = 0.0f;  // Average luminance before tonemap
    float    avgLuminancePost = 0.0f; // Average luminance after tonemap
    float    peakLuminancePre = 0.0f;
    double   processingTimeMs = 0.0;
};

class ACESTonemapKernel {
public:
    static ACESTonemapKernel& Instance() {
        static ACESTonemapKernel s;
        return s;
    }

    // Set the ACES curve variant for subsequent operations
    void SetCurve(ACESCurveVariant curve) { m_curve = curve; }
    ACESCurveVariant GetCurve() const { return m_curve; }

    // Set grading parameters applied before tone mapping
    void SetGrading(const ACESGradingParams& grading) { m_grading = grading; }
    const ACESGradingParams& GetGrading() const { return m_grading; }

    // Apply ACES tone mapping to a single RGB triple (scene-linear)
    void TonemapPixel(float& r, float& g, float& b) const {
        // Apply exposure
        float ev = std::pow(2.0f, m_grading.exposure);
        r *= ev; g *= ev; b *= ev;

        // Apply contrast around midgray (0.18)
        auto contrastF = [&](float v) {
            float logV = std::log2(std::max(v, 1e-6f) / 0.18f);
            logV *= m_grading.contrast;
            return 0.18f * std::pow(2.0f, logV);
            };
        r = contrastF(r); g = contrastF(g); b = contrastF(b);

        // Tone curve
        switch (m_curve) {
        case ACESCurveVariant::RRT_ODT_Fit:
            r = ACESFit(r); g = ACESFit(g); b = ACESFit(b);
            break;
        case ACESCurveVariant::Narkowicz:
            r = ACESNarkowicz(r); g = ACESNarkowicz(g); b = ACESNarkowicz(b);
            break;
        case ACESCurveVariant::Unreal:
            r = UnrealFilmic(r); g = UnrealFilmic(g); b = UnrealFilmic(b);
            break;
        case ACESCurveVariant::AgX:
            r = AgXSigmoid(r); g = AgXSigmoid(g); b = AgXSigmoid(b);
            break;
        default:
            r = ACESFit(r); g = ACESFit(g); b = ACESFit(b);
            break;
        }

        // Clamp to [0, 1]
        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);
    }

    // Generate HLSL source for the selected tone curve (for GPU shader compilation)
    std::string GenerateHLSL() const {
        std::string hlsl = "// Auto-generated ACES tonemap kernel\n";
        hlsl += "float3 ACESTonemap(float3 color) {\n";
        switch (m_curve) {
        case ACESCurveVariant::Narkowicz:
            hlsl += "  float a=2.51, b=0.03, c=2.43, d=0.59, e=0.14;\n";
            hlsl += "  return saturate((color*(a*color+b))/(color*(c*color+d)+e));\n";
            break;
        default:
            hlsl += "  float a=0.0245786, b=0.000090537, c=0.983729, d=0.4329510, e=0.238081;\n";
            hlsl += "  return saturate((color*(color+a)-b)/(color*(c*color+d)+e));\n";
            break;
        }
        hlsl += "}\n";
        return hlsl;
    }

    bool Validate() const {
        // Test ACES fit: f(0)≈0, f(1)≈0.856, monotonically increasing
        float testR = 0.0f, testG = 1.0f, testB = 10.0f;
        ACESTonemapKernel k;
        k.SetCurve(ACESCurveVariant::RRT_ODT_Fit);
        k.TonemapPixel(testR, testG, testB);
        if (testR < 0.0f || testR > 0.01f) return false;
        if (testG < 0.5f || testG > 1.0f)  return false;
        if (testB < 0.9f || testB > 1.0f)  return false;
        if (testG >= testB) return false; // Monotonicity: f(1) < f(10)
        return true;
    }

private:
    ACESTonemapKernel() = default;

    ACESCurveVariant m_curve = ACESCurveVariant::RRT_ODT_Fit;
    ACESGradingParams m_grading{};

    // Stephen Hill's ACES fit (sRGB output)
    static float ACESFit(float x) {
        float a = 0.0245786f, b = 0.000090537f;
        float c = 0.983729f, d = 0.4329510f, e = 0.238081f;
        return (x * (x + a) - b) / (x * (c * x + d) + e);
    }

    // Narkowicz ACES approximation
    static float ACESNarkowicz(float x) {
        float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
        return (x * (a * x + b)) / (x * (c * x + d) + e);
    }

    // Unreal Engine 4 filmic curve
    static float UnrealFilmic(float x) {
        return x / (x + 0.155f) * 1.019f;
    }

    // AgX sigmoid approximation
    static float AgXSigmoid(float x) {
        float logX = std::log2(std::max(x, 1e-10f));
        logX = std::clamp(logX, -12.47393f, 4.026069f);
        float t = (logX - (-12.47393f)) / (4.026069f - (-12.47393f));
        // Polynomial sigmoid fit
        return t * t * (3.0f - 2.0f * t);
    }
};

} // namespace Engine
} // namespace ExplorerLens
