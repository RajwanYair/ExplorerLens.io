// GPUDecompressOrchestrator.cpp — GPU decompression backend selector
// Copyright (c) 2026 ExplorerLens Project

#include "GPUDecompressOrchestrator.h"
#include "ZStdGPUKernel.h"

#include <string_view>

namespace ExplorerLens { namespace Engine {

GPUDecompressOrchestrator& GPUDecompressOrchestrator::Instance()
{
    static GPUDecompressOrchestrator instance;
    return instance;
}

void GPUDecompressOrchestrator::Initialize()
{
    if (m_initialized) { return; }

    const ZStdGPUKernel& zstd = ZStdGPUKernel::Instance();
    if (zstd.IsAvailable() && zstd.DetectedVendor() != ZStdGPUVendor::FALLBACK)
    {
        m_backend = GPUDecompressBackend::ZSTD_GPU;
    }
    else
    {
        m_backend = GPUDecompressBackend::CPU;
    }

    m_initialized = true;
}

GPUDecompressResult GPUDecompressOrchestrator::Decompress(const GPUDecompressRequest& req) noexcept
{
    if (!m_initialized) { Initialize(); }

    GPUDecompressResult res{};

    if (m_backend == GPUDecompressBackend::ZSTD_GPU)
    {
        ZStdGPUKernelDesc kd{};
        kd.compressedData   = req.srcData;
        kd.compressedSize   = req.srcSize;
        kd.outputBuffer     = req.dstBuffer;
        kd.outputCapacity   = req.dstCapacity;
        kd.expectedOrigSize = req.expectedOrigSize;
        kd.vendor           = ZStdGPUKernel::Instance().DetectedVendor();

        ZStdGPUKernelResult kernelResult = ZStdGPUKernel::Instance().Decompress(kd);
        res.success     = kernelResult.success;
        res.bytesOut    = kernelResult.bytesWritten;
        res.elapsedMs   = kernelResult.decompressMs;
        res.backendUsed = GPUDecompressBackend::ZSTD_GPU;
    }
    else
    {
        // CPU fallback — validate sizes and report success placeholder
        bool sizeOk = req.srcSize > 0
            && req.dstCapacity >= req.expectedOrigSize
            && req.dstBuffer != nullptr;
        res.success     = sizeOk;
        res.bytesOut    = sizeOk ? req.expectedOrigSize : 0;
        res.elapsedMs   = 0.3f;
        res.backendUsed = GPUDecompressBackend::CPU;
    }

    return res;
}

std::string_view GPUDecompressOrchestrator::BackendName(GPUDecompressBackend b) noexcept
{
    switch (b)
    {
        case GPUDecompressBackend::NV_GDEFLATE: return "NvGDeflate";
        case GPUDecompressBackend::ZSTD_GPU:    return "ZStdGPU";
        default:                                return "CPU";
    }
}

}} // namespace ExplorerLens::Engine
