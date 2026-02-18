#pragma once
// ============================================================================
// VersionDriftDetector.h — Sprint 145
// Automated version-drift CI detection across docs, headers, and configs
// ============================================================================

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <regex>
#include <algorithm>
#include <numeric>

namespace DarkThumbs {

// ── Version component ──────────────────────────────────────────────────────

struct SemanticVersion {
    int major = 0;
    int minor = 0;
    int patch = 0;
    std::string preRelease;   // e.g. "beta.1"
    std::string buildMeta;    // e.g. "20260218"

    bool operator==(const SemanticVersion& o) const {
        return major == o.major && minor == o.minor && patch == o.patch
            && preRelease == o.preRelease;
    }
    bool operator!=(const SemanticVersion& o) const { return !(*this == o); }

    bool operator<(const SemanticVersion& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        if (patch != o.patch) return patch < o.patch;
        if (preRelease.empty() && !o.preRelease.empty()) return false;
        if (!preRelease.empty() && o.preRelease.empty()) return true;
        return preRelease < o.preRelease;
    }

    std::string ToString() const {
        std::string s = std::to_string(major) + "." +
                        std::to_string(minor) + "." +
                        std::to_string(patch);
        if (!preRelease.empty()) s += "-" + preRelease;
        if (!buildMeta.empty())  s += "+" + buildMeta;
        return s;
    }

    static SemanticVersion Parse(const std::string& str) {
        SemanticVersion v;
        // Pattern: major.minor.patch[-pre][+meta]
        std::regex re(R"((\d+)\.(\d+)\.(\d+)(?:-([a-zA-Z0-9.]+))?(?:\+([a-zA-Z0-9.]+))?)");
        std::smatch m;
        if (std::regex_search(str, m, re)) {
            v.major = std::stoi(m[1].str());
            v.minor = std::stoi(m[2].str());
            v.patch = std::stoi(m[3].str());
            if (m[4].matched) v.preRelease = m[4].str();
            if (m[5].matched) v.buildMeta  = m[5].str();
        }
        return v;
    }
};

// ── Drift entry ────────────────────────────────────────────────────────────

enum class DriftSeverity {
    Info,       // Minor cosmetic drift
    Warning,    // Version behind by patch
    Error,      // Version behind by minor or major
    Critical    // Version conflict in release-facing artifact
};

enum class ArtifactKind {
    Header,
    Documentation,
    BuildScript,
    Config,
    Installer,
    ReleaseNote,
    TestFixture
};

struct DriftEntry {
    std::string filePath;
    int         lineNumber = 0;
    std::string foundVersion;
    std::string expectedVersion;
    DriftSeverity severity = DriftSeverity::Warning;
    ArtifactKind  artifact = ArtifactKind::Documentation;
    std::string   context;     // surrounding text snippet
};

// ── Scan policy ────────────────────────────────────────────────────────────

struct DriftScanPolicy {
    SemanticVersion canonicalVersion;         // the "truth" version
    bool  allowPatchDrift   = true;           // patch mismatch → warning not error
    bool  checkPreRelease   = false;          // compare pre-release tags
    bool  checkBuildMeta    = false;          // compare build metadata
    int   maxAcceptableDrift = 1;             // max patch-level gap before error
    std::vector<std::string> excludePatterns; // glob patterns to skip
    std::vector<std::string> includePatterns; // glob patterns to scan
};

static DriftScanPolicy DefaultPolicy() {
    DriftScanPolicy p;
    p.canonicalVersion = SemanticVersion{7, 1, 0};
    p.includePatterns  = {"*.md", "*.h", "*.ps1", "*.cmake", "*.wxs"};
    p.excludePatterns  = {"external/*", "build/*", "x64/*"};
    return p;
}

// ── Scan result ────────────────────────────────────────────────────────────

struct DriftScanResult {
    int totalFilesScanned = 0;
    int totalDriftEntries = 0;
    int infoCount    = 0;
    int warningCount = 0;
    int errorCount   = 0;
    int criticalCount = 0;
    std::vector<DriftEntry> entries;
    std::chrono::milliseconds scanDuration{0};

    bool IsClean() const { return errorCount == 0 && criticalCount == 0; }

    int Score() const {
        // 100 = pristine, deductions for drift
        int score = 100;
        score -= criticalCount * 25;
        score -= errorCount    * 10;
        score -= warningCount  * 3;
        score -= infoCount     * 1;
        return std::max(0, score);
    }
};

// ── Detector ───────────────────────────────────────────────────────────────

class VersionDriftDetector {
public:
    explicit VersionDriftDetector(DriftScanPolicy policy = DefaultPolicy())
        : m_policy(std::move(policy)) {}

    // Register an artifact for scanning
    void AddArtifact(const std::string& path, ArtifactKind kind) {
        m_artifacts.push_back({path, kind});
    }

    // Scan a single content string as if from a file
    std::vector<DriftEntry> ScanContent(const std::string& path,
                                         const std::string& content,
                                         ArtifactKind kind) const {
        std::vector<DriftEntry> drifts;
        std::regex versionRe(R"(v?(\d+\.\d+\.\d+(?:-[a-zA-Z0-9.]+)?))");

        int lineNum = 0;
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            ++lineNum;
            std::sregex_iterator it(line.begin(), line.end(), versionRe);
            std::sregex_iterator end;
            for (; it != end; ++it) {
                auto found = SemanticVersion::Parse((*it)[1].str());
                if (found.major == 0 && found.minor == 0 && found.patch == 0)
                    continue; // skip 0.0.0
                if (found == m_policy.canonicalVersion)
                    continue; // matches — no drift

                DriftEntry e;
                e.filePath        = path;
                e.lineNumber      = lineNum;
                e.foundVersion    = found.ToString();
                e.expectedVersion = m_policy.canonicalVersion.ToString();
                e.artifact        = kind;
                e.context         = line.substr(0, 120);
                e.severity        = ClassifySeverity(found);
                drifts.push_back(std::move(e));
            }
        }
        return drifts;
    }

    // Run scan over all registered artifacts (using content provider)
    DriftScanResult Scan(std::function<std::string(const std::string&)> contentProvider) {
        auto start = std::chrono::steady_clock::now();
        DriftScanResult result;

        for (auto& [path, kind] : m_artifacts) {
            if (IsExcluded(path)) continue;
            result.totalFilesScanned++;

            auto content = contentProvider(path);
            auto entries = ScanContent(path, content, kind);
            for (auto& e : entries) {
                switch (e.severity) {
                    case DriftSeverity::Info:     result.infoCount++;     break;
                    case DriftSeverity::Warning:  result.warningCount++;  break;
                    case DriftSeverity::Error:    result.errorCount++;    break;
                    case DriftSeverity::Critical: result.criticalCount++; break;
                }
                result.entries.push_back(std::move(e));
            }
        }

        result.totalDriftEntries = static_cast<int>(result.entries.size());
        auto end = std::chrono::steady_clock::now();
        result.scanDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return result;
    }

    // Generate CI-friendly report
    static std::string FormatReport(const DriftScanResult& r) {
        std::string report;
        report += "=== Version Drift CI Report ===\n";
        report += "Files scanned: " + std::to_string(r.totalFilesScanned) + "\n";
        report += "Drift entries: " + std::to_string(r.totalDriftEntries) + "\n";
        report += "  Critical: " + std::to_string(r.criticalCount) + "\n";
        report += "  Error:    " + std::to_string(r.errorCount) + "\n";
        report += "  Warning:  " + std::to_string(r.warningCount) + "\n";
        report += "  Info:     " + std::to_string(r.infoCount) + "\n";
        report += "Score: " + std::to_string(r.Score()) + "/100\n";
        report += "Status: " + std::string(r.IsClean() ? "PASS" : "FAIL") + "\n";
        if (!r.entries.empty()) {
            report += "\n--- Drift Details ---\n";
            for (auto& e : r.entries) {
                report += "[" + SeverityLabel(e.severity) + "] "
                       + e.filePath + ":" + std::to_string(e.lineNumber)
                       + " — found " + e.foundVersion
                       + " expected " + e.expectedVersion + "\n";
            }
        }
        return report;
    }

    const DriftScanPolicy& Policy() const { return m_policy; }

private:
    DriftSeverity ClassifySeverity(const SemanticVersion& found) const {
        auto& canon = m_policy.canonicalVersion;
        if (found.major != canon.major)
            return DriftSeverity::Critical;
        if (found.minor != canon.minor)
            return DriftSeverity::Error;
        int patchGap = std::abs(canon.patch - found.patch);
        if (patchGap > m_policy.maxAcceptableDrift)
            return DriftSeverity::Error;
        if (patchGap > 0)
            return m_policy.allowPatchDrift ? DriftSeverity::Warning : DriftSeverity::Error;
        return DriftSeverity::Info;
    }

    bool IsExcluded(const std::string& path) const {
        for (auto& pat : m_policy.excludePatterns) {
            // Simple prefix/contains check (full glob not needed for CI)
            auto prefix = pat.substr(0, pat.find('*'));
            if (!prefix.empty() && path.find(prefix) != std::string::npos)
                return true;
        }
        return false;
    }

    static std::string SeverityLabel(DriftSeverity s) {
        switch (s) {
            case DriftSeverity::Info:     return "INFO";
            case DriftSeverity::Warning:  return "WARN";
            case DriftSeverity::Error:    return "ERROR";
            case DriftSeverity::Critical: return "CRIT";
        }
        return "UNKNOWN";
    }

    DriftScanPolicy m_policy;
    std::vector<std::pair<std::string, ArtifactKind>> m_artifacts;
};

} // namespace DarkThumbs
