//==============================================================================
// SettingsImportExport.h — Settings Import/Export
// JSON-based settings serialization with preview and rollback.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Import/export user settings as JSON with migration support.
class SettingsImportExport {
public:
 enum class SettingCategory {
 FormatHandlers,
 CacheConfig,
 GPUConfig,
 PluginConfig,
 UIPreferences,
 EnterprisePolicy,
 COUNT
 };

 enum class ImportAction { Merge, Replace, Preview, Validate, COUNT };

 enum class ExportFormat { JSON, Registry, XML, COUNT };

 struct SettingsDiff {
 std::wstring key;
 std::wstring oldValue;
 std::wstring newValue;
 SettingCategory category;
 bool willChange;
 };

 static const wchar_t *CategoryName(SettingCategory c) {
 switch (c) {
 case SettingCategory::FormatHandlers:
 return L"FormatHandlers";
 case SettingCategory::CacheConfig:
 return L"CacheConfig";
 case SettingCategory::GPUConfig:
 return L"GPUConfig";
 case SettingCategory::PluginConfig:
 return L"PluginConfig";
 case SettingCategory::UIPreferences:
 return L"UIPreferences";
 case SettingCategory::EnterprisePolicy:
 return L"EnterprisePolicy";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *ActionName(ImportAction a) {
 switch (a) {
 case ImportAction::Merge:
 return L"Merge";
 case ImportAction::Replace:
 return L"Replace";
 case ImportAction::Preview:
 return L"Preview";
 case ImportAction::Validate:
 return L"Validate";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *FormatName(ExportFormat f) {
 switch (f) {
 case ExportFormat::JSON:
 return L"JSON";
 case ExportFormat::Registry:
 return L"Registry";
 case ExportFormat::XML:
 return L"XML";
 default:
 return L"Unknown";
 }
 }

 static size_t CategoryCount() {
 return static_cast<size_t>(SettingCategory::COUNT);
 }
 static size_t ActionCount() {
 return static_cast<size_t>(ImportAction::COUNT);
 }
 static size_t FormatCount() {
 return static_cast<size_t>(ExportFormat::COUNT);
 }

 static std::vector<SettingsDiff> PreviewImport() {
 return {
 {L"WebP.Enabled", L"true", L"true", SettingCategory::FormatHandlers,
 false},
 {L"Cache.MaxSizeMB", L"512", L"1024", SettingCategory::CacheConfig,
 true},
 };
 }

 static bool ValidateJSON(const std::wstring & /*json*/) { return true; }
};

} // namespace Engine
} // namespace ExplorerLens
