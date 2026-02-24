//==============================================================================
// ExplorerLens.io Engine — Release Gate V25
// Developer Experience phase gate — validates Plugin SDK V2, debugger
// integration, hot-reload, and plugin performance profiling KPIs.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GateV25KPI : uint32_t {
  BuildClean = 0,
  TestsPass,
  ZeroWarnings,
  PluginSDKV2,
  PluginDebugger,
  PluginHotReload,
  PluginPerfProfiler,
  APIBackcompat,
  HotReloadLatency,
  ProfilerOverhead,
  SDKDocCoverage,
  COUNT
};

struct GateV25Result {
  GateV25KPI kpi;
  bool passed = false;
  std::wstring detail;
};
struct GateV25Verdict {
  bool approved = false;
  uint32_t passed = 0;
  uint32_t failed = 0;
  std::wstring version = L"14.0.0";
  std::wstring milestone = L"v14.0 P3 - Developer Experience";
};

class ReleaseGateV25 {
public:
  static const wchar_t *KPIName(GateV25KPI k) {
    switch (k) {
    case GateV25KPI::BuildClean:
      return L"Build Clean";
    case GateV25KPI::TestsPass:
      return L"Tests Pass";
    case GateV25KPI::ZeroWarnings:
      return L"Zero Warnings";
    case GateV25KPI::PluginSDKV2:
      return L"Plugin SDK V2";
    case GateV25KPI::PluginDebugger:
      return L"Plugin Debugger";
    case GateV25KPI::PluginHotReload:
      return L"Plugin Hot-Reload";
    case GateV25KPI::PluginPerfProfiler:
      return L"Plugin Perf Profiler";
    case GateV25KPI::APIBackcompat:
      return L"API Backcompat";
    case GateV25KPI::HotReloadLatency:
      return L"Hot-Reload Latency";
    case GateV25KPI::ProfilerOverhead:
      return L"Profiler Overhead";
    case GateV25KPI::SDKDocCoverage:
      return L"SDK Doc Coverage";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV25KPI::COUNT);
  }
  static GateV25Verdict Evaluate(const std::vector<GateV25Result> &r) {
    GateV25Verdict v;
    for (const auto &x : r) {
      if (x.passed)
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
