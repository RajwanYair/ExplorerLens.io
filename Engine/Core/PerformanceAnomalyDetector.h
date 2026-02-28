#pragma once
// PerformanceAnomalyDetector.h — Performance Anomaly Detector
// Runtime performance monitoring that detects decode time spikes, memory leaks,
// GPU stalls, and throughput degradation — alerts via ETW and UI.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Anomaly type
enum class AnomalyType : uint8_t {
 None = 0,
 DecodeTimeSpike, // Single decode >> P99 latency
 ThroughputDrop, // Batch throughput drops below threshold
 MemoryLeak, // Monotonic memory growth over time
 GPUStall, // GPU pipeline stall > threshold
 CacheEvictionStorm, // Mass eviction event
 DiskIOBottleneck, // I/O wait time spike
 ThreadContention, // Lock contention detected
 COUNT
};

/// Anomaly severity
enum class AnomalySeverity : uint8_t {
 Minor = 0, // < 2x baseline
 Moderate, // 2-5x baseline
 Major, // 5-10x baseline
 Critical, // > 10x baseline
 COUNT
};

struct AnomalyEvent {
 AnomalyType type = AnomalyType::None;
 AnomalySeverity severity = AnomalySeverity::Minor;
 double baselineValue = 0.0;
 double observedValue = 0.0;
 double deviationFactor = 1.0;
 uint64_t timestampTicks = 0;
 const wchar_t *context = nullptr;
};

struct AnomalyDetectorConfig {
 bool enabled = true;
 double spikeThresholdMultiplier = 3.0;
 double leakThresholdMB = 50.0;
 double gpuStallThresholdMs = 100.0;
 uint32_t windowSamples = 100; // sliding window size
 bool reportToETW = true;
};

class PerformanceAnomalyDetector {
public:
 static constexpr size_t TypeCount() {
 return static_cast<size_t>(AnomalyType::COUNT);
 }
 static constexpr size_t SeverityCount() {
 return static_cast<size_t>(AnomalySeverity::COUNT);
 }

 static const wchar_t *TypeName(AnomalyType t) {
 switch (t) {
 case AnomalyType::None:
 return L"None";
 case AnomalyType::DecodeTimeSpike:
 return L"Decode Time Spike";
 case AnomalyType::ThroughputDrop:
 return L"Throughput Drop";
 case AnomalyType::MemoryLeak:
 return L"Memory Leak";
 case AnomalyType::GPUStall:
 return L"GPU Stall";
 case AnomalyType::CacheEvictionStorm:
 return L"Cache Eviction Storm";
 case AnomalyType::DiskIOBottleneck:
 return L"Disk I/O Bottleneck";
 case AnomalyType::ThreadContention:
 return L"Thread Contention";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *SeverityName(AnomalySeverity s) {
 switch (s) {
 case AnomalySeverity::Minor:
 return L"Minor";
 case AnomalySeverity::Moderate:
 return L"Moderate";
 case AnomalySeverity::Major:
 return L"Major";
 case AnomalySeverity::Critical:
 return L"Critical";
 default:
 return L"Unknown";
 }
 }

 /// Classify severity based on deviation factor
 static AnomalySeverity ClassifySeverity(double deviationFactor) {
 if (deviationFactor < 2.0)
 return AnomalySeverity::Minor;
 if (deviationFactor < 5.0)
 return AnomalySeverity::Moderate;
 if (deviationFactor < 10.0)
 return AnomalySeverity::Major;
 return AnomalySeverity::Critical;
 }

 /// Check if a value is anomalous compared to baseline
 static bool IsAnomaly(double observed, double baseline, double threshold) {
 if (baseline <= 0.0)
 return false;
 return (observed / baseline) > threshold;
 }
};

} // namespace Engine
} // namespace ExplorerLens
