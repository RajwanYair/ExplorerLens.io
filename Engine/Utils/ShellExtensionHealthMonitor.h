// ShellExtensionHealthMonitor.h — Shell Extension Health Monitor V2
// Copyright (c) 2026 ExplorerLens Project
//
// Higher-level health monitoring service for LENSShell.dll runtime metrics:
// thumbnail SLA tracking, error rate aggregation, and P99 latency alerts.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ShellHealthStatus {
    Healthy,
    Degraded,
    Unavailable,
    Crashed
};

inline const char* ShellHealthStatusName(ShellHealthStatus s) noexcept
{
    switch (s) {
        case ShellHealthStatus::Healthy:
            return "Healthy";
        case ShellHealthStatus::Degraded:
            return "Degraded";
        case ShellHealthStatus::Unavailable:
            return "Unavailable";
        case ShellHealthStatus::Crashed:
            return "Crashed";
        default:
            return "Unknown";
    }
}

enum class RecoveryAction : uint8_t {
    Restart = 0,
    Reload = 1,
    Escalate = 2,
    Ignore = 3
};

inline const char* RecoveryActionName(RecoveryAction a) noexcept
{
    switch (a) {
        case RecoveryAction::Restart:
            return "Restart";
        case RecoveryAction::Reload:
            return "Reload";
        case RecoveryAction::Escalate:
            return "Escalate";
        case RecoveryAction::Ignore:
            return "Ignore";
        default:
            return "Unknown";
    }
}

struct ShellHealthCheckResult
{
    ShellHealthStatus status = ShellHealthStatus::Healthy;
};

struct ShellHealthSnapshot
{
    ShellHealthStatus status = ShellHealthStatus::Healthy;
    float avgLatencyMs = 0.0f;
    float p99LatencyMs = 0.0f;
    uint32_t errorRate = 0;  // per-mille
    uint32_t uptime = 0;     // seconds
    uint32_t restartCount = 0;
    bool comRegistered = false;
    std::string version;
};

using HealthAlertCallback = std::function<void(const ShellHealthSnapshot&)>;

class ShellExtHealthMonitorV2
{
  public:
    ShellExtHealthMonitorV2() = default;

    bool Initialize(const std::string& dllVersion = "")
    {
        m_version = dllVersion;
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }

    void SetAlertCallback(HealthAlertCallback cb)
    {
        m_alertCb = std::move(cb);
    }

    void RecordSuccess(float latencyMs)
    {
        ++m_totalOps;
        m_latencySamples.push_back(latencyMs);
        if (m_latencySamples.size() > 100)
            m_latencySamples.erase(m_latencySamples.begin());
        m_sumLatency += latencyMs;
    }

    void RecordError()
    {
        ++m_totalOps;
        ++m_errorCount;
        if (m_errorCount > 3)
            TriggerAlert();
    }

    ShellHealthSnapshot GetSnapshot() const
    {
        ShellHealthSnapshot snap;
        snap.version = m_version;
        snap.comRegistered = true;
        snap.uptime = m_uptimeSec;
        snap.restartCount = m_restartCount;

        if (m_totalOps > 0) {
            snap.avgLatencyMs = static_cast<float>(m_sumLatency / m_totalOps);
            uint32_t errPerMille = static_cast<uint32_t>(m_errorCount * 1000 / m_totalOps);
            snap.errorRate = errPerMille;
        }

        if (m_latencySamples.size() >= 2) {
            auto sorted = m_latencySamples;
            for (size_t i = 1; i < sorted.size(); ++i) {
                float v = sorted[i];
                size_t j = i;
                while (j > 0 && sorted[j - 1] > v) {
                    sorted[j] = sorted[j - 1];
                    --j;
                }
                sorted[j] = v;
            }
            snap.p99LatencyMs = sorted[static_cast<size_t>(sorted.size() * 99 / 100)];
        }

        if (snap.p99LatencyMs > 500.0f)
            snap.status = ShellHealthStatus::Degraded;
        else if (snap.errorRate > 50)
            snap.status = ShellHealthStatus::Degraded;
        else
            snap.status = ShellHealthStatus::Healthy;
        return snap;
    }

    void SetUptimeSec(uint32_t s)
    {
        m_uptimeSec = s;
    }

    void Shutdown()
    {
        m_ready = false;
    }

  private:
    bool m_ready = false;
    std::string m_version;
    uint64_t m_totalOps = 0;
    uint64_t m_errorCount = 0;
    double m_sumLatency = 0.0;
    uint32_t m_uptimeSec = 0;
    uint32_t m_restartCount = 0;
    std::vector<float> m_latencySamples;
    HealthAlertCallback m_alertCb;

    void TriggerAlert()
    {
        if (m_alertCb)
            m_alertCb(GetSnapshot());
        m_errorCount = 0;
    }
};

class ShellExtensionHealthMonitor
{
  public:
    ShellHealthCheckResult CheckHealth()
    {
        ++m_checkCount;
        ShellHealthCheckResult result;
        result.status = m_status;
        return result;
    }

    void SimulateFailure(ShellHealthStatus status) noexcept
    {
        m_status = status;
    }

    bool AutoRecover()
    {
        m_status = ShellHealthStatus::Healthy;
        return true;
    }

    ShellHealthStatus GetStatus() const noexcept
    {
        return m_status;
    }
    uint32_t GetCheckCount() const noexcept
    {
        return m_checkCount;
    }

  private:
    ShellHealthStatus m_status = ShellHealthStatus::Healthy;
    uint32_t m_checkCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
