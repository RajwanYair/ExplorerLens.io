#pragma once

#include <string>
#include <vector>
#include <future>
#include <cstdint>

namespace DarkThumbs::Engine::AI::Models {

    enum class ModelType {
        SaliencyDetection,  // For Smart Crop
        ObjectClassification, // For tagging
        TextDetection,      // For document OCR/layout analysis
        NSFWFilter          // Safety
    };

    struct ModelInfo {
        std::wstring id;
        ModelType type;
        std::wstring version;
        uint64_t sizeBytes;
        std::wstring signatureHash;
        bool isDownloaded;
    };

    class IModelRegistry {
    public:
        virtual ~IModelRegistry() = default;

        // Get metadata for a specific model type
        virtual ModelInfo GetModelInfo(ModelType type) = 0;

        // Ensure the model is present on disk and verified. 
        // If not, attempts to download it (if policy allows).
        virtual std::future<bool> EnsureModelAvailable(ModelType type) = 0;

        // Load the model bytes into memory for the InferenceEngine
        virtual std::future<std::vector<uint8_t>> LoadModelBytes(ModelType type) = 0;

        // Verify the cryptographic signature of the model file
        virtual bool VerifySignature(const std::wstring& modelId) = 0;
    };

}
