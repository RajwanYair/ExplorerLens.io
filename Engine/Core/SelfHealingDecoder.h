#pragma once
// SelfHealingDecoder.h — Self-Healing Decoder Pipeline
// Auto-recovery from decoder crashes via process isolation, watchdog timers,
// and automatic retry with degraded quality fallback.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Recovery strategy
enum class SelfHealStrategy : uint8_t {
    RetryImmediate = 0,  // Retry same decoder immediately
    RetryWithFallback,   // Retry with simpler decoder
    ReduceQuality,       // Lower resolution/quality and retry
    SkipAndPlaceholder,  // Give up, show placeholder
    IsolateAndReport,    // Isolate faulty decoder, report telemetry
    COUNT
};

/// Decoder health state
enum class DecoderHealthState : uint8_t {
    Healthy = 0,
    Degraded,     // Occasional failures
    Unstable,     // Frequent failures (> 10%)
    Quarantined,  // Disabled after repeated crashes
    COUNT
};

struct DecoderHealthInfo
{
    const wchar_t* decoderName = nullptr;
    DecoderHealthState state = DecoderHealthState::Healthy;
    uint32_t successCount = 0;
    uint32_t failureCount = 0;
    uint32_t crashCount = 0;
    uint32_t timeoutCount = 0;
    double avgDecodeMs = 0.0;
    double lastFailureMs = 0.0;
};

struct SelfHealConfig
{
    uint32_t maxRetries = 3;
    uint32_t watchdogTimeoutMs = 5000;
    float quarantineThreshold = 0.2f;  // 20% failure rate
    bool enableProcessIsolation = false;
    SelfHealStrategy defaultStrategy = SelfHealStrategy::RetryWithFallback;
};

class SelfHealingDecoder
{
  public:
    static constexpr size_t StrategyCount()
    {
        return static_cast<size_t>(SelfHealStrategy::COUNT);
    }
    static constexpr size_t HealthCount()
    {
        return static_cast<size_t>(DecoderHealthState::COUNT);
    }

    static const wchar_t* StrategyName(SelfHealStrategy s)
    {
        switch (s) {
            case SelfHealStrategy::RetryImmediate:
                return L"Retry Immediate";
            case SelfHealStrategy::RetryWithFallback:
                return L"Retry with Fallback";
            case SelfHealStrategy::ReduceQuality:
                return L"Reduce Quality";
            case SelfHealStrategy::SkipAndPlaceholder:
                return L"Skip (Placeholder)";
            case SelfHealStrategy::IsolateAndReport:
                return L"Isolate & Report";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* HealthName(DecoderHealthState h)
    {
        switch (h) {
            case DecoderHealthState::Healthy:
                return L"Healthy";
            case DecoderHealthState::Degraded:
                return L"Degraded";
            case DecoderHealthState::Unstable:
                return L"Unstable";
            case DecoderHealthState::Quarantined:
                return L"Quarantined";
            default:
                return L"Unknown";
        }
    }

    /// Calculate failure rate
    static float FailureRate(uint32_t successes, uint32_t failures)
    {
        uint32_t total = successes + failures;
        return total > 0 ? static_cast<float>(failures) / total : 0.0f;
    }

    /// Determine health state from failure rate
    static DecoderHealthState ClassifyHealth(float failureRate)
    {
        if (failureRate < 0.01f)
            return DecoderHealthState::Healthy;
        if (failureRate < 0.05f)
            return DecoderHealthState::Degraded;
        if (failureRate < 0.20f)
            return DecoderHealthState::Unstable;
        return DecoderHealthState::Quarantined;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
