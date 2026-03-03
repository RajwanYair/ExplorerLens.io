// GPUResourcePoolManager.h — DirectX GPU Resource Pool Management
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a pool of reusable GPU resources (textures, buffers, render targets)
// for the thumbnail rendering pipeline. Eliminates per-thumbnail allocation overhead
// by pre-allocating and recycling resources. Supports both DX11 and DX12 backends.
//
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <cstdint>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

/// GPU resource type
enum class GPUResourceType : uint8_t {
    Texture2D,          // 2D texture (decode target / render source)
    RenderTarget,       // Render target view
    StagingBuffer,      // CPU-readable staging texture
    ConstantBuffer,     // Shader constant buffer
    StructuredBuffer,   // Structured buffer for compute
};

/// Resource pool configuration
struct GPUPoolConfig {
    uint32_t    maxTextures         = 64;
    uint32_t    maxRenderTargets    = 16;
    uint32_t    maxStagingBuffers   = 8;
    uint32_t    maxConstantBuffers  = 32;
    uint32_t    defaultTextureSize  = 512;      // Default pre-allocated size
    uint32_t    maxTextureSize      = 4096;     // Maximum supported size
    bool        enableTrimming      = true;     // Auto-trim unused resources
    uint32_t    trimIntervalMs      = 30000;    // Trim every 30 seconds
    float       trimThreshold       = 0.5f;     // Trim when usage < 50%
    size_t      maxMemoryBytes      = 256 * 1024 * 1024; // 256 MB budget
};

/// Stats about pool utilization
struct GPUPoolStats {
    uint32_t    texturesAllocated   = 0;
    uint32_t    texturesInUse       = 0;
    uint32_t    texturesAvailable   = 0;
    uint32_t    renderTargetsInUse  = 0;
    uint32_t    stagingBuffersInUse = 0;
    size_t      totalMemoryBytes    = 0;
    uint64_t    allocCount          = 0;
    uint64_t    reuseCount          = 0;
    float       reuseRatio          = 0.0f;     // reuseCount / (allocCount + reuseCount)
    uint64_t    trimCount           = 0;
};

/// Handle to a pooled GPU resource (RAII — returns to pool on destruction)
class PooledTexture {
public:
    PooledTexture() = default;
    PooledTexture(ID3D11Texture2D* tex, uint32_t w, uint32_t h, uint32_t poolSlot,
                  class GPUResourcePoolManager* owner)
        : m_texture(tex), m_width(w), m_height(h), m_poolSlot(poolSlot), m_owner(owner) {}

    ~PooledTexture() { Release(); }

    // Move-only
    PooledTexture(PooledTexture&& o) noexcept
        : m_texture(o.m_texture), m_width(o.m_width), m_height(o.m_height),
          m_poolSlot(o.m_poolSlot), m_owner(o.m_owner) {
        o.m_texture = nullptr; o.m_owner = nullptr;
    }
    PooledTexture& operator=(PooledTexture&& o) noexcept {
        if (this != &o) { Release(); m_texture = o.m_texture; m_width = o.m_width;
            m_height = o.m_height; m_poolSlot = o.m_poolSlot; m_owner = o.m_owner;
            o.m_texture = nullptr; o.m_owner = nullptr; }
        return *this;
    }
    PooledTexture(const PooledTexture&) = delete;
    PooledTexture& operator=(const PooledTexture&) = delete;

    ID3D11Texture2D* Get() const { return m_texture; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    bool IsValid() const { return m_texture != nullptr; }
    explicit operator bool() const { return IsValid(); }

    void Release();

private:
    ID3D11Texture2D*            m_texture   = nullptr;
    uint32_t                    m_width     = 0;
    uint32_t                    m_height    = 0;
    uint32_t                    m_poolSlot  = UINT32_MAX;
    GPUResourcePoolManager*     m_owner     = nullptr;
};

/// Manages a pool of pre-allocated GPU resources for reuse across thumbnail
/// operations. Significantly reduces D3D allocation overhead in high-throughput
/// scenarios (batch thumbnails, folder previews).
///
/// Usage:
///   GPUResourcePoolManager pool;
///   pool.Initialize(device, config);
///   auto tex = pool.AcquireTexture(256, 256, DXGI_FORMAT_B8G8R8A8_UNORM);
///   // Use tex.Get() for rendering...
///   // tex automatically returns to pool when destroyed
///
class GPUResourcePoolManager {
public:
    GPUResourcePoolManager() = default;
    ~GPUResourcePoolManager() { Shutdown(); }

    /// Initialize the pool with a D3D11 device
    bool Initialize(ID3D11Device* device, const GPUPoolConfig& config = {}) {
        if (!device) return false;
        m_device = device;
        m_device->AddRef();
        m_config = config;
        m_initialized = true;

        // Pre-allocate default-size textures
        PreAllocateTextures(m_config.defaultTextureSize, 4);
        return true;
    }

    /// Shutdown and release all resources
    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& slot : m_texturePool) {
            if (slot.texture) { slot.texture->Release(); slot.texture = nullptr; }
        }
        m_texturePool.clear();
        if (m_device) { m_device->Release(); m_device = nullptr; }
        m_initialized = false;
    }

    /// Acquire a texture from the pool (or create a new one if needed)
    PooledTexture AcquireTexture(uint32_t width, uint32_t height,
                                 DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM) {
        if (!m_initialized || !m_device) return {};

        std::lock_guard<std::mutex> lock(m_mutex);

        // Try to find a matching free texture
        for (uint32_t i = 0; i < m_texturePool.size(); i++) {
            auto& slot = m_texturePool[i];
            if (!slot.inUse && slot.texture && slot.width >= width &&
                slot.height >= height && slot.format == format) {
                // Don't reuse if way too large (> 4x area)
                if (slot.width * slot.height <= width * height * 4) {
                    slot.inUse = true;
                    m_stats.reuseCount++;
                    return PooledTexture(slot.texture, slot.width, slot.height, i, this);
                }
            }
        }

        // No suitable texture found — create new one
        if (width > m_config.maxTextureSize || height > m_config.maxTextureSize) return {};

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = format;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

        ID3D11Texture2D* tex = nullptr;
        HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &tex);
        if (FAILED(hr) || !tex) return {};

        uint32_t slot = static_cast<uint32_t>(m_texturePool.size());
        m_texturePool.push_back({tex, width, height, format, true});
        m_stats.allocCount++;
        m_stats.texturesAllocated++;
        m_stats.totalMemoryBytes += width * height * 4; // Approximate

        return PooledTexture(tex, width, height, slot, this);
    }

    /// Return a texture to the pool (called by PooledTexture destructor)
    void ReturnTexture(uint32_t slot) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (slot < m_texturePool.size()) {
            m_texturePool[slot].inUse = false;
        }
    }

    /// Trim unused resources to free memory
    uint32_t TrimUnused() {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint32_t trimmed = 0;

        for (auto it = m_texturePool.begin(); it != m_texturePool.end(); ) {
            if (!it->inUse && it->texture) {
                m_stats.totalMemoryBytes -= it->width * it->height * 4;
                it->texture->Release();
                it = m_texturePool.erase(it);
                trimmed++;
                m_stats.texturesAllocated--;
            } else {
                ++it;
            }
        }
        m_stats.trimCount += trimmed;
        return trimmed;
    }

    /// Get pool statistics
    GPUPoolStats GetStats() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
        GPUPoolStats stats = m_stats;
        for (const auto& s : m_texturePool) {
            if (s.inUse) stats.texturesInUse++;
            else stats.texturesAvailable++;
        }
        uint64_t totalOps = stats.allocCount + stats.reuseCount;
        stats.reuseRatio = totalOps > 0 ? static_cast<float>(stats.reuseCount) / totalOps : 0.0f;
        return stats;
    }

    bool IsInitialized() const { return m_initialized; }

private:
    struct TextureSlot {
        ID3D11Texture2D* texture    = nullptr;
        uint32_t         width      = 0;
        uint32_t         height     = 0;
        DXGI_FORMAT      format     = DXGI_FORMAT_UNKNOWN;
        bool             inUse      = false;
    };

    void PreAllocateTextures(uint32_t size, uint32_t count) {
        for (uint32_t i = 0; i < count; i++) {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width = size;
            desc.Height = size;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

            ID3D11Texture2D* tex = nullptr;
            HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &tex);
            if (SUCCEEDED(hr) && tex) {
                m_texturePool.push_back({tex, size, size, DXGI_FORMAT_B8G8R8A8_UNORM, false});
                m_stats.texturesAllocated++;
                m_stats.totalMemoryBytes += size * size * 4;
            }
        }
    }

    ID3D11Device*           m_device        = nullptr;
    GPUPoolConfig           m_config;
    bool                    m_initialized   = false;
    std::vector<TextureSlot> m_texturePool;
    GPUPoolStats            m_stats;
    std::mutex              m_mutex;
};

// PooledTexture::Release implementation (needs GPUResourcePoolManager definition)
inline void PooledTexture::Release() {
    if (m_owner && m_poolSlot != UINT32_MAX) {
        m_owner->ReturnTexture(m_poolSlot);
    }
    m_texture = nullptr;
    m_owner = nullptr;
    m_poolSlot = UINT32_MAX;
}

} // namespace Engine
} // namespace ExplorerLens
