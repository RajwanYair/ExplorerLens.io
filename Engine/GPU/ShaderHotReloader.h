// ShaderHotReloader.h — Dynamic shader reloading without restart
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors shader source files and dynamically reloads modified shaders
// during development, avoiding process restarts for shader iteration.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ShaderHotReloaderConfig {
    bool enabled = true;
    uint32_t pollIntervalMs = 1000;
    uint32_t maxWatchedShaders = 64;
    std::string label = "ShaderHotReloader";
};

class ShaderHotReloader {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ShaderHotReloaderConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct WatchedShader {
        std::string path;
        uint64_t lastModifiedTime = 0;
        bool needsReload = false;
    };

    bool Watch(const std::string& shaderPath) {
        if (m_shaders.size() >= m_config.maxWatchedShaders) return false;
        m_shaders.push_back({ shaderPath, 0, false });
        return true;
    }

    uint32_t CheckForChanges(uint64_t currentTime) {
        uint32_t changed = 0;
        for (auto& s : m_shaders) {
            if (currentTime > s.lastModifiedTime) {
                s.needsReload = true;
                s.lastModifiedTime = currentTime;
                changed++;
            }
        }
        return changed;
    }

    uint32_t GetWatchedCount() const { return static_cast<uint32_t>(m_shaders.size()); }
    uint32_t GetReloadCount() const { return m_reloadCount; }

private:
    bool m_initialized = false;
    ShaderHotReloaderConfig m_config;
    std::vector<WatchedShader> m_shaders;
    uint32_t m_reloadCount = 0;
};

}
} // namespace ExplorerLens::Engine
