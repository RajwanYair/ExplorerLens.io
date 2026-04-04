// GenerativeThumbnailEngine.h — Generative AI Thumbnail Orchestration
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates generative AI pipelines for creating context-aware thumbnails
// from file content, routing requests across DirectML, ONNX, OpenVINO, and CPU backends.
//
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GenerativeBackend : uint8_t {
    DirectML,
    ONNX,
    OpenVINO,
    CPU
};

enum class GenerationMode : uint8_t {
    TextToImage,
    ImageVariation,
    StyleAdaptation,
    ContentSynthesis
};

struct GenerationRequest
{
    GenerativeBackend backend = GenerativeBackend::CPU;
    GenerationMode mode = GenerationMode::ContentSynthesis;
    std::string promptHint;
    uint32_t maxTokens = 256;
    uint64_t seed = 0;
};

class GenerativeThumbnailEngine
{
  public:
    GenerativeThumbnailEngine() = default;
    ~GenerativeThumbnailEngine() = default;

    GenerativeThumbnailEngine(GenerativeThumbnailEngine const&) = delete;
    GenerativeThumbnailEngine& operator=(GenerativeThumbnailEngine const&) = delete;
    GenerativeThumbnailEngine(GenerativeThumbnailEngine&&) = default;
    GenerativeThumbnailEngine& operator=(GenerativeThumbnailEngine&&) = default;

    bool Generate(GenerationRequest const& request);
    void SetBackend(GenerativeBackend backend);

    [[nodiscard]] std::vector<GenerationMode> GetSupportedModes() const;
    [[nodiscard]] bool IsAcceleratorAvailable(GenerativeBackend backend) const;

    void Shutdown();

  private:
    GenerativeBackend m_backend = GenerativeBackend::CPU;
    std::string m_activeModel;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
