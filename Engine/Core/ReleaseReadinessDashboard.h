#pragma once
// ============================================================================
// ReleaseReadinessDashboard.h — Sprint 148
// Aggregated release go/no-go dashboard combining all CI quality gates
// ============================================================================

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <functional>

namespace DarkThumbs {

// ── Gate categories ────────────────────────────────────────────────────────

enum class GateCategory {
    Build,           // Zero warnings, reproducible builds
    Tests,           // Unit test pass rate
    Performance,     // KPI thresholds met
    VersionDrift,    // No stale version references
    CodeQuality,     // Static analysis, clang-tidy
    Packaging,       // Installer builds, signing
    Documentation,   // Docs up to date, no broken links
    Security         // CVE audit, dependency scan
};

inline const char* GateCategoryName(GateCategory c) {
    switch (c) {
        case GateCategory::Build:         return "Build";
        case GateCategory::Tests:         return "Tests";
        case GateCategory::Performance:   return "Performance";
        case GateCategory::VersionDrift:  return "VersionDrift";
        case GateCategory::CodeQuality:   return "CodeQuality";
        case GateCategory::Packaging:     return "Packaging";
        case GateCategory::Documentation: return "Documentation";
        case GateCategory::Security:      return "Security";
    }
    return "Unknown";
}

// ── Individual gate status ─────────────────────────────────────────────────

enum class ReadinessLevel {
    Green,     // All clear
    Yellow,    // Minor issues, release possible
    Red,       // Blocking issues, do not release
    Unknown    // Not yet evaluated
};

struct GateStatus {
    GateCategory   category  = GateCategory::Build;
    ReadinessLevel level     = ReadinessLevel::Unknown;
    std::string    summary;
    int            passedChecks = 0;
    int            totalChecks  = 0;
    std::vector<std::string> blockers;    // Red-level issues
    std::vector<std::string> warnings;    // Yellow-level issues
    std::chrono::system_clock::time_point lastEvaluated;

    double PassRate() const {
        return totalChecks > 0 ? passedChecks * 100.0 / totalChecks : 0.0;
    }
};

// ── Release metadata ───────────────────────────────────────────────────────

struct ReleaseCandidate {
    std::string version;        // e.g. "7.1.0"
    std::string commitHash;
    std::string branchName;
    std::string buildNumber;
    std::string buildDate;
    std::string targetPlatform; // "x64"
    bool        isSigned = false;
};

// ── Dashboard result ───────────────────────────────────────────────────────

struct DashboardResult {
    ReleaseCandidate           candidate;
    ReadinessLevel             overall     = ReadinessLevel::Unknown;
    std::vector<GateStatus>    gates;
    int                        greenCount  = 0;
    int                        yellowCount = 0;
    int                        redCount    = 0;
    int                        unknownCount = 0;
    std::chrono::milliseconds  evalDuration{0};

    bool IsReleaseReady() const { return overall == ReadinessLevel::Green; }
    bool IsConditionallyReady() const {
        return overall == ReadinessLevel::Green || overall == ReadinessLevel::Yellow;
    }

    std::vector<std::string> AllBlockers() const {
        std::vector<std::string> all;
        for (auto& g : gates)
            for (auto& b : g.blockers)
                all.push_back("[" + std::string(GateCategoryName(g.category)) + "] " + b);
        return all;
    }
};

// ── Dashboard checklist items ──────────────────────────────────────────────

struct ChecklistItem {
    std::string    description;
    GateCategory   category;
    bool           checked   = false;
    bool           required  = true;     // required for Green status
    std::string    evidence;             // link or description of verification
};

// ── Release Readiness Dashboard ────────────────────────────────────────────

class ReleaseReadinessDashboard {
public:
    ReleaseReadinessDashboard() { SetDefaultChecklist(); }

    // Set release candidate info
    void SetCandidate(ReleaseCandidate candidate) {
        m_candidate = std::move(candidate);
    }

    // Submit a gate evaluation
    void SubmitGate(GateStatus gate) {
        gate.lastEvaluated = std::chrono::system_clock::now();
        m_gates[gate.category] = std::move(gate);
    }

    // Submit a checklist completion
    void CheckItem(const std::string& description, const std::string& evidence = "") {
        for (auto& item : m_checklist) {
            if (item.description == description) {
                item.checked  = true;
                item.evidence = evidence;
                break;
            }
        }
    }

    // Add custom checklist item
    void AddChecklistItem(ChecklistItem item) {
        m_checklist.push_back(std::move(item));
    }

    // Evaluate overall readiness
    DashboardResult Evaluate() const {
        auto start = std::chrono::steady_clock::now();
        DashboardResult result;
        result.candidate = m_candidate;

        for (auto& [cat, gate] : m_gates) {
            result.gates.push_back(gate);
            switch (gate.level) {
                case ReadinessLevel::Green:   result.greenCount++;   break;
                case ReadinessLevel::Yellow:  result.yellowCount++;  break;
                case ReadinessLevel::Red:     result.redCount++;     break;
                case ReadinessLevel::Unknown: result.unknownCount++; break;
            }
        }

        // Check required checklist items
        for (auto& item : m_checklist) {
            if (item.required && !item.checked) {
                result.redCount++;
                GateStatus unmet;
                unmet.category = item.category;
                unmet.level    = ReadinessLevel::Red;
                unmet.summary  = "Checklist item not completed";
                unmet.blockers.push_back("Missing: " + item.description);
            }
        }

        // Determine overall
        if (result.redCount > 0)
            result.overall = ReadinessLevel::Red;
        else if (result.unknownCount > 0 || result.yellowCount > 0)
            result.overall = ReadinessLevel::Yellow;
        else
            result.overall = ReadinessLevel::Green;

        auto end = std::chrono::steady_clock::now();
        result.evalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return result;
    }

    // Get pending checklist items
    std::vector<ChecklistItem> PendingItems() const {
        std::vector<ChecklistItem> pending;
        for (auto& item : m_checklist)
            if (!item.checked) pending.push_back(item);
        return pending;
    }

    // Generate dashboard report
    static std::string FormatReport(const DashboardResult& r) {
        std::string rpt;
        rpt += "╔══════════════════════════════════════════╗\n";
        rpt += "║     Release Readiness Dashboard          ║\n";
        rpt += "╚══════════════════════════════════════════╝\n\n";

        rpt += "Candidate: " + r.candidate.version;
        if (!r.candidate.commitHash.empty())
            rpt += " (" + r.candidate.commitHash.substr(0, 8) + ")";
        rpt += "\n";
        rpt += "Platform:  " + r.candidate.targetPlatform + "\n";
        rpt += "Signed:    " + std::string(r.candidate.isSigned ? "Yes" : "No") + "\n\n";

        rpt += "Overall: " + LevelLabel(r.overall) + " "
            + LevelIcon(r.overall) + "\n\n";

        rpt += "Gate Summary:\n";
        rpt += "  Green:   " + std::to_string(r.greenCount) + "\n";
        rpt += "  Yellow:  " + std::to_string(r.yellowCount) + "\n";
        rpt += "  Red:     " + std::to_string(r.redCount) + "\n";
        rpt += "  Unknown: " + std::to_string(r.unknownCount) + "\n\n";

        for (auto& g : r.gates) {
            rpt += LevelIcon(g.level) + " " + GateCategoryName(g.category)
                + " — " + g.summary;
            if (g.totalChecks > 0) {
                char buf[32];
                snprintf(buf, sizeof(buf), " (%.0f%%)", g.PassRate());
                rpt += buf;
            }
            rpt += "\n";
            for (auto& b : g.blockers)
                rpt += "    BLOCKER: " + b + "\n";
            for (auto& w : g.warnings)
                rpt += "    WARNING: " + w + "\n";
        }

        auto blockers = r.AllBlockers();
        if (!blockers.empty()) {
            rpt += "\nRelease Blockers:\n";
            for (auto& b : blockers) rpt += "  - " + b + "\n";
        }

        return rpt;
    }

    const std::vector<ChecklistItem>& Checklist() const { return m_checklist; }

private:
    void SetDefaultChecklist() {
        m_checklist = {
            {"Zero build warnings in Release", GateCategory::Build, false, true},
            {"All unit tests passing", GateCategory::Tests, false, true},
            {"Performance KPIs within thresholds", GateCategory::Performance, false, true},
            {"Version strings consistent across docs", GateCategory::VersionDrift, false, true},
            {"MSI/MSIX package builds successfully", GateCategory::Packaging, false, true},
            {"Binaries code-signed", GateCategory::Packaging, false, false},
            {"Release notes written", GateCategory::Documentation, false, true},
            {"Dependency CVE audit passed", GateCategory::Security, false, true},
            {"clang-tidy clean", GateCategory::CodeQuality, false, false},
            {"MASTER_PLAN.md updated", GateCategory::Documentation, false, true}
        };
    }

    static std::string LevelLabel(ReadinessLevel l) {
        switch (l) {
            case ReadinessLevel::Green:   return "GO";
            case ReadinessLevel::Yellow:  return "CONDITIONAL";
            case ReadinessLevel::Red:     return "NO-GO";
            case ReadinessLevel::Unknown: return "PENDING";
        }
        return "?";
    }

    static std::string LevelIcon(ReadinessLevel l) {
        switch (l) {
            case ReadinessLevel::Green:   return "[OK]";
            case ReadinessLevel::Yellow:  return "[!!]";
            case ReadinessLevel::Red:     return "[XX]";
            case ReadinessLevel::Unknown: return "[??]";
        }
        return "[??]";
    }

    ReleaseCandidate m_candidate;
    std::map<GateCategory, GateStatus> m_gates;
    std::vector<ChecklistItem> m_checklist;
};

} // namespace DarkThumbs
