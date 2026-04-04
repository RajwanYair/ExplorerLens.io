// DiagnosticCollector.h — Aggregates diagnostic information for troubleshooting
// Copyright (c) 2026 ExplorerLens Project
//
// Collects last N errors (ring buffer), decoder failure rates, average decode
// times, memory pressure events, GPU fallback events. Thread-safe with mutex.
// Provides DiagnosticSnapshot for point-in-time capture and ToJson() export.
//
#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include "StructuredErrorDomain.h"

namespace ExplorerLens {
namespace Engine {

/// Status for a diagnostic subsystem check
enum class DiagnosticStatus : uint8_t {
    Pass = 0,
    Warn = 1,
    Fail = 2
};

/// Human-readable name for DiagnosticStatus
inline const char* DiagnosticStatusName(DiagnosticStatus s) noexcept
{
    switch (s) {
        case DiagnosticStatus::Pass:
            return "Pass";
        case DiagnosticStatus::Warn:
            return "Warn";
        case DiagnosticStatus::Fail:
            return "Fail";
        default:
            return "Unknown";
    }
}

/// Diagnostic entry recorded in the ring buffer
struct DiagnosticEntry
{
    uint64_t timestampMs = 0;
    StructuredErrorDomain domain = StructuredErrorDomain::Decode;
    ErrorSeverity severity = ErrorSeverity::Error;
    std::string message;
    HRESULT code = S_OK;
};

/// Per-decoder performance statistics
struct DecoderDiagnostics
{
    std::string decoderName;
    uint64_t totalDecodes = 0;
    uint64_t totalFailures = 0;
    double totalTimeMs = 0.0;

    /// Failure rate as a fraction [0.0, 1.0]
    double FailureRate() const noexcept
    {
        return (totalDecodes > 0) ? static_cast<double>(totalFailures) / static_cast<double>(totalDecodes) : 0.0;
    }

    /// Average decode time in milliseconds
    double AverageTimeMs() const noexcept
    {
        return (totalDecodes > 0) ? totalTimeMs / static_cast<double>(totalDecodes) : 0.0;
    }
};

/// Per-subsystem diagnostic summary
struct SubsystemDiagnostic
{
    std::string name;
    DiagnosticStatus status = DiagnosticStatus::Pass;
    uint64_t errorCount = 0;
    uint64_t warningCount = 0;
    std::string details;
};

/// Complete diagnostic summary across all subsystems
struct DiagnosticSummary
{
    std::vector<SubsystemDiagnostic> subsystems;
    DiagnosticStatus overallStatus = DiagnosticStatus::Pass;

    /// Compute overall status from subsystem statuses
    void ComputeOverall() noexcept
    {
        overallStatus = DiagnosticStatus::Pass;
        for (const auto& sub : subsystems) {
            if (sub.status == DiagnosticStatus::Fail) {
                overallStatus = DiagnosticStatus::Fail;
                return;
            }
            if (sub.status == DiagnosticStatus::Warn) {
                overallStatus = DiagnosticStatus::Warn;
            }
        }
    }
};

/// Point-in-time snapshot of diagnostic data
struct DiagnosticSnapshot
{
    uint64_t captureTimestampMs = 0;
    std::vector<DiagnosticEntry> recentErrors;
    std::vector<DecoderDiagnostics> decoderStats;
    uint64_t memoryPressureEvents = 0;
    uint64_t gpuFallbackEvents = 0;
    DiagnosticSummary summary;

    /// Export snapshot to JSON string
    std::string ToJson() const
    {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"captureTimestamp\": " << captureTimestampMs << ",\n";

        // Recent errors
        oss << "  \"recentErrors\": [\n";
        for (size_t i = 0; i < recentErrors.size(); ++i) {
            const auto& entry = recentErrors[i];
            oss << "    {\"timestamp\": " << entry.timestampMs << ", \"domain\": \""
                << StructuredErrorDomainName(entry.domain) << "\", \"severity\": \""
                << ErrorSeverityName(entry.severity) << "\", \"message\": \"" << EscapeJson(entry.message)
                << "\", \"code\": " << static_cast<int32_t>(entry.code) << "}";
            if (i + 1 < recentErrors.size())
                oss << ",";
            oss << "\n";
        }
        oss << "  ],\n";

        // Decoder stats
        oss << "  \"decoderStats\": [\n";
        for (size_t i = 0; i < decoderStats.size(); ++i) {
            const auto& ds = decoderStats[i];
            oss << "    {\"name\": \"" << EscapeJson(ds.decoderName) << "\", \"totalDecodes\": " << ds.totalDecodes
                << ", \"failures\": " << ds.totalFailures << ", \"failureRate\": " << std::fixed << std::setprecision(4)
                << ds.FailureRate() << ", \"avgTimeMs\": " << std::fixed << std::setprecision(2) << ds.AverageTimeMs()
                << "}";
            if (i + 1 < decoderStats.size())
                oss << ",";
            oss << "\n";
        }
        oss << "  ],\n";

        oss << "  \"memoryPressureEvents\": " << memoryPressureEvents << ",\n";
        oss << "  \"gpuFallbackEvents\": " << gpuFallbackEvents << ",\n";

        // Summary
        oss << "  \"overallStatus\": \"" << DiagnosticStatusName(summary.overallStatus) << "\",\n";
        oss << "  \"subsystems\": [\n";
        for (size_t i = 0; i < summary.subsystems.size(); ++i) {
            const auto& sub = summary.subsystems[i];
            oss << "    {\"name\": \"" << EscapeJson(sub.name) << "\", \"status\": \""
                << DiagnosticStatusName(sub.status) << "\", \"errors\": " << sub.errorCount
                << ", \"warnings\": " << sub.warningCount << "}";
            if (i + 1 < summary.subsystems.size())
                oss << ",";
            oss << "\n";
        }
        oss << "  ]\n";

        oss << "}";
        return oss.str();
    }

  private:
    /// Simple JSON string escape (no full unicode handling)
    static std::string EscapeJson(const std::string& input)
    {
        std::string output;
        output.reserve(input.size());
        for (char ch : input) {
            switch (ch) {
                case '"':
                    output += "\\\"";
                    break;
                case '\\':
                    output += "\\\\";
                    break;
                case '\n':
                    output += "\\n";
                    break;
                case '\r':
                    output += "\\r";
                    break;
                case '\t':
                    output += "\\t";
                    break;
                default:
                    output += ch;
                    break;
            }
        }
        return output;
    }
};

/// Thread-safe diagnostic collector with ring buffer for recent errors.
/// Records decoder performance, memory pressure events, and GPU fallbacks.
class DiagnosticCollector
{
  public:
    /// @param ringBufferCapacity  Maximum number of recent errors to keep
    explicit DiagnosticCollector(size_t ringBufferCapacity = 256) : m_ringCapacity(ringBufferCapacity)
    {
        m_ringBuffer.resize(ringBufferCapacity);
    }

    /// Record an error in the ring buffer
    void RecordError(StructuredErrorDomain domain, ErrorSeverity severity, const std::string& message,
                     HRESULT code = E_FAIL)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        DiagnosticEntry entry;
        entry.timestampMs = GetCurrentTimestampMs();
        entry.domain = domain;
        entry.severity = severity;
        entry.message = message;
        entry.code = code;

        m_ringBuffer[m_ringHead] = std::move(entry);
        m_ringHead = (m_ringHead + 1) % m_ringCapacity;
        if (m_ringCount < m_ringCapacity)
            ++m_ringCount;
    }

    /// Record a decoder operation (success or failure)
    void RecordDecoderOperation(const std::string& decoderName, bool succeeded, double elapsedMs)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto* stats = FindOrCreateDecoderStats(decoderName);
        stats->totalDecodes++;
        stats->totalTimeMs += elapsedMs;
        if (!succeeded) {
            stats->totalFailures++;
        }
    }

    /// Record a memory pressure event
    void RecordMemoryPressureEvent() noexcept
    {
        m_memoryPressureEvents.fetch_add(1, std::memory_order_relaxed);
    }

    /// Record a GPU fallback event
    void RecordGPUFallbackEvent() noexcept
    {
        m_gpuFallbackEvents.fetch_add(1, std::memory_order_relaxed);
    }

    /// Get the number of errors currently in the ring buffer
    size_t GetErrorCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_ringCount;
    }

    /// Get the ring buffer capacity
    size_t GetCapacity() const noexcept
    {
        return m_ringCapacity;
    }

    /// Capture a point-in-time diagnostic snapshot
    DiagnosticSnapshot CaptureSnapshot() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        DiagnosticSnapshot snapshot;
        snapshot.captureTimestampMs = GetCurrentTimestampMs();

        // Copy ring buffer (ordered oldest-to-newest)
        snapshot.recentErrors.reserve(m_ringCount);
        if (m_ringCount > 0) {
            size_t startIdx = 0;
            if (m_ringCount == m_ringCapacity) {
                startIdx = m_ringHead;  // Oldest entry is at head when full
            }
            for (size_t i = 0; i < m_ringCount; ++i) {
                size_t idx = (startIdx + i) % m_ringCapacity;
                snapshot.recentErrors.push_back(m_ringBuffer[idx]);
            }
        }

        // Copy decoder stats
        snapshot.decoderStats = m_decoderStats;

        // Atomics
        snapshot.memoryPressureEvents = m_memoryPressureEvents.load(std::memory_order_relaxed);
        snapshot.gpuFallbackEvents = m_gpuFallbackEvents.load(std::memory_order_relaxed);

        // Build summary
        snapshot.summary = BuildSummary();

        return snapshot;
    }

    /// Build a diagnostic summary for all tracked subsystems
    DiagnosticSummary BuildSummary() const
    {
        // Must be called under lock or from CaptureSnapshot
        DiagnosticSummary summary;

        // Decoder subsystem
        SubsystemDiagnostic decoderSub;
        decoderSub.name = "Decoders";
        uint64_t totalDecoderErrors = 0;
        for (const auto& ds : m_decoderStats) {
            totalDecoderErrors += ds.totalFailures;
        }
        decoderSub.errorCount = totalDecoderErrors;
        if (totalDecoderErrors > 100)
            decoderSub.status = DiagnosticStatus::Fail;
        else if (totalDecoderErrors > 10)
            decoderSub.status = DiagnosticStatus::Warn;
        summary.subsystems.push_back(decoderSub);

        // Memory subsystem
        SubsystemDiagnostic memorySub;
        memorySub.name = "Memory";
        uint64_t memEvents = m_memoryPressureEvents.load(std::memory_order_relaxed);
        memorySub.warningCount = memEvents;
        if (memEvents > 50)
            memorySub.status = DiagnosticStatus::Fail;
        else if (memEvents > 5)
            memorySub.status = DiagnosticStatus::Warn;
        summary.subsystems.push_back(memorySub);

        // GPU subsystem
        SubsystemDiagnostic gpuSub;
        gpuSub.name = "GPU";
        uint64_t gpuEvents = m_gpuFallbackEvents.load(std::memory_order_relaxed);
        gpuSub.warningCount = gpuEvents;
        if (gpuEvents > 20)
            gpuSub.status = DiagnosticStatus::Fail;
        else if (gpuEvents > 3)
            gpuSub.status = DiagnosticStatus::Warn;
        summary.subsystems.push_back(gpuSub);

        summary.ComputeOverall();
        return summary;
    }

    /// Reset all collected diagnostics
    void Reset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ringHead = 0;
        m_ringCount = 0;
        m_decoderStats.clear();
        m_memoryPressureEvents.store(0, std::memory_order_relaxed);
        m_gpuFallbackEvents.store(0, std::memory_order_relaxed);
    }

  private:
    static uint64_t GetCurrentTimestampMs()
    {
        auto now = std::chrono::steady_clock::now();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    }

    DecoderDiagnostics* FindOrCreateDecoderStats(const std::string& name)
    {
        for (auto& ds : m_decoderStats) {
            if (ds.decoderName == name)
                return &ds;
        }
        m_decoderStats.push_back(DecoderDiagnostics{name});
        return &m_decoderStats.back();
    }

    mutable std::mutex m_mutex;
    size_t m_ringCapacity = 256;
    std::vector<DiagnosticEntry> m_ringBuffer;
    size_t m_ringHead = 0;
    size_t m_ringCount = 0;
    std::vector<DecoderDiagnostics> m_decoderStats;
    std::atomic<uint64_t> m_memoryPressureEvents{0};
    std::atomic<uint64_t> m_gpuFallbackEvents{0};
};

}  // namespace Engine
}  // namespace ExplorerLens
