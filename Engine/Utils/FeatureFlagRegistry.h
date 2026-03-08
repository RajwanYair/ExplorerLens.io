// FeatureFlagRegistry.h — Centralized feature flag management
// Copyright (c) 2026 ExplorerLens Project
//
// Manages compile-time and runtime feature flags for the engine — enables
// gradual feature rollout, A/B testing, and emergency kill switches.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct FeatureFlagRegistryConfig {
    bool enabled = true;
    uint32_t maxFlags = 256;
    std::string label = "FeatureFlagRegistry";
};

class FeatureFlagRegistry {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    FeatureFlagRegistryConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    bool RegisterFlag(const std::string& name, bool defaultValue) {
        if (m_flags.size() >= m_config.maxFlags) return false;
        m_flags[name] = defaultValue;
        return true;
    }

    bool IsEnabled(const std::string& name) const {
        auto it = m_flags.find(name);
        return (it != m_flags.end()) ? it->second : false;
    }

    void SetFlag(const std::string& name, bool value) { m_flags[name] = value; }
    size_t GetFlagCount() const { return m_flags.size(); }

private:
    bool m_initialized = false;
    FeatureFlagRegistryConfig m_config;
    std::unordered_map<std::string, bool> m_flags;
};

}
} // namespace ExplorerLens::Engine
