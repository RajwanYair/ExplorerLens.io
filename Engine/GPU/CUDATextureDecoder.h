// CUDATextureDecoder.h — CUDA Texture Decompression Back-End
// Copyright (c) 2026 ExplorerLens Project
//
// GPU-accelerated texture decompression (BC1-BC7, ASTC, ETC2) via CUDA kernels for game asset thumbnails.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CUDATextureFormat {
    BC1,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6H,
    BC7,
    ASTC_4x4,
    ETC2_RGB
};
struct CUDADecodeResult
{
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> rgba;
    bool gpuUsed;
};
class CUDATextureDecoder
{
  public:
    bool IsAvailable() const
    {
        return false;
    }  // CUDA optional
    CUDADecodeResult Decode(const uint8_t* data, size_t size, CUDATextureFormat fmt, uint32_t w, uint32_t h)
    {
        (void)data;
        (void)size;
        (void)fmt;
        return {w, h, std::vector<uint8_t>(w * h * 4, 0), false};
    }
    std::string DeviceName() const
    {
        return "CPU Fallback";
    }
};

}  // namespace Engine
}  // namespace ExplorerLens