// PowerAwareDecodeScheduler.h — Battery-Aware Decode Scheduling
// Copyright (c) 2026 ExplorerLens Project
//
// Adjusts thumbnail decode scheduling based on system power state,
// reducing quality/throughput on battery to preserve battery life.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class PowerState : uint8_t {
    ACPower,
    BatteryHigh,
    BatteryMedium,
    BatteryLow,
    BatteryCritical,
    Unknown
};

enum class DecodeQualityLevel : uint8_t {
    Maximum,
    High,
    Medium,
    Low,
    Minimal
};

struct PowerDecodePolicy {
    PowerState powerState = PowerState::ACPower;
    DecodeQualityLevel maxQuality = DecodeQualityLevel::Maximum;
    uint32_t maxConcurrentDecodes = 8;
    bool enableGPUDecode = true;
    bool enablePrefetch = true;
    float throttleFactor = 1.0f;
};

struct PowerStatus {
    PowerState state = PowerState::Unknown;
    uint8_t batteryPercent = 100;
    bool isCharging = false;
    uint32_t estimatedMinutesRemaining = 0;
};

class PowerAwareDecodeScheduler {
public:
    PowerAwareDecodeScheduler() { SetupDefaultPolicies(); }

    PowerDecodePolicy GetCurrentPolicy() const {
        return GetPolicyForState(m_currentStatus.state);
    }

    void UpdatePowerStatus(const PowerStatus& status) {
        m_currentStatus = status;
    }

    PowerStatus GetPowerStatus() const { return m_currentStatus; }

    bool ShouldThrottle() const {
        return m_currentStatus.state >= PowerState::BatteryMedium;
    }

    bool IsGPUDecodeAllowed() const {
        return GetCurrentPolicy().enableGPUDecode;
    }

    uint32_t GetMaxConcurrency() const {
        return GetCurrentPolicy().maxConcurrentDecodes;
    }

    void SetCustomPolicy(PowerState state, const PowerDecodePolicy& policy) {
        for (auto& p : m_policies) {
            if (p.powerState == state) { p = policy; return; }
        }
        m_policies.push_back(policy);
    }

private:
    void SetupDefaultPolicies() {
        m_policies = {
            {PowerState::ACPower, DecodeQualityLevel::Maximum, 8, true, true, 1.0f},
            {PowerState::BatteryHigh, DecodeQualityLevel::High, 6, true, true, 0.9f},
            {PowerState::BatteryMedium, DecodeQualityLevel::Medium, 4, true, false, 0.7f},
            {PowerState::BatteryLow, DecodeQualityLevel::Low, 2, false, false, 0.4f},
            {PowerState::BatteryCritical, DecodeQualityLevel::Minimal, 1, false, false, 0.2f}
        };
    }

    PowerDecodePolicy GetPolicyForState(PowerState state) const {
        for (const auto& p : m_policies) {
            if (p.powerState == state) return p;
        }
        return m_policies.empty() ? PowerDecodePolicy{} : m_policies[0];
    }

    std::vector<PowerDecodePolicy> m_policies;
    PowerStatus m_currentStatus;
};

} // namespace Engine
} // namespace ExplorerLens
