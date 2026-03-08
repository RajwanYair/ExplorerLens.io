// Utils\VersionMatrixValidator.h - VersionMatrixValidator
// Copyright (c) 2026 ExplorerLens Project
//
// Cross-component version consistency validation engine
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class VersionMatrixValidator {
public:
    struct Config {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "VersionMatrixValidator";
    };

    VersionMatrixValidator() = default;
    explicit VersionMatrixValidator(const Config& cfg) : m_config(cfg) {}

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
    std::string m_versionMap{};
    uint32_t m_driftThreshold{};
    std::string m_lastAuditResult{};
};

} // namespace Engine
} // namespace ExplorerLens
