// GenerativeUpscalerV3.h — AI-Powered Image Upscaling for High-DPI Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Provides AI-powered image upscaling using diffusion and super-resolution models,
// supporting ESRGAN, RealSR, Stable Diffusion SR, and Bicubic with GPU acceleration.
//
#pragma once

#include <cstdint>
#include <optional>
#include <utility>

namespace ExplorerLens {
namespace Engine {

enum class UpscaleModel : uint8_t {
    ESRGAN,
    RealSR,
    StableDiffusionSR,
    Bicubic
};

enum class UpscaleFactor : uint8_t {
    x2,
    x3,
    x4,
    x8
};

struct UpscaleJob
{
    UpscaleModel model = UpscaleModel::ESRGAN;
    UpscaleFactor factor = UpscaleFactor::x2;
    float denoisingStrength = 0.4f;
    uint32_t tileSize = 512;
    bool useGPU = true;
};

class GenerativeUpscalerV3
{
  public:
    GenerativeUpscalerV3() = default;
    ~GenerativeUpscalerV3() = default;

    GenerativeUpscalerV3(GenerativeUpscalerV3 const&) = delete;
    GenerativeUpscalerV3& operator=(GenerativeUpscalerV3 const&) = delete;
    GenerativeUpscalerV3(GenerativeUpscalerV3&&) = default;
    GenerativeUpscalerV3& operator=(GenerativeUpscalerV3&&) = default;

    bool Upscale(void const* src, uint32_t srcW, uint32_t srcH, void* dst, UpscaleJob const& job);

    [[nodiscard]] std::pair<uint32_t, uint32_t> GetOutputDimensions(uint32_t srcW, uint32_t srcH,
                                                                    UpscaleFactor factor) const;

    [[nodiscard]] UpscaleModel SelectBestModel(uint32_t srcW, uint32_t srcH) const;
    [[nodiscard]] uint64_t GetVRAMRequiredBytes(UpscaleJob const& job) const;
    [[nodiscard]] bool IsModelAvailable(UpscaleModel model) const;

  private:
    std::optional<UpscaleModel> m_loadedModel;
    bool m_gpuAvailable = false;
    uint32_t m_tileSize = 512;
};

}  // namespace Engine
}  // namespace ExplorerLens
