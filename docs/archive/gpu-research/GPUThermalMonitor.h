// GPUThermalMonitor.h — GPU Temperature Monitoring and Throttling
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors GPU temperature and throttles decode workloads when thermal
// limits are approached to prevent system instability and protect hardware.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ThermalZone : uint8_t {
    Cool,
    Warm,
    Hot,
    Critical,
    Emergency
};

struct ThermalReading
{
    float temperatureC = 0.0f;
    ThermalZone zone = ThermalZone::Cool;
    uint64_t timestamp = 0;
    float fanSpeedPercent = 0.0f;
    float powerDrawWatts = 0.0f;
};

struct ThermalPolicy
{
    float warmThresholdC = 70.0f;
    float hotThresholdC = 80.0f;
    float criticalThresholdC = 90.0f;
    float emergencyThresholdC = 95.0f;
    float throttleStepPercent = 25.0f;
};

struct ThermalMetrics
{
    float currentTempC = 0.0f;
    float peakTempC = 0.0f;
    float avgTempC = 0.0f;
    uint64_t throttleEvents = 0;
    uint64_t readingsCount = 0;
    float currentThrottlePercent = 0.0f;
};

class GPUThermalMonitor
{
  public:
    GPUThermalMonitor() = default;

    ThermalZone RecordReading(const ThermalReading& reading)
    {
        m_lastReading = reading;
        m_readings.push_back(reading);
        m_metrics.readingsCount++;
        m_metrics.currentTempC = reading.temperatureC;
        if (reading.temperatureC > m_metrics.peakTempC)
            m_metrics.peakTempC = reading.temperatureC;
        m_totalTemp += reading.temperatureC;
        m_metrics.avgTempC = static_cast<float>(m_totalTemp / m_metrics.readingsCount);

        ThermalZone zone = ClassifyTemperature(reading.temperatureC);
        if (zone >= ThermalZone::Hot) {
            m_metrics.throttleEvents++;
            m_metrics.currentThrottlePercent = GetThrottlePercent(zone);
        } else {
            m_metrics.currentThrottlePercent = 0.0f;
        }
        return zone;
    }

    ThermalReading GetLastReading() const
    {
        return m_lastReading;
    }
    ThermalMetrics GetMetrics() const
    {
        return m_metrics;
    }
    bool IsThrottling() const
    {
        return m_metrics.currentThrottlePercent > 0.0f;
    }
    float GetThrottlePercent() const
    {
        return m_metrics.currentThrottlePercent;
    }

    void SetPolicy(const ThermalPolicy& policy)
    {
        m_policy = policy;
    }
    ThermalPolicy GetPolicy() const
    {
        return m_policy;
    }

  private:
    ThermalZone ClassifyTemperature(float tempC) const
    {
        if (tempC >= m_policy.emergencyThresholdC)
            return ThermalZone::Emergency;
        if (tempC >= m_policy.criticalThresholdC)
            return ThermalZone::Critical;
        if (tempC >= m_policy.hotThresholdC)
            return ThermalZone::Hot;
        if (tempC >= m_policy.warmThresholdC)
            return ThermalZone::Warm;
        return ThermalZone::Cool;
    }

    float GetThrottlePercent(ThermalZone zone) const
    {
        switch (zone) {
            case ThermalZone::Hot:
                return m_policy.throttleStepPercent;
            case ThermalZone::Critical:
                return m_policy.throttleStepPercent * 2;
            case ThermalZone::Emergency:
                return 100.0f;
            default:
                return 0.0f;
        }
    }

    ThermalReading m_lastReading;
    std::vector<ThermalReading> m_readings;
    ThermalPolicy m_policy;
    ThermalMetrics m_metrics;
    double m_totalTemp = 0.0;
};

}  // namespace Engine
}  // namespace ExplorerLens
