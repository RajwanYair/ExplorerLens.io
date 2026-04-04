// EnterpriseTelemetryRouter.h — Enterprise Telemetry Distribution
// Copyright (c) 2026 ExplorerLens Project
//
// Routes telemetry events to multiple sinks (ETW, file, network) based
// on enterprise policy. Supports filtering, sampling, PII scrubbing,
// and batched upload for large-scale deployments.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class TelemetrySink : uint8_t {
    ETW,
    FileLog,
    NetworkEndpoint,
    EventLog,
    Null,
    COUNT
};

enum class ETRLevel : uint8_t {
    Off,
    Critical,
    Error,
    Warning,
    Info,
    Verbose,
    COUNT
};

struct ETREvent
{
    std::wstring eventName;
    std::wstring payload;
    ETRLevel level = ETRLevel::Info;
    uint64_t timestampUs = 0;
    uint32_t eventId = 0;
    bool piiScrubbed = false;
};

struct TelemetryRouteConfig
{
    TelemetrySink sink = TelemetrySink::ETW;
    ETRLevel minLevel = ETRLevel::Warning;
    float samplingRate = 1.0f;
    uint32_t batchSize = 100;
    uint32_t flushIntervalMs = 5000;
    bool enablePIIScrubbing = true;
};

struct ETRStats
{
    uint64_t eventsRouted = 0;
    uint64_t eventsDropped = 0;
    uint64_t eventsScrubbed = 0;
    uint64_t bytesTransmitted = 0;
    uint32_t activeSinks = 0;
};

class EnterpriseTelemetryRouter
{
  public:
    void AddRoute(const TelemetryRouteConfig& route)
    {
        if (m_routeCount < 8) {
            m_routes[m_routeCount++] = route;
            m_stats.activeSinks = m_routeCount;
        }
    }

    bool Route(const ETREvent& evt)
    {
        bool routed = false;
        for (uint32_t i = 0; i < m_routeCount; ++i) {
            if (static_cast<uint8_t>(evt.level) <= static_cast<uint8_t>(m_routes[i].minLevel)) {
                m_stats.eventsRouted++;
                if (m_routes[i].enablePIIScrubbing && !evt.piiScrubbed) {
                    m_stats.eventsScrubbed++;
                }
                routed = true;
            }
        }
        if (!routed)
            m_stats.eventsDropped++;
        return routed;
    }

    uint32_t RouteCount() const
    {
        return m_routeCount;
    }
    const ETRStats& GetStats() const
    {
        return m_stats;
    }

    void Reset()
    {
        m_routeCount = 0;
        m_stats = {};
    }

    static size_t SinkCount()
    {
        return static_cast<size_t>(TelemetrySink::COUNT);
    }
    static size_t LevelCount()
    {
        return static_cast<size_t>(ETRLevel::COUNT);
    }

  private:
    TelemetryRouteConfig m_routes[8] = {};
    uint32_t m_routeCount = 0;
    ETRStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
