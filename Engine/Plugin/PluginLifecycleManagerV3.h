// PluginLifecycleManagerV3.h — Plugin Lifecycle Manager v3
// Copyright (c) 2026 ExplorerLens Project
//
// Manages plugin load, activate, suspend, hot-swap, and unload lifecycle transitions.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

enum class PLMv3State { Unloaded, Loaded, Active, Suspended, Failed };

struct PLMv3PluginEntry {
    std::string pluginId;
    std::string version;
    PLMv3State  state = PLMv3State::Unloaded;
};

struct PLMv3HotSwapResult {
    bool        success    = false;
    std::string oldVersion;
    std::string newVersion;
    std::string errorMsg;
};

class PluginLifecycleManagerV3 {
public:
    bool Load(const std::string& pluginId, const std::string& version) {
        m_plugins[pluginId] = { pluginId, version, PLMv3State::Loaded };
        return true;
    }

    bool Activate(const std::string& pluginId) {
        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end() || it->second.state == PLMv3State::Unloaded) return false;
        it->second.state = PLMv3State::Active;
        return true;
    }

    bool Suspend(const std::string& pluginId) {
        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end()) return false;
        it->second.state = PLMv3State::Suspended;
        return true;
    }

    PLMv3HotSwapResult HotSwap(const std::string& pluginId,
                                const std::string& newVersion) {
        PLMv3HotSwapResult r;
        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end()) { r.errorMsg = "Plugin not loaded"; return r; }
        r.oldVersion       = it->second.version;
        r.newVersion       = newVersion;
        it->second.version = newVersion;
        it->second.state   = PLMv3State::Active;
        r.success          = true;
        return r;
    }

    PLMv3State GetState(const std::string& pluginId) const {
        auto it = m_plugins.find(pluginId);
        return (it != m_plugins.end()) ? it->second.state : PLMv3State::Unloaded;
    }

private:
    std::unordered_map<std::string, PLMv3PluginEntry> m_plugins;
};

}} // namespace ExplorerLens::Engine
