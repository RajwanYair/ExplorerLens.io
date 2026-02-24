//==============================================================================
// ExplorerLens.io Engine — Release Gate V28
// AI & ML Expansion phase gate — validates scene understanding, smart crop,
// image quality assessor, and AI search integration KPIs.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GateV28KPI : uint32_t {
  BuildClean = 0,
  TestsPass,
  ZeroWarnings,
  SceneUnderstanding,
  SmartCropV2,
  ImageQualityAssessor,
  AISearchIntegration,
  InferenceLatency,
  CropAccuracy,
  IQAAccuracy,
  SearchPrecision,
  COUNT
};
struct GateV28Result {
  GateV28KPI kpi;
  bool passed = false;
  std::wstring detail;
};
struct GateV28Verdict {
  bool approved = false;
  uint32_t passed = 0;
  uint32_t failed = 0;
  std::wstring version = L"14.0.0";
  std::wstring milestone = L"v14.0 P6 - AI & ML Expansion";
};
class ReleaseGateV28 {
public:
  static const wchar_t *KPIName(GateV28KPI k) {
    switch (k) {
    case GateV28KPI::BuildClean:
      return L"Build Clean";
    case GateV28KPI::TestsPass:
      return L"Tests Pass";
    case GateV28KPI::ZeroWarnings:
      return L"Zero Warnings";
    case GateV28KPI::SceneUnderstanding:
      return L"Scene Understanding";
    case GateV28KPI::SmartCropV2:
      return L"Smart Crop V2";
    case GateV28KPI::ImageQualityAssessor:
      return L"Image Quality Assessor";
    case GateV28KPI::AISearchIntegration:
      return L"AI Search Integration";
    case GateV28KPI::InferenceLatency:
      return L"Inference <50ms";
    case GateV28KPI::CropAccuracy:
      return L"Crop Accuracy >90%";
    case GateV28KPI::IQAAccuracy:
      return L"IQA Correlation >0.8";
    case GateV28KPI::SearchPrecision:
      return L"Search P@10 >80%";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV28KPI::COUNT);
  }
  static GateV28Verdict Evaluate(const std::vector<GateV28Result> &r) {
    GateV28Verdict v;
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
