#pragma once

#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <memory>

namespace DarkThumbs::Engine::AI::Inference {

    enum class ExecutionProvider {
        CPU,
        DirectML,   // GPU
        OpenVINO,   // Intel optimized
        CUDA        // Nvidia optimized
    };

    struct TensorShape {
        std::vector<int64_t> dims;
    };

    struct Tensor {
        std::string name;
        TensorShape shape;
        std::vector<float> data; // Simplified float tensor
    };

    class IInferenceEngine {
    public:
        virtual ~IInferenceEngine() = default;

        // Load a model from a file buffer
        // Returns a handle to the loaded session
        virtual uint64_t LoadSession(const std::vector<uint8_t>& modelData, ExecutionProvider provider) = 0;

        // Run inference
        // sessionId: Handle returned by LoadSession
        // inputs: Map of input name -> Tensor
        // outputNames: List of output tensor names to retrieve
        virtual std::map<std::string, Tensor> Run(uint64_t sessionId, const std::map<std::string, Tensor>& inputs, const std::vector<std::string>& outputNames) = 0;

        // Cleanup
        virtual void UnloadSession(uint64_t sessionId) = 0;

        // Check if a provider is available on this machine
        virtual bool IsProviderAvailable(ExecutionProvider provider) const = 0;
    };

    // Factory
    std::shared_ptr<IInferenceEngine> CreateInferenceEngine();
}
