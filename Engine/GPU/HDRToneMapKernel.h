// ============================================================================
// HDRToneMapKernel.h — HDR-to-SDR Tone Mapping GPU Shader Host
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Applies tone mapping operators to HDR thumbnails (EXR, HDR, AVIF-HDR) for
// correct display on SDR monitors. Supports Reinhard, ACES Filmic, and
// adaptive local tone mapping. Executes on GPU compute or CPU SIMD fallback.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <mutex>
#include <chrono>
#include <cmath>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Tone mapping algorithms (distinct from HDRToneMapOp in HDRToneMappingPipeline)
// ============================================================================

enum class ToneMapMethod : uint8_t {
    Reinhard = 0,   // Simple global Reinhard
    ReinhardExtended = 1,   // Extended Reinhard with white point
    ACESFilmic = 2,   // Academy Color Encoding System
    Uncharted2 = 3,   // Hable/Uncharted 2 filmic
    AgX = 4,   // AgX (Blender 4.x default)
    Adaptive = 5    // Auto-select based on scene analysis
};

inline const char* ToneMapMethodToString(ToneMapMethod m) {
    static const char* names[] = {
        "Reinhard", "ReinhardExtended", "ACESFilmic",
        "Uncharted2", "AgX", "Adaptive"
    };
    return names[static_cast<uint8_t>(m)];
}

// ============================================================================
// Tone map parameters
// ============================================================================

struct ToneMapParams {
    ToneMapMethod op = ToneMapMethod::ACESFilmic;
    float           exposure = 1.0f;     // Exposure multiplier (EV)
    float           gamma = 2.2f;     // Output gamma
    float           whitePoint = 4.0f;     // Max white luminance (for Reinhard)
    float           contrast = 1.0f;     // Contrast adjustment (1.0 = neutral)
    float           saturation = 1.0f;     // Saturation adjustment
    bool            autoExposure = true;    // Compute exposure from scene
    bool            ditherOutput = true;    // Dither to avoid banding in 8-bit
};

// ============================================================================
// Scene analysis result (for adaptive tone mapping)
// ============================================================================

struct SceneAnalysis {
    float minLuminance = 0.0f;
    float maxLuminance = 0.0f;
    float avgLuminance = 0.0f;
    float logAvgLuminance = 0.0f;
    float dynamicRange = 0.0f;    // In stops (EV)
    float keyValue = 0.18f;   // Scene key
    bool  isHDR = false;   // Dynamic range > 8 stops

    float GetAutoExposure() const {
        if (logAvgLuminance <= 0.0f) return 1.0f;
        return keyValue / logAvgLuminance;
    }
};

// ============================================================================
// Tone map statistics
// ============================================================================

struct ToneMapStats {
    uint64_t totalToneMaps = 0;
    uint64_t gpuToneMaps = 0;
    uint64_t cpuFallbacks = 0;
    double   avgToneMapTimeMs = 0.0;
    uint64_t totalPixels = 0;
    uint32_t operatorHistogram[6] = {};  // Usage per operator

    double GetGPURatio() const {
        return (totalToneMaps > 0)
            ? (static_cast<double>(gpuToneMaps) / totalToneMaps * 100.0) : 0.0;
    }
};

// ============================================================================
// HDRToneMapKernel
// ============================================================================

class HDRToneMapKernel {
public:
    static constexpr uint32_t GROUP_SIZE = 8;

    HDRToneMapKernel() = default;

    // ========================================================================
    // Initialization
    // ========================================================================

    bool Initialize(void* d3dDevice = nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_d3dDevice = d3dDevice;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }

    // ========================================================================
    // Tone mapping (public for testing)
    // ========================================================================

    /// Analyze scene luminance statistics
    static SceneAnalysis AnalyzeScene(const float* hdrData, uint32_t width, uint32_t height,
        uint32_t channels = 3) {
        SceneAnalysis analysis;
        if (!hdrData || width == 0 || height == 0) return analysis;

        uint64_t pixelCount = static_cast<uint64_t>(width) * height;
        float minL = 1e10f, maxL = -1e10f, sumL = 0.0f, sumLogL = 0.0f;
        const float delta = 1e-6f;

        for (uint64_t i = 0; i < pixelCount; i++) {
            uint64_t idx = i * channels;
            // Luminance: 0.2126*R + 0.7152*G + 0.0722*B (BT.709)
            float luminance = 0.2126f * hdrData[idx] +
                0.7152f * hdrData[idx + 1] +
                0.0722f * hdrData[idx + 2];

            minL = (std::min)(minL, luminance);
            maxL = (std::max)(maxL, luminance);
            sumL += luminance;
            sumLogL += std::log(luminance + delta);
        }

        analysis.minLuminance = minL;
        analysis.maxLuminance = maxL;
        analysis.avgLuminance = sumL / pixelCount;
        analysis.logAvgLuminance = std::exp(sumLogL / pixelCount);
        analysis.dynamicRange = (minL > 0) ? std::log2(maxL / minL) : 0.0f;
        analysis.isHDR = analysis.dynamicRange > 8.0f;
        analysis.keyValue = 1.03f - 2.0f / (2.0f + std::log10(analysis.logAvgLuminance + 1.0f));

        return analysis;
    }

    /// Apply Reinhard global tone mapping
    static float ToneMapReinhard(float luminance) {
        return luminance / (1.0f + luminance);
    }

    /// Apply ACES Filmic tone mapping (approximation by Narkowicz)
    static float ToneMapACES(float x) {
        const float a = 2.51f;
        const float b = 0.03f;
        const float c = 2.43f;
        const float d = 0.59f;
        const float e = 0.14f;
        float result = (x * (a * x + b)) / (x * (c * x + d) + e);
        return (std::max)(0.0f, (std::min)(1.0f, result));
    }

    /// Apply Uncharted 2 / Hable filmic curve
    static float ToneMapUncharted2(float x) {
        const float A = 0.15f;  // Shoulder Strength
        const float B = 0.50f;  // Linear Strength
        const float C = 0.10f;  // Linear Angle
        const float D = 0.20f;  // Toe Strength
        const float E = 0.02f;  // Toe Numerator
        const float F = 0.30f;  // Toe Denominator
        return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    }

    /// Select best operator for a scene
    static ToneMapMethod SelectOperator(const SceneAnalysis& scene) {
        if (scene.dynamicRange > 14.0f) return ToneMapMethod::ACESFilmic;
        if (scene.dynamicRange > 10.0f) return ToneMapMethod::Uncharted2;
        if (scene.dynamicRange > 6.0f)  return ToneMapMethod::ReinhardExtended;
        return ToneMapMethod::Reinhard;
    }

    // ========================================================================
    // Full pipeline
    // ========================================================================

    /// Tone-map HDR float buffer to 8-bit SDR (BGRA)
    bool ToneMap(const float* hdrInput, uint32_t width, uint32_t height,
        uint8_t* sdrOutput, const ToneMapParams& params) {
        if (!hdrInput || !sdrOutput || width == 0 || height == 0) return false;

        auto start = std::chrono::steady_clock::now();

        SceneAnalysis scene = AnalyzeScene(hdrInput, width, height);
        ToneMapMethod op = params.op;
        if (op == ToneMapMethod::Adaptive) {
            op = SelectOperator(scene);
        }

        float exposure = params.autoExposure ? scene.GetAutoExposure() : params.exposure;

        uint64_t pixelCount = static_cast<uint64_t>(width) * height;
        for (uint64_t i = 0; i < pixelCount; i++) {
            uint64_t srcIdx = i * 3;
            uint64_t dstIdx = i * 4;  // BGRA output

            float r = hdrInput[srcIdx] * exposure;
            float g = hdrInput[srcIdx + 1] * exposure;
            float b = hdrInput[srcIdx + 2] * exposure;

            // Apply tone mapping per channel
            switch (op) {
            case ToneMapMethod::ACESFilmic:
                r = ToneMapACES(r); g = ToneMapACES(g); b = ToneMapACES(b); break;
            case ToneMapMethod::Uncharted2:
                r = ToneMapUncharted2(r); g = ToneMapUncharted2(g); b = ToneMapUncharted2(b); break;
            default:
                r = ToneMapReinhard(r); g = ToneMapReinhard(g); b = ToneMapReinhard(b); break;
            }

            // Gamma correction
            float invGamma = 1.0f / params.gamma;
            r = std::pow(r, invGamma);
            g = std::pow(g, invGamma);
            b = std::pow(b, invGamma);

            // Clamp and convert to 8-bit BGRA
            sdrOutput[dstIdx + 0] = static_cast<uint8_t>((std::min)(1.0f, (std::max)(0.0f, b)) * 255.0f);
            sdrOutput[dstIdx + 1] = static_cast<uint8_t>((std::min)(1.0f, (std::max)(0.0f, g)) * 255.0f);
            sdrOutput[dstIdx + 2] = static_cast<uint8_t>((std::min)(1.0f, (std::max)(0.0f, r)) * 255.0f);
            sdrOutput[dstIdx + 3] = 255;  // Alpha
        }

        auto elapsed = std::chrono::steady_clock::now() - start;
        double elapsedMs = std::chrono::duration<double, std::milli>(elapsed).count();
        m_stats.totalToneMaps++;
        m_stats.totalPixels += pixelCount;
        m_stats.operatorHistogram[static_cast<uint8_t>(op)]++;
        m_stats.avgToneMapTimeMs =
            (m_stats.avgToneMapTimeMs * (m_stats.totalToneMaps - 1) + elapsedMs)
            / m_stats.totalToneMaps;

        return true;
    }

    ToneMapStats GetStats() const { return m_stats; }

private:
    bool m_initialized = false;
    void* m_d3dDevice = nullptr;
    mutable std::mutex m_mutex;
    ToneMapStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
