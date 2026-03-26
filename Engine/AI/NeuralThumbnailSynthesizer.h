// NeuralThumbnailSynthesizer.h — AI Thumbnail Generation from File Metadata
// Copyright (c) 2026 ExplorerLens Project
//
// Uses a lightweight generative model (DirectML/ONNX) to synthesize a representative
// thumbnail for files where decode is unavailable (e.g. encrypted, zero-byte, corrupt).
// Draws on filename tokens, extension, MIME type, and size class as conditioning input.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Input Conditioning -----------------------------------------------------

struct SynthesizerInput {
    std::string  filename;          // Including extension, no directory
    std::string  mimeType;          // "image/heic", "application/pdf", etc.
    uint64_t     fileSizeBytes = 0;
    std::string  lastModifiedISO;
    std::string  customHint;        // Optional free-text conditioning hint
};

// ---- Output -----------------------------------------------------------------

enum class SynthesisStatus {
    Success          = 0,
    ModelNotLoaded   = 1,
    InferenceError   = 2,
    InputTooShort    = 3,   // Not enough conditioning tokens
    InternalError    = 99,
};

struct SynthesisResult {
    SynthesisStatus      status    = SynthesisStatus::InternalError;
    std::vector<uint8_t> pixels;   // BGRA, 256×256 default
    uint32_t             width     = 0;
    uint32_t             height    = 0;
    float                confidence = 0.0f;  // Model logit confidence [0.0-1.0]
    std::string          backendUsed; // "directml", "onnx-cpu", "onnx-gpu"
};

// ---- NeuralThumbnailSynthesizer ---------------------------------------------

class NeuralThumbnailSynthesizer {
public:
    NeuralThumbnailSynthesizer();
    ~NeuralThumbnailSynthesizer();

    // Load the ONNX model from file (or embedded resource if path is empty).
    bool LoadModel(const std::string& modelPath = "");
    void UnloadModel();
    bool IsModelLoaded() const;

    // Synthesize a thumbnail for a file where normal decode is unavailable.
    SynthesisResult Synthesize(
        const SynthesizerInput& input,
        uint32_t targetWidth  = 256,
        uint32_t targetHeight = 256) const;

    // Recommend use: only call Synthesize() when no real decode path is available.
    static bool ShouldSynthesize(const std::string& localPath);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
