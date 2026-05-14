//==============================================================================
// D3D12ComputePipeline
// GPU compute shader scaling with D3D11/CPU fallback
//==============================================================================

#include "D3D12ComputePipeline.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

//------------------------------------------------------------------------------
// Construction / Destruction
//------------------------------------------------------------------------------

D3D12ComputePipeline::D3D12ComputePipeline() : m_config() {}

D3D12ComputePipeline::D3D12ComputePipeline(const ComputePipelineConfig& config) : m_config(config) {}

D3D12ComputePipeline::~D3D12ComputePipeline()
{
    Shutdown();
}

//------------------------------------------------------------------------------
// Lifecycle
//------------------------------------------------------------------------------

bool D3D12ComputePipeline::Initialize()
{
    if (m_initialized)
        return true;

    m_activeBackend = SelectBackend();
    m_initialized = true;
    return true;
}

void D3D12ComputePipeline::Shutdown()
{
    if (!m_initialized)
        return;
    m_initialized = false;
    m_activeBackend = GPUBackend::CPU;
}

D3D12Requirements D3D12ComputePipeline::ProbeHardware() const
{
    D3D12Requirements reqs;
    // In production, this would use D3D12CreateDevice + CheckFeatureSupport
    // For now, report CPU-only capability
    reqs.computeShaderSupport = false;
    reqs.typedUAVLoadSupport = false;
    reqs.rootSignature1_1 = false;
    reqs.maxComputeWorkGroupSize = 0;
    reqs.dedicatedVideoMemory = 0;
    reqs.adapterDescription = L"CPU Fallback";
    return reqs;
}

//------------------------------------------------------------------------------
// Compute Operations
//------------------------------------------------------------------------------

ComputeResult D3D12ComputePipeline::Resize(const uint8_t* inputData, uint32_t inputWidth, uint32_t inputHeight,
                                           uint32_t outputWidth, uint32_t outputHeight)
{
    if (!m_initialized) {
        ComputeResult result;
        result.success = false;
        result.errorMessage = L"Pipeline not initialized";
        return result;
    }

    auto startTime = std::chrono::steady_clock::now();

    // Currently uses CPU fallback — D3D12 dispatch to be added
    auto result = ResizeCPU(inputData, inputWidth, inputHeight, outputWidth, outputHeight);

    auto endTime = std::chrono::steady_clock::now();
    result.totalTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    result.usedBackend = m_activeBackend;

    // Update stats
    m_stats.totalDispatches++;
    m_stats.cpuFallbacks++;
    m_stats.totalPixelsProcessed += static_cast<uint64_t>(outputWidth) * outputHeight;

    return result;
}

ComputeResult D3D12ComputePipeline::ToneMap(const uint8_t* inputData, uint32_t width, uint32_t height,
                                            ToneMapOperator op)
{
    ComputeResult result;
    if (!m_initialized || !inputData || width == 0 || height == 0) {
        result.success = false;
        result.errorMessage = L"Invalid input for tone mapping";
        return result;
    }

    auto startTime = std::chrono::steady_clock::now();

    // CPU tone mapping fallback
    uint32_t pixelCount = width * height;
    result.outputData.resize(pixelCount * 4);
    const uint8_t* src = inputData;
    uint8_t* dst = result.outputData.data();

    for (uint32_t i = 0; i < pixelCount; ++i) {
        float r = src[i * 4 + 0] / 255.0f;
        float g = src[i * 4 + 1] / 255.0f;
        float b = src[i * 4 + 2] / 255.0f;
        uint8_t a = src[i * 4 + 3];

        switch (op) {
            case ToneMapOperator::Reinhard:
                r = r / (1.0f + r);
                g = g / (1.0f + g);
                b = b / (1.0f + b);
                break;
            case ToneMapOperator::ACES: {
                // Simplified ACES filmic
                auto aces = [](float x) {
                    float a2 = 2.51f, b2 = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
                    return (std::min)(1.0f, (std::max)(0.0f, (x * (a2 * x + b2)) / (x * (c * x + d) + e)));
                };
                r = aces(r);
                g = aces(g);
                b = aces(b);
                break;
            }
            default:
                break;  // No-op for None
        }

        dst[i * 4 + 0] = static_cast<uint8_t>(r * 255.0f);
        dst[i * 4 + 1] = static_cast<uint8_t>(g * 255.0f);
        dst[i * 4 + 2] = static_cast<uint8_t>(b * 255.0f);
        dst[i * 4 + 3] = a;
    }

    auto endTime = std::chrono::steady_clock::now();
    result.success = true;
    result.outputWidth = width;
    result.outputHeight = height;
    result.usedBackend = GPUBackend::CPU;
    result.totalTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    result.gpuTimeMs = 0.0;

    m_stats.totalDispatches++;
    m_stats.cpuFallbacks++;
    return result;
}

ComputeResult D3D12ComputePipeline::ConvertColorSpace(const uint8_t* inputData, uint32_t width, uint32_t height,
                                                      GPUColorSpace srcSpace, GPUColorSpace dstSpace)
{
    ComputeResult result;
    if (!m_initialized || !inputData || width == 0 || height == 0) {
        result.success = false;
        result.errorMessage = L"Invalid input for color space conversion";
        return result;
    }

    if (srcSpace == dstSpace) {
        // No conversion needed — copy input
        uint32_t size = width * height * 4;
        result.outputData.assign(inputData, inputData + size);
        result.success = true;
        result.outputWidth = width;
        result.outputHeight = height;
        result.usedBackend = GPUBackend::CPU;
        return result;
    }

    auto startTime = std::chrono::steady_clock::now();

    uint32_t pixelCount = width * height;
    result.outputData.resize(pixelCount * 4);

    // sRGB ↔ Linear conversion
    for (uint32_t i = 0; i < pixelCount; ++i) {
        for (int c = 0; c < 3; ++c) {
            float v = inputData[i * 4 + c] / 255.0f;
            float converted = v;

            if (srcSpace == GPUColorSpace::SRGB && dstSpace == GPUColorSpace::LinearRGB) {
                // sRGB to linear
                converted = (v <= 0.04045f) ? (v / 12.92f) : std::pow((v + 0.055f) / 1.055f, 2.4f);
            } else if (srcSpace == GPUColorSpace::LinearRGB && dstSpace == GPUColorSpace::SRGB) {
                // Linear to sRGB
                converted = (v <= 0.0031308f) ? (v * 12.92f) : (1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f);
            }

            result.outputData[i * 4 + c] =
                static_cast<uint8_t>((std::min)(255.0f, (std::max)(0.0f, converted * 255.0f)));
        }
        result.outputData[i * 4 + 3] = inputData[i * 4 + 3];  // Alpha passthrough
    }

    auto endTime = std::chrono::steady_clock::now();
    result.success = true;
    result.outputWidth = width;
    result.outputHeight = height;
    result.usedBackend = GPUBackend::CPU;
    result.totalTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    m_stats.totalDispatches++;
    m_stats.cpuFallbacks++;
    return result;
}

//------------------------------------------------------------------------------
// Statistics & State
//------------------------------------------------------------------------------

ComputeStats D3D12ComputePipeline::GetStats() const
{
    return m_stats;
}

bool D3D12ComputePipeline::IsInitialized() const
{
    return m_initialized;
}

GPUBackend D3D12ComputePipeline::GetActiveBackend() const
{
    return m_activeBackend;
}

//------------------------------------------------------------------------------
// Static Helpers
//------------------------------------------------------------------------------

const wchar_t* D3D12ComputePipeline::GetBackendName(GPUBackend backend)
{
    switch (backend) {
        case GPUBackend::Auto:
            return L"Auto";
        case GPUBackend::D3D12:
            return L"D3D12";
        case GPUBackend::D3D11:
            return L"D3D11";
        case GPUBackend::CPU:
            return L"CPU";
        case GPUBackend::Disabled:
            return L"Disabled";
        default:
            return L"Unknown";
    }
}

const wchar_t* D3D12ComputePipeline::GetAlgorithmName(ScalingAlgorithm algo)
{
    switch (algo) {
        case ScalingAlgorithm::NearestNeighbor:
            return L"NearestNeighbor";
        case ScalingAlgorithm::Bilinear:
            return L"Bilinear";
        case ScalingAlgorithm::Bicubic:
            return L"Bicubic";
        case ScalingAlgorithm::Lanczos3:
            return L"Lanczos3";
        case ScalingAlgorithm::Adaptive:
            return L"Adaptive";
        default:
            return L"Unknown";
    }
}

const wchar_t* D3D12ComputePipeline::GetColorSpaceName(GPUColorSpace cs)
{
    switch (cs) {
        case GPUColorSpace::SRGB:
            return L"sRGB";
        case GPUColorSpace::LinearRGB:
            return L"LinearRGB";
        case GPUColorSpace::HDR10:
            return L"HDR10";
        case GPUColorSpace::HLG:
            return L"HLG";
        case GPUColorSpace::DolbyVision:
            return L"DolbyVision";
        default:
            return L"Unknown";
    }
}

const wchar_t* D3D12ComputePipeline::GetToneMapName(ToneMapOperator op)
{
    switch (op) {
        case ToneMapOperator::None:
            return L"None";
        case ToneMapOperator::Reinhard:
            return L"Reinhard";
        case ToneMapOperator::ReinhardExtended:
            return L"ReinhardExtended";
        case ToneMapOperator::ACES:
            return L"ACES";
        case ToneMapOperator::Uncharted2:
            return L"Uncharted2";
        case ToneMapOperator::HableFilmic:
            return L"HableFilmic";
        default:
            return L"Unknown";
    }
}

//------------------------------------------------------------------------------
// Private Implementation
//------------------------------------------------------------------------------

ComputeResult D3D12ComputePipeline::ResizeCPU(const uint8_t* inputData, uint32_t inputWidth, uint32_t inputHeight,
                                              uint32_t outputWidth, uint32_t outputHeight)
{
    ComputeResult result;

    if (!inputData || inputWidth == 0 || inputHeight == 0 || outputWidth == 0 || outputHeight == 0) {
        result.success = false;
        result.errorMessage = L"Invalid dimensions for resize";
        return result;
    }

    result.outputData.resize(outputWidth * outputHeight * 4);

    // Bilinear interpolation (CPU fallback)
    float scaleX = static_cast<float>(inputWidth) / outputWidth;
    float scaleY = static_cast<float>(inputHeight) / outputHeight;

    for (uint32_t y = 0; y < outputHeight; ++y) {
        for (uint32_t x = 0; x < outputWidth; ++x) {
            float srcX = x * scaleX;
            float srcY = y * scaleY;

            uint32_t x0 = static_cast<uint32_t>(srcX);
            uint32_t y0 = static_cast<uint32_t>(srcY);
            uint32_t x1 = (std::min)(x0 + 1, inputWidth - 1);
            uint32_t y1 = (std::min)(y0 + 1, inputHeight - 1);

            float fx = srcX - x0;
            float fy = srcY - y0;

            uint32_t dstIdx = (y * outputWidth + x) * 4;

            for (int c = 0; c < 4; ++c) {
                float v00 = inputData[(y0 * inputWidth + x0) * 4 + c];
                float v10 = inputData[(y0 * inputWidth + x1) * 4 + c];
                float v01 = inputData[(y1 * inputWidth + x0) * 4 + c];
                float v11 = inputData[(y1 * inputWidth + x1) * 4 + c];

                float v = v00 * (1 - fx) * (1 - fy) + v10 * fx * (1 - fy) + v01 * (1 - fx) * fy + v11 * fx * fy;

                result.outputData[dstIdx + c] = static_cast<uint8_t>((std::min)(255.0f, (std::max)(0.0f, v)));
            }
        }
    }

    result.success = true;
    result.outputWidth = outputWidth;
    result.outputHeight = outputHeight;
    result.usedBackend = GPUBackend::CPU;
    return result;
}

GPUBackend D3D12ComputePipeline::SelectBackend() const
{
    if (m_config.preferredBackend != GPUBackend::Auto)
        return m_config.preferredBackend;

    // Auto-detect: try D3D12 → D3D11 → CPU
    // In production, check D3D12CreateDevice availability
    auto reqs = ProbeHardware();
    if (reqs.computeShaderSupport)
        return GPUBackend::D3D12;

    // Fallback to CPU for now
    return GPUBackend::CPU;
}

void D3D12ComputePipeline::CalculateDispatch(uint32_t width, uint32_t height, uint32_t& groupsX, uint32_t& groupsY) const
{
    groupsX = (width + m_config.workGroupSizeX - 1) / m_config.workGroupSizeX;
    groupsY = (height + m_config.workGroupSizeY - 1) / m_config.workGroupSizeY;
}

}  // namespace Engine
}  // namespace ExplorerLens
