#pragma once
//==============================================================================
// VersionDriftGate.h
// Automated version consistency enforcement across documentation and code.
// Detects stale version references and enforces canonical version strings.
//
// Usage:
// #include "Core/VersionDriftGate.h"
// auto gate = ExplorerLens::VersionDrift::VersionDriftGate::Create("7.1.0");
// gate.RegisterSource("MASTER_PLAN.md", "7.1.0");
// auto report = gate.Validate();
//==============================================================================

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <numeric>
#include <regex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace VersionDrift {

/// Semantic version with parse, compare, and format support
struct GateSemVer
{
    int major = 0;
    int minor = 0;
    int patch = 0;
    std::string preRelease;  // e.g., "beta.1"
    std::string buildMeta;   // e.g., "20260218"

    static GateSemVer Parse(const std::string& versionStr)
    {
        GateSemVer v;
        std::regex re(R"((\d+)\.(\d+)\.(\d+)(?:-([a-zA-Z0-9.]+))?(?:\+([a-zA-Z0-9.]+))?)");
        std::smatch m;
        if (std::regex_match(versionStr, m, re)) {
            v.major = std::stoi(m[1].str());
            v.minor = std::stoi(m[2].str());
            v.patch = std::stoi(m[3].str());
            if (m[4].matched)
                v.preRelease = m[4].str();
            if (m[5].matched)
                v.buildMeta = m[5].str();
        }
        return v;
    }

    std::string ToString() const
    {
        std::string s = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
        if (!preRelease.empty())
            s += "-" + preRelease;
        if (!buildMeta.empty())
            s += "+" + buildMeta;
        return s;
    }

    bool operator==(const GateSemVer& o) const
    {
        return major == o.major && minor == o.minor && patch == o.patch && preRelease == o.preRelease;
    }
    bool operator!=(const GateSemVer& o) const
    {
        return !(*this == o);
    }
    bool operator<(const GateSemVer& o) const
    {
        if (major != o.major)
            return major < o.major;
        if (minor != o.minor)
            return minor < o.minor;
        if (patch != o.patch)
            return patch < o.patch;
        // Pre-release < release; alpha < beta < rc
        if (preRelease.empty() && !o.preRelease.empty())
            return false;
        if (!preRelease.empty() && o.preRelease.empty())
            return true;
        return preRelease < o.preRelease;
    }
    bool operator>(const GateSemVer& o) const
    {
        return o < *this;
    }
    bool operator<=(const GateSemVer& o) const
    {
        return !(o < *this);
    }
    bool operator>=(const GateSemVer& o) const
    {
        return !(*this < o);
    }

    bool IsNewerThan(const GateSemVer& o) const
    {
        return *this > o;
    }
    bool IsOlderThan(const GateSemVer& o) const
    {
        return *this < o;
    }
};

/// Drift severity classification
enum class GateDriftSeverity : uint8_t {
    None = 0,      // Version matches canonical
    Minor = 1,     // Patch version behind (e.g., 7.1.0 vs 7.1.1)
    Moderate = 2,  // Minor version behind (e.g., 7.0.0 vs 7.1.0)
    Major = 3,     // Major version behind (e.g., 6.x vs 7.x)
    Critical = 4   // Multiple major versions behind or contradictory
};

/// A single version reference found in a source file
struct VersionReference
{
    std::string filePath;
    int lineNumber = 0;
    std::string foundVersion;
    std::string context;  // Surrounding text for debugging
    GateDriftSeverity severity = GateDriftSeverity::None;
};

/// Source file registration for drift checking
struct VersionSource
{
    std::string filePath;
    std::string declaredVersion;
    std::string category;      // "doc", "code", "script", "ci"
    bool isCanonical = false;  // True for MASTER_PLAN.md
    std::vector<VersionReference> references;
};

/// Drift validation report
struct DriftReport
{
    std::string canonicalVersion;
    int totalSources = 0;
    int driftingSources = 0;
    int totalReferences = 0;
    int staleReferences = 0;
    GateDriftSeverity worstSeverity = GateDriftSeverity::None;
    std::vector<VersionReference> violations;
    std::chrono::system_clock::time_point timestamp;

    bool IsClean() const
    {
        return driftingSources == 0 && staleReferences == 0;
    }

    double CompliancePercent() const
    {
        if (totalReferences == 0)
            return 100.0;
        return 100.0 * (totalReferences - staleReferences) / totalReferences;
    }

    std::string SeverityText() const
    {
        switch (worstSeverity) {
            case GateDriftSeverity::None:
                return "CLEAN";
            case GateDriftSeverity::Minor:
                return "MINOR_DRIFT";
            case GateDriftSeverity::Moderate:
                return "MODERATE_DRIFT";
            case GateDriftSeverity::Major:
                return "MAJOR_DRIFT";
            case GateDriftSeverity::Critical:
                return "CRITICAL_DRIFT";
            default:
                return "UNKNOWN";
        }
    }
};

/// Known stale version patterns to detect
struct StaleVersionPattern
{
    std::string pattern;      // Regex pattern
    std::string description;  // Human-readable explanation
    GateDriftSeverity severity = GateDriftSeverity::Moderate;
};

/// Gate policy for CI enforcement
struct GatePolicy
{
    GateDriftSeverity maxAllowed = GateDriftSeverity::Minor;
    double minCompliancePercent = 95.0;
    bool failOnAnyMajorDrift = true;
    bool failOnCanonicalMismatch = true;
    std::vector<std::string> exemptFiles;  // Files allowed to have old versions

    bool IsExempt(const std::string& filePath) const
    {
        return std::find(exemptFiles.begin(), exemptFiles.end(), filePath) != exemptFiles.end();
    }
};

/// Gate check result
struct DriftGateResult
{
    bool passed = false;
    std::string reason;
    DriftReport report;
    GatePolicy policy;
};

/// Main version drift detection and enforcement engine
class VersionDriftGate
{
  public:
    static VersionDriftGate Create(const std::string& canonicalVersion)
    {
        VersionDriftGate gate;
        gate.m_canonicalVersion = GateSemVer::Parse(canonicalVersion);
        gate.m_canonicalString = canonicalVersion;
        gate.InitializeStalePatterns();
        return gate;
    }

    // ─── Source Registration ─────────────────────────────────────
    void RegisterSource(const std::string& filePath, const std::string& version, const std::string& category = "doc",
                        bool isCanonical = false)
    {
        VersionSource src;
        src.filePath = filePath;
        src.declaredVersion = version;
        src.category = category;
        src.isCanonical = isCanonical;
        m_sources[filePath] = src;
    }

    void AddReference(const std::string& filePath, int lineNumber, const std::string& foundVersion,
                      const std::string& context = "")
    {
        VersionReference ref;
        ref.filePath = filePath;
        ref.lineNumber = lineNumber;
        ref.foundVersion = foundVersion;
        ref.context = context;
        ref.severity = ClassifyDrift(foundVersion);
        m_sources[filePath].references.push_back(ref);
    }

    // ─── Validation ──────────────────────────────────────────────
    DriftReport Validate() const
    {
        DriftReport report;
        report.canonicalVersion = m_canonicalString;
        report.timestamp = std::chrono::system_clock::now();
        report.totalSources = static_cast<int>(m_sources.size());

        for (const auto& [path, src] : m_sources) {
            auto declVer = GateSemVer::Parse(src.declaredVersion);
            bool sourceHasDrift = (declVer != m_canonicalVersion);
            if (sourceHasDrift)
                report.driftingSources++;

            for (const auto& ref : src.references) {
                report.totalReferences++;
                if (ref.severity > GateDriftSeverity::None) {
                    report.staleReferences++;
                    report.violations.push_back(ref);
                    if (ref.severity > report.worstSeverity) {
                        report.worstSeverity = ref.severity;
                    }
                }
            }
        }
        return report;
    }

    DriftGateResult CheckGate(const GatePolicy& policy) const
    {
        DriftGateResult result;
        result.report = Validate();
        result.policy = policy;
        result.passed = true;
        result.reason = "All version references are consistent";

        if (result.report.worstSeverity > policy.maxAllowed) {
            result.passed = false;
            result.reason = "Drift severity " + result.report.SeverityText() + " exceeds policy max "
                            + std::to_string(static_cast<int>(policy.maxAllowed));
        }
        if (result.report.CompliancePercent() < policy.minCompliancePercent) {
            result.passed = false;
            result.reason = "Compliance " + std::to_string(result.report.CompliancePercent()) + "% below minimum "
                            + std::to_string(policy.minCompliancePercent) + "%";
        }
        if (policy.failOnAnyMajorDrift && result.report.worstSeverity >= GateDriftSeverity::Major) {
            result.passed = false;
            result.reason = "Major version drift detected — policy requires zero major drift";
        }
        return result;
    }

    // ─── Stale Pattern Matching ──────────────────────────────────
    GateDriftSeverity ClassifyDrift(const std::string& foundVersion) const
    {
        auto found = GateSemVer::Parse(foundVersion);
        if (found == m_canonicalVersion)
            return GateDriftSeverity::None;
        if (found.major != m_canonicalVersion.major) {
            int delta = m_canonicalVersion.major - found.major;
            return (delta > 1) ? GateDriftSeverity::Critical : GateDriftSeverity::Major;
        }
        if (found.minor != m_canonicalVersion.minor)
            return GateDriftSeverity::Moderate;
        if (found.patch != m_canonicalVersion.patch)
            return GateDriftSeverity::Minor;
        return GateDriftSeverity::None;
    }

    // ─── Accessors ───────────────────────────────────────────────
    const GateSemVer& CanonicalVersion() const
    {
        return m_canonicalVersion;
    }
    const std::string& CanonicalString() const
    {
        return m_canonicalString;
    }
    size_t SourceCount() const
    {
        return m_sources.size();
    }
    const std::vector<StaleVersionPattern>& StalePatterns() const
    {
        return m_stalePatterns;
    }

  private:
    GateSemVer m_canonicalVersion;
    std::string m_canonicalString;
    std::map<std::string, VersionSource> m_sources;
    std::vector<StaleVersionPattern> m_stalePatterns;

    void InitializeStalePatterns()
    {
        m_stalePatterns = {
            {R"(v5\.\d+\.\d+)", "v5.x reference (2+ major versions behind)", GateDriftSeverity::Critical},
            {R"(v6\.\d+\.\d+)", "v6.x reference (1 major version behind)", GateDriftSeverity::Major},
            {R"(v7\.0\.\d+)", "v7.0.x reference (minor version behind)", GateDriftSeverity::Moderate},
            {R"(Version:\s*6)", "Version header showing v6", GateDriftSeverity::Major},
            {R"(Version:\s*5)", "Version header showing v5", GateDriftSeverity::Critical},
        };
    }
};

/// Preset gate policies for different environments
struct GatePolicies
{
    static GatePolicy Strict()
    {
        GatePolicy p;
        p.maxAllowed = GateDriftSeverity::None;
        p.minCompliancePercent = 100.0;
        p.failOnAnyMajorDrift = true;
        p.failOnCanonicalMismatch = true;
        return p;
    }
    static GatePolicy CI()
    {
        GatePolicy p;
        p.maxAllowed = GateDriftSeverity::Minor;
        p.minCompliancePercent = 95.0;
        p.failOnAnyMajorDrift = true;
        p.failOnCanonicalMismatch = true;
        return p;
    }
    static GatePolicy Permissive()
    {
        GatePolicy p;
        p.maxAllowed = GateDriftSeverity::Moderate;
        p.minCompliancePercent = 80.0;
        p.failOnAnyMajorDrift = false;
        p.failOnCanonicalMismatch = false;
        return p;
    }
};

}  // namespace VersionDrift
}  // namespace ExplorerLens
