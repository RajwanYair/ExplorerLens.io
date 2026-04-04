// UsageAnomalyDetector.h — Statistical Z-Score Usage Anomaly Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Provides UsageAnomalyDetector for real-time statistical anomaly detection across
// decoder usage metrics using configurable Z-score thresholds and baseline tracking.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

enum class AnomalySeverity : uint8_t {
    Low = 0,
    Medium = 1,
    High = 2,
    Critical = 3
};

enum class AnomalyType : uint8_t {
    SpikeThroughput = 0,
    ErrorBurst = 1,
    LatencyCliff = 2,
    ResourceLeak = 3
};

struct AnomalyEvent
{
    AnomalyType type{AnomalyType::SpikeThroughput};
    AnomalySeverity severity{AnomalySeverity::Low};
    double zScore{0.0};
    std::string affectedDecoder;
    std::string description;
    std::chrono::system_clock::time_point detectedAt;
    bool resolved{false};
};

struct BaselineStats
{
    double mean{0.0};
    double stdDev{0.0};
    uint64_t sampleCount{0};
    double minValue{0.0};
    double maxValue{0.0};
};

struct ThresholdConfig
{
    double lowSeverityZ{2.0};
    double mediumSeverityZ{3.0};
    double highSeverityZ{4.0};
    double criticalSeverityZ{5.0};
    uint32_t minSamplesForDetection{30};
};

class UsageAnomalyDetector
{
  public:
    explicit UsageAnomalyDetector(ThresholdConfig config = {});
    ~UsageAnomalyDetector() = default;

    UsageAnomalyDetector(const UsageAnomalyDetector&) = delete;
    UsageAnomalyDetector& operator=(const UsageAnomalyDetector&) = delete;

    // Sample ingestion
    void RecordSample(const std::string& decoderId, AnomalyType type, double value);

    // Detection
    std::vector<AnomalyEvent> Detect(const std::string& decoderId) const;
    std::vector<AnomalyEvent> DetectAll() const;

    // Active anomaly management
    std::vector<AnomalyEvent> GetActiveAnomalies() const;
    bool ResolveAnomaly(const std::string& decoderId, AnomalyType type);

    // Threshold configuration
    void SetThreshold(ThresholdConfig config);
    const ThresholdConfig& GetThreshold() const noexcept;

    // Baseline access
    std::optional<BaselineStats> GetBaseline(const std::string& decoderId, AnomalyType type) const;
    bool ResetBaseline(const std::string& decoderId);
    void ResetAllBaselines();

    // Alert callback
    using AlertCallback = std::function<void(const AnomalyEvent&)>;
    void SetAlertCallback(AlertCallback cb);

  private:
    ThresholdConfig m_config;
    AlertCallback m_alertCallback;

    double ComputeZScore(double value, const BaselineStats& baseline) const;
    AnomalySeverity ClassifySeverity(double zScore) const noexcept;
    void UpdateBaseline(const std::string& decoderId, AnomalyType type, double value);
};

}  // namespace ExplorerLens::Engine
