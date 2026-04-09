// ZeroCopyGPUSurface.cpp — Zero-Copy CPU→GPU BGRA Surface
// Copyright (c) 2026 ExplorerLens Project
//
#include "GPU/ZeroCopyGPUSurface.h"
#include <cstring>
#include <cstdlib>

namespace ExplorerLens { namespace Engine {

struct ZeroCopyGPUSurface::Impl {
    // D3D11 / D3D12 resource handles assigned by AllocD3D11/AllocD3D12.
    void* d3dResource = nullptr;
};

ZeroCopyGPUSurface::ZeroCopyGPUSurface()
    : m_impl(std::make_unique<Impl>())
{}

ZeroCopyGPUSurface::~ZeroCopyGPUSurface()
{
    Release();
}

bool ZeroCopyGPUSurface::Allocate(uint32_t width, uint32_t height,
                                   SurfaceAllocMode mode) noexcept
{
    Release();
    if (width == 0 || height == 0) return false;

    m_desc.width  = width;
    m_desc.height = height;
    m_desc.allocMode = mode;

    if (mode == SurfaceAllocMode::D3D11Upload || mode == SurfaceAllocMode::Default)
        if (AllocD3D11()) return true;

    if (mode == SurfaceAllocMode::D3D12Upload)
        if (AllocD3D12()) return true;

    return AllocSystem();
}

bool ZeroCopyGPUSurface::AllocD3D11() noexcept
{
    // D3D11 STAGING texture allocation — requires D3D11 runtime.
    // Stub: falls through to system memory.
    return false;
}

bool ZeroCopyGPUSurface::AllocD3D12() noexcept
{
    // D3D12 UPLOAD heap allocation — requires D3D12 runtime.
    // Stub: falls through to system memory.
    return false;
}

bool ZeroCopyGPUSurface::AllocSystem() noexcept
{
    const uint32_t rowPitch = m_desc.width * 4u;
    const size_t   total    = static_cast<size_t>(rowPitch) * m_desc.height;
    m_desc.rowPitch  = rowPitch;
    m_desc.allocMode = SurfaceAllocMode::SystemMemory;
    m_desc.pData     = static_cast<uint8_t*>(std::malloc(total));
    return m_desc.pData != nullptr;
}

uint8_t* ZeroCopyGPUSurface::Map() noexcept
{
    if (!m_desc.pData) return nullptr;
    m_mapped = true;
    return m_desc.pData;
}

void ZeroCopyGPUSurface::Unmap() noexcept
{
    m_mapped = false;
    // For write-combined GPU memory, a memory-fence / flush would occur here.
}

bool ZeroCopyGPUSurface::CopyFrom(const uint8_t* srcBGRA, uint32_t srcRowPitch) noexcept
{
    uint8_t* dst = Map();
    if (!dst || !srcBGRA) return false;

    for (uint32_t row = 0; row < m_desc.height; ++row) {
        std::memcpy(dst + row * m_desc.rowPitch,
                    srcBGRA + row * srcRowPitch,
                    m_desc.width * 4u);
    }
    Unmap();
    return true;
}

void ZeroCopyGPUSurface::Release() noexcept
{
    if (m_desc.allocMode == SurfaceAllocMode::SystemMemory && m_desc.pData) {
        std::free(m_desc.pData);
    }
    m_desc     = {};
    m_mapped   = false;
    if (m_impl) m_impl->d3dResource = nullptr;
}

}} // namespace ExplorerLens::Engine
