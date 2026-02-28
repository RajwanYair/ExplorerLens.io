#pragma once
//==============================================================================
// ConfigMigrationEngine
// Handles configuration migration between ExplorerLens versions.
// Supports schema versioning, value mapping, backup/restore, and validation.
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace ExplorerLens { namespace Engine {

enum class ConfigVersion : uint8_t {
 V7_0 = 0,
 V8_0 = 1,
 V8_3 = 2,
 V8_4 = 3,
 V9_0 = 4,
 V9_2 = 5,
 V10_0 = 6,
 VersionCount = 7
};

enum class MigrationAction : uint8_t {
 Keep = 0,
 Rename = 1,
 Transform = 2,
 Remove = 3,
 AddDefault = 4
};

struct MigrationRule {
 std::wstring sourceKey;
 std::wstring targetKey;
 MigrationAction action = MigrationAction::Keep;
 std::wstring defaultValue;
};

struct ConfigEntry {
 std::wstring key;
 std::wstring value;
 ConfigVersion addedIn = ConfigVersion::V7_0;
 bool isRequired = false;
};

struct MigrationResult {
 bool success = false;
 ConfigVersion sourceVersion = ConfigVersion::V7_0;
 ConfigVersion targetVersion = ConfigVersion::V10_0;
 uint32_t keysKept = 0;
 uint32_t keysRenamed = 0;
 uint32_t keysRemoved = 0;
 uint32_t keysAdded = 0;
 uint32_t keysTransformed = 0;
 double migrationTimeMs = 0.0;
 std::vector<std::wstring> warnings;
};

class ConfigMigrationEngine {
public:
 ConfigMigrationEngine();

 void AddRule(const MigrationRule& rule);
 void SetSourceConfig(const std::map<std::wstring, std::wstring>& config);
 MigrationResult Migrate(ConfigVersion from, ConfigVersion to);

 std::map<std::wstring, std::wstring> GetMigratedConfig() const { return m_migratedConfig; }
 const std::vector<MigrationRule>& GetRules() const { return m_rules; }

 bool CreateBackup(const std::wstring& backupPath) const;
 bool RestoreBackup(const std::wstring& backupPath);
 bool ValidateConfig(const std::map<std::wstring, std::wstring>& config) const;

 static const wchar_t* GetVersionName(ConfigVersion version);
 static uint32_t GetVersionCount() { return static_cast<uint32_t>(ConfigVersion::VersionCount); }
 static const wchar_t* GetActionName(MigrationAction action);

private:
 std::vector<MigrationRule> m_rules;
 std::map<std::wstring, std::wstring> m_sourceConfig;
 std::map<std::wstring, std::wstring> m_migratedConfig;
 std::vector<ConfigEntry> m_schema;

 void ApplyRule(const MigrationRule& rule, MigrationResult& result);
 void AddDefaults(ConfigVersion targetVersion, MigrationResult& result);
};

}} // namespace ExplorerLens::Engine

