#pragma once
// DiagnosticReportGeneratorV2.h — Diagnostic Report Generator V2
// Comprehensive one-click diagnostic report generation covering system info,
// GPU capabilities, decoder health, cache stats, and recent errors.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Report section
enum class DiagReportSection : uint8_t {
 SystemInfo = 0,
 GPUCapabilities,
 DecoderHealth,
 CacheStatistics,
 MemoryUsage,
 RecentErrors,
 PerformanceMetrics,
 PluginStatus,
 PolicyConfiguration,
 COUNT
};

/// Report format
enum class DiagReportFormat : uint8_t {
 PlainText = 0,
 HTML,
 JSON,
 XML,
 Markdown,
 COUNT
};

struct DiagReportConfig {
 DiagReportFormat format = DiagReportFormat::HTML;
 bool includeSensitiveData = false;
 bool includeScreenshots = false;
 bool anonymize = true;
 uint32_t recentErrorCount = 50;
 uint32_t performanceWindowMin = 60;
};

struct DiagReportStats {
 uint32_t sectionsGenerated = 0;
 uint32_t sectionsSkipped = 0;
 uint64_t reportSizeBytes = 0;
 double generationTimeMs = 0.0;
};

class DiagnosticReportGeneratorV2 {
public:
 static constexpr size_t SectionCount() {
 return static_cast<size_t>(DiagReportSection::COUNT);
 }
 static constexpr size_t FormatCount() {
 return static_cast<size_t>(DiagReportFormat::COUNT);
 }

 static const wchar_t *SectionName(DiagReportSection s) {
 switch (s) {
 case DiagReportSection::SystemInfo:
 return L"System Info";
 case DiagReportSection::GPUCapabilities:
 return L"GPU Capabilities";
 case DiagReportSection::DecoderHealth:
 return L"Decoder Health";
 case DiagReportSection::CacheStatistics:
 return L"Cache Statistics";
 case DiagReportSection::MemoryUsage:
 return L"Memory Usage";
 case DiagReportSection::RecentErrors:
 return L"Recent Errors";
 case DiagReportSection::PerformanceMetrics:
 return L"Performance Metrics";
 case DiagReportSection::PluginStatus:
 return L"Plugin Status";
 case DiagReportSection::PolicyConfiguration:
 return L"Policy Configuration";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *FormatName(DiagReportFormat f) {
 switch (f) {
 case DiagReportFormat::PlainText:
 return L"Plain Text";
 case DiagReportFormat::HTML:
 return L"HTML";
 case DiagReportFormat::JSON:
 return L"JSON";
 case DiagReportFormat::XML:
 return L"XML";
 case DiagReportFormat::Markdown:
 return L"Markdown";
 default:
 return L"Unknown";
 }
 }

 /// Get MIME type for report format
 static const char *MimeType(DiagReportFormat f) {
 switch (f) {
 case DiagReportFormat::PlainText:
 return "text/plain";
 case DiagReportFormat::HTML:
 return "text/html";
 case DiagReportFormat::JSON:
 return "application/json";
 case DiagReportFormat::XML:
 return "application/xml";
 case DiagReportFormat::Markdown:
 return "text/markdown";
 default:
 return "application/octet-stream";
 }
 }
};

} // namespace Engine
} // namespace ExplorerLens
