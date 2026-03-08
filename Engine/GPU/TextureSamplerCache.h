// TextureSamplerCache.h — Caches texture sampler state objects
// Copyright (c) 2026 ExplorerLens Project
//
// Deduplicates and caches D3D sampler state objects to avoid redundant
// GPU state creation for common sampling patterns (bilinear, aniso, point).
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct TextureSamplerCacheConfig {
    bool enabled = true;
    uint32_t maxCachedSamplers = 64;
    std::string label = "TextureSamplerCache";
};

class TextureSamplerCache {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    TextureSamplerCacheConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class FilterMode : uint8_t { Point, Bilinear, Trilinear, Anisotropic };

    uint32_t GetSamplerKey(FilterMode filter, bool clamp) const {
        return (static_cast<uint32_t>(filter) << 1) | (clamp ? 1u : 0u);
    }

    bool HasCached(uint32_t key) const { return m_cache.count(key) > 0; }
    size_t GetCachedCount() const { return m_cache.size(); }

    bool Insert(uint32_t key, uint64_t handle) {
        if (m_cache.size() >= m_config.maxCachedSamplers) return false;
        m_cache[key] = handle;
        return true;
    }

private:
    bool m_initialized = false;
    TextureSamplerCacheConfig m_config;
    std::unordered_map<uint32_t, uint64_t> m_cache;
};

}
} // namespace ExplorerLens::Engine
