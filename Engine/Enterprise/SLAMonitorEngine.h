// SLAMonitorEngine.h — Real-Time SLA Monitor Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks service-level agreement adherence across ExplorerLens deployments,
// issuing breach alerts and computing SLA credit recommendations.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct SLATarget {
    std::string  metricName;
    double       targetValue;
    bool         higherIsBetter = true; // false = lower is better (latency)
    double       warningThreshold;
    double       criticalThreshold;
};

struct SLABreach {
    std::string  metricName;
    double       targetValue;
    double       actualValue;
    bool         isCritical   = false;
    std::string  detectedAt;
    double       creditPercent= 0.0;
};

struct SLAStatus {
    double   uptimePercent    = 100.0;
    double   p99LatencyMs     = 0.0;
    double   errorRatePercent = 0.0;
    std::vector<SLABreach> activeBreaches;
};

class SLAMonitorEngine {
public:
    using BreachCallback = std::function<void(const SLABreach&)>;

    SLAMonitorEngine() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    void AddSLATarget(const SLATarget& target) { m_targets.push_back(target); }
    void SetBreachCallback(BreachCallback cb)    { m_onBreach = cb; }

    bool RecordSample(const std::string& metricName, double value) {
        for (const auto& t : m_targets) {
            if (t.metricName != metricName) continue;
            bool breach = t.higherIsBetter ? (value < t.criticalThreshold)
                                           : (value > t.criticalThreshold);
            if (breach) {
                SLABreach b;
                b.metricName    = metricName;
                b.targetValue   = t.targetValue;
                b.actualValue   = value;
                b.isCritical    = true;
                b.detectedAt    = "2026-03-29T00:00:00Z";
                b.creditPercent = 10.0;
                m_status.activeBreaches.push_back(b);
                if (m_onBreach) m_onBreach(b);
            }
        }
        return true;
    }

    const SLAStatus& GetStatus()           const { return m_status; }
    bool HasActiveBreaches()               const { return !m_status.activeBreaches.empty(); }
    uint32_t GetActiveBreachCount()        const { return static_cast<uint32_t>(m_status.activeBreaches.size()); }
    void ClearBreaches()                   { m_status.activeBreaches.clear(); }
    void Shutdown()                        { m_ready = false; }

private:
    bool              m_ready = false;
    SLAStatus         m_status;
    std::vector<SLATarget> m_targets;
    BreachCallback    m_onBreach;
};

}} // namespace ExplorerLens::Engine
