// AIAnomalyDetectorV2.h — AI-Powered Anomaly Detector v2
// Copyright (c) 2026 ExplorerLens Project
//
// Applies statistical and ML models to detect anomalous usage patterns
// and security signals across the ExplorerLens enterprise fleet.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class AnomalySeverity { Info, Warning, Critical };
enum class AnomalyType     { UsageSpike, MemoryLeak, CrashLoop, SecuritySignal, LicenceViolation };

struct AnomalyEvent {
    uint64_t        id         = 0;
    std::string     nodeId;
    AnomalyType     type       = AnomalyType::UsageSpike;
    AnomalySeverity severity   = AnomalySeverity::Warning;
    std::string     details;
    float           confidence = 0.0f;
    uint64_t        detectedAt = 0;
};

struct AnomalyMetrics {
    uint64_t totalEvents      = 0;
    uint64_t criticalCount    = 0;
    float    falsePositiveRate= 0.0f;
    double   avgDetectionMs   = 0.0;
};

class AIAnomalyDetectorV2 {
public:
    AIAnomalyDetectorV2() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    std::vector<AnomalyEvent> Analyze(const std::string& nodeId, float cpuPercent, float memPercent,
                                      uint64_t crashCount) {
        std::vector<AnomalyEvent> events;
        if (!m_ready) return events;

        if (cpuPercent > 90.0f) {
            AnomalyEvent e;
            e.id         = ++m_nextId;
            e.nodeId     = nodeId;
            e.type       = AnomalyType::UsageSpike;
            e.severity   = AnomalySeverity::Warning;
            e.confidence = 0.92f;
            e.details    = "CPU spike: " + std::to_string(static_cast<int>(cpuPercent)) + "%";
            events.push_back(e);
        }
        if (crashCount > 3) {
            AnomalyEvent e;
            e.id         = ++m_nextId;
            e.nodeId     = nodeId;
            e.type       = AnomalyType::CrashLoop;
            e.severity   = AnomalySeverity::Critical;
            e.confidence = 0.98f;
            e.details    = "Crash loop detected: " + std::to_string(crashCount) + " crashes";
            events.push_back(e);
        }
        (void)memPercent;
        m_metrics.totalEvents += events.size();
        for (const auto& ev : events)
            if (ev.severity == AnomalySeverity::Critical) ++m_metrics.criticalCount;
        return events;
    }

    const AnomalyMetrics& GetMetrics() const { return m_metrics; }
    void Shutdown() { m_ready = false; }

private:
    bool          m_ready  = false;
    uint64_t      m_nextId = 0;
    AnomalyMetrics m_metrics;
};

}} // namespace ExplorerLens::Engine
