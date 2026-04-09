// GPUJPEGDecodeAccelerator.h — NVJPEG + Intel QSV JPEG Hardware Decode
// Copyright (c) 2026 ExplorerLens Project
//
// Provides JPEG hardware decode via NVIDIA NVJPEG (CUDA 12.6+) and
// Intel QuickSync (oneVPL 2024.2+). Falls back gracefully when no
// compatible GPU is present. Produces a BGRA32 thumbnail surface.
//
#pragma once
#include <cstdint>
#include <memory>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class JPEGHWAccelBackend : uint8_t {
    None        = 0,
    NVJPEG      = 1,  // NVIDIA hardware JPEG decode (CUDA 12.6+)
    IntelQSV    = 2,  // Intel QuickSync hardware JPEG decode (oneVPL)
    WIC_GPU     = 3,  // WIC GPU-backed JPEG via D2D factory
};

struct JPEGDecodeResult {
    uint8_t* pixelsBGRA = nullptr;  // Caller-owned output buffer (w × h × 4 bytes)
    uint32_t width      = 0;
    uint32_t height     = 0;
    bool     success    = false;
    float    decodeMs   = 0.0f;     // Wall-clock decode time
    JPEGHWAccelBackend backendUsed = JPEGHWAccelBackend::None;
};

struct JPEGDecodeRequest {
    const uint8_t* srcData      = nullptr;  // Raw JPEG bytes
    size_t         srcSize      = 0;
    uint32_t       targetWidth  = 256;      // Output thumbnail width
    uint32_t       targetHeight = 256;
    bool           highQuality  = false;    // Use Lanczos downscale if true
};

class GPUJPEGDecodeAccelerator {
public:
    GPUJPEGDecodeAccelerator();
    ~GPUJPEGDecodeAccelerator();

    // Returns true if a hardware-capable backend was found.
    bool Initialize() noexcept;

    // Decode srcData JPEG bytes to a BGRA32 thumbnail; caller owns result.pixelsBGRA.
    JPEGDecodeResult Decode(const JPEGDecodeRequest& req) noexcept;

    JPEGHWAccelBackend GetActiveBackend() const noexcept { return m_backend; }
    bool               IsAvailable()      const noexcept { return m_initialized; }

    // Benchmark: decode `iterations` times and return mean decode time in ms.
    float BenchmarkMs(const JPEGDecodeRequest& req, int iterations = 10) noexcept;

private:
    bool TryNVJPEG()   noexcept;
    bool TryIntelQSV() noexcept;
    bool TryWICGPU()   noexcept;

    bool                  m_initialized = false;
    JPEGHWAccelBackend    m_backend     = JPEGHWAccelBackend::None;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}} // namespace ExplorerLens::Engine
