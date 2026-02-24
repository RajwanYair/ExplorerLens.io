#pragma once
//==============================================================================
// ExplorerLens.io — Release Gate V2
// Production release validation system for the v8.3.0 (Apex) release block.
// Evaluates build quality, performance, plugin conformance, and advisory
// checks.
//
// Copyright (c) 2026 — ExplorerLens.io Project
//==============================================================================

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Core {

//------------------------------------------------------------------------------
// Gate dimensions — each represents one aspect of release readiness
//------------------------------------------------------------------------------

enum class GateDimension : uint32_t {
  BuildZeroWarnings = 0,      // compiler emits zero warnings
  TestPassRate = 1,           // all 1187+ tests pass
  PerformanceSingleThumb = 2, // single thumbnail <= 17 ms
  PerformanceBatch = 3,       // batch throughput >= 235 img/sec
  CacheHit = 4,               // cache hit latency <= 5 ms
  MemorySafety = 5,           // no leaks or sanitizer findings
  DocumentationSync = 6,      // VERSION / CHANGELOG / README in sync
  GPUPipelineStable = 7,      // DX11/DX12/Vulkan pipeline stable
  PluginConformance = 8,      // plugin ABI and trust chain valid
  ARM64Matrix = 9,            // ARM64 library matrix (advisory)
};

// Convert a gate dimension to a human-readable name for logging.
inline std::string ToString(GateDimension d) {
  switch (d) {
  case GateDimension::BuildZeroWarnings:
    return "BuildZeroWarnings";
  case GateDimension::TestPassRate:
    return "TestPassRate";
  case GateDimension::PerformanceSingleThumb:
    return "PerformanceSingleThumb";
  case GateDimension::PerformanceBatch:
    return "PerformanceBatch";
  case GateDimension::CacheHit:
    return "CacheHit";
  case GateDimension::MemorySafety:
    return "MemorySafety";
  case GateDimension::DocumentationSync:
    return "DocumentationSync";
  case GateDimension::GPUPipelineStable:
    return "GPUPipelineStable";
  case GateDimension::PluginConformance:
    return "PluginConformance";
  case GateDimension::ARM64Matrix:
    return "ARM64Matrix";
  default:
    return "Unknown";
  }
}

//------------------------------------------------------------------------------
// Numeric KPI thresholds for a specific release tag
//------------------------------------------------------------------------------

struct ReleaseKPIThresholds {
  double minThroughputImgSec{0.0}; // minimum batch images/second
  double maxLatencyP95Ms{0.0};     // P95 single-thumbnail latency (ms)
  uint32_t maxBuildWarnings{0};    // compiler warnings allowed
  uint32_t minTestPassCount{0};    // minimum tests that must pass

  // KPI thresholds committed for the Apex v8.3.0 release.
  static ReleaseKPIThresholds ForV83() {
    ReleaseKPIThresholds t;
    t.minThroughputImgSec = 235.0;
    t.maxLatencyP95Ms = 17.0;
    t.maxBuildWarnings = 0;
    t.minTestPassCount = 1187;
    return t;
  }
};

//------------------------------------------------------------------------------
// A single evaluated criterion within a release gate report
//------------------------------------------------------------------------------

struct GateCriterion {
  GateDimension dimension{GateDimension::BuildZeroWarnings};
  std::string description; // plain-text description of the check
  bool passed{false};      // did this criterion pass?
  bool blocking{true};     // does failure block the release?
  std::string notes;       // optional detail (measured value, etc.)
};

//------------------------------------------------------------------------------
// Full release gate report — aggregates all criteria for one release cycle
//------------------------------------------------------------------------------

struct ReleaseGateV2Report {
  bool gateOpen{false};                // true when all blocking criteria pass
  uint32_t blockerCount{0};            // number of blocking failures
  std::string releaseTag;              // e.g. "v8.3.0"
  std::string sprintRef;               // e.g. "Sprint174"
  std::vector<GateCriterion> criteria; // one entry per GateDimension checked

  // Count how many criteria passed.
  uint32_t PassedCount() const {
    uint32_t n = 0;
    for (const auto &c : criteria)
      if (c.passed)
        ++n;
    return n;
  }

  // Build a synthetic report for unit testing.
  // allPass = true  → all blocking criteria pass, gate is open.
  // allPass = false → first blocking criterion fails, gate is closed.
  static ReleaseGateV2Report CreateMock(bool allPass) {
    ReleaseGateV2Report r;
    r.releaseTag = "v8.3.0";
    r.sprintRef = "Sprint174";

    // Blocking criteria
    const GateDimension blocking[] = {
        GateDimension::BuildZeroWarnings,
        GateDimension::TestPassRate,
        GateDimension::PerformanceSingleThumb,
        GateDimension::PerformanceBatch,
        GateDimension::CacheHit,
        GateDimension::MemorySafety,
        GateDimension::DocumentationSync,
        GateDimension::GPUPipelineStable,
        GateDimension::PluginConformance,
    };

    bool firstBlockerSet = false;
    for (auto d : blocking) {
      GateCriterion c;
      c.dimension = d;
      c.description = ToString(d);
      c.blocking = true;
      if (allPass) {
        c.passed = true;
      } else if (!firstBlockerSet) {
        c.passed = false;
        firstBlockerSet = true;
      } else {
        c.passed = true;
      }
      r.criteria.push_back(c);
    }

    // Advisory criterion (ARM64Matrix) — never blocks release
    GateCriterion advisory;
    advisory.dimension = GateDimension::ARM64Matrix;
    advisory.description = "ARM64Matrix";
    advisory.blocking = false;
    advisory.passed = allPass;
    r.criteria.push_back(advisory);

    // Compute aggregates
    r.blockerCount = 0;
    for (const auto &c : r.criteria)
      if (c.blocking && !c.passed)
        ++r.blockerCount;
    r.gateOpen = (r.blockerCount == 0);

    return r;
  }
};

//------------------------------------------------------------------------------
// Evaluator — decides whether a report satisfies the release gate
//------------------------------------------------------------------------------

class ReleaseGateV2 {
public:
  // Returns true when the report has no blocking failures.
  bool Evaluate(const ReleaseGateV2Report &report) const {
    return report.gateOpen;
  }
};

} // namespace ExplorerLens::Core
