// PluginDiagnosticsExporter.h — Export Plugin Diagnostics Bundle
// Copyright (c) 2026 ExplorerLens Project
//
// Collects and exports plugin diagnostic information in JSON/HTML/ZIP
// formats for troubleshooting and support scenarios.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <sstream>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class PluginDiagExportFormat : uint32_t {
    JSON = 0,
    HTML = 1,
    ZIP = 2
};

struct PluginDiagEntry {
    std::string pluginId;
    std::string pluginName;
    std::string version;
    std::string status;       // "loaded", "failed", "disabled"
    uint64_t    loadTimeMs = 0;
    uint64_t    memoryUsed = 0;
    uint32_t    errorCount = 0;
    std::string lastError;
    std::vector<std::string> logMessages;
};

struct PluginDiagReport {
    std::string              generatedAt;
    std::string              hostVersion;
    uint32_t                 totalPlugins = 0;
    uint32_t                 loadedPlugins = 0;
    uint32_t                 failedPlugins = 0;
    std::vector<PluginDiagEntry> entries;
    uint64_t                 totalMemoryUsed = 0;

    std::string Summary() const {
        std::ostringstream oss;
        oss << totalPlugins << " plugins (" << loadedPlugins << " loaded, "
            << failedPlugins << " failed), Memory: "
            << (totalMemoryUsed / (1024 * 1024)) << " MB";
        return oss.str();
    }
};

class PluginDiagnosticsExporter {
public:
    static PluginDiagnosticsExporter& Instance() {
        static PluginDiagnosticsExporter s;
        return s;
    }

    void AddEntry(const PluginDiagEntry& entry) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries[entry.pluginId] = entry;
    }

    PluginDiagReport CollectDiag() {
        std::lock_guard<std::mutex> lock(m_mutex);
        PluginDiagReport report;
        report.hostVersion = "15.0.0";

        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        wsprintfA(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        report.generatedAt = buf;

        for (const auto& [id, entry] : m_entries) {
            report.entries.push_back(entry);
            report.totalPlugins++;
            report.totalMemoryUsed += entry.memoryUsed;
            if (entry.status == "loaded") report.loadedPlugins++;
            else if (entry.status == "failed") report.failedPlugins++;
        }

        m_lastReport = report;
        return report;
    }

    bool Export(const std::wstring& path, PluginDiagExportFormat format) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string content;

        switch (format) {
        case PluginDiagExportFormat::JSON:
            content = GenerateJSON();
            break;
        case PluginDiagExportFormat::HTML:
            content = GenerateHTML();
            break;
        case PluginDiagExportFormat::ZIP:
            content = GenerateJSON(); // Simplified: write JSON for now
            break;
        }

        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        DWORD written;
        WriteFile(hFile, content.c_str(), static_cast<DWORD>(content.size()), &written, nullptr);
        CloseHandle(hFile);
        return written == content.size();
    }

    std::string GetReportSummary() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_lastReport.Summary();
    }

    PluginDiagReport GetLastReport() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_lastReport;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
        m_lastReport = PluginDiagReport{};
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [id, entry] : m_entries) {
            if (entry.pluginId.empty()) return false;
            if (entry.pluginId != id) return false;
            if (entry.status != "loaded" && entry.status != "failed" &&
                entry.status != "disabled") return false;
        }
        return true;
    }

private:
    PluginDiagnosticsExporter() = default;
    ~PluginDiagnosticsExporter() = default;
    PluginDiagnosticsExporter(const PluginDiagnosticsExporter&) = delete;
    PluginDiagnosticsExporter& operator=(const PluginDiagnosticsExporter&) = delete;

    std::string GenerateJSON() const {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"generated\": \"" << m_lastReport.generatedAt << "\",\n";
        oss << "  \"hostVersion\": \"" << m_lastReport.hostVersion << "\",\n";
        oss << "  \"totalPlugins\": " << m_lastReport.totalPlugins << ",\n";
        oss << "  \"plugins\": [\n";
        bool first = true;
        for (const auto& e : m_lastReport.entries) {
            if (!first) oss << ",\n";
            first = false;
            oss << "    {\"id\": \"" << e.pluginId << "\", \"name\": \"" << e.pluginName
                << "\", \"status\": \"" << e.status << "\", \"loadTimeMs\": "
                << e.loadTimeMs << ", \"errors\": " << e.errorCount << "}";
        }
        oss << "\n  ]\n}\n";
        return oss.str();
    }

    std::string GenerateHTML() const {
        std::ostringstream oss;
        oss << "<!DOCTYPE html><html><head><title>Plugin Diagnostics</title></head><body>\n";
        oss << "<h1>Plugin Diagnostics Report</h1>\n";
        oss << "<p>Generated: " << m_lastReport.generatedAt << "</p>\n";
        oss << "<p>Host: " << m_lastReport.hostVersion << "</p>\n";
        oss << "<table border='1'><tr><th>ID</th><th>Name</th><th>Status</th>"
            << "<th>Load Time</th><th>Errors</th></tr>\n";
        for (const auto& e : m_lastReport.entries) {
            oss << "<tr><td>" << e.pluginId << "</td><td>" << e.pluginName
                << "</td><td>" << e.status << "</td><td>" << e.loadTimeMs
                << "ms</td><td>" << e.errorCount << "</td></tr>\n";
        }
        oss << "</table></body></html>\n";
        return oss.str();
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, PluginDiagEntry> m_entries;
    PluginDiagReport m_lastReport;
};

}
} // namespace ExplorerLens::Engine
