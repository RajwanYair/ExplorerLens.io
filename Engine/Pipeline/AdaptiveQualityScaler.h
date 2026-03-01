#pragma once
// AdaptiveQualityScaler.h — Dynamic quality scaling based on system load
// Sprint 412 · Batch 6 · ExplorerLens v15.0.0

#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Enums ────────────────────────────────────────────────────────────────────

enum class QualityTier : uint8_t {
    Ultra = 0,
    High = 1,
    Medium = 2,
    Low = 3,
    Minimum = 4
};

inline const char* QualityTierName(QualityTier t) {
    switch (t) {
    case QualityTier::Ultra:   return "Ultra";
    case QualityTier::High:    return "High";
    case QualityTier::Medium:  return "Medium";
    case QualityTier::Low:     return "Low";
    case QualityTier::Minimum: return "Minimum";
    default:                   return "Unknown";
    }
}

enum class ScalingReason : uint8_t {
    CPULoad = 0,
    MemoryPressure = 1,
    GPULoad = 2,
    BatteryLow = 3,
    UserRequest = 4
};

inline const char* ScalingReasonName(ScalingReason r) {
    switch (r) {
    case ScalingReason::CPULoad:        return "CPULoad";
    case ScalingReason::MemoryPressure: return "MemoryPressure";
    case ScalingReason::GPULoad:        return "GPULoad";
    case ScalingReason::BatteryLow:     return "BatteryLow";
    case ScalingReason::UserRequest:    return "UserRequest";
    default:                            return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct QualityDecision {
    QualityTier   tier = QualityTier::High;
    ScalingReason reason = ScalingReason::UserRequest;
    float         targetMs = 17.0f;
    float         downsampleFactor = 1.0f;
};

// ── Class ────────────────────────────────────────────────────────────────────

class AdaptiveQualityScaler {
public:
    AdaptiveQualityScaler() = default;
    ~AdaptiveQualityScaler() = default;

    // Evaluate system conditions and produce a quality decision
    QualityDecision Evaluate(float cpuPercent, float memPercent,
        float gpuPercent, bool onBattery) {
        QualityDecision decision;
        decision.tier = QualityTier::Ultra;
        decision.reason = ScalingReason::UserRequest;
        decision.downsampleFactor = 1.0f;
        decision.targetMs = 17.0f;

        // CPU pressure
        if (cpuPercent > 90.0f) {
            decision.tier = QualityTier::Minimum;
            decision.reason = ScalingReason::CPULoad;
            decision.downsampleFactor = 4.0f;
            decision.targetMs = 50.0f;
        }
        else if (cpuPercent > 75.0f) {
            decision.tier = QualityTier::Low;
            decision.reason = ScalingReason::CPULoad;
            decision.downsampleFactor = 2.0f;
            decision.targetMs = 35.0f;
        }
        // Memory pressure overrides CPU if worse
        if (memPercent > 85.0f && static_cast<uint8_t>(decision.tier) < 3) {
            decision.tier = QualityTier::Low;
            decision.reason = ScalingReason::MemoryPressure;
            decision.downsampleFactor = 2.0f;
        }
        // GPU overload
        if (gpuPercent > 95.0f && static_cast<uint8_t>(decision.tier) < 4) {
            decision.tier = QualityTier::Minimum;
            decision.reason = ScalingReason::GPULoad;
            decision.downsampleFactor = 4.0f;
        }
        // Battery conservation
        if (onBattery && decision.tier == QualityTier::Ultra) {
            decision.tier = QualityTier::Medium;
            decision.reason = ScalingReason::BatteryLow;
            decision.downsampleFactor = 1.5f;
        }

        m_currentDecision = decision;
        m_evaluations++;
        return decision;
    }

    QualityTier GetCurrentTier() const { return m_currentDecision.tier; }

    QualityDecision GetOptimalQuality() const { return m_currentDecision; }

    uint64_t GetEvaluationCount() const { return m_evaluations; }

    void Reset() {
        m_currentDecision = QualityDecision{};
        m_evaluations = 0;
    }

private:
    QualityDecision m_currentDecision;
    uint64_t        m_evaluations = 0;
};

} // namespace Engine
} // namespace ExplorerLens
