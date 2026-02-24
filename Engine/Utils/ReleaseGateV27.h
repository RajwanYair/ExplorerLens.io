//==============================================================================
// ExplorerLens.io Engine — Release Gate V27
// UX Excellence phase gate — validates progressive loader, animation engine
// V2, preview panel V2, and Quick Look integration KPIs.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GateV27KPI : uint32_t {
  BuildClean = 0,
  TestsPass,
  ZeroWarnings,
  ProgressiveLoader,
  AnimEngineV2,
  PreviewPanelV2,
  QuickLookIntegration,
  FirstByteLatency,
  AnimSmoothness,
  PreviewAccuracy,
  QuickLookLaunchMs,
  COUNT
};
struct GateV27Result {
  GateV27KPI kpi;
  bool passed = false;
  std::wstring detail;
};
struct GateV27Verdict {
  bool approved = false;
  uint32_t passed = 0;
  uint32_t failed = 0;
  std::wstring version = L"14.0.0";
  std::wstring milestone = L"v14.0 P5 - UX Excellence";
};
class ReleaseGateV27 {
public:
  static const wchar_t *KPIName(GateV27KPI k) {
    switch (k) {
    case GateV27KPI::BuildClean:
      return L"Build Clean";
    case GateV27KPI::TestsPass:
      return L"Tests Pass";
    case GateV27KPI::ZeroWarnings:
      return L"Zero Warnings";
    case GateV27KPI::ProgressiveLoader:
      return L"Progressive Loader";
    case GateV27KPI::AnimEngineV2:
      return L"Anim Engine V2";
    case GateV27KPI::PreviewPanelV2:
      return L"Preview Panel V2";
    case GateV27KPI::QuickLookIntegration:
      return L"Quick Look";
    case GateV27KPI::FirstByteLatency:
      return L"First Byte Latency";
    case GateV27KPI::AnimSmoothness:
      return L"Anim Smoothness";
    case GateV27KPI::PreviewAccuracy:
      return L"Preview Accuracy";
    case GateV27KPI::QuickLookLaunchMs:
      return L"QK Launch <100ms";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV27KPI::COUNT);
  }
  static GateV27Verdict Evaluate(const std::vector<GateV27Result> &r) {
    GateV27Verdict v;
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
