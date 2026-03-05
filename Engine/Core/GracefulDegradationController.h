// GracefulDegradationController.h — Graceful Feature Fallback Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Manages graceful degradation when subsystems fail or resources are
// constrained. Disables non-essential features progressively while
// maintaining core thumbnail functionality with clear status reporting.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class FeatureTier : uint8_t {
    Essential, Important, Enhancement, Optional, Experimental, COUNT
};

enum class DegradationLevel : uint8_t {
    FullFeature, ReducedQuality, MinimalFeatures, CoreOnly, EmergencyMode, COUNT
};

struct FeatureState {
    std::wstring featureName;
    FeatureTier tier = FeatureTier::Essential;
    bool enabled = true;
    bool available = true;
    std::wstring disableReason;
};

struct DegradationConfig {
    DegradationLevel maxLevel = DegradationLevel::CoreOnly;
    bool autoDegrade = true;
    bool notifyUser = true;
    uint32_t recoveryCheckMs = 30000;
};

struct DegradationStatus {
    DegradationLevel current = DegradationLevel::FullFeature;
    uint32_t enabledFeatures = 0;
    uint32_t disabledFeatures = 0;
    uint32_t degradationEvents = 0;
    bool recovering = false;
};

class GracefulDegradationController {
public:
    void Configure(const DegradationConfig& cfg) { m_config = cfg; }
    const DegradationConfig& GetConfig() const { return m_config; }

    void SetLevel(DegradationLevel level) {
        if (level != m_status.current) {
            m_status.current = level;
            m_status.degradationEvents++;
        }
    }

    DegradationLevel GetLevel() const { return m_status.current; }

    bool IsFeatureAllowed(FeatureTier tier) const {
        switch (m_status.current) {
        case DegradationLevel::FullFeature: return true;
        case DegradationLevel::ReducedQuality:
            return tier <= FeatureTier::Enhancement;
        case DegradationLevel::MinimalFeatures:
            return tier <= FeatureTier::Important;
        case DegradationLevel::CoreOnly:
            return tier == FeatureTier::Essential;
        case DegradationLevel::EmergencyMode:
            return tier == FeatureTier::Essential;
        default: return false;
        }
    }

    void NotifyFailure(const std::wstring& subsystem) {
        m_failureCount++;
        if (m_config.autoDegrade && m_failureCount >= 3) {
            auto next = static_cast<uint8_t>(m_status.current);
            if (next < static_cast<uint8_t>(m_config.maxLevel)) {
                SetLevel(static_cast<DegradationLevel>(next + 1));
            }
        }
    }

    void NotifyRecovery() {
        if (m_status.current != DegradationLevel::FullFeature) {
            auto prev = static_cast<uint8_t>(m_status.current);
            if (prev > 0) {
                SetLevel(static_cast<DegradationLevel>(prev - 1));
                m_status.recovering = true;
            }
        }
        m_failureCount = 0;
    }

    const DegradationStatus& GetStatus() const { return m_status; }
    uint32_t FailureCount() const { return m_failureCount; }

    void Reset() {
        m_status = {};
        m_failureCount = 0;
    }

    static size_t TierCount() { return static_cast<size_t>(FeatureTier::COUNT); }
    static size_t LevelCount() { return static_cast<size_t>(DegradationLevel::COUNT); }

private:
    DegradationConfig m_config;
    DegradationStatus m_status;
    uint32_t m_failureCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
