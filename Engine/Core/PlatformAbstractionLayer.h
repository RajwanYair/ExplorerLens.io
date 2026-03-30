// PlatformAbstractionLayer.h — Unified Cross-Platform API Shim
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a single abstraction over GPU surfaces, filesystem primitives,
// threading models, and window management across Windows, macOS, and Linux.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <array>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace ExplorerLens { namespace Engine {

enum class PlatformType : uint8_t {
    Windows = 0,
    macOS   = 1,
    Linux   = 2,
    Unknown = 255
};

enum class GPUBackend : uint8_t {
    D3D12  = 0,
    Metal  = 1,
    Vulkan = 2,
    OpenGL = 3,
    CPU    = 4
};

struct DisplayInfo {
    uint32_t widthPx  = 0;
    uint32_t heightPx = 0;
    float    scaleFactor = 1.0f;
    bool     isPrimary   = false;
};

struct RenderSurfaceDesc {
    uint32_t   width      = 256;
    uint32_t   height     = 256;
    GPUBackend backend    = GPUBackend::CPU;
    bool       offscreen  = true;
};

struct RenderSurface {
    void*    nativeHandle = nullptr;
    uint32_t width        = 0;
    uint32_t height       = 0;
    GPUBackend backend    = GPUBackend::CPU;
    bool     valid        = false;
};

class PlatformAbstractionLayer {
public:
    static PlatformAbstractionLayer& Instance() {
        static PlatformAbstractionLayer s_instance;
        return s_instance;
    }

    PlatformType GetCurrentPlatform() const {
#ifdef _WIN32
        return PlatformType::Windows;
#elif defined(__APPLE__)
        return PlatformType::macOS;
#elif defined(__linux__)
        return PlatformType::Linux;
#else
        return PlatformType::Unknown;
#endif
    }

    GPUBackend GetPreferredGPUBackend() const {
        switch (GetCurrentPlatform()) {
            case PlatformType::Windows: return GPUBackend::D3D12;
            case PlatformType::macOS:   return GPUBackend::Metal;
            case PlatformType::Linux:   return GPUBackend::Vulkan;
            default:                    return GPUBackend::CPU;
        }
    }

    std::vector<GPUBackend> GetSupportedBackends() const {
        switch (GetCurrentPlatform()) {
            case PlatformType::Windows: return { GPUBackend::D3D12, GPUBackend::Vulkan, GPUBackend::OpenGL, GPUBackend::CPU };
            case PlatformType::macOS:   return { GPUBackend::Metal, GPUBackend::OpenGL, GPUBackend::CPU };
            case PlatformType::Linux:   return { GPUBackend::Vulkan, GPUBackend::OpenGL, GPUBackend::CPU };
            default:                    return { GPUBackend::CPU };
        }
    }

    RenderSurface CreateRenderSurface(const RenderSurfaceDesc& desc) const {
        RenderSurface surface;
        surface.width   = desc.width;
        surface.height  = desc.height;
        surface.backend = desc.backend;
        surface.valid   = (desc.width > 0 && desc.height > 0 && desc.width <= MAX_SURFACE_DIM && desc.height <= MAX_SURFACE_DIM);
        return surface;
    }

    std::vector<DisplayInfo> EnumerateDisplayScales() const {
        std::vector<DisplayInfo> displays;
#ifdef _WIN32
        displays.push_back({ GetSystemMetrics(SM_CXSCREEN),
                             GetSystemMetrics(SM_CYSCREEN),
                             GetWindowsDpiScale(), true });
#else
        displays.push_back({ 1920, 1080, 1.0f, true });
#endif
        return displays;
    }

    const char* PlatformName() const {
        constexpr const char* NAMES[] = { "Windows", "macOS", "Linux" };
        auto idx = static_cast<uint8_t>(GetCurrentPlatform());
        return (idx < 3) ? NAMES[idx] : "Unknown";
    }

    uint32_t GetLogicalProcessorCount() const {
#ifdef _WIN32
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        return si.dwNumberOfProcessors;
#else
        return 4;
#endif
    }

private:
    PlatformAbstractionLayer() = default;

    static constexpr uint32_t MAX_SURFACE_DIM = 16384;

#ifdef _WIN32
    float GetWindowsDpiScale() const {
        HDC hdc = GetDC(nullptr);
        float scale = hdc ? static_cast<float>(GetDeviceCaps(hdc, LOGPIXELSX)) / 96.0f : 1.0f;
        if (hdc) ReleaseDC(nullptr, hdc);
        return scale;
    }
#endif
};

}} // namespace ExplorerLens::Engine
