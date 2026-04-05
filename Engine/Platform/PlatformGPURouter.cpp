// PlatformGPURouter.cpp — Platform-aware GPU backend dispatcher
// Copyright (c) 2026 ExplorerLens Project
//
#include "PlatformGPURouter.h"

namespace ExplorerLens { namespace Engine {

PlatformGPURouter PlatformGPURouter::s_instance;

PlatformGPURouter::PlatformGPURouter()  = default;
PlatformGPURouter::~PlatformGPURouter() { Shutdown(); }

PlatformGPURouter& PlatformGPURouter::Instance() noexcept { return s_instance; }

bool PlatformGPURouter::Initialize()
{
#if defined(_WIN32)
    m_backend = GPURouterBackend::D3D12;
#elif defined(__APPLE__)
    m_backend = GPURouterBackend::Metal;
#elif defined(__linux__)
    m_backend = GPURouterBackend::VulkanEGL;
#else
    m_backend = GPURouterBackend::Software;
#endif
    return true;
}

void PlatformGPURouter::Shutdown()
{
    m_backend = GPURouterBackend::Unknown;
    m_stats   = {};
}

const char* PlatformGPURouter::BackendName() const noexcept
{
    switch (m_backend)
    {
    case GPURouterBackend::D3D12:     return "D3D12";
    case GPURouterBackend::Metal:     return "Metal-v2";
    case GPURouterBackend::VulkanEGL: return "Vulkan-EGL";
    case GPURouterBackend::Software:  return "Software";
    default:                          return "Unknown";
    }
}

bool PlatformGPURouter::RouteDecodeOp(const void* /*input*/, uint32_t /*size*/,
                                       void* /*outputBuf*/, uint32_t /*bufSize*/)
{
    ++m_stats.routedOps;
    return m_backend != GPURouterBackend::Unknown;
}

void PlatformGPURouter::ResetStats() noexcept
{
    m_stats = {};
}

bool PlatformGPURouter::IsReady() const noexcept
{
    return m_backend != GPURouterBackend::Unknown;
}

}} // namespace ExplorerLens::Engine
