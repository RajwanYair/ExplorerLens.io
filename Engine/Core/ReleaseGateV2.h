#pragma once
// Sprint 172 — Release Gate V2
// Multi-dimension gate: build validation, test pass rate, performance KPIs,
// documentation sync, memory budget, zero-warning enforce.

#include <string>
<parameter name="content">
#pragma once
// Sprint 172 — Release Gate V2
// Multi-dimension gate: build validation, test pass rate, performance KPIs,
// documentation sync, memory budget, zero-warning enforce.

#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs::Core {

// ─── Gate dimension ──────────────────────────────────────────────────────────

enum class GateDimension : uint32_t {
    BuildZeroWarnings   = 0,
    TestPassRate        = 1,
    ThroughputKPI       = 2,
    LatencyKPI          = 3,
    MemoryBudget        = 4,
    DocumentationSync   = 5,
    ChangelogUpdated    = 6,
    ARM64Matrix         = 7,
    PluginConformance   = 8,
};

inline std::string ToString(GateDimension g) {
    switch (g) {
        case GateDimension::BuildZeroWarnings: return "BuildZeroWarnings";
        case GateDimension::TestPassRate:      return "TestPassRate";
        case GateDimension::ThroughputKPI:     return "ThroughputKPI";
        case GateDimension::LatencyKPI:        return "LatencyKPI";
        case GateDimension::MemoryBudget:      return "MemoryBudget";
        case GateDimension::DocumentationSync: return "DocumentationSync";
        case GateDimension::ChangelogUpdated:  return "ChangelogUpdated";
        case GateDimension::ARM64Matrix:       return "ARM64Matrix";
        case GateDimension::PluginConformance: return "PluginConformance";
        default: return "Unknown";
    }
}

// ─── Gate criterion ──────────────────────────────────────────────────────────

struct GateCriterion {
    GateDimension   dimension;
    bool            passed          { false };
    bool            blocking        { true };   // false = advisory only
    double          measuredValue   { 0.0 };
    double          threshold       { 0.0 };
    std::string     detail;

    bool IsAdvisory() const { return !blocking; }
};

// ─── KPI thresholds ──────────────────────────────────────────────────────────

struct ReleaseKPIThresholds {
    double      minThroughputImgSec     { 235.0 };
    double      maxLatencyP95Ms         { 17.0 };
    double      maxCacheHitMs           {  5.0 };
    double      maxMemoryWorkingSetMB   { 512.0 };
    double      minTestPassRatePct      { 100.0 };
    uint32_t    maxBuildWarnings        { 0 };
    uint32_t    minDocSyncChecksPassed  { 5 };

    static ReleaseKPIThresholds ForV83() {
        ReleaseKPIThresholds t;
        t.minThroughputImgSec  = 235.0;
        t.maxLatencyP95Ms      = 17.0;
        return t;
    }
};

// ─── Gate report ─────────────────────────────────────────────────────────────

struct ReleaseGateV2Report {
    std::string                     releaseTag;     // e.g., "v8.3.0"
    std::string                     sprintRef;      // e.g., "Sprint172"
    std::vector<GateCriterion>      criteria;
    bool                            gateOpen        { false };
    uint32_t                        blockerCount    { 0 };
    uint32_t                        advisoryCount   { 0 };
    double                          evaluatedMs     { 0.0 };

    uint32_t PassedCount() const {
        uint32_t n = 0;
        for (const auto& c : criteria) if (c.passed) ++n;
        return n;
    }

    static ReleaseGateV2Report CreateMock(bool allPass = true) {
        ReleaseGateV2Report r;
        r.releaseTag = "v8.3.0";
        r.sprintRef  = "Sprint172";

        auto thresholds = ReleaseKPIThresholds::ForV83();

        auto addCrit = [&](GateDimension dim, bool pass, double meas, double thresh, bool blocking = true) {
            GateCriterion c;
            c.dimension     = dim;
            c.passed        = pass;
            c.blocking      = blocking;
            c.measuredValue = meas;
            c.threshold     = thresh;
            r.criteria.push_back(c);
            if (!pass) {
                if (blocking) ++r.blockerCount;
                else          ++r.advisoryCount;
            }
        };

        addCrit(GateDimension::BuildZeroWarnings, allPass, 0,             0);
        addCrit(GateDimension::TestPassRate,      allPass, allPass ? 100 : 98, thresholds.minTestPassRatePct);
        addCrit(GateDimension::ThroughputKPI,     allPass, 237.0,          thresholds.minThroughputImgSec);
        addCrit(GateDimension::LatencyKPI,        allPass,  16.5,          thresholds.maxLatencyP95Ms);
        addCrit(GateDimension::MemoryBudget,      allPass, 480.0,          thresholds.maxMemoryWorkingSetMB);
        addCrit(GateDimension::DocumentationSync, allPass, allPass ? 5 : 3, thresholds.minDocSyncChecksPassed);
        addCrit(GateDimension::ChangelogUpdated,  allPass,  1,              1);
        addCrit(GateDimension::ARM64Matrix,       true,    7,               7, false);  // advisory
        addCrit(GateDimension::PluginConformance, allPass,  5,              5);

        r.gateOpen    = r.blockerCount == 0;
        r.evaluatedMs = 120.0;
        return r;
    }
};

// ─── Release Gate V2 evaluator ────────────────────────────────────────────────

class ReleaseGateV2 {
public:
    explicit ReleaseGateV2(ReleaseKPIThresholds thresholds = ReleaseKPIThresholds::ForV83())
        : m_thresholds(std::move(thresholds)) {}

    bool Evaluate(ReleaseGateV2Report& report) const {
        report.blockerCount  = 0;
        report.advisoryCount = 0;
        for (auto& c : report.criteria) {
            if (!c.passed) {
                if (c.blocking) ++report.blockerCount;
                else            ++report.advisoryCount;
            }
        }
        report.gateOpen = report.blockerCount == 0;
        return report.gateOpen;
    }

    const ReleaseKPIThresholds& Thresholds() const { return m_thresholds; }

private:
    ReleaseKPIThresholds m_thresholds;
};

} // namespace DarkThumbs::Core
