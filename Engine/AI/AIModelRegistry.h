// AIModelRegistry.h — AI Inference Model Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Manages ONNX model files for all ExplorerLens AI modules: lists available
// models, tracks their versions, loads them into the DirectML/ONNX runtime,
// and supports hot-swap model updates without restarting the shell extension.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Model Descriptor -------------------------------------------------------

enum class AIModelRole : uint8_t {
    ContentClassifier       = 0,
    SmartCrop               = 1,
    BlurDetector            = 2,
    Deblur                  = 3,
    ThumbnailSynthesizer    = 4,
    NSFWGuard               = 5,
    SearchEmbedding         = 6,
    SceneUnderstanding      = 7,
    IQAScorer               = 8,
};

enum class AIBackend : uint8_t {
    DirectML    = 0,   // Windows ML / DirectX 12 GPU
    ONNXCPU     = 1,   // ONNX Runtime CPU EP
    OpenVINO    = 2,   // Intel iGPU / NPU
    CUDA        = 3,   // NVIDIA CUDA EP (optional)
};

struct AIModelDescriptor {
    AIModelRole   role;
    std::string   modelId;        // "explorerlens-classifier-v2"
    std::string   modelPath;      // Absolute path to .onnx file
    std::string   version;        // SemVer
    std::string   checksum;       // SHA-256
    uint64_t      fileSizeBytes   = 0;
    uint64_t      vramEstimateKB  = 0;
    bool          isLoaded        = false;
    AIBackend     preferredBackend = AIBackend::DirectML;
};

// ---- Registry ---------------------------------------------------------------

class AIModelRegistry {
public:
    AIModelRegistry();
    ~AIModelRegistry();

    // Scan a directory for .onnx model files and register them.
    void ScanDirectory(const std::string& modelDir);

    // Manually register a model.
    void Register(const AIModelDescriptor& desc);

    // Load a model into the inference runtime.
    bool Load(AIModelRole role, std::string& outError);

    // Unload a model, freeing VRAM.
    void Unload(AIModelRole role);

    // Hot-swap a model file without DLL reload (waits for active inferences).
    bool HotSwap(AIModelRole role, const std::string& newModelPath, std::string& outError);

    // Check if a model is loaded and ready.
    bool IsLoaded(AIModelRole role) const;

    // Get descriptor for a loaded model.
    bool GetDescriptor(AIModelRole role, AIModelDescriptor& out) const;

    // Total VRAM used by all loaded models (KB).
    uint64_t TotalVRAMUsedKB() const;

    // Load all models in modelDir (call once at startup).
    void LoadAll(const std::string& modelDir = "");
    void UnloadAll();

    static AIModelRegistry& Instance();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
