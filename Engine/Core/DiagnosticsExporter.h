#pragma once
// Diagnostics Export Finalization
// One-click ZIP bundle export for support diagnostics.
// Collects system info, decoder health, ETW logs, and config into a ZIP.

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <sstream>

namespace ExplorerLens::Core {

// ─── Diagnostic data category ───────────────────────────────────
enum class DiagCategory : uint8_t {
 SystemInfo,
 DecoderHealth,
 ETWLogs,
 Configuration,
 PerformanceData,
 ErrorLogs,
 RegistrySnapshot,
 CacheStats,
 GPUDiagnostics,
 PluginStatus
};

inline const char* DiagCategoryName(DiagCategory c) {
 switch (c) {
 case DiagCategory::SystemInfo: return "SystemInfo";
 case DiagCategory::DecoderHealth: return "DecoderHealth";
 case DiagCategory::ETWLogs: return "ETWLogs";
 case DiagCategory::Configuration: return "Configuration";
 case DiagCategory::PerformanceData: return "PerformanceData";
 case DiagCategory::ErrorLogs: return "ErrorLogs";
 case DiagCategory::RegistrySnapshot: return "RegistrySnapshot";
 case DiagCategory::CacheStats: return "CacheStats";
 case DiagCategory::GPUDiagnostics: return "GPUDiagnostics";
 case DiagCategory::PluginStatus: return "PluginStatus";
 default: return "Unknown";
 }
}

// ─── Diagnostic entry ───────────────────────────────────────────
struct DiagEntry {
 DiagCategory category = DiagCategory::SystemInfo;
 std::string filename;
 std::string content;
 size_t sizeBytes = 0;
 bool sensitive = false; // contains PII or paths

 static DiagEntry Create(DiagCategory cat, const std::string& file, const std::string& data) {
 DiagEntry e;
 e.category = cat;
 e.filename = file;
 e.content = data;
 e.sizeBytes = data.size();
 return e;
 }
};

// ─── Export configuration ───────────────────────────────────────
struct DiagExportConfig {
 std::string outputPath = "ExplorerLens_Diagnostics.zip";
 bool includeSensitive = false;
 bool includeETW = true;
 bool includePerformance = true;
 bool includeRegistry = true;
 bool includeGPU = true;
 bool anonymizePaths = true;
 size_t maxLogSizeBytes = 10 * 1024 * 1024; // 10 MB per file

 static DiagExportConfig Default() { return {}; }

 static DiagExportConfig Minimal() {
 DiagExportConfig c;
 c.includeETW = false;
 c.includePerformance = false;
 c.includeRegistry = false;
 c.includeGPU = false;
 c.maxLogSizeBytes = 1 * 1024 * 1024;
 return c;
 }

 static DiagExportConfig Full() {
 DiagExportConfig c;
 c.includeSensitive = true;
 c.anonymizePaths = false;
 c.maxLogSizeBytes = 50 * 1024 * 1024;
 return c;
 }
};

// ─── Export result ───────────────────────────────────────────────
enum class ExportStatus : uint8_t {
 Success,
 PartialSuccess,
 NoData,
 PathError,
 SizeExceeded,
 InternalError
};

inline const char* ExportStatusName(ExportStatus s) {
 switch (s) {
 case ExportStatus::Success: return "Success";
 case ExportStatus::PartialSuccess: return "PartialSuccess";
 case ExportStatus::NoData: return "NoData";
 case ExportStatus::PathError: return "PathError";
 case ExportStatus::SizeExceeded: return "SizeExceeded";
 case ExportStatus::InternalError: return "InternalError";
 default: return "Unknown";
 }
}

struct ExportResult {
 ExportStatus status = ExportStatus::NoData;
 std::string outputPath;
 size_t totalSizeBytes = 0;
 size_t fileCount = 0;
 size_t skippedCount = 0;
 std::vector<std::string> includedFiles;
 std::vector<std::string> skippedFiles;

 bool IsSuccess() const {
 return status == ExportStatus::Success || status == ExportStatus::PartialSuccess;
 }

 std::string Summary() const {
 std::string s = "DiagExport: status=";
 s += ExportStatusName(status);
 s += ", files=" + std::to_string(fileCount);
 s += ", size=" + std::to_string(totalSizeBytes / 1024) + "KB";
 if (skippedCount > 0) s += ", skipped=" + std::to_string(skippedCount);
 return s;
 }
};

// ─── Diagnostics exporter ───────────────────────────────────────
class DiagnosticsExporter {
public:
 static DiagnosticsExporter Create(const DiagExportConfig& config = DiagExportConfig::Default()) {
 DiagnosticsExporter exp;
 exp.m_config = config;
 return exp;
 }

 // Add a diagnostic entry
 void AddEntry(const DiagEntry& entry) {
 m_entries.push_back(entry);
 }

 // Add system info
 void AddSystemInfo(const std::string& info) {
 AddEntry(DiagEntry::Create(DiagCategory::SystemInfo, "system_info.txt", info));
 }

 // Add decoder health snapshot
 void AddDecoderHealth(const std::string& healthJson) {
 AddEntry(DiagEntry::Create(DiagCategory::DecoderHealth, "decoder_health.json", healthJson));
 }

 // Add error log
 void AddErrorLog(const std::string& log) {
 AddEntry(DiagEntry::Create(DiagCategory::ErrorLogs, "error_log.txt", log));
 }

 size_t EntryCount() const { return m_entries.size(); }

 // Filter entries based on config
 std::vector<DiagEntry> FilteredEntries() const {
 std::vector<DiagEntry> filtered;
 for (const auto& e : m_entries) {
 if (e.sensitive && !m_config.includeSensitive) continue;
 if (e.category == DiagCategory::ETWLogs && !m_config.includeETW) continue;
 if (e.category == DiagCategory::PerformanceData && !m_config.includePerformance) continue;
 if (e.category == DiagCategory::RegistrySnapshot && !m_config.includeRegistry) continue;
 if (e.category == DiagCategory::GPUDiagnostics && !m_config.includeGPU) continue;
 if (e.sizeBytes > m_config.maxLogSizeBytes) continue;
 filtered.push_back(e);
 }
 return filtered;
 }

 // Simulate export (in production: write ZIP)
 ExportResult Export() const {
 ExportResult result;
 result.outputPath = m_config.outputPath;

 auto filtered = FilteredEntries();
 if (filtered.empty()) {
 result.status = ExportStatus::NoData;
 return result;
 }

 for (const auto& e : filtered) {
 result.includedFiles.push_back(e.filename);
 result.totalSizeBytes += e.sizeBytes;
 result.fileCount++;
 }

 size_t skipped = m_entries.size() - filtered.size();
 result.skippedCount = skipped;
 for (const auto& e : m_entries) {
 bool found = false;
 for (const auto& f : filtered)
 if (f.filename == e.filename) { found = true; break; }
 if (!found) result.skippedFiles.push_back(e.filename);
 }

 result.status = (skipped > 0) ? ExportStatus::PartialSuccess : ExportStatus::Success;
 return result;
 }

 // Anonymize a path
 static std::string AnonymizePath(const std::string& path) {
 // Replace user directory with <USER>
 auto pos = path.find("Users\\");
 if (pos != std::string::npos) {
 auto nextSlash = path.find('\\', pos + 6);
 if (nextSlash != std::string::npos) {
 return path.substr(0, pos) + "Users\\<USER>" + path.substr(nextSlash);
 }
 }
 return path;
 }

 const DiagExportConfig& Config() const { return m_config; }

private:
 DiagExportConfig m_config;
 std::vector<DiagEntry> m_entries;
};

} // namespace ExplorerLens::Core

