// GPURawDemosaicKernel.cpp — GPU Compute Kernel for RAW Camera Demosaic
// Copyright (c) 2026 ExplorerLens Project
//
#include "GPU/GPURawDemosaicKernel.h"
#include <chrono>
#include <cstring>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct GPURawDemosaicKernel::Impl {
    // D3D12 / Vulkan compute pipeline handles would live here.
};

GPURawDemosaicKernel::GPURawDemosaicKernel()
    : m_impl(std::make_unique<Impl>())
{}

GPURawDemosaicKernel::~GPURawDemosaicKernel() = default;

bool GPURawDemosaicKernel::Initialize() noexcept
{
    if (InitD3D12Kernel())  { m_useD3D12 = true;  m_initialized = true; return true; }
    if (InitVulkanKernel()) { m_useD3D12 = false; m_initialized = true; return true; }
    // CPU fallback always succeeds.
    m_initialized = true;
    return true;
}

bool GPURawDemosaicKernel::InitD3D12Kernel() noexcept
{
    // D3D12 compute shader setup — stub (requires D3D12 runtime, not linked here).
    return false;
}

bool GPURawDemosaicKernel::InitVulkanKernel() noexcept
{
    // Vulkan compute pipeline setup — stub.
    return false;
}

RAWDemosaicResult GPURawDemosaicKernel::Demosaic(
    const uint16_t*          rawData,
    uint32_t                 rawWidth,
    uint32_t                 rawHeight,
    const RAWDemosaicParams& params,
    uint32_t                 targetWidth,
    uint32_t                 targetHeight) noexcept
{
    RAWDemosaicResult result{};
    if (!m_initialized || !rawData || rawWidth == 0 || rawHeight == 0) return result;

    const auto t0 = std::chrono::high_resolution_clock::now();

    const size_t bufSize = static_cast<size_t>(targetWidth) * targetHeight * 4u;
    result.pixelsBGRA = new (std::nothrow) uint8_t[bufSize];
    if (!result.pixelsBGRA) return result;

    // Simple CPU bilinear demosaic (placeholder for the compute shader path).
    // For each output pixel, sample the Bayer grid with basic interpolation.
    const float scaleX = static_cast<float>(rawWidth)  / targetWidth;
    const float scaleY = static_cast<float>(rawHeight) / targetHeight;
    const float rangeScale = 255.0f / static_cast<float>(
        params.whiteLevel - params.blackLevel);

    for (uint32_t y = 0; y < targetHeight; ++y) {
        for (uint32_t x = 0; x < targetWidth; ++x) {
            const uint32_t sx = static_cast<uint32_t>(x * scaleX) & ~1u;  // align to Bayer cell
            const uint32_t sy = static_cast<uint32_t>(y * scaleY) & ~1u;

            auto clampedPx = [&](uint32_t px, uint32_t py) -> uint16_t {
                px = std::min(px, rawWidth  - 1);
                py = std::min(py, rawHeight - 1);
                int v = static_cast<int>(rawData[py * rawWidth + px]) - params.blackLevel;
                if (v < 0) v = 0;
                return static_cast<uint16_t>(v);
            };

            // RGGB cell at (sx, sy)
            const float r = clampedPx(sx,     sy)     * params.whiteBalanceR * rangeScale;
            const float g = (clampedPx(sx + 1, sy) + clampedPx(sx, sy + 1)) * 0.5f
                            * params.whiteBalanceG * rangeScale;
            const float b = clampedPx(sx + 1, sy + 1) * params.whiteBalanceB * rangeScale;

            auto clamp8 = [](float v) -> uint8_t {
                if (v < 0.0f) return 0;
                if (v > 255.0f) return 255;
                return static_cast<uint8_t>(v);
            };

            uint8_t* px = result.pixelsBGRA + (y * targetWidth + x) * 4;
            px[0] = clamp8(b);  // B
            px[1] = clamp8(g);  // G
            px[2] = clamp8(r);  // R
            px[3] = 0xFF;       // A
        }
    }

    result.width      = targetWidth;
    result.height     = targetHeight;
    result.success    = true;
    result.usedGPU    = m_useD3D12;

    const auto t1 = std::chrono::high_resolution_clock::now();
    result.demosaicMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
    return result;
}

}} // namespace ExplorerLens::Engine
