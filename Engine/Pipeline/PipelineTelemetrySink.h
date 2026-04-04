// PipelineTelemetrySink.h — Structured Pipeline Telemetry
// Copyright (c) 2026 ExplorerLens Project
//
// Collects structured telemetry from all pipeline stages for performance
// monitoring, bottleneck detection, and operational diagnostics.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SinkTelemetryLevel : uint8_t {
    None,
    Minimal,
    Standard,
    Detailed,
    Verbose
};

struct SinkTelemetryEvent
{
    std::string stageName;
    std::string eventType;
    double durationMs = 0.0;
    uint64_t timestamp = 0;
    uint32_t threadId = 0;
    bool success = true;
    std::string metadata;
};

struct SinkTelemetrySummary
{
    uint64_t totalEvents = 0;
    uint64_t successCount = 0;
    uint64_t failureCount = 0;
    double avgDurationMs = 0.0;
    double maxDurationMs = 0.0;
    double p95DurationMs = 0.0;
};

class PipelineTelemetrySink
{
  public:
    explicit PipelineTelemetrySink(SinkTelemetryLevel level = SinkTelemetryLevel::Standard) : m_level(level) {}

    void RecordEvent(const SinkTelemetryEvent& event)
    {
        if (m_level == SinkTelemetryLevel::None)
            return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.push_back(event);
        m_totalDurationMs += event.durationMs;
        if (event.success)
            m_successCount++;
        else
            m_failureCount++;
        if (event.durationMs > m_maxDurationMs)
            m_maxDurationMs = event.durationMs;
    }

    SinkTelemetrySummary GetSummary() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        SinkTelemetrySummary summary;
        summary.totalEvents = m_events.size();
        summary.successCount = m_successCount;
        summary.failureCount = m_failureCount;
        summary.maxDurationMs = m_maxDurationMs;
        summary.avgDurationMs = m_events.empty() ? 0.0 : m_totalDurationMs / m_events.size();
        return summary;
    }

    std::vector<SinkTelemetryEvent> GetEvents(size_t maxCount = 100) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_events.size() <= maxCount)
            return m_events;
        return std::vector<SinkTelemetryEvent>(m_events.end() - static_cast<ptrdiff_t>(maxCount), m_events.end());
    }

    void SetLevel(SinkTelemetryLevel level)
    {
        m_level = level;
    }
    SinkTelemetryLevel GetLevel() const
    {
        return m_level;
    }

    void Flush()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.clear();
        m_totalDurationMs = 0.0;
        m_maxDurationMs = 0.0;
        m_successCount = 0;
        m_failureCount = 0;
    }

  private:
    mutable std::mutex m_mutex;
    std::vector<SinkTelemetryEvent> m_events;
    SinkTelemetryLevel m_level;
    double m_totalDurationMs = 0.0;
    double m_maxDurationMs = 0.0;
    uint64_t m_successCount = 0;
    uint64_t m_failureCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
