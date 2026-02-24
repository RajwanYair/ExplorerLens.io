//==============================================================================
// ExplorerLens.io Engine — Release Gate V16
// Validates all v10.6 changes: version sync, format registry, shell expansion.
//==============================================================================
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Release Gate V16 KPI identifiers
enum class GateV16KPI : uint8_t {
  VersionSync,         // All files at v10.5.0+
  BuildZeroWarnings,   // 0 warnings
  BuildZeroErrors,     // 0 errors
  TestPassRate,        // 100% pass
  FormatRegistryValid, // FormatRegistry validation passes
  ShellRegComplete,    // No missing shell registrations
  ChangelogCurrent,    // CHANGELOG has latest version entry
  ReadmeVersionSync,   // README version matches
  CMakeVersionSync,    // CMakeLists version matches
  BuildConfigSync,     // BuildConfig.h version matches
  SprintDocsCurrent,   // Sprint docs exist for all sprints
  CodeCoverageTarget,  // >= 70% line coverage
  FormatLookupTable,   // FormatTypeLookup has >= 80 mappings
  DecoderCount,        // >= 30 production decoders
  ShellRegCount,       // >= 93 shell registrations
  PerfSingleThumb,     // < 20ms single thumbnail
  PerfBatchThroughput, // > 200 img/sec batch
  PerfCacheHit,        // < 5ms cache hit
  NewTestCount,        // >= 5 new tests per sprint
  DocGovernance,       // Documentation governance rules met
  COUNT
};

/// KPI evaluation result
struct GateV16Result {
  GateV16KPI kpi;
  bool passed = false;
  std::wstring detail;
  float value = 0.0f;
  float threshold = 0.0f;
};

/// Release Gate V16 evaluator
class ReleaseGateV16 {
public:
  /// KPI name
  static const wchar_t *KPIName(GateV16KPI kpi) {
    switch (kpi) {
    case GateV16KPI::VersionSync:
      return L"VersionSync";
    case GateV16KPI::BuildZeroWarnings:
      return L"BuildZeroWarnings";
    case GateV16KPI::BuildZeroErrors:
      return L"BuildZeroErrors";
    case GateV16KPI::TestPassRate:
      return L"TestPassRate";
    case GateV16KPI::FormatRegistryValid:
      return L"FormatRegistryValid";
    case GateV16KPI::ShellRegComplete:
      return L"ShellRegComplete";
    case GateV16KPI::ChangelogCurrent:
      return L"ChangelogCurrent";
    case GateV16KPI::ReadmeVersionSync:
      return L"ReadmeVersionSync";
    case GateV16KPI::CMakeVersionSync:
      return L"CMakeVersionSync";
    case GateV16KPI::BuildConfigSync:
      return L"BuildConfigSync";
    case GateV16KPI::SprintDocsCurrent:
      return L"SprintDocsCurrent";
    case GateV16KPI::CodeCoverageTarget:
      return L"CodeCoverageTarget";
    case GateV16KPI::FormatLookupTable:
      return L"FormatLookupTable";
    case GateV16KPI::DecoderCount:
      return L"DecoderCount";
    case GateV16KPI::ShellRegCount:
      return L"ShellRegCount";
    case GateV16KPI::PerfSingleThumb:
      return L"PerfSingleThumb";
    case GateV16KPI::PerfBatchThroughput:
      return L"PerfBatchThroughput";
    case GateV16KPI::PerfCacheHit:
      return L"PerfCacheHit";
    case GateV16KPI::NewTestCount:
      return L"NewTestCount";
    case GateV16KPI::DocGovernance:
      return L"DocGovernance";
    default:
      return L"Unknown";
    }
  }

  /// Total KPI count
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV16KPI::COUNT);
  }

  /// Evaluate a single KPI
  GateV16Result EvaluateKPI(GateV16KPI kpi, float value) const {
    GateV16Result r;
    r.kpi = kpi;
    r.value = value;
    switch (kpi) {
    case GateV16KPI::TestPassRate:
      r.threshold = 100.0f;
      break;
    case GateV16KPI::CodeCoverageTarget:
      r.threshold = 70.0f;
      break;
    case GateV16KPI::FormatLookupTable:
      r.threshold = 80.0f;
      break;
    case GateV16KPI::DecoderCount:
      r.threshold = 30.0f;
      break;
    case GateV16KPI::ShellRegCount:
      r.threshold = 93.0f;
      break;
    case GateV16KPI::PerfSingleThumb:
      r.threshold = 20.0f;
      r.passed = value <= r.threshold;
      return r;
    case GateV16KPI::PerfCacheHit:
      r.threshold = 5.0f;
      r.passed = value <= r.threshold;
      return r;
    case GateV16KPI::PerfBatchThroughput:
      r.threshold = 200.0f;
      break;
    case GateV16KPI::NewTestCount:
      r.threshold = 5.0f;
      break;
    default:
      r.threshold = 1.0f;
      break;
    }
    r.passed = value >= r.threshold;
    return r;
  }

  /// Evaluate all and compute overall verdict
  struct GateVerdict {
    bool approved = false;
    size_t passed = 0;
    size_t failed = 0;
    size_t total = 0;
    std::wstring version = L"10.6.0";
  };

  GateVerdict Evaluate(const std::vector<GateV16Result> &results) const {
    GateVerdict v;
    v.total = results.size();
    for (auto &r : results) {
      if (r.passed)
        v.passed++;
      else
        v.failed++;
    }
    v.approved = (v.failed == 0);
    return v;
  }
};

} // namespace Engine
} // namespace ExplorerLens
