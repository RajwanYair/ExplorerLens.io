// StructuredDiagnosticLogger.h — JSON-Structured Diagnostic Logging
// Copyright (c) 2026 ExplorerLens Project
//
// Emits structured (JSON) log entries with correlation IDs, timing,
// severity, and contextual metadata. Integrates with ETW and file
// sinks for production diagnostics without printf-style ad-hoc logging.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class LogSeverity : uint8_t {
    Trace, Debug, Info, Warning, Error, Fatal, COUNT
};

enum class LogCategory : uint8_t {
    Decoder, Pipeline, Cache, GPU, Plugin, Memory, Shell, General, COUNT
};

struct LogEntry {
    LogSeverity severity = LogSeverity::Info;
    LogCategory category = LogCategory::General;
    std::wstring message;
    std::wstring correlationId;
    uint64_t timestampUs = 0;
    double elapsedMs = 0.0;
    uint32_t threadId = 0;
};

struct LoggerConfig {
    LogSeverity minSeverity = LogSeverity::Info;
    bool enableETW = true;
    bool enableFileLog = true;
    bool enableConsole = false;
    uint32_t maxEntriesInMem = 10000;
    bool structuredJSON = true;
};

struct LoggerStats {
    uint64_t totalEntries = 0;
    uint64_t entriesByLevel[static_cast<size_t>(LogSeverity::COUNT)] = {};
    uint64_t droppedEntries = 0;
    bool overflow = false;
};

class StructuredDiagnosticLogger {
public:
    void Configure(const LoggerConfig& cfg) { m_config = cfg; }
    const LoggerConfig& GetConfig() const { return m_config; }

    bool Log(const LogEntry& entry) {
        if (static_cast<uint8_t>(entry.severity) < static_cast<uint8_t>(m_config.minSeverity))
            return false;

        if (m_entries.size() >= m_config.maxEntriesInMem) {
            m_stats.droppedEntries++;
            m_stats.overflow = true;
            return false;
        }

        m_entries.push_back(entry);
        m_stats.totalEntries++;
        m_stats.entriesByLevel[static_cast<size_t>(entry.severity)]++;
        return true;
    }

    bool LogMessage(LogSeverity sev, LogCategory cat, const std::wstring& msg) {
        LogEntry entry;
        entry.severity = sev;
        entry.category = cat;
        entry.message = msg;
        return Log(entry);
    }

    const std::vector<LogEntry>& GetEntries() const { return m_entries; }
    const LoggerStats& GetStats() const { return m_stats; }
    size_t EntryCount() const { return m_entries.size(); }

    void Clear() {
        m_entries.clear();
        m_stats = {};
    }

    static size_t SeverityCount() { return static_cast<size_t>(LogSeverity::COUNT); }
    static size_t CategoryCount() { return static_cast<size_t>(LogCategory::COUNT); }

private:
    LoggerConfig m_config;
    std::vector<LogEntry> m_entries;
    LoggerStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
