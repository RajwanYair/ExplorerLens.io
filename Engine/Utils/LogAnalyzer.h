// LogAnalyzer.h — Structured Log Pattern Analyzer
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes structured log entries to detect error patterns, performance
// anomalies, and recurring issues for automated troubleshooting.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class AnalyzerLogLevel : uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5
};

struct AnalyzerLogEntry {
    uint64_t timestampMs = 0;
    AnalyzerLogLevel level = AnalyzerLogLevel::Info;
    std::string source;
    std::string message;
    uint32_t threadId = 0;
};

struct LogPattern {
    std::string patternName;
    std::string matchSubstring;
    AnalyzerLogLevel minLevel = AnalyzerLogLevel::Warning;
    uint32_t occurrences = 0;
    uint64_t firstSeenMs = 0;
    uint64_t lastSeenMs = 0;
};

struct LogAnalysis {
    uint32_t totalEntries = 0;
    uint32_t errorCount = 0;
    uint32_t warningCount = 0;
    uint32_t fatalCount = 0;
    std::vector<LogPattern> detectedPatterns;
    std::string topErrorSource;
    uint32_t topErrorCount = 0;
    double errorsPerMinute = 0.0;
};

class LogAnalyzer {
public:
    void RegisterPattern(const std::string& name, const std::string& match,
        AnalyzerLogLevel minLevel = AnalyzerLogLevel::Warning) {
        LogPattern p;
        p.patternName = name;
        p.matchSubstring = match;
        p.minLevel = minLevel;
        m_patterns.push_back(p);
    }

    void Ingest(const AnalyzerLogEntry& entry) {
        m_analysis.totalEntries++;
        if (entry.level == AnalyzerLogLevel::Error) m_analysis.errorCount++;
        if (entry.level == AnalyzerLogLevel::Warning) m_analysis.warningCount++;
        if (entry.level == AnalyzerLogLevel::Fatal) m_analysis.fatalCount++;

        if (entry.level >= AnalyzerLogLevel::Error) {
            m_sourceCounts[entry.source]++;
        }

        for (auto& p : m_patterns) {
            if (entry.level >= p.minLevel &&
                entry.message.find(p.matchSubstring) != std::string::npos) {
                p.occurrences++;
                if (p.firstSeenMs == 0) p.firstSeenMs = entry.timestampMs;
                p.lastSeenMs = entry.timestampMs;
            }
        }
    }

    LogAnalysis GetAnalysis() const {
        auto result = m_analysis;
        result.detectedPatterns = m_patterns;

        // Find top error source
        uint32_t maxCount = 0;
        for (const auto& [source, count] : m_sourceCounts) {
            if (count > maxCount) {
                maxCount = count;
                result.topErrorSource = source;
                result.topErrorCount = count;
            }
        }
        return result;
    }

    void Reset() {
        m_analysis = {};
        m_sourceCounts.clear();
        for (auto& p : m_patterns) {
            p.occurrences = 0;
            p.firstSeenMs = 0;
            p.lastSeenMs = 0;
        }
    }

private:
    LogAnalysis m_analysis;
    std::vector<LogPattern> m_patterns;
    std::unordered_map<std::string, uint32_t> m_sourceCounts;
};

} // namespace Engine
} // namespace ExplorerLens
