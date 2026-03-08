// PluginCapabilityNegotiator.h — Plugin capability handshake negotiation
// Copyright (c) 2026 ExplorerLens Project
//
// Negotiates capabilities between the host engine and plugins during
// registration, ensuring ABI compatibility and feature-set alignment.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct PluginCapabilityNegotiatorConfig {
    bool enabled = true;
    uint32_t minAbiVersion = 1;
    uint32_t maxAbiVersion = 3;
    std::string label = "PluginCapabilityNegotiator";
};

class PluginCapabilityNegotiator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PluginCapabilityNegotiatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct NegotiationResult {
        bool compatible = false;
        uint32_t agreedAbiVersion = 0;
        bool supportsAsync = false;
        bool supportsGPU = false;
    };

    NegotiationResult Negotiate(uint32_t pluginAbiVersion, bool pluginSupportsAsync,
        bool pluginSupportsGPU) const {
        NegotiationResult result;
        result.compatible = pluginAbiVersion >= m_config.minAbiVersion &&
            pluginAbiVersion <= m_config.maxAbiVersion;
        result.agreedAbiVersion = result.compatible ? pluginAbiVersion : 0;
        result.supportsAsync = pluginSupportsAsync;
        result.supportsGPU = pluginSupportsGPU;
        return result;
    }

    bool IsAbiCompatible(uint32_t pluginVersion) const {
        return pluginVersion >= m_config.minAbiVersion &&
            pluginVersion <= m_config.maxAbiVersion;
    }

private:
    bool m_initialized = false;
    PluginCapabilityNegotiatorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
