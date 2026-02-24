#pragma once
#include <string>
#include <vector>
#include <future>

namespace ExplorerLens::Manager::Services {

    struct LogEntry {
        std::wstring Timestamp;
        std::wstring Level; // INFO, WARN, ERROR
        std::wstring Component;
        std::wstring Message;
        uint64_t CorrelationId;
    };

    class IDiagnosticsService {
    public:
        virtual ~IDiagnosticsService() = default;

        // Retrieve recent logs
        virtual std::future<std::vector<LogEntry>> GetRecentLogsAsync(int limit) = 0;

        // Create the ZIP bundle (Process dump, logs, config)
        // Returns the path to the created zip file.
        virtual std::future<std::wstring> ExportDiagnosticsBundleAsync(std::wstring targetFolder) = 0;
        
        // Check if tracing is enabled
        virtual bool IsETWTracingEnabled() = 0;
        virtual void ToggleETWTracing(bool enable) = 0;
    };
}

