#pragma once
// PerformanceAnomalyDetector.h — Performance Anomaly Detector
// Runtime performance monitoring that detects decode time spikes, memory leaks,
// GPU stalls, and throughput degradation — alerts via ETW and UI.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Anomaly type (prefixed Perf to avoid collision with UsageAnomalyDetector.h)
enum class PerfAnomalyType : uint8_t {
 None = 0,
 // Single decode latency greatly exceeds P99 baseline
 DecodeTimeSpike,
 // Batch throughput drops below configured threshold
 ThroughputDrop,
 // Monotonic memory growth detected over sliding window
 MemoryLeak,
 // GPU pipeline stall exceeds threshold
 GPUStall,
 // Mass cache eviction event
 CacheEvictionStorm,
 // I/O wait time spike detected
 DiskIOBottleneck,
 // Lock contention detected on decode threads
 ThreadContention,
 COUNT
};

/// Anomaly severity (prefixed Perf to avoid collision with UsageAnomalyDetector.h)
enum class PerfAnomalySeverity : uint8_t {
 Minor = 0,
 // Deviation 2–5× baseline
 Moderate,
 // Deviation 5–10× baseline
 Major,
 Critical,
 COUNT
};

struct PerfAnomalyEvent {
 PerfAnomalyType type = PerfAnomalyType::None;
 PerfAnomalySeverity severity = PerfAnomalySeverity::Minor;
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
 // Sliding window size in samples for anomaly detection
 uint32_t windowSamples = 100;
 bool reportToETW = true;
};

class PerformanceAnomalyDetector {
public:
 static constexpr size_t TypeCount() {
 return static_cast<size_t>(PerfAnomalyType::COUNT);
 }
 static constexpr size_t SeverityCount() {
 return static_cast<size_t>(PerfAnomalySeverity::COUNT);
 }

 static const wchar_t *TypeName(PerfAnomalyType t) {
 switch (t) {
 case PerfAnomalyType::None:
 return L"None";
 case PerfAnomalyType::DecodeTimeSpike:
 return L"Decode Time Spike";
 case PerfAnomalyType::ThroughputDrop:
 return L"Throughput Drop";
 case PerfAnomalyType::MemoryLeak:
 return L"Memory Leak";
 case PerfAnomalyType::GPUStall:
 return L"GPU Stall";
 case PerfAnomalyType::CacheEvictionStorm:
 return L"Cache Eviction Storm";
 case PerfAnomalyType::DiskIOBottleneck:
 return L"Disk I/O Bottleneck";
 case PerfAnomalyType::ThreadContention:
 return L"Thread Contention";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *SeverityName(PerfAnomalySeverity s) {
 switch (s) {
 case PerfAnomalySeverity::Minor:
 return L"Minor";
 case PerfAnomalySeverity::Moderate:
 return L"Moderate";
 case PerfAnomalySeverity::Major:
 return L"Major";
 case PerfAnomalySeverity::Critical:
 return L"Critical";
 default:
 return L"Unknown";
 }
 }

 /// Classify severity based on deviation factor
 static PerfAnomalySeverity ClassifySeverity(double deviationFactor) {
 if (deviationFactor < 2.0)
 return PerfAnomalySeverity::Minor;
 if (deviationFactor < 5.0)
 return PerfAnomalySeverity::Moderate;
 if (deviationFactor < 10.0)
 return PerfAnomalySeverity::Major;
 return PerfAnomalySeverity::Critical;
 }

 /// Check if a value is anomalous compared to baseline
 static bool IsAnomaly(double observed, double baseline, double threshold) {
 if (baseline <= 0.0)
 return false;
 return (observed / baseline) > threshold;
 }
};

class DecoderPerformanceProfiler {
public:
    static int GranularityCount() { return 4; }
    static bool IsRegression(double current, double baseline, double threshold) {
        return baseline > 0.0 && ((current - baseline) / baseline) > threshold;
    }
    DecoderPerformanceProfiler() = delete;
};

} // namespace Engine
} // namespace ExplorerLens
