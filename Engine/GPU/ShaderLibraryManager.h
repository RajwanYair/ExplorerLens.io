// ShaderLibraryManager.h — GPU Shader Compilation Cache
// Copyright (c) 2026 ExplorerLens Project
//
// Manages compilation and caching of GPU shaders for the rendering pipeline.
// Supports vertex, pixel, compute, and geometry shader stages.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

enum class ShaderLibraryStage : uint8_t {
    Vertex = 0,
    Pixel = 1,
    Compute = 2,
    Geometry = 3,
    Hull = 4,
    Domain = 5
};

struct CompiledShaderEntry {
    std::string          name;
    ShaderLibraryStage   stage = ShaderLibraryStage::Vertex;
    std::vector<uint8_t> bytecode;
    uint64_t             hashKey = 0;
    uint64_t             compiledAtMs = 0;
    uint32_t             shaderModel = 50;  // SM 5.0
    bool                 isValid = false;
    std::string          entryPoint = "main";
    std::string          sourceHash;
};

struct ShaderLibraryStats {
    uint32_t totalShaders = 0;
    uint32_t cacheHits = 0;
    uint32_t cacheMisses = 0;
    uint64_t totalBytecode = 0;
};

class ShaderLibraryManager {
public:
    static ShaderLibraryManager& Instance() { static ShaderLibraryManager s; return s; }

    bool CompileShader(const std::string& name, ShaderLibraryStage stage,
        const std::string& source, const std::string& entryPoint = "main") {
        if (name.empty() || source.empty()) return false;

        uint64_t hash = ComputeHash(source);
        auto it = m_cache.find(hash);
        if (it != m_cache.end() && it->second.isValid) {
            ++m_stats.cacheHits;
            return true;
        }

        CompiledShaderEntry entry{};
        entry.name = name;
        entry.stage = stage;
        entry.hashKey = hash;
        entry.entryPoint = entryPoint;
        entry.compiledAtMs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        entry.sourceHash = std::to_string(hash);

        // Simulated bytecode (actual D3DCompile would go here)
        entry.bytecode.resize(source.size());
        for (size_t i = 0; i < source.size(); ++i)
            entry.bytecode[i] = static_cast<uint8_t>(source[i] ^ 0x5A);
        entry.isValid = true;

        m_cache[hash] = entry;
        m_stats.totalShaders = static_cast<uint32_t>(m_cache.size());
        m_stats.totalBytecode += entry.bytecode.size();
        ++m_stats.cacheMisses;
        return true;
    }

    const CompiledShaderEntry* GetCachedShader(const std::string& source) const {
        uint64_t hash = ComputeHash(source);
        auto it = m_cache.find(hash);
        if (it != m_cache.end() && it->second.isValid)
            return &it->second;
        return nullptr;
    }

    const CompiledShaderEntry* GetShaderByName(const std::string& name) const {
        for (const auto& [k, v] : m_cache) {
            if (v.name == name) return &v;
        }
        return nullptr;
    }

    void InvalidateCache() {
        m_cache.clear();
        m_stats.totalShaders = 0;
        m_stats.totalBytecode = 0;
    }

    void InvalidateShader(const std::string& source) {
        uint64_t hash = ComputeHash(source);
        auto it = m_cache.find(hash);
        if (it != m_cache.end()) {
            m_stats.totalBytecode -= it->second.bytecode.size();
            m_cache.erase(it);
            m_stats.totalShaders = static_cast<uint32_t>(m_cache.size());
        }
    }

    const ShaderLibraryStats& GetStats() const { return m_stats; }
    size_t CacheSize() const { return m_cache.size(); }

    static const char* StageToString(ShaderLibraryStage stage) {
        switch (stage) {
        case ShaderLibraryStage::Vertex:   return "vs";
        case ShaderLibraryStage::Pixel:    return "ps";
        case ShaderLibraryStage::Compute:  return "cs";
        case ShaderLibraryStage::Geometry: return "gs";
        case ShaderLibraryStage::Hull:     return "hs";
        case ShaderLibraryStage::Domain:   return "ds";
        default:                    return "unknown";
        }
    }

    bool Validate() const {
        for (const auto& [k, v] : m_cache) {
            if (v.hashKey != k) return false;
            if (!v.isValid) return false;
            if (v.name.empty()) return false;
            if (v.bytecode.empty()) return false;
        }
        if (m_stats.totalShaders != static_cast<uint32_t>(m_cache.size())) return false;
        return true;
    }

private:
    ShaderLibraryManager() = default;
    ~ShaderLibraryManager() = default;
    ShaderLibraryManager(const ShaderLibraryManager&) = delete;
    ShaderLibraryManager& operator=(const ShaderLibraryManager&) = delete;

    static uint64_t ComputeHash(const std::string& s) {
        // FNV-1a 64-bit
        uint64_t hash = 14695981039346656037ULL;
        for (char c : s) {
            hash ^= static_cast<uint64_t>(static_cast<uint8_t>(c));
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    std::unordered_map<uint64_t, CompiledShaderEntry> m_cache;
    ShaderLibraryStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
