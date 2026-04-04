// PlatformAbstractionLayer.h — Unified Cross-Platform API Shim
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a single abstraction over GPU surfaces, filesystem primitives,
// threading models, and window management across Windows, macOS, and Linux.
//
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
    #include <Windows.h>
#endif

namespace ExplorerLens {
namespace Engine {

// GPU backend enum scoped to this abstraction layer to avoid conflicts with
// per-API backend enums (D3D12PipelineActivation, VulkanComputePipeline, etc.)
enum class PlatGPUBackend : uint8_t {
    D3D12 = 0,
    Metal = 1,
    Vulkan = 2,
    OpenGL = 3,
    CPU = 4
};

struct PlatRenderSurface
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t strideBytes = 0;
    bool valid = false;
};

class PlatformAbstractionLayer
{
  public:
    PlatformAbstractionLayer() = default;
    ~PlatformAbstractionLayer() = default;

    PlatformAbstractionLayer(const PlatformAbstractionLayer&) = delete;
    PlatformAbstractionLayer& operator=(const PlatformAbstractionLayer&) = delete;

    static PlatformAbstractionLayer& Instance()
    {
        static PlatformAbstractionLayer s_instance;
        return s_instance;
    }

    PlatformType GetCurrentPlatform() const
    {
#ifdef _WIN32
        return PlatformType::Windows;
#elif defined(__APPLE__)
        return PlatformType::macOS;
#elif defined(__linux__)
        return PlatformType::Linux;
#else
        return PlatformType::WASM;
#endif
    }

    PlatGPUBackend GetPreferredGPUBackend() const
    {
        switch (GetCurrentPlatform()) {
            case PlatformType::Windows:
                return PlatGPUBackend::D3D12;
            case PlatformType::macOS:
                return PlatGPUBackend::Metal;
            case PlatformType::Linux:
                return PlatGPUBackend::Vulkan;
            default:
                return PlatGPUBackend::CPU;
        }
    }

    std::vector<PlatGPUBackend> GetSupportedBackends() const
    {
        switch (GetCurrentPlatform()) {
            case PlatformType::Windows:
                return {PlatGPUBackend::D3D12, PlatGPUBackend::Vulkan, PlatGPUBackend::OpenGL, PlatGPUBackend::CPU};
            case PlatformType::macOS:
                return {PlatGPUBackend::Metal, PlatGPUBackend::OpenGL, PlatGPUBackend::CPU};
            case PlatformType::Linux:
                return {PlatGPUBackend::Vulkan, PlatGPUBackend::OpenGL, PlatGPUBackend::CPU};
            default:
                return {PlatGPUBackend::CPU};
        }
    }

    PlatRenderSurface CreateRenderSurface(uint32_t width, uint32_t height) const
    {
        PlatRenderSurface surface;
        surface.width = width;
        surface.height = height;
        surface.strideBytes = (width > 0) ? width * 4u : 0u;  // BGRA8
        surface.valid = (width > 0 && height > 0 && width <= MAX_SURFACE_DIM && height <= MAX_SURFACE_DIM);
        return surface;
    }

    std::vector<float> EnumerateDisplayScales() const
    {
        std::vector<float> scales;
#ifdef _WIN32
        float dpi = GetWindowsDpiScale();
        scales.push_back(dpi > 0.0f ? dpi : 1.0f);
#else
        scales.push_back(1.0f);
#endif
        return scales;
    }

    const char* PlatformName() const
    {
        switch (GetCurrentPlatform()) {
            case PlatformType::Windows:
                return "Windows";
            case PlatformType::macOS:
                return "macOS";
            case PlatformType::Linux:
                return "Linux";
            default:
                return "Unknown";
        }
    }

    uint32_t GetLogicalCPUCount() const
    {
#ifdef _WIN32
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        return si.dwNumberOfProcessors;
#else
        return 4;
#endif
    }

    uint32_t GetOptimalThreadCount() const
    {
        uint32_t cpus = GetLogicalCPUCount();
        return (cpus > 1) ? cpus : 1u;
    }

    uint32_t GetPageSize() const
    {
#ifdef _WIN32
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        return si.dwPageSize;
#else
        return 4096;
#endif
    }

    uint64_t GetTotalSystemMemoryMB() const
    {
#ifdef _WIN32
        MEMORYSTATUSEX ms{};
        ms.dwLength = sizeof(ms);
        if (GlobalMemoryStatusEx(&ms))
            return static_cast<uint64_t>(ms.ullTotalPhys / (1024ULL * 1024ULL));
        return 4096;
#else
        return 4096;
#endif
    }

    std::wstring GetTempDirectory() const
    {
#ifdef _WIN32
        wchar_t buf[MAX_PATH] = {};
        DWORD len = GetTempPathW(MAX_PATH, buf);
        return (len > 0) ? std::wstring(buf, len) : L"C:\\Temp";
#else
        return L"/tmp";
#endif
    }

  private:
    static constexpr uint32_t MAX_SURFACE_DIM = 16384;

#ifdef _WIN32
    float GetWindowsDpiScale() const
    {
        HDC hdc = GetDC(nullptr);
        float scale = hdc ? static_cast<float>(GetDeviceCaps(hdc, LOGPIXELSX)) / 96.0f : 1.0f;
        if (hdc)
            ReleaseDC(nullptr, hdc);
        return scale;
    }
#endif
};

}  // namespace Engine
}  // namespace ExplorerLens
