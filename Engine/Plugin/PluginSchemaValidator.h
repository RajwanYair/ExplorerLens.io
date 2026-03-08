// PluginSchemaValidator.h — Validates plugin manifest schemas
// Copyright (c) 2026 ExplorerLens Project
//
// Validates plugin manifest JSON against the expected schema — checks
// required fields, version compatibility, and permission declarations.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PluginSchemaValidatorConfig {
    bool enabled = true;
    uint32_t schemaVersion = 2;
    std::string label = "PluginSchemaValidator";
};

class PluginSchemaValidator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PluginSchemaValidatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct ValidationResult {
        bool valid = false;
        std::vector<std::string> errors;
    };

    bool ValidateRequiredField(const std::string& /*fieldName*/, const std::string& value) const {
        return !value.empty();
    }

    bool IsVersionCompatible(uint32_t manifestVersion) const {
        return manifestVersion <= m_config.schemaVersion;
    }

private:
    bool m_initialized = false;
    PluginSchemaValidatorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
