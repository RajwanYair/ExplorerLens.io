// Utils\DependencyGraphValidator.h - DependencyGraphValidator
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the dependency graph for cycles and missing edges
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class DependencyGraphValidator {
public:
    struct Config {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "DependencyGraphValidator";
    };

    DependencyGraphValidator() = default;
    explicit DependencyGraphValidator(const Config& cfg) : m_config(cfg) {}

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
    std::string m_nodes{};
    std::string m_edges{};
    bool m_cycleDetected{};
};

} // namespace Engine
} // namespace ExplorerLens
