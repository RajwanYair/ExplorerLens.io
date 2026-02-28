// =============================================================================
// ConfigMigrationEngine.cpp — Settings Migration Between Versions
// ExplorerLens Engine — Core Module
// =============================================================================

#include "ConfigMigrationEngine.h"
#include <algorithm>

namespace ExplorerLens {
namespace Core {

ConfigMigrationEngine::ConfigMigrationEngine() {}

void ConfigMigrationEngine::SetSourceVersion(const std::wstring &version) {
 m_sourceVersion = version;
}

void ConfigMigrationEngine::SetTargetVersion(const std::wstring &version) {
 m_targetVersion = version;
}

void ConfigMigrationEngine::AddRule(const MigrationRule &rule) {
 m_rules.push_back(rule);
}

void ConfigMigrationEngine::AddSetting(const std::wstring &key,
 const std::wstring &value) {
 m_settings[key] = value;
}

MigrationReport ConfigMigrationEngine::Migrate() {
 MigrationReport report;
 report.sourceVersion = m_sourceVersion;
 report.targetVersion = m_targetVersion;
 report.totalRules = static_cast<uint32_t>(m_rules.size());

 // Backup current settings for rollback
 m_backup = m_settings;
 m_status = MigrationStatus::InProgress;
 report.status = MigrationStatus::InProgress;

 for (const auto &rule : m_rules) {
 MigrationRuleResult rResult;
 rResult.rule = rule;

 switch (rule.action) {
 case MigrationAction::Copy: {
 auto it = m_settings.find(rule.sourceKey);
 if (it != m_settings.end()) {
 rResult.oldValue = it->second;
 rResult.newValue = it->second;
 if (!rule.targetKey.empty() && rule.targetKey != rule.sourceKey) {
 m_settings[rule.targetKey] = it->second;
 }
 rResult.success = true;
 } else {
 rResult.success = false;
 rResult.errorMessage = L"Source key not found";
 report.skippedCount++;
 }
 break;
 }
 case MigrationAction::Rename: {
 auto it = m_settings.find(rule.sourceKey);
 if (it != m_settings.end()) {
 rResult.oldValue = it->second;
 rResult.newValue = it->second;
 m_settings[rule.targetKey] = it->second;
 m_settings.erase(it);
 rResult.success = true;
 } else {
 rResult.success = false;
 rResult.errorMessage = L"Source key not found";
 report.skippedCount++;
 }
 break;
 }
 case MigrationAction::Delete: {
 auto it = m_settings.find(rule.sourceKey);
 if (it != m_settings.end()) {
 rResult.oldValue = it->second;
 m_settings.erase(it);
 rResult.success = true;
 } else {
 rResult.success = true; // Already gone
 }
 break;
 }
 case MigrationAction::SetDefault: {
 if (m_settings.find(rule.targetKey) == m_settings.end()) {
 m_settings[rule.targetKey] = rule.defaultValue;
 rResult.newValue = rule.defaultValue;
 rResult.success = true;
 } else {
 rResult.success = true; // Already exists
 rResult.newValue = m_settings[rule.targetKey];
 report.skippedCount++;
 }
 break;
 }
 case MigrationAction::Transform: {
 // Transform requires external function — mark as copy for now
 auto it = m_settings.find(rule.sourceKey);
 if (it != m_settings.end()) {
 rResult.oldValue = it->second;
 rResult.newValue = it->second;
 rResult.success = true;
 } else {
 rResult.success = false;
 rResult.errorMessage = L"Source key not found";
 }
 break;
 }
 default:
 rResult.success = false;
 rResult.errorMessage = L"Unknown action";
 break;
 }

 if (rResult.success)
 report.successCount++;
 else if (rResult.errorMessage.find(L"not found") == std::wstring::npos)
 report.failureCount++;

 report.results.push_back(rResult);
 }

 m_status = (report.failureCount == 0) ? MigrationStatus::Completed
 : MigrationStatus::Failed;
 report.status = m_status;
 return report;
}

bool ConfigMigrationEngine::Rollback() {
 if (m_backup.empty())
 return false;
 m_settings = m_backup;
 m_backup.clear();
 m_status = MigrationStatus::RolledBack;
 return true;
}

std::wstring ConfigMigrationEngine::GetSetting(const std::wstring &key) const {
 auto it = m_settings.find(key);
 return (it != m_settings.end()) ? it->second : L"";
}

bool ConfigMigrationEngine::HasSetting(const std::wstring &key) const {
 return m_settings.find(key) != m_settings.end();
}

const wchar_t *ConfigMigrationEngine::GetActionName(MigrationAction action) {
 switch (action) {
 case MigrationAction::Copy:
 return L"Copy";
 case MigrationAction::Rename:
 return L"Rename";
 case MigrationAction::Transform:
 return L"Transform";
 case MigrationAction::Delete:
 return L"Delete";
 case MigrationAction::SetDefault:
 return L"Set Default";
 default:
 return L"Unknown";
 }
}

const wchar_t *ConfigMigrationEngine::GetStatusName(MigrationStatus status) {
 switch (status) {
 case MigrationStatus::NotStarted:
 return L"Not Started";
 case MigrationStatus::InProgress:
 return L"In Progress";
 case MigrationStatus::Completed:
 return L"Completed";
 case MigrationStatus::Failed:
 return L"Failed";
 case MigrationStatus::RolledBack:
 return L"Rolled Back";
 default:
 return L"Unknown";
 }
}

} // namespace Core
} // namespace ExplorerLens
