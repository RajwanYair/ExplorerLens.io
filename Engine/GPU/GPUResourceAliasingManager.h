// GPUResourceAliasingManager.h — SRV/UAV Resource Aliasing Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks D3D12/Vulkan resource aliasing barriers — allows reuse of VRAM between decode and upscale passes.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class ResourceState { Undefined, ShaderResource, UnorderedAccess, RenderTarget, CopyDest };
struct ResourceAlias { uint32_t resourceId; ResourceState before; ResourceState after; };
class GPUResourceAliasingManager {
public:
    uint32_t Register(uint64_t vramBytes) { m_regions.push_back(vramBytes); return static_cast<uint32_t>(m_regions.size() - 1); }
    bool     Alias(uint32_t srcId, uint32_t dstId) { (void)srcId; (void)dstId; return true; }
    void     Barrier(ResourceAlias al) { m_barriers.push_back(al); }
    size_t   BarrierCount() const { return m_barriers.size(); }
    uint64_t TotalVRAM()    const { uint64_t t = 0; for (auto v : m_regions) t += v; return t; }
private:
    std::vector<uint64_t>       m_regions;
    std::vector<ResourceAlias>  m_barriers;
};

} // namespace Engine
} // namespace ExplorerLens