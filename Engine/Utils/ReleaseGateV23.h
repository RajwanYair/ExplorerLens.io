//==============================================================================
// ExplorerLens.io Engine — Release Gate V23
// GPU Pipeline V3 phase release gate — validates GPU V3, shader compiler,
// PSO cache, and GPU memory pool KPIs for v14.0 P1 phase approval.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GateV23KPI : uint32_t {
  BuildClean = 0,
  TestsPass,
  ZeroWarnings,
  VersionSyncV14,
  GPUPipelineV3,
  ShaderCompilerV2,
  PSOCacheV2,
  GPUMemoryPoolV2,
  GPUFramerate,
  ShaderCacheHitRate,
  MemoryBudgetRespected,
  FallbackStability,
  COUNT
};

struct GateV23Result {
  GateV23KPI kpi;
  bool passed = false;
  std::wstring detail;
};

struct GateV23Verdict {
  bool approved = false;
  uint32_t passed = 0;
  uint32_t failed = 0;
  std::wstring version = L"14.0.0";
  std::wstring milestone = L"v14.0 P1 - GPU Pipeline V3";
};

class ReleaseGateV23 {
public:
  static const wchar_t *KPIName(GateV23KPI kpi) {
    switch (kpi) {
    case GateV23KPI::BuildClean:
      return L"Build Clean";
    case GateV23KPI::TestsPass:
      return L"Tests Pass";
    case GateV23KPI::ZeroWarnings:
      return L"Zero Warnings";
    case GateV23KPI::VersionSyncV14:
      return L"Version Sync V14";
    case GateV23KPI::GPUPipelineV3:
      return L"GPU Pipeline V3";
    case GateV23KPI::ShaderCompilerV2:
      return L"Shader Compiler V2";
    case GateV23KPI::PSOCacheV2:
      return L"PSO Cache V2";
    case GateV23KPI::GPUMemoryPoolV2:
      return L"GPU Memory Pool V2";
    case GateV23KPI::GPUFramerate:
      return L"GPU Framerate KPI";
    case GateV23KPI::ShaderCacheHitRate:
      return L"Shader Cache Hit Rate";
    case GateV23KPI::MemoryBudgetRespected:
      return L"Memory Budget";
    case GateV23KPI::FallbackStability:
      return L"Fallback Stability";
    default:
      return L"Unknown";
    }
  }

  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV23KPI::COUNT);
  }

  static GateV23Verdict Evaluate(const std::vector<GateV23Result> &results) {
    GateV23Verdict v;
    for (const auto &r : results) {
      if (r.passed)
        v.passed++;
      else
        v.failed++;
    }
    v.approved = (v.failed == 0 && v.passed == KPICount());
    return v;
  }
};

} // namespace Engine
} // namespace ExplorerLens
