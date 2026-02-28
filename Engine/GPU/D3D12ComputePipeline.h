#pragma once
//==============================================================================
// D3D12ComputePipeline
// GPU compute shader scaling via DirectX 12 with D3D11 fallback
//
// Architecture:
// 1. Check D3D12 availability at runtime
// 2. Create compute pipeline state with scaling shader
// 3. Execute GPU resize via compute dispatch
// 4. Fall back to D3D11 or CPU if D3D12 unavailable
//
// Shader operations:
// - Bilinear/bicubic/Lanczos resize
// - Color space conversion (sRGB ↔ linear)
// - HDR tone mapping (Reinhard/ACES)
// - Alpha premultiply/unpremultiply
//==============================================================================

#include "../Core/D3D12PipelineActivation.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Scaling algorithm for compute shaders
enum class ScalingAlgorithm : uint8_t {
 NearestNeighbor, ///< Fastest, worst quality
 Bilinear, ///< Good balance
 Bicubic, ///< Better quality
 Lanczos3, ///< Best quality, slowest
 Adaptive ///< Auto-select based on scale factor
};

/// Color space for GPU operations
enum class GPUColorSpace : uint8_t { SRGB, LinearRGB, HDR10, HLG, DolbyVision };

/// Tone mapping operator for HDR content
enum class ToneMapOperator : uint8_t {
 None,
 Reinhard,
 ReinhardExtended,
 ACES,
 Uncharted2,
 HableFilmic
};

/// D3D12 feature level requirements
struct D3D12Requirements {
 bool computeShaderSupport = false;
 bool typedUAVLoadSupport = false;
 bool rootSignature1_1 = false;
 uint32_t maxComputeWorkGroupSize = 0;
 uint64_t dedicatedVideoMemory = 0;
 std::wstring adapterDescription;
};

/// Compute pipeline configuration
struct ComputePipelineConfig {
 GPUBackend preferredBackend = GPUBackend::Auto;
 ScalingAlgorithm scalingAlgorithm = ScalingAlgorithm::Bilinear;
 GPUColorSpace inputColorSpace = GPUColorSpace::SRGB;
 GPUColorSpace outputColorSpace = GPUColorSpace::SRGB;
 ToneMapOperator toneMapper = ToneMapOperator::None;
 uint32_t workGroupSizeX = 8;
 uint32_t workGroupSizeY = 8;
 bool enableAlphaPremultiply = false;
 bool enableGammaCorrection = true;
 bool enableAsyncCompute = false;
 uint32_t maxTextureSize = 16384;
};

/// Result of a GPU compute operation
struct ComputeResult {
 bool success = false;
 GPUBackend usedBackend = GPUBackend::CPU;
 uint32_t outputWidth = 0;
 uint32_t outputHeight = 0;
 std::vector<uint8_t> outputData; ///< BGRA32 pixel data
 double gpuTimeMs = 0.0;
 double totalTimeMs = 0.0;
 std::wstring errorMessage;
};

/// GPU compute statistics
struct ComputeStats {
 uint64_t totalDispatches = 0;
 uint64_t d3d12Dispatches = 0;
 uint64_t d3d11Fallbacks = 0;
 uint64_t cpuFallbacks = 0;
 double avgGpuTimeMs = 0.0;
 double avgCpuFallbackTimeMs = 0.0;
 uint64_t totalPixelsProcessed = 0;
 GPUBackend activeBackend = GPUBackend::CPU;
};

//==============================================================================
// D3D12ComputePipeline
//==============================================================================
class D3D12ComputePipeline {
public:
 D3D12ComputePipeline();
 explicit D3D12ComputePipeline(const ComputePipelineConfig &config);
 ~D3D12ComputePipeline();

 // Non-copyable
 D3D12ComputePipeline(const D3D12ComputePipeline &) = delete;
 D3D12ComputePipeline &operator=(const D3D12ComputePipeline &) = delete;

 /// Probe GPU capabilities and initialize pipeline
 bool Initialize();

 /// Shutdown and release GPU resources
 void Shutdown();

 /// Check D3D12 hardware requirements
 D3D12Requirements ProbeHardware() const;

 /// Resize an image using GPU compute pipeline
 ComputeResult Resize(const uint8_t *inputData, uint32_t inputWidth,
 uint32_t inputHeight, uint32_t outputWidth,
 uint32_t outputHeight);

 /// Apply tone mapping to HDR image
 ComputeResult ToneMap(const uint8_t *inputData, uint32_t width,
 uint32_t height, ToneMapOperator op);

 /// Convert color space
 ComputeResult ConvertColorSpace(const uint8_t *inputData, uint32_t width,
 uint32_t height, GPUColorSpace srcSpace,
 GPUColorSpace dstSpace);

 /// Get compute statistics
 ComputeStats GetStats() const;

 /// Check if pipeline is initialized
 bool IsInitialized() const;

 /// Get active GPU backend
 GPUBackend GetActiveBackend() const;

 /// Get configuration
 const ComputePipelineConfig &GetConfig() const { return m_config; }

 /// Static helpers
 static const wchar_t *GetBackendName(GPUBackend backend);
 static const wchar_t *GetAlgorithmName(ScalingAlgorithm algo);
 static const wchar_t *GetColorSpaceName(GPUColorSpace cs);
 static const wchar_t *GetToneMapName(ToneMapOperator op);

private:
 /// CPU fallback resize implementation
 ComputeResult ResizeCPU(const uint8_t *inputData, uint32_t inputWidth,
 uint32_t inputHeight, uint32_t outputWidth,
 uint32_t outputHeight);

 /// Select best available backend
 GPUBackend SelectBackend() const;

 /// Calculate dispatch dimensions
 void CalculateDispatch(uint32_t width, uint32_t height, uint32_t &groupsX,
 uint32_t &groupsY) const;

 ComputePipelineConfig m_config;
 bool m_initialized = false;
 GPUBackend m_activeBackend = GPUBackend::CPU;
 ComputeStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
