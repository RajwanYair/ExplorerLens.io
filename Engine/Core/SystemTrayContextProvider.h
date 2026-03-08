// Core\SystemTrayContextProvider.h - SystemTrayContextProvider
// Copyright (c) 2026 ExplorerLens Project
//
// Provides context menu items and balloon tips for the system tray icon
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class SystemTrayContextProvider {
public:
    struct Config {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "SystemTrayContextProvider";
    };

    SystemTrayContextProvider() = default;
    explicit SystemTrayContextProvider(const Config& cfg) : m_config(cfg) {}

    bool Initialize() {
        if (!m_config.enabled) return false;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }
    const Config& GetConfig() const { return m_config; }
    const std::string& GetName() const { return m_config.label; }

private:
    Config m_config{};
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
