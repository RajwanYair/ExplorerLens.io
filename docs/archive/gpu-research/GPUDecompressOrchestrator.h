// GPUDecompressOrchestrator.h — GPU decompression backend selector
// Copyright (c) 2026 ExplorerLens Project
//
// Runtime dispatcher that selects NvGDeflate (NVIDIA) or ZStdGPUKernel
// (AMD/Intel) based on detected GPU hardware. Falls back to CPU zstd when
// no GPU decompression is available. Called from DirectStorageManager on
// every DirectStorage I/O completion to route decompression workloads.
//
#pragma once

#include <cstdint>
#include <string_view>

namespace ExplorerLens { namespace Engine {

enum class GPUDecompressBackend : uint8_t { NV_GDEFLATE, ZSTD_GPU, CPU };

struct GPUDecompressRequest {
    const uint8_t* srcData          = nullptr;
    uint32_t       srcSize          = 0;
    uint8_t*       dstBuffer        = nullptr;
    uint32_t       dstCapacity      = 0;
    uint32_t       expectedOrigSize = 0;
};

struct GPUDecompressResult {
    bool                 success     = false;
    uint32_t             bytesOut    = 0;
    float                elapsedMs   = 0.0f;
    GPUDecompressBackend backendUsed = GPUDecompressBackend::CPU;
};

class GPUDecompressOrchestrator {
public:
    static GPUDecompressOrchestrator& Instance();

    void                 Initialize();
    GPUDecompressResult  Decompress(const GPUDecompressRequest& req) noexcept;
    GPUDecompressBackend PreferredBackend() const noexcept { return m_backend; }

    static std::string_view BackendName(GPUDecompressBackend b) noexcept;

private:
    bool                 m_initialized = false;
    GPUDecompressBackend m_backend     = GPUDecompressBackend::CPU;
};

}} // namespace ExplorerLens::Engine
