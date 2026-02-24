//==============================================================================
// ExplorerLens.io Engine — Release Gate V32 — v14.0 "Apex" Final
// Definitive release gate for v14.0. Validates all 23 ship-blocking KPIs
// across GPU V3, Format Intelligence, Developer Experience, Security,
// UX Excellence, AI/ML, Enterprise, Platform Hardening, and Perf Summit.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class GateV32KPI : uint8_t {
  // GPU Pipeline V3
  GPUV3PipelineStable = 0,
  // Format Intelligence
  SmartFormatDetectorAccuracy = 1,
  // Developer Experience
  PluginSDKV2Complete = 2,
  // Security Excellence
  ThreatModelApproved = 3,
  MemorySafetyClean = 4,
  // UX Excellence
  ProgressiveLoadActive = 5,
  AccessibilityWCAGAA = 6,
  // AI / ML
  AISearchIndexReady = 7,
  SceneUnderstandingPrecision = 8,
  // Enterprise & Cloud V2
  EnterprisePolicyCompliant = 9,
  ComplianceAuditPassed = 10,
  // Platform Hardening
  Windows12CompatVerified = 11,
  MSIXCertificationPass = 12,
  // Performance Summit
  SingleThumb17ms = 13,
  BatchThroughput235 = 14,
  CacheHit5ms = 15,
  SubMsCacheActive = 16,
  GPUDecodeAccelActive = 17,
  // Quality and Docs
  QAMatrixShipSignal = 18,
  DocCoverage90Pct = 19,
  ZeroWarningsBuild = 20,
  // Test Suite
  AllTestsPass = 21,
  ChangeLogUpdated = 22,
  COUNT
};

struct ReleaseGateV32Result {
  bool allKPIsPass = false;
  uint8_t kpiPassCount = 0;
  uint8_t kpiTotalCount = static_cast<uint8_t>(GateV32KPI::COUNT);
  float gateScore = 0.0f;
  bool v14ShipApproved = false;
};

class ReleaseGateV32 {
public:
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV32KPI::COUNT);
  }
  static const wchar_t *KPIName(GateV32KPI k) {
    switch (k) {
    case GateV32KPI::GPUV3PipelineStable:
      return L"GPU V3 Pipeline Stable";
    case GateV32KPI::SmartFormatDetectorAccuracy:
      return L"Format Detector Accuracy ≥ 99%";
    case GateV32KPI::PluginSDKV2Complete:
      return L"Plugin SDK V2 Complete";
    case GateV32KPI::ThreatModelApproved:
      return L"Threat Model V2 Approved";
    case GateV32KPI::MemorySafetyClean:
      return L"Memory Safety Audit Clean";
    case GateV32KPI::ProgressiveLoadActive:
      return L"Progressive Thumbnail Load Active";
    case GateV32KPI::AccessibilityWCAGAA:
      return L"Accessibility WCAG 2.2 AA";
    case GateV32KPI::AISearchIndexReady:
      return L"AI Search Index Ready";
    case GateV32KPI::SceneUnderstandingPrecision:
      return L"Scene Understanding Precision ≥ 90%";
    case GateV32KPI::EnterprisePolicyCompliant:
      return L"Enterprise Policy Compliance ≥ 95%";
    case GateV32KPI::ComplianceAuditPassed:
      return L"Compliance Audit Passed";
    case GateV32KPI::Windows12CompatVerified:
      return L"Windows 12 Compat Verified";
    case GateV32KPI::MSIXCertificationPass:
      return L"MSIX Store Certification Pass";
    case GateV32KPI::SingleThumb17ms:
      return L"Single Thumbnail ≤ 17 ms";
    case GateV32KPI::BatchThroughput235:
      return L"Batch Throughput ≥ 235 img/sec";
    case GateV32KPI::CacheHit5ms:
      return L"Cache Hit ≤ 5 ms";
    case GateV32KPI::SubMsCacheActive:
      return L"Sub-ms Cache Active";
    case GateV32KPI::GPUDecodeAccelActive:
      return L"GPU Decode Acceleration Active";
    case GateV32KPI::QAMatrixShipSignal:
      return L"QA Matrix Signal: SHIP";
    case GateV32KPI::DocCoverage90Pct:
      return L"Doc Coverage ≥ 90%";
    case GateV32KPI::ZeroWarningsBuild:
      return L"Zero Warnings Build";
    case GateV32KPI::AllTestsPass:
      return L"All 350 Tests Pass";
    case GateV32KPI::ChangeLogUpdated:
      return L"CHANGELOG Updated for v14.0";
    default:
      return L"Unknown KPI";
    }
  }
  static ReleaseGateV32Result Evaluate(bool kpiResults[]) {
    ReleaseGateV32Result r;
    for (size_t i = 0; i < KPICount(); ++i)
      if (kpiResults[i])
        ++r.kpiPassCount;
    r.gateScore =
        (static_cast<float>(r.kpiPassCount) / static_cast<float>(KPICount())) *
        100.0f;
    r.allKPIsPass = (r.kpiPassCount == r.kpiTotalCount);
    r.v14ShipApproved = r.allKPIsPass && r.gateScore >= 95.0f;
    return r;
  }
};

} // namespace Engine
} // namespace ExplorerLens
