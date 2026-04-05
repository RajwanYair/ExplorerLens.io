// PlatformGPURouter.h — Platform-aware GPU backend dispatcher
// Copyright (c) 2026 ExplorerLens Project
//
// Detects the current platform at runtime and routes GPU decode/render operations
// to the appropriate backend: D3D12 on Windows, Metal v2 on macOS, Vulkan-EGL on
// Linux. Falls back to software rendering when no GPU backend is available.
//
#pragma once

#include "PlatformShellProvider.h"
#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class GPURouterBackend : uint8_t
{
    D3D12      = 0,
    Metal      = 1,
    VulkanEGL  = 2,
    Software   = 3,
    Unknown    = 255,
};

struct GPURouterStats
{
    uint32_t routedOps   = 0;
    uint32_t fallbackOps = 0;
    float    avgLatencyMs = 0.0f;
};

class PlatformGPURouter
{
public:
    PlatformGPURouter();
    ~PlatformGPURouter();

    PlatformGPURouter(const PlatformGPURouter&)            = delete;
    PlatformGPURouter& operator=(const PlatformGPURouter&) = delete;

    bool             Initialize();
    void             Shutdown();
    GPURouterBackend SelectedBackend() const noexcept { return m_backend; }
    const char*      BackendName()     const noexcept;
    bool             RouteDecodeOp(const void* input, uint32_t size,
                                   void* outputBuf, uint32_t bufSize);
    GPURouterStats   GetStats()        const noexcept { return m_stats; }
    void             ResetStats()      noexcept;
    bool             IsReady()         const noexcept;

    static PlatformGPURouter& Instance() noexcept;

private:
    GPURouterBackend   m_backend  = GPURouterBackend::Unknown;
    GPURouterStats     m_stats;
    static PlatformGPURouter s_instance;
};

}} // namespace ExplorerLens::Engine
