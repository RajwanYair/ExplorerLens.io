//==============================================================================
// ExplorerLens.io Engine — Release Gate V31
// Performance Summit phase gate — validates all Phase 9 KPIs before
// advancing to v14.0 Release (Phase 10).
//==============================================================================
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class GateV31KPI : uint8_t {
  SubMsCacheP99 = 0,         // Cache P99 latency < 1 ms
  GPUDecodeSpeedup = 1,      // GPU decode ≥ 2× faster than CPU
  ParallelIOThroughput = 2,  // I/O throughput ≥ 2 GB/s (NVMe)
  WorkingSetTarget = 3,      // Working set ≤ 128 MB steady-state
  SingleThumbTarget = 4,     // Single thumbnail ≤ 17 ms
  BatchThroughputTarget = 5, // Batch throughput ≥ 235 img/sec
  CacheHitLatency = 6,       // Cache hit ≤ 5 ms P99
  FragmentationScore = 7,    // Heap fragmentation ≤ 0.10
  LargePageAdoption = 8,     // Large pages active for GPU staging
  PerfRegressionSuite = 9,   // Perf regression suite 100% pass
  COUNT
};

struct ReleaseGateV31Result {
  bool allKPIsPass = false;
  uint8_t kpiPassCount = 0;
  uint8_t kpiTotalCount = static_cast<uint8_t>(GateV31KPI::COUNT);
  float gateScore = 0.0f;
  bool advanceRecommended = false;
};

class ReleaseGateV31 {
public:
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV31KPI::COUNT);
  }
  static const wchar_t *KPIName(GateV31KPI k) {
    switch (k) {
    case GateV31KPI::SubMsCacheP99:
      return L"Cache P99 < 1 ms";
    case GateV31KPI::GPUDecodeSpeedup:
      return L"GPU Decode ≥ 2× CPU";
    case GateV31KPI::ParallelIOThroughput:
      return L"Parallel I/O ≥ 2 GB/s";
    case GateV31KPI::WorkingSetTarget:
      return L"Working Set ≤ 128 MB";
    case GateV31KPI::SingleThumbTarget:
      return L"Single Thumb ≤ 17 ms";
    case GateV31KPI::BatchThroughputTarget:
      return L"Batch ≥ 235 img/sec";
    case GateV31KPI::CacheHitLatency:
      return L"Cache Hit ≤ 5 ms P99";
    case GateV31KPI::FragmentationScore:
      return L"Heap Frag ≤ 0.10";
    case GateV31KPI::LargePageAdoption:
      return L"Large Pages Active";
    case GateV31KPI::PerfRegressionSuite:
      return L"Perf Regression 100%";
    default:
      return L"Unknown KPI";
    }
  }
  static ReleaseGateV31Result Evaluate(bool kpiResults[]) {
    ReleaseGateV31Result r;
    for (size_t i = 0; i < KPICount(); ++i)
      if (kpiResults[i])
        ++r.kpiPassCount;
    r.gateScore =
        (static_cast<float>(r.kpiPassCount) / static_cast<float>(KPICount())) *
        100.0f;
    r.allKPIsPass = (r.kpiPassCount == r.kpiTotalCount);
    r.advanceRecommended = r.gateScore >= 90.0f;
    return r;
  }
};

} // namespace Engine
} // namespace ExplorerLens
