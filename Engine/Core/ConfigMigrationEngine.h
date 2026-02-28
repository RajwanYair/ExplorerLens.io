#pragma once
// =============================================================================
// ConfigMigrationEngine.h — Settings Migration Between Versions
// ExplorerLens Engine — Core Module
// =============================================================================

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Core {

/// Migration action type
enum class MigrationAction : uint32_t {
 Copy = 0, ///< Copy value as-is
 Rename = 1, ///< Rename key
 Transform = 2, ///< Apply transformation function
 Delete = 3, ///< Remove deprecated key
 SetDefault = 4, ///< Set new default value
 Count = 5
};

/// Migration status
enum class MigrationStatus : uint32_t {
 NotStarted = 0,
 InProgress = 1,
 Completed = 2,
 Failed = 3,
 RolledBack = 4
};

/// A single migration rule
struct MigrationRule {
 std::wstring sourceKey;
 std::wstring targetKey;
 MigrationAction action = MigrationAction::Copy;
 std::wstring defaultValue;
 std::wstring description;
};

/// Migration result for a single rule
struct MigrationRuleResult {
 MigrationRule rule;
 bool success = false;
 std::wstring oldValue;
 std::wstring newValue;
 std::wstring errorMessage;
};

/// Overall migration report
struct MigrationReport {
 std::wstring sourceVersion;
 std::wstring targetVersion;
 MigrationStatus status = MigrationStatus::NotStarted;
 uint32_t totalRules = 0;
 uint32_t successCount = 0;
 uint32_t failureCount = 0;
 uint32_t skippedCount = 0;
 std::vector<MigrationRuleResult> results;
};

/// ConfigMigrationEngine — migrates settings between ExplorerLens versions
class ConfigMigrationEngine {
public:
 ConfigMigrationEngine();

 // Configuration
 void SetSourceVersion(const std::wstring &version);
 void SetTargetVersion(const std::wstring &version);
 void AddRule(const MigrationRule &rule);
 void AddSetting(const std::wstring &key, const std::wstring &value);

 // Migration
 MigrationReport Migrate();
 bool Rollback();

 // Queries
 std::wstring GetSetting(const std::wstring &key) const;
 bool HasSetting(const std::wstring &key) const;
 uint32_t GetRuleCount() const {
 return static_cast<uint32_t>(m_rules.size());
 }
 uint32_t GetSettingCount() const {
 return static_cast<uint32_t>(m_settings.size());
 }
 const std::wstring &GetSourceVersion() const { return m_sourceVersion; }
 const std::wstring &GetTargetVersion() const { return m_targetVersion; }

 // Static
 static const wchar_t *GetActionName(MigrationAction action);
 static const wchar_t *GetStatusName(MigrationStatus status);
 static constexpr uint32_t GetActionCount() {
 return static_cast<uint32_t>(MigrationAction::Count);
 }

private:
 std::wstring m_sourceVersion;
 std::wstring m_targetVersion;
 std::vector<MigrationRule> m_rules;
 std::map<std::wstring, std::wstring> m_settings;
 std::map<std::wstring, std::wstring> m_backup; ///< For rollback
 MigrationStatus m_status = MigrationStatus::NotStarted;
};

} // namespace Core
} // namespace ExplorerLens
