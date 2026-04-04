// DiagnosticTelemetryCollector.h — Structured Diagnostic Telemetry Collection
// Copyright (c) 2026 ExplorerLens Project
//
// Collects structured telemetry records for decode operations, cache hits, GPU
// utilization, and error events. Stores a bounded ring buffer of records for
// diagnostic export. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DiagnosticCategory : uint8_t {
    Decode,
    Cache,
    GPU,
    Memory,
    Pipeline,
    Error,
    Performance
};

enum class DiagnosticSeverity : uint8_t {
    Trace,
    Info,
    Warning,
    Error,
    Critical
};

struct DiagnosticTelemetryRecord
{
    uint64_t sequenceId = 0;
    DiagnosticCategory category = DiagnosticCategory::Decode;
    DiagnosticSeverity severity = DiagnosticSeverity::Info;
    std::wstring source;
    std::wstring message;
    float valueMetric = 0.0f;
    uint64_t timestampMs = 0;
};

struct TelemetryCollectorStats
{
    uint64_t totalRecorded = 0;
    uint64_t totalDropped = 0;
    uint64_t errorCount = 0;
    uint64_t criticalCount = 0;
    bool initialized = false;
};

class DiagnosticTelemetryCollector
{
  public:
    static DiagnosticTelemetryCollector& Instance()
    {
        static DiagnosticTelemetryCollector instance;
        return instance;
    }

    void Initialize(uint32_t maxRecords = 10000)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxRecords = maxRecords;
        m_records.clear();
        m_records.reserve((std::min)(maxRecords, 1024u));
        m_nextSequenceId = 1;
        m_stats = {};
        m_stats.initialized = true;
    }

    void Record(DiagnosticCategory category, DiagnosticSeverity severity, const std::wstring& source,
                const std::wstring& message, float value = 0.0f)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalRecorded++;

        if (severity == DiagnosticSeverity::Error)
            m_stats.errorCount++;
        if (severity == DiagnosticSeverity::Critical)
            m_stats.criticalCount++;

        if (m_records.size() >= m_maxRecords) {
            m_records.erase(m_records.begin());
            m_stats.totalDropped++;
        }

        DiagnosticTelemetryRecord rec;
        rec.sequenceId = m_nextSequenceId++;
        rec.category = category;
        rec.severity = severity;
        rec.source = source;
        rec.message = message;
        rec.valueMetric = value;
        m_records.push_back(std::move(rec));
    }

    std::vector<DiagnosticTelemetryRecord> GetRecords(DiagnosticCategory category) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<DiagnosticTelemetryRecord> filtered;
        for (const auto& r : m_records) {
            if (r.category == category)
                filtered.push_back(r);
        }
        return filtered;
    }

    std::vector<DiagnosticTelemetryRecord> GetRecentErrors(uint32_t maxCount = 100) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<DiagnosticTelemetryRecord> errors;
        for (auto it = m_records.rbegin(); it != m_records.rend() && errors.size() < maxCount; ++it) {
            if (it->severity >= DiagnosticSeverity::Error) {
                errors.push_back(*it);
            }
        }
        return errors;
    }

    uint32_t GetRecordCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_records.size());
    }

    bool IsInitialized() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    TelemetryCollectorStats GetStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_records.clear();
    }

  private:
    DiagnosticTelemetryCollector() = default;
    ~DiagnosticTelemetryCollector() = default;
    DiagnosticTelemetryCollector(const DiagnosticTelemetryCollector&) = delete;
    DiagnosticTelemetryCollector& operator=(const DiagnosticTelemetryCollector&) = delete;

    mutable std::mutex m_mutex;
    uint32_t m_maxRecords = 10000;
    uint64_t m_nextSequenceId = 1;
    std::vector<DiagnosticTelemetryRecord> m_records;
    TelemetryCollectorStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
