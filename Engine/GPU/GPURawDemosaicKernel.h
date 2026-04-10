// GPURawDemosaicKernel.h — GPU Compute Kernel for RAW Camera Demosaic
// Copyright (c) 2026 ExplorerLens Project
//
// GPU-accelerated Bayer-pattern demosaic + white balance compute kernel.
// Supports RGGB / BGGR / GRBG / GBRG CFA patterns and all standard
// white balance presets. Targets sub-9 ms for 24 MP RAW files (v34.1.0 T2).
//
#pragma once
#include <cstdint>
#include <array>
#include <memory>

namespace ExplorerLens { namespace Engine {

enum class BayerPattern : uint8_t {
    RGGB = 0,
    BGGR = 1,
    GRBG = 2,
    GBRG = 3,
};

enum class DemosaicAlgorithm : uint8_t {
    Bilinear    = 0,  // Fast; acceptable at thumbnail scale
    PPG         = 1,  // Patterned pixel grouping; good balance
    AHD         = 2,  // Adaptive Homogeneity-Directed; best quality
};

struct RAWDemosaicParams {
    BayerPattern    bayerPattern      = BayerPattern::RGGB;
    DemosaicAlgorithm algorithm       = DemosaicAlgorithm::Bilinear;
    float           whiteBalanceR     = 1.0f;
    float           whiteBalanceG     = 1.0f;
    float           whiteBalanceB     = 1.0f;
    uint16_t        blackLevel        = 0;
    uint16_t        whiteLevel        = 16383;  // 14-bit sensor default
    bool            applyGamma        = true;   // Apply sRGB gamma to output
};

struct RAWDemosaicResult {
    uint8_t* pixelsBGRA  = nullptr;  // Caller-owned; w × h × 4 bytes
    uint32_t width       = 0;
    uint32_t height      = 0;
    bool     success     = false;
    float    demosaicMs  = 0.0f;     // Wall-clock time for demosaic pass
    bool     usedGPU     = false;    // True if compute shader executed
};

class GPURawDemosaicKernel {
public:
    GPURawDemosaicKernel();
    ~GPURawDemosaicKernel();

    // Returns true if a D3D12 / Vulkan compute capable device was found.
    bool Initialize() noexcept;

    // Demosaic `rawData` (packed 16-bit Bayer) to BGRA32 thumbnail.
    // `rawData` must be rawWidth × rawHeight × 2 bytes (uint16_t pixels).
    RAWDemosaicResult Demosaic(
        const uint16_t*         rawData,
        uint32_t                rawWidth,
        uint32_t                rawHeight,
        const RAWDemosaicParams& params,
        uint32_t                targetWidth  = 256,
        uint32_t                targetHeight = 256) noexcept;

    bool IsAvailable() const noexcept { return m_initialized; }

    // Inline white balance helpers.
    static std::array<float, 3> DaylightWB()   noexcept { return {2.0f, 1.0f, 1.5f}; }
    static std::array<float, 3> TungstenWB()   noexcept { return {1.2f, 1.0f, 2.4f}; }
    static std::array<float, 3> FlorescentWB() noexcept { return {1.8f, 1.0f, 1.9f}; }

private:
    bool InitD3D12Kernel() noexcept;
    bool InitVulkanKernel() noexcept;

    bool m_initialized = false;
    bool m_useD3D12    = false;   // true = D3D12, false = Vulkan

    struct Impl;
    std::unique_ptr<struct Impl> m_impl;
};

}} // namespace ExplorerLens::Engine
