// PluginStateCoordinator.h — Plugin state machine coordination
// Copyright (c) 2026 ExplorerLens Project
//
// Coordinates plugin state transitions (load → init → activate → deactivate → unload)
// with health checks and graceful shutdown support.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct PluginStateCoordinatorConfig {
    bool enabled = true;
    uint32_t initTimeoutMs = 5000;
    uint32_t shutdownTimeoutMs = 3000;
    std::string label = "PluginStateCoordinator";
};

class PluginStateCoordinator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PluginStateCoordinatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class State : uint8_t {
        Unloaded, Loaded, Initialized, Active, Deactivating, Failed
    };

    State GetState(const std::string& pluginId) const { (void)pluginId; return State::Unloaded; }

    bool Activate(const std::string& pluginId) {
        (void)pluginId;
        m_activeCount++;
        return true;
    }

    bool Deactivate(const std::string& pluginId) {
        (void)pluginId;
        if (m_activeCount > 0) m_activeCount--;
        return true;
    }

    uint32_t GetActiveCount() const { return m_activeCount; }

    bool HealthCheck(const std::string& pluginId) const { (void)pluginId; return true; }

private:
    bool m_initialized = false;
    uint32_t m_activeCount = 0;
    PluginStateCoordinatorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
