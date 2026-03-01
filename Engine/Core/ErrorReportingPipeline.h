#pragma once
// ============================================================================
// ErrorReportingPipeline.h — Structured error collection and reporting
//
// Purpose:   Structured error collection and reporting
// Provides:  ErrorSeverity, ErrorCategory enums, ErrorReport struct,
//            ErrorReportingPipeline class
// Used by:   All engine modules for centralized error handling
// ============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

/// Domain from which an error originated
enum class ErrorDomain : uint8_t {
    Decoder = 0,   // Image/archive decoder failure
    Cache = 1,   // Cache read/write/eviction error
    GPU = 2,   // GPU pipeline or shader error
    Pipeline = 3,   // General pipeline orchestration error
    COM = 4    // COM interface or registration error
};

inline const char* ErrorDomainName(ErrorDomain d) noexcept {
    switch (d) {
    case ErrorDomain::Decoder:  return "Decoder";
    case ErrorDomain::Cache:    return "Cache";
    case ErrorDomain::GPU:      return "GPU";
    case ErrorDomain::Pipeline: return "Pipeline";
    case ErrorDomain::COM:      return "COM";
    default:                    return "Unknown";
    }
}

/// How errors are grouped for reporting
enum class ErrorAggregation : uint8_t {
    PerFile = 0,   // One bucket per distinct file path
    PerFormat = 1,   // Grouped by file format / extension
    PerSession = 2,   // Aggregated over the entire session
    PerHour = 3,   // Rolling one-hour windows
    Total = 4    // Global cumulative counter
};

inline const char* ErrorAggregationName(ErrorAggregation a) noexcept {
    switch (a) {
    case ErrorAggregation::PerFile:    return "PerFile";
    case ErrorAggregation::PerFormat:  return "PerFormat";
    case ErrorAggregation::PerSession: return "PerSession";
    case ErrorAggregation::PerHour:    return "PerHour";
    case ErrorAggregation::Total:      return "Total";
    default:                           return "Unknown";
    }
}

/// A single aggregated error report
struct ErrorReport {
    ErrorDomain      domain = ErrorDomain::Pipeline;
    ErrorAggregation aggregation = ErrorAggregation::Total;
    uint64_t         count = 0;   // Occurrences in this bucket
    uint64_t         firstOccurrence = 0;   // Epoch ms of first hit
    uint64_t         lastOccurrence = 0;   // Epoch ms of most recent hit
    std::string      message;               // Human-readable description
};

/// Centralized error reporting pipeline that collects, aggregates, and
/// exposes structured error data for diagnostics and telemetry.
class ErrorReportingPipeline {
public:
    ErrorReportingPipeline() = default;
    ~ErrorReportingPipeline() = default;

    ErrorReportingPipeline(const ErrorReportingPipeline&) = delete;
    ErrorReportingPipeline& operator=(const ErrorReportingPipeline&) = delete;
    ErrorReportingPipeline(ErrorReportingPipeline&&) noexcept = default;
    ErrorReportingPipeline& operator=(ErrorReportingPipeline&&) noexcept = default;

    /// Report a new error occurrence
    void Report(ErrorDomain domain, ErrorAggregation aggregation,
        const std::string& message) {
        // Try to merge into existing bucket
        for (auto& r : m_reports) {
            if (r.domain == domain && r.aggregation == aggregation &&
                r.message == message) {
                r.count++;
                r.lastOccurrence = GetCurrentTimestamp();
                return;
            }
        }
        // New bucket
        ErrorReport rpt{};
        rpt.domain = domain;
        rpt.aggregation = aggregation;
        rpt.count = 1;
        rpt.firstOccurrence = GetCurrentTimestamp();
        rpt.lastOccurrence = rpt.firstOccurrence;
        rpt.message = message;
        m_reports.push_back(rpt);
    }

    /// Get all current reports
    const std::vector<ErrorReport>& GetReports() const noexcept {
        return m_reports;
    }

    /// Return the top-N most frequent errors
    std::vector<ErrorReport> GetTopErrors(uint32_t topN) const {
        auto sorted = m_reports;
        std::sort(sorted.begin(), sorted.end(),
            [](const ErrorReport& a, const ErrorReport& b) {
                return a.count > b.count;
            });
        if (sorted.size() > topN) sorted.resize(topN);
        return sorted;
    }

    /// Clear all accumulated error reports
    void ClearReports() noexcept { m_reports.clear(); }

    /// Total number of distinct error buckets
    size_t GetBucketCount() const noexcept { return m_reports.size(); }

    /// Sum of all error counts across buckets
    uint64_t GetTotalErrorCount() const noexcept {
        uint64_t total = 0;
        for (const auto& r : m_reports) total += r.count;
        return total;
    }

private:
    static uint64_t GetCurrentTimestamp() noexcept { return 0; }

    std::vector<ErrorReport> m_reports;
};

} // namespace Engine
} // namespace ExplorerLens
