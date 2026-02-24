//==============================================================================
// ConfigMigrationEngine
//==============================================================================

#include "ConfigMigrationEngine.h"
#include <chrono>
#include <fstream>

namespace ExplorerLens { namespace Engine {

ConfigMigrationEngine::ConfigMigrationEngine() {
    // Default schema entries
    m_schema.push_back({L"ThumbnailSize", L"256", ConfigVersion::V7_0, true});
    m_schema.push_back({L"GPUEnabled", L"1", ConfigVersion::V7_0, true});
    m_schema.push_back({L"CacheEnabled", L"1", ConfigVersion::V8_0, true});
    m_schema.push_back({L"CacheSizeMB", L"512", ConfigVersion::V8_3, false});
    m_schema.push_back({L"PluginsEnabled", L"1", ConfigVersion::V8_4, false});
    m_schema.push_back({L"AsyncMode", L"1", ConfigVersion::V9_0, false});
    m_schema.push_back({L"TelemetryEnabled", L"0", ConfigVersion::V9_2, false});
    m_schema.push_back({L"SIMDLevel", L"auto", ConfigVersion::V10_0, false});
}

void ConfigMigrationEngine::AddRule(const MigrationRule& rule) {
    m_rules.push_back(rule);
}

void ConfigMigrationEngine::SetSourceConfig(
    const std::map<std::wstring, std::wstring>& config)
{
    m_sourceConfig = config;
}

MigrationResult ConfigMigrationEngine::Migrate(ConfigVersion from, ConfigVersion to) {
    MigrationResult result;
    auto start = std::chrono::high_resolution_clock::now();

    result.sourceVersion = from;
    result.targetVersion = to;

    // Start with source config
    m_migratedConfig = m_sourceConfig;

    // Apply all rules
    for (const auto& rule : m_rules) {
        ApplyRule(rule, result);
    }

    // Add defaults for new keys in target version
    AddDefaults(to, result);

    auto end = std::chrono::high_resolution_clock::now();
    result.migrationTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    result.success = true;
    return result;
}

void ConfigMigrationEngine::ApplyRule(
    const MigrationRule& rule, MigrationResult& result)
{
    switch (rule.action) {
        case MigrationAction::Keep:
            if (m_migratedConfig.count(rule.sourceKey)) {
                result.keysKept++;
            }
            break;

        case MigrationAction::Rename: {
            auto it = m_migratedConfig.find(rule.sourceKey);
            if (it != m_migratedConfig.end()) {
                m_migratedConfig[rule.targetKey] = it->second;
                m_migratedConfig.erase(it);
                result.keysRenamed++;
            }
            break;
        }

        case MigrationAction::Transform: {
            auto it = m_migratedConfig.find(rule.sourceKey);
            if (it != m_migratedConfig.end()) {
                // Simple transform: apply target key with default value format
                m_migratedConfig[rule.targetKey.empty() ? rule.sourceKey : rule.targetKey]
                    = rule.defaultValue.empty() ? it->second : rule.defaultValue;
                if (!rule.targetKey.empty() && rule.targetKey != rule.sourceKey) {
                    m_migratedConfig.erase(it);
                }
                result.keysTransformed++;
            }
            break;
        }

        case MigrationAction::Remove: {
            auto it = m_migratedConfig.find(rule.sourceKey);
            if (it != m_migratedConfig.end()) {
                m_migratedConfig.erase(it);
                result.keysRemoved++;
            }
            break;
        }

        case MigrationAction::AddDefault:
            if (m_migratedConfig.find(rule.targetKey) == m_migratedConfig.end()) {
                m_migratedConfig[rule.targetKey] = rule.defaultValue;
                result.keysAdded++;
            }
            break;
    }
}

void ConfigMigrationEngine::AddDefaults(
    ConfigVersion targetVersion, MigrationResult& result)
{
    for (const auto& entry : m_schema) {
        if (entry.addedIn <= targetVersion) {
            if (m_migratedConfig.find(entry.key) == m_migratedConfig.end()) {
                m_migratedConfig[entry.key] = entry.value;
                result.keysAdded++;
            }
        }
    }
}

bool ConfigMigrationEngine::CreateBackup(const std::wstring& /*backupPath*/) const {
    // In production: serialize m_sourceConfig to file
    return !m_sourceConfig.empty();
}

bool ConfigMigrationEngine::RestoreBackup(const std::wstring& /*backupPath*/) {
    // In production: deserialize from file
    return true;
}

bool ConfigMigrationEngine::ValidateConfig(
    const std::map<std::wstring, std::wstring>& config) const
{
    for (const auto& entry : m_schema) {
        if (entry.isRequired) {
            if (config.find(entry.key) == config.end()) {
                return false;
            }
        }
    }
    return true;
}

const wchar_t* ConfigMigrationEngine::GetVersionName(ConfigVersion version) {
    switch (version) {
        case ConfigVersion::V7_0:  return L"v7.0";
        case ConfigVersion::V8_0:  return L"v8.0";
        case ConfigVersion::V8_3:  return L"v8.3";
        case ConfigVersion::V8_4:  return L"v8.4";
        case ConfigVersion::V9_0:  return L"v9.0";
        case ConfigVersion::V9_2:  return L"v9.2";
        case ConfigVersion::V10_0: return L"v10.0";
        default: return L"Unknown";
    }
}

const wchar_t* ConfigMigrationEngine::GetActionName(MigrationAction action) {
    switch (action) {
        case MigrationAction::Keep:       return L"Keep";
        case MigrationAction::Rename:     return L"Rename";
        case MigrationAction::Transform:  return L"Transform";
        case MigrationAction::Remove:     return L"Remove";
        case MigrationAction::AddDefault: return L"Add Default";
        default: return L"Unknown";
    }
}

}} // namespace ExplorerLens::Engine

