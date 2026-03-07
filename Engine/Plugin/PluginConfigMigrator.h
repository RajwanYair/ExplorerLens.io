or execution logs.// PluginConfigMigrator.h — Plugin Configuration Schema Migration
// Copyright (c) 2026 ExplorerLens Project
//
// Migrates plugin configuration files between schema versions when the
// plugin API changes, preserving user customizations across upgrades.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct ConfigField {
    std::string key;
    std::string value;
    std::string type; // "string", "int", "bool", "float"
};

struct ConfigMigrationRule {
    uint32_t fromVersion = 0;
    uint32_t toVersion = 0;
    std::string description;
    std::function<std::vector<ConfigField>(const std::vector<ConfigField>&)> transform;
};

struct ConfigMigrationResult {
    bool success = false;
    uint32_t fromVersion = 0;
    uint32_t toVersion = 0;
    uint32_t fieldsPreserved = 0;
    uint32_t fieldsAdded = 0;
    uint32_t fieldsRemoved = 0;
    std::vector<std::string> warnings;
};

class PluginConfigMigrator {
public:
    void RegisterRule(ConfigMigrationRule rule) {
        m_rules.push_back(std::move(rule));
    }

    bool CanMigrate(uint32_t fromVersion, uint32_t toVersion) const {
        uint32_t current = fromVersion;
        while (current < toVersion) {
            bool found = false;
            for (const auto& r : m_rules) {
                if (r.fromVersion == current) {
                    current = r.toVersion;
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return current == toVersion;
    }

    ConfigMigrationResult Migrate(const std::vector<ConfigField>& fields,
        uint32_t fromVersion, uint32_t toVersion) const {
        ConfigMigrationResult result;
        result.fromVersion = fromVersion;
        result.toVersion = toVersion;

        auto currentFields = fields;
        uint32_t current = fromVersion;

        while (current < toVersion) {
            bool found = false;
            for (const auto& r : m_rules) {
                if (r.fromVersion == current && r.transform) {
                    auto newFields = r.transform(currentFields);
                    result.fieldsAdded += static_cast<uint32_t>(
                        newFields.size() > currentFields.size()
                        ? newFields.size() - currentFields.size() : 0);
                    result.fieldsRemoved += static_cast<uint32_t>(
                        currentFields.size() > newFields.size()
                        ? currentFields.size() - newFields.size() : 0);
                    currentFields = std::move(newFields);
                    current = r.toVersion;
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.warnings.push_back("No migration rule for version " +
                    std::to_string(current));
                return result;
            }
        }

        result.fieldsPreserved = static_cast<uint32_t>(currentFields.size());
        result.success = true;
        return result;
    }

    size_t RuleCount() const { return m_rules.size(); }

private:
    std::vector<ConfigMigrationRule> m_rules;
};

} // namespace Engine
} // namespace ExplorerLens
