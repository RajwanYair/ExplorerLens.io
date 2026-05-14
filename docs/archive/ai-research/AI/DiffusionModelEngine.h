// DiffusionModelEngine.h — On-device diffusion model inference engine
// Copyright (c) 2026 ExplorerLens Project
//
// Manages the on-device diffusion model lifecycle for generative thumbnail
// synthesis. Handles model loading, VRAM allocation, denoising step scheduling,
// and latent-to-pixel decoding. Routes inference to NPU (DirectML), ONNX, or CPU.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class DiffusionModelState : uint8_t
{
    Unloaded   = 0,
    Loading    = 1,
    Ready      = 2,
    Inference  = 3,
    Error      = 4,
};

struct DiffusionModelConfig
{
    uint32_t    latentWidth  = 32;
    uint32_t    latentHeight = 32;
    uint32_t    channels     = 4;
    uint32_t    steps        = 4;
    std::string modelPath;
};

struct LatentVector
{
    std::vector<float> data;
    uint32_t           width    = 0;
    uint32_t           height   = 0;
    uint32_t           channels = 4;
};

class DiffusionModelEngine
{
public:
    DiffusionModelEngine();
    ~DiffusionModelEngine();

    DiffusionModelEngine(const DiffusionModelEngine&)            = delete;
    DiffusionModelEngine& operator=(const DiffusionModelEngine&) = delete;

    bool                 LoadModel(const DiffusionModelConfig& config);
    void                 UnloadModel();
    LatentVector         EncodePrompt(const std::string& prompt) const;
    std::vector<uint8_t> Decode(const LatentVector& latent, uint32_t width, uint32_t height) const;
    DiffusionModelState  GetState()  const noexcept { return m_state; }
    bool                 IsReady()   const noexcept { return m_state == DiffusionModelState::Ready; }
    const char*          ModelName() const noexcept { return m_modelName.c_str(); }

    static DiffusionModelEngine& Instance() noexcept;

private:
    DiffusionModelState  m_state     = DiffusionModelState::Unloaded;
    std::string          m_modelName;
    static DiffusionModelEngine s_instance;
};

}} // namespace ExplorerLens::Engine
