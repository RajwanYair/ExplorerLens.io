// AIUpscaler.h — AI-Powered Thumbnail Upscaling (ONNX / DLSS / XeSS)
// Copyright (c) 2026 ExplorerLens Project
//
// GPU-accelerated super-resolution upscaling pipeline for ExplorerLens thumbnails.
// Supports three backend strategies:
//   - ONNX Runtime (DirectML EP) — universal, works on any DX12 GPU
//   - NVIDIA DLSS 3 (via DLSS SDK / NGX) — NVIDIA RTX series
//   - Intel XeSS (via XeSS SDK) — Intel Arc + other DX12 GPUs
//
// The pipeline executes in the existing GPU render thread and outputs BGRA pixels
// at the requested output resolution.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Upscaling backend selection.
enum class AIUpscaleBackend : uint8_t {
    Auto = 0,          // Auto-select best available (DLSS > XeSS > ONNX > bicubic)
    OnnxDirectML = 1,  // ONNX Runtime with DirectML execution provider
    NvidiaDlss = 2,    // NVIDIA DLSS 3 (requires RTX GPU + DLSS SDK)
    IntelXeSS = 3,     // Intel XeSS (requires DX12 GPU + XeSS SDK)
    Bicubic = 4,       // CPU bicubic — guaranteed fallback, no AI
};

// Quality preset maps to model input/output scale factor.
enum class UpscaleQuality : uint8_t {
    Performance = 0,  // 2× scale with fast ONNX model (RealESRGAN-x2-fast)
    Balanced = 1,     // 2× scale with full model (RealESRGAN-x2plus)
    Quality = 2,      // 4× scale (RealESRGAN-x4plus)
    Ultra = 3,        // 4× scale + post-sharpen (RealESRGAN-x4plus + RRDB)
};

// Per-request upscaling options.
struct UpscaleOptions
{
    AIUpscaleBackend backend{AIUpscaleBackend::Auto};
    UpscaleQuality quality{UpscaleQuality::Balanced};
    uint32_t targetWidth{0};  // 0 = infer from scale factor
    uint32_t targetHeight{0};
    bool enableSharpening{true};
    float sharpenStrength{0.5f};
};

// Result of an upscale operation.
struct UpscaleResult
{
    std::vector<uint8_t> pixels;  // BGRA raw pixels
    uint32_t width{0};
    uint32_t height{0};
    AIUpscaleBackend backendUsed;
    double latencyMs{0.0};
    bool success{false};
    std::string errorMessage;
};

// AIUpscaler — Super-resolution thumbnail upscaler.
//
// Initialise once per process; call Upscale() on any BGRA input buffer.
// Thread-safe for concurrent calls from different decoder threads.
class AIUpscaler
{
  public:
    AIUpscaler() noexcept;
    ~AIUpscaler() noexcept;

    AIUpscaler(const AIUpscaler&) = delete;
    AIUpscaler& operator=(const AIUpscaler&) = delete;

    // Detect available backends on the current GPU.
    // Returns false only if even bicubic is unavailable (should never happen).
    bool Initialize() noexcept;

    // Check if a specific backend is available.
    bool IsBackendAvailable(AIUpscaleBackend backend) const noexcept;

    // Get the best automatically-selected backend.
    AIUpscaleBackend GetBestBackend() const noexcept;

    // Upscale a BGRA input buffer.
    UpscaleResult Upscale(const uint8_t* inputBgra, uint32_t inputWidth, uint32_t inputHeight,
                          const UpscaleOptions& opts = {}) noexcept;

    // Load/unload an ONNX model file.  Only needed if using OnnxDirectML backend.
    bool LoadOnnxModel(const std::string& modelPath) noexcept;
    void UnloadOnnxModel() noexcept;

    // Singleton accessor.
    static AIUpscaler& Instance() noexcept;

  private:
    bool m_initialized{false};
    AIUpscaleBackend m_bestBackend{AIUpscaleBackend::Bicubic};

    struct Impl
    {};
    Impl* m_impl{nullptr};
};

}  // namespace Engine
}  // namespace ExplorerLens
