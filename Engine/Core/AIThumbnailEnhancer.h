// AIThumbnailEnhancer.h — AI-Enhanced Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// ML-based thumbnail quality enhancement: super-resolution, auto-crop, saliency.
// NOTE: Cannot forward to Engine/AI/AIThumbnailEnhancer.h — requires ONNX Runtime SDK.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// AI enhancement type
enum class AIEnhancement : uint8_t {
    SuperResolution,    // 2x–4x upscale via neural network
    AutoCrop,           // Content-aware smart crop
    SaliencyDetection,  // Focus on interesting regions
    NoiseReduction,     // Denoise low-light images
    ColorCorrection,    // Auto white balance / exposure
    FaceDetection,      // Center thumbnail on faces
    TextEnhance,        // Sharpen text in document previews
    COUNT
};

/// AI model backend
enum class AIModelBackend : uint8_t {
    DirectML,     // Windows ML via DirectML
    ONNX,         // ONNX Runtime
    OpenVINO,     // Intel OpenVINO
    CPUFallback,  // Simple CPU algorithms
    COUNT
};

/// AI enhancement config
struct AIEnhancementConfig
{
    AIEnhancement type = AIEnhancement::SuperResolution;
    AIModelBackend backend = AIModelBackend::CPUFallback;
    float qualityTarget = 0.85f;  // 0-1 quality score target
    uint32_t maxProcessMs = 100;  // Max time per thumbnail
    bool gpuAccelerate = true;
    bool batchMode = false;
};

/// AI enhancement result
struct AIEnhancementResult
{
    bool applied = false;
    float confidence = 0.0f;
    uint32_t processMs = 0;
    std::wstring modelUsed;
};

/// AI thumbnail enhancement manager
class AIThumbnailEnhancer
{
  public:
    static const wchar_t* EnhancementName(AIEnhancement e)
    {
        switch (e) {
            case AIEnhancement::SuperResolution:
                return L"Super Resolution";
            case AIEnhancement::AutoCrop:
                return L"Auto Crop";
            case AIEnhancement::SaliencyDetection:
                return L"Saliency Detection";
            case AIEnhancement::NoiseReduction:
                return L"Noise Reduction";
            case AIEnhancement::ColorCorrection:
                return L"Color Correction";
            case AIEnhancement::FaceDetection:
                return L"Face Detection";
            case AIEnhancement::TextEnhance:
                return L"Text Enhance";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* BackendName(AIModelBackend b)
    {
        switch (b) {
            case AIModelBackend::DirectML:
                return L"DirectML";
            case AIModelBackend::ONNX:
                return L"ONNX Runtime";
            case AIModelBackend::OpenVINO:
                return L"OpenVINO";
            case AIModelBackend::CPUFallback:
                return L"CPU Fallback";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t EnhancementCount()
    {
        return static_cast<size_t>(AIEnhancement::COUNT);
    }
    static constexpr size_t BackendCount()
    {
        return static_cast<size_t>(AIModelBackend::COUNT);
    }

    static bool IsQualityValid(float q)
    {
        return q >= 0.0f && q <= 1.0f;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
