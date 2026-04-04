// ExplorerIntegrationMonitor.h — Shell Extension Health Monitor
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors the health of Explorer shell integration: COM registration,
// handler responsiveness, crash frequency, and auto-recovery triggers.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class IntegrationStatus : uint8_t {
    Healthy,
    Degraded,
    Unresponsive,
    COMError,
    Unregistered
};

struct IntegrationHealth
{
    IntegrationStatus status = IntegrationStatus::Healthy;
    uint64_t thumbnailsServed = 0;
    uint64_t errors = 0;
    uint64_t avgResponseUs = 0;
    bool comRegistered = true;
    bool handlerLoaded = true;
    std::wstring lastError;
};

class ExplorerIntegrationMonitor
{
  public:
    static ExplorerIntegrationMonitor& Instance()
    {
        static ExplorerIntegrationMonitor s;
        return s;
    }

    IntegrationHealth CheckHealth() const
    {
        return m_health;
    }

    void RecordSuccess(uint64_t responseUs)
    {
        m_health.thumbnailsServed++;
        // Exponential moving average
        m_health.avgResponseUs = (m_health.avgResponseUs * 7 + responseUs) / 8;
        m_health.status = IntegrationStatus::Healthy;
    }

    void RecordError(const std::wstring& error)
    {
        m_health.errors++;
        m_health.lastError = error;
        if (m_health.errors > 10)
            m_health.status = IntegrationStatus::Degraded;
    }

    void SetCOMRegistered(bool registered)
    {
        m_health.comRegistered = registered;
        if (!registered)
            m_health.status = IntegrationStatus::Unregistered;
    }

    bool NeedsRecovery() const
    {
        return m_health.status == IntegrationStatus::Unresponsive || m_health.status == IntegrationStatus::COMError;
    }

    void Reset()
    {
        m_health = {};
    }

  private:
    ExplorerIntegrationMonitor() = default;
    IntegrationHealth m_health;
};

}  // namespace Engine
}  // namespace ExplorerLens
