// GPUDecodeFormatRouter.h — Format-to-GPU-Path Routing Table
// Copyright (c) 2026 ExplorerLens Project
//
// Routes each file extension/MIME type to the optimal GPU decode path
// (NVJPEG, Intel QSV, AMD AMF, DirectX, Vulkan, or CPU fallback).
// Central dispatch table for the GPU-First Decode Pipeline (v34.1.0 T2).
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class GPUDecodePathId : uint8_t {
    None        = 0,
    NVJPEG      = 1,   // NVIDIA NVJPEG hardware JPEG/JPEG2000 decode
    QSV_JPEG    = 2,   // Intel QuickSync JPEG decode
    AMD_VCE     = 3,   // AMD VCE hardware decode
    NVDEC_AV1   = 4,   // NVIDIA NVDEC AV1 (AVIF/AV1 video)
    QSV_AV1     = 5,   // Intel QSV AV1 decode
    NVDEC_HEVC  = 6,   // NVIDIA NVDEC HEVC (HEIC)
    QSV_HEVC    = 7,   // Intel QSV HEVC
    DirectCompute_PNG = 8,  // DirectCompute inflate+unfilter PNG
    D2D_PDF     = 9,   // D2D + DirectWrite GPU PDF rasterizer
    GPUDemosaic = 10,  // GPU RAW demosaic compute kernel
    CPUFallback = 255, // Software CPU path
};

inline const char* GPUDecodePathName(GPUDecodePathId id) noexcept
{
    switch (id) {
        case GPUDecodePathId::NVJPEG:             return "NVJPEG";
        case GPUDecodePathId::QSV_JPEG:           return "Intel QSV JPEG";
        case GPUDecodePathId::AMD_VCE:            return "AMD VCE";
        case GPUDecodePathId::NVDEC_AV1:          return "NVDEC AV1";
        case GPUDecodePathId::QSV_AV1:            return "Intel QSV AV1";
        case GPUDecodePathId::NVDEC_HEVC:         return "NVDEC HEVC";
        case GPUDecodePathId::QSV_HEVC:           return "Intel QSV HEVC";
        case GPUDecodePathId::DirectCompute_PNG:  return "DirectCompute PNG";
        case GPUDecodePathId::D2D_PDF:            return "D2D PDF";
        case GPUDecodePathId::GPUDemosaic:        return "GPU Demosaic";
        case GPUDecodePathId::CPUFallback:        return "CPU Fallback";
        default:                                  return "None";
    }
}

struct GPUDecodeRoute {
    GPUDecodePathId primaryPath   = GPUDecodePathId::CPUFallback;
    GPUDecodePathId secondaryPath = GPUDecodePathId::CPUFallback;
    float           targetMs      = 0.0f;  // P50 decode time target in milliseconds
};

class GPUDecodeFormatRouter {
public:
    GPUDecodeFormatRouter();

    // Returns the preferred GPU decode route for a file extension (e.g. L".jpg").
    // Falls back to CPUFallback if no GPU route is registered.
    GPUDecodeRoute RouteByExtension(const wchar_t* ext) const noexcept;

    // Returns the preferred GPU decode route for a MIME type string.
    GPUDecodeRoute RouteByMime(const char* mime) const noexcept;

    // Returns all registered routes for diagnostics.
    const std::unordered_map<std::wstring, GPUDecodeRoute>& GetExtensionTable() const noexcept
    {
        return m_extTable;
    }

    // Override a route at runtime (e.g. after detecting GPU capability).
    void SetRoute(const wchar_t* ext, GPUDecodePathId primary, GPUDecodePathId secondary, float targetMs);

private:
    void BuildDefaultTable();

    std::unordered_map<std::wstring, GPUDecodeRoute> m_extTable;
    std::unordered_map<std::string,  GPUDecodeRoute> m_mimeTable;
};

// Singleton accessor — one router per process.
GPUDecodeFormatRouter& GetGlobalFormatRouter() noexcept;

}} // namespace ExplorerLens::Engine
