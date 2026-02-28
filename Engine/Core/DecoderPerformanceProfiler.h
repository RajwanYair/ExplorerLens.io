#pragma once
// DecoderPerformanceProfiler.h — Decoder Performance Profiler
// Per-decoder performance profiling with flame-graph-ready trace data,
// bottleneck identification, and regression detection.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Profiling granularity
enum class ProfileGranularity : uint8_t {
 Summary = 0, // Per-decoder totals only
 PerCall, // Each decode invocation
 PerStage, // Pipeline stage breakdown
 Instruction, // CPU instruction-level (sampling)
 COUNT
};

/// Bottleneck type
enum class BottleneckType : uint8_t {
 None = 0,
 CPU_Decode,
 GPU_Transfer,
 DiskIO,
 MemoryAllocation,
 LockContention,
 CacheMiss,
 COUNT
};

struct DecoderProfile {
 const wchar_t *decoderName = nullptr;
 uint64_t callCount = 0;
 double totalMs = 0.0;
 double avgMs = 0.0;
 double minMs = 99999.0;
 double maxMs = 0.0;
 double p50Ms = 0.0;
 double p95Ms = 0.0;
 double p99Ms = 0.0;
 BottleneckType bottleneck = BottleneckType::None;
};

struct ProfileConfig {
 ProfileGranularity granularity = ProfileGranularity::Summary;
 bool enabled = false;
 bool traceToETW = false;
 bool detectRegressions = true;
 double regressionThresholdPct = 20.0; // flag if > 20% slower
 uint32_t sampleRateHz = 1000;
};

class DecoderPerformanceProfiler {
public:
 static constexpr size_t GranularityCount() {
 return static_cast<size_t>(ProfileGranularity::COUNT);
 }
 static constexpr size_t BottleneckCount() {
 return static_cast<size_t>(BottleneckType::COUNT);
 }

 static const wchar_t *GranularityName(ProfileGranularity g) {
 switch (g) {
 case ProfileGranularity::Summary:
 return L"Summary";
 case ProfileGranularity::PerCall:
 return L"Per-Call";
 case ProfileGranularity::PerStage:
 return L"Per-Stage";
 case ProfileGranularity::Instruction:
 return L"Instruction";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *BottleneckName(BottleneckType b) {
 switch (b) {
 case BottleneckType::None:
 return L"None";
 case BottleneckType::CPU_Decode:
 return L"CPU Decode";
 case BottleneckType::GPU_Transfer:
 return L"GPU Transfer";
 case BottleneckType::DiskIO:
 return L"Disk I/O";
 case BottleneckType::MemoryAllocation:
 return L"Memory Allocation";
 case BottleneckType::LockContention:
 return L"Lock Contention";
 case BottleneckType::CacheMiss:
 return L"Cache Miss";
 default:
 return L"Unknown";
 }
 }

 /// Check if performance regressed
 static bool IsRegression(double currentMs, double baselineMs,
 double thresholdPct) {
 if (baselineMs <= 0.0)
 return false;
 double increase = ((currentMs - baselineMs) / baselineMs) * 100.0;
 return increase > thresholdPct;
 }
};

} // namespace Engine
} // namespace ExplorerLens
