#pragma once
//==============================================================================
// ExplorerLens.io — Unified Release Gate
// Consolidates all release gate versions (V3–V32) into a single,
// feature-complete release-quality validation system.
//
// Copyright (c) 2026 — ExplorerLens.io Project
//==============================================================================

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

//------------------------------------------------------------------------------
// Release KPI Categories
//------------------------------------------------------------------------------

enum class ReleaseKPI : uint8_t {
  // Build Quality
  ZeroWarningsBuild = 0,
  AllTestsPass,
  ChangeLogUpdated,
  DocCoverage90Pct,
  ReproducibleBuild,

  // Performance
  SingleThumb17ms,
  BatchThroughput235,
  CacheHit5ms,
  SubMsCacheActive,
  GPUDecodeAccelActive,

  // GPU Pipeline
  GPUV3PipelineStable,

  // Format Support
  SmartFormatDetectorAccuracy,

  // Plugin System
  PluginSDKV2Complete,

  // Security
  ThreatModelApproved,
  MemorySafetyClean,
  ComplianceAuditPassed,

  // User Experience
  ProgressiveLoadActive,
  AccessibilityWCAGAA,

  // AI/ML
  AISearchIndexReady,
  SceneUnderstandingPrecision,

  // Enterprise
  EnterprisePolicyCompliant,

  // Platform
  Windows12CompatVerified,
  MSIXCertificationPass,

  // Quality Gate
  QAMatrixShipSignal,

  COUNT
};

//------------------------------------------------------------------------------
// KPI Measurement (rich form, for detailed reporting)
//------------------------------------------------------------------------------

struct KPIMeasurement {
  ReleaseKPI kpi{};
  std::wstring name;
  double value = 0.0;
  double threshold = 0.0;
  bool passed = false;
  std::wstring unit;
  std::wstring notes;
};

//------------------------------------------------------------------------------
// Release Gate Result
//------------------------------------------------------------------------------

struct ReleaseGateResult {
  bool allKPIsPass = false;
  uint32_t kpiPassCount = 0;
  uint32_t kpiTotalCount = static_cast<uint32_t>(ReleaseKPI::COUNT);
  float gateScore = 0.0f;
  bool shipApproved = false;

  std::vector<KPIMeasurement> measurements;
  std::vector<std::wstring> blockers;
  std::vector<std::wstring> warnings;
};

//------------------------------------------------------------------------------
// Release Gate — unified release-quality validator
//------------------------------------------------------------------------------

class ReleaseGate {
public:
  static constexpr size_t KPICount() noexcept {
    return static_cast<size_t>(ReleaseKPI::COUNT);
  }

  /// Human-readable KPI name
  static const wchar_t *KPIName(ReleaseKPI k) noexcept {
    switch (k) {
    case ReleaseKPI::ZeroWarningsBuild:
      return L"Zero Warnings Build";
    case ReleaseKPI::AllTestsPass:
      return L"All Tests Pass";
    case ReleaseKPI::ChangeLogUpdated:
      return L"Changelog Updated";
    case ReleaseKPI::DocCoverage90Pct:
      return L"Documentation Coverage ≥90%";
    case ReleaseKPI::ReproducibleBuild:
      return L"Reproducible Build";
    case ReleaseKPI::SingleThumb17ms:
      return L"Single Thumbnail ≤17ms";
    case ReleaseKPI::BatchThroughput235:
      return L"Batch Throughput ≥235 img/s";
    case ReleaseKPI::CacheHit5ms:
      return L"Cache Hit <5ms";
    case ReleaseKPI::SubMsCacheActive:
      return L"Sub-ms Cache Active";
    case ReleaseKPI::GPUDecodeAccelActive:
      return L"GPU Decode Acceleration Active";
    case ReleaseKPI::GPUV3PipelineStable:
      return L"GPU V3 Pipeline Stable";
    case ReleaseKPI::SmartFormatDetectorAccuracy:
      return L"Smart Format Detector Accuracy";
    case ReleaseKPI::PluginSDKV2Complete:
      return L"Plugin SDK V2 Complete";
    case ReleaseKPI::ThreatModelApproved:
      return L"Threat Model Approved";
    case ReleaseKPI::MemorySafetyClean:
      return L"Memory Safety Clean";
    case ReleaseKPI::ComplianceAuditPassed:
      return L"Compliance Audit Passed";
    case ReleaseKPI::ProgressiveLoadActive:
      return L"Progressive Thumbnail Loading Active";
    case ReleaseKPI::AccessibilityWCAGAA:
      return L"WCAG AA Accessibility";
    case ReleaseKPI::AISearchIndexReady:
      return L"AI Search Index Ready";
    case ReleaseKPI::SceneUnderstandingPrecision:
      return L"Scene Understanding Precision";
    case ReleaseKPI::EnterprisePolicyCompliant:
      return L"Enterprise Policy Compliant";
    case ReleaseKPI::Windows12CompatVerified:
      return L"Windows 12 Compatibility Verified";
    case ReleaseKPI::MSIXCertificationPass:
      return L"MSIX Certification Pass";
    case ReleaseKPI::QAMatrixShipSignal:
      return L"QA Matrix Ship Signal";
    default:
      return L"Unknown KPI";
    }
  }

  /// Simple evaluation: pass a bool array of KPICount() results
  static ReleaseGateResult Evaluate(const bool kpiResults[]) {
    ReleaseGateResult r;
    r.kpiTotalCount = static_cast<uint32_t>(KPICount());
    for (size_t i = 0; i < KPICount(); ++i) {
      if (kpiResults[i])
        ++r.kpiPassCount;
      KPIMeasurement m;
      m.kpi = static_cast<ReleaseKPI>(i);
      m.name = KPIName(m.kpi);
      m.passed = kpiResults[i];
      m.value = kpiResults[i] ? 1.0 : 0.0;
      m.threshold = 1.0;
      r.measurements.push_back(std::move(m));
      if (!kpiResults[i]) {
        r.blockers.push_back(m.name + L" — FAILED");
      }
    }
    r.gateScore = r.kpiTotalCount > 0 ? (static_cast<float>(r.kpiPassCount) /
                                         static_cast<float>(r.kpiTotalCount)) *
                                            100.0f
                                      : 0.0f;
    r.allKPIsPass = (r.kpiPassCount == r.kpiTotalCount);
    r.shipApproved = r.allKPIsPass && r.gateScore >= 95.0f;
    return r;
  }

  /// Rich evaluation: pass pre-built KPIMeasurement vector
  static ReleaseGateResult
  Evaluate(const std::vector<KPIMeasurement> &measurements) {
    ReleaseGateResult r;
    r.measurements = measurements;
    r.kpiTotalCount = static_cast<uint32_t>(measurements.size());
    for (const auto &m : measurements) {
      if (m.passed)
        ++r.kpiPassCount;
      else
        r.blockers.push_back(m.name + L" — FAILED");
    }
    r.gateScore = r.kpiTotalCount > 0 ? (static_cast<float>(r.kpiPassCount) /
                                         static_cast<float>(r.kpiTotalCount)) *
                                            100.0f
                                      : 0.0f;
    r.allKPIsPass = (r.kpiPassCount == r.kpiTotalCount);
    r.shipApproved = r.allKPIsPass && r.gateScore >= 95.0f;
    return r;
  }

  /// Generate a release checklist (summary of all KPIs and their status)
  static std::wstring GenerateChecklist(const ReleaseGateResult &result) {
    std::wstringstream ss;
    ss << L"=== ExplorerLens.io Release Gate Checklist ===\n\n";
    for (const auto &m : result.measurements) {
      ss << (m.passed ? L"[PASS] " : L"[FAIL] ") << m.name << L"\n";
    }
    ss << L"\n--- Summary ---\n"
       << L"Passed: " << result.kpiPassCount << L"/" << result.kpiTotalCount
       << L" (Score: " << static_cast<int>(result.gateScore) << L"%)\n"
       << L"Ship Approved: " << (result.shipApproved ? L"YES" : L"NO") << L"\n";
    return ss.str();
  }
};

} // namespace Engine
} // namespace ExplorerLens
