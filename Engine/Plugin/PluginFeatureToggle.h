// PluginFeatureToggle.h — Runtime feature flags for plugin capabilities
// Copyright (c) 2026 ExplorerLens Project
//
// Allows plugins to declare optional features that can be enabled/disabled
// at runtime without reloading — supports A/B testing and gradual rollout.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct PluginFeatureToggleConfig {
    bool enabled = true;
    uint32_t maxFeatures = 128;
    std::string label = "PluginFeatureToggle";
};

class PluginFeatureToggle {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PluginFeatureToggleConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    bool RegisterFeature(const std::string& name, bool defaultState) {
        if (m_features.size() >= m_config.maxFeatures) return false;
        m_features[name] = defaultState;
        return true;
    }

    bool IsFeatureEnabled(const std::string& name) const {
        auto it = m_features.find(name);
        return (it != m_features.end()) ? it->second : false;
    }

    void SetFeature(const std::string& name, bool state) {
        m_features[name] = state;
    }

    size_t GetFeatureCount() const { return m_features.size(); }

private:
    bool m_initialized = false;
    PluginFeatureToggleConfig m_config;
    std::unordered_map<std::string, bool> m_features;
};

}
} // namespace ExplorerLens::Engine
