#pragma once
//==============================================================================
// VulkanComputePipeline — Sprint 202
// Cross-API GPU compute pipeline for thumbnail scaling/processing.
//
// Features:
//   - Vulkan instance/device/queue abstraction
//   - Compute shader dispatch for image resize and color conversion
//   - Memory management with staging buffers
//   - Fallback to D3D12/D3D11/CPU when Vulkan unavailable
//   - Pipeline caching for repeated operations
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs { namespace Engine {

/// GPU API backend
enum class GPUBackend : uint8_t {
    None,
    Vulkan,
    D3D12,
    D3D11,
    CPU
};

/// Vulkan device info
struct VulkanDevice {
    std::wstring name;
    uint32_t vendorId = 0;
    uint32_t deviceId = 0;
    uint32_t driverVersion = 0;
    uint32_t apiVersion = 0;
    uint64_t totalMemory = 0;
    uint32_t computeQueueFamily = UINT32_MAX;
    uint32_t maxComputeWorkGroupSize = 256;
    bool supportsFloat16 = false;
    bool supportsInt8 = false;
};

/// Compute shader type
enum class ComputeShaderType : uint8_t {
    BilinearResize,
    LanczosResize,
    ColorSpaceConvert,
    GammaCorrect,
    Sharpen,
    ToneMap
};

/// Compute dispatch parameters
struct ComputeDispatch {
    ComputeShaderType shader = ComputeShaderType::BilinearResize;
    uint32_t srcWidth = 0;
    uint32_t srcHeight = 0;
    uint32_t dstWidth = 256;
    uint32_t dstHeight = 256;
    uint32_t workGroupSizeX = 16;
    uint32_t workGroupSizeY = 16;
    float gamma = 2.2f;
    float sharpenStrength = 0.5f;
};

/// Compute pipeline statistics
struct ComputeStats {
    uint64_t dispatchCount = 0;
    double totalTimeMs = 0.0;
    double avgTimeMs = 0.0;
    uint64_t bytesProcessed = 0;
    GPUBackend activeBackend = GPUBackend::None;
};

/// Pipeline cache entry
struct PipelineCacheEntry {
    ComputeShaderType shader;
    uint64_t pipelineHandle = 0;
    bool valid = false;
};

//==============================================================================
// VulkanComputePipeline
//==============================================================================
class VulkanComputePipeline {
public:
    VulkanComputePipeline();
    ~VulkanComputePipeline();

    /// Initialize Vulkan (or fallback backend)
    bool Initialize();

    /// Check Vulkan availability
    static bool IsVulkanAvailable();

    /// Get selected GPU backend
    GPUBackend GetActiveBackend() const { return m_backend; }

    /// Get device info
    const VulkanDevice& GetDevice() const { return m_device; }

    /// Dispatch compute shader
    bool Dispatch(const ComputeDispatch& params,
                  const uint8_t* srcData, size_t srcSize,
                  uint8_t* dstData, size_t dstSize);

    /// Execute resize operation
    std::vector<uint8_t> Resize(const uint8_t* srcBGRA,
                                 uint32_t srcW, uint32_t srcH,
                                 uint32_t dstW, uint32_t dstH);

    /// Get pipeline statistics
    const ComputeStats& GetStats() const { return m_stats; }

    /// Get supported shader count
    static uint32_t GetShaderCount();

    /// Get shader name
    static const wchar_t* GetShaderName(ComputeShaderType type);
    static const wchar_t* GetBackendName(GPUBackend backend);

    /// Warm up pipeline cache
    void WarmCache(ComputeShaderType shader);

    /// Clear statistics
    void ResetStats();

private:
    GPUBackend m_backend = GPUBackend::None;
    VulkanDevice m_device;
    ComputeStats m_stats;
    std::vector<PipelineCacheEntry> m_pipelineCache;

    bool InitializeVulkan();
    bool InitializeFallback();
    void CPUResize(const uint8_t* src, uint32_t srcW, uint32_t srcH,
                   uint8_t* dst, uint32_t dstW, uint32_t dstH);
};

}} // namespace DarkThumbs::Engine
