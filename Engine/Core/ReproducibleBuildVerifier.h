#pragma once
// ============================================================================
// ReproducibleBuildVerifier.h
// Deterministic build verification: hash comparison, timestamp stripping,
// artifact fingerprinting for reproducible builds
// ============================================================================

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <array>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <functional>

namespace ExplorerLens {

// ── Hash representation ────────────────────────────────────────────────────

struct BuildHash {
    std::array<uint8_t, 32> sha256{};   // SHA-256 digest
    std::string hexString;               // hex-encoded string

    bool operator==(const BuildHash& o) const { return sha256 == o.sha256; }
    bool operator!=(const BuildHash& o) const { return !(*this == o); }

    bool IsEmpty() const {
        return std::all_of(sha256.begin(), sha256.end(),
                           [](uint8_t b) { return b == 0; });
    }

    static BuildHash FromHex(const std::string& hex) {
        BuildHash h;
        h.hexString = hex;
        for (size_t i = 0; i + 1 < hex.size() && i / 2 < 32; i += 2) {
            h.sha256[i / 2] = static_cast<uint8_t>(
                std::stoul(hex.substr(i, 2), nullptr, 16));
        }
        return h;
    }
};

// ── Artifact types ─────────────────────────────────────────────────────────

enum class ArtifactType {
    DLL,
    EXE,
    LIB,
    PDB,
    MSI,
    Config,
    Header,
    Resource
};

inline const char* ArtifactTypeName(ArtifactType t) {
    switch (t) {
        case ArtifactType::DLL:      return "DLL";
        case ArtifactType::EXE:      return "EXE";
        case ArtifactType::LIB:      return "LIB";
        case ArtifactType::PDB:      return "PDB";
        case ArtifactType::MSI:      return "MSI";
        case ArtifactType::Config:   return "Config";
        case ArtifactType::Header:   return "Header";
        case ArtifactType::Resource: return "Resource";
    }
    return "Unknown";
}

// ── Build artifact descriptor ──────────────────────────────────────────────

struct BuildArtifact {
    std::string   path;
    ArtifactType  type        = ArtifactType::DLL;
    uint64_t      sizeBytes   = 0;
    BuildHash     contentHash;
    BuildHash     strippedHash;    // hash with timestamps/PDB paths stripped
    std::string   version;
    bool          isSigned    = false;
    std::string   compilerVersion;
    std::string   linkerVersion;
};

// ── Verification result ────────────────────────────────────────────────────

enum class VerifyStatus {
    Reproducible,       // hashes match across builds
    NonReproducible,    // hashes differ
    TimestampDrift,     // only timestamp sections differ
    MissingArtifact,    // expected artifact not found
    SizeMismatch,       // unexpected size difference
    Skipped             // excluded from verification
};

struct ArtifactVerification {
    std::string    artifactPath;
    VerifyStatus   status       = VerifyStatus::Skipped;
    BuildHash      buildAHash;
    BuildHash      buildBHash;
    int64_t        sizeDeltaBytes = 0;
    std::string    detail;
};

struct VerificationResult {
    int totalArtifacts     = 0;
    int reproducibleCount  = 0;
    int nonReproducibleCount = 0;
    int timestampDriftCount  = 0;
    int missingCount       = 0;
    int skippedCount       = 0;
    std::vector<ArtifactVerification> verifications;
    std::chrono::milliseconds duration{0};

    bool IsFullyReproducible() const {
        return nonReproducibleCount == 0 && missingCount == 0;
    }

    double ReproducibilityScore() const {
        if (totalArtifacts == 0) return 100.0;
        return (reproducibleCount + timestampDriftCount) * 100.0 / totalArtifacts;
    }
};

// ── Build manifest ─────────────────────────────────────────────────────────

struct BuildManifest {
    std::string commitHash;
    std::string branchName;
    std::string buildTimestamp;
    std::string toolchainVersion;
    std::string buildConfig;       // "Release", "Debug"
    std::string platform;          // "x64", "ARM64"
    std::vector<BuildArtifact> artifacts;

    BuildArtifact* FindArtifact(const std::string& path) {
        for (auto& a : artifacts)
            if (a.path == path) return &a;
        return nullptr;
    }
};

// ── Reproducibility policy ─────────────────────────────────────────────────

struct ReproducibilityPolicy {
    bool   stripTimestamps     = true;    // ignore PE timestamp fields
    bool   stripPDBPaths       = true;    // ignore absolute PDB paths
    bool   stripBuildMetadata  = true;    // ignore VERSIONINFO build fields
    bool   requireSigning      = false;   // signed artifacts must match
    double maxSizeDriftPct     = 1.0;     // acceptable size variance %
    std::vector<ArtifactType> requiredTypes = {
        ArtifactType::DLL, ArtifactType::EXE, ArtifactType::LIB
    };
    std::vector<std::string> excludePaths;
};

static ReproducibilityPolicy StrictPolicy() {
    ReproducibilityPolicy p;
    p.stripTimestamps = true;
    p.stripPDBPaths   = true;
    p.maxSizeDriftPct = 0.1;
    return p;
}

static ReproducibilityPolicy RelaxedPolicy() {
    ReproducibilityPolicy p;
    p.stripTimestamps    = true;
    p.stripPDBPaths      = true;
    p.stripBuildMetadata = true;
    p.maxSizeDriftPct    = 5.0;
    return p;
}

// ── Verifier ───────────────────────────────────────────────────────────────

class ReproducibleBuildVerifier {
public:
    explicit ReproducibleBuildVerifier(ReproducibilityPolicy policy = StrictPolicy())
        : m_policy(std::move(policy)) {}

    // Compare two build manifests
    VerificationResult Verify(const BuildManifest& buildA,
                              const BuildManifest& buildB) const {
        auto start = std::chrono::steady_clock::now();
        VerificationResult result;

        // Collect all paths from both builds
        std::map<std::string, std::pair<const BuildArtifact*, const BuildArtifact*>> merged;
        for (auto& a : buildA.artifacts) merged[a.path].first  = &a;
        for (auto& b : buildB.artifacts) merged[b.path].second = &b;

        for (auto& [path, pair] : merged) {
            if (IsExcluded(path)) {
                ArtifactVerification av;
                av.artifactPath = path;
                av.status = VerifyStatus::Skipped;
                result.skippedCount++;
                result.verifications.push_back(av);
                continue;
            }

            result.totalArtifacts++;
            auto av = CompareArtifact(pair.first, pair.second, path);
            switch (av.status) {
                case VerifyStatus::Reproducible:    result.reproducibleCount++;    break;
                case VerifyStatus::NonReproducible: result.nonReproducibleCount++; break;
                case VerifyStatus::TimestampDrift:  result.timestampDriftCount++;  break;
                case VerifyStatus::MissingArtifact: result.missingCount++;         break;
                case VerifyStatus::SizeMismatch:    result.nonReproducibleCount++; break;
                case VerifyStatus::Skipped:         result.skippedCount++;         break;
            }
            result.verifications.push_back(av);
        }

        auto end = std::chrono::steady_clock::now();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return result;
    }

    // Generate a manifest from a list of artifacts
    static BuildManifest CreateManifest(const std::string& commit,
                                         const std::string& branch,
                                         const std::string& config,
                                         const std::vector<BuildArtifact>& artifacts) {
        BuildManifest m;
        m.commitHash  = commit;
        m.branchName  = branch;
        m.buildConfig = config;
        m.platform    = "x64";
        m.artifacts   = artifacts;
        return m;
    }

    // Generate CI report
    static std::string FormatReport(const VerificationResult& r) {
        std::string rpt;
        rpt += "=== Reproducible Build Verification ===\n";
        rpt += "Total artifacts: " + std::to_string(r.totalArtifacts) + "\n";
        rpt += "Reproducible:    " + std::to_string(r.reproducibleCount) + "\n";
        rpt += "Timestamp drift: " + std::to_string(r.timestampDriftCount) + "\n";
        rpt += "Non-reproducible:" + std::to_string(r.nonReproducibleCount) + "\n";
        rpt += "Missing:         " + std::to_string(r.missingCount) + "\n";
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", r.ReproducibilityScore());
        rpt += "Score: " + std::string(buf) + "%\n";
        rpt += "Status: " + std::string(r.IsFullyReproducible() ? "REPRODUCIBLE" : "NOT REPRODUCIBLE") + "\n";

        if (!r.verifications.empty()) {
            rpt += "\n--- Details ---\n";
            for (auto& v : r.verifications) {
                if (v.status == VerifyStatus::Skipped) continue;
                rpt += "[" + StatusLabel(v.status) + "] " + v.artifactPath;
                if (!v.detail.empty()) rpt += " — " + v.detail;
                rpt += "\n";
            }
        }
        return rpt;
    }

    const ReproducibilityPolicy& Policy() const { return m_policy; }

private:
    ArtifactVerification CompareArtifact(const BuildArtifact* a,
                                          const BuildArtifact* b,
                                          const std::string& path) const {
        ArtifactVerification av;
        av.artifactPath = path;

        if (!a || !b) {
            av.status = VerifyStatus::MissingArtifact;
            av.detail = !a ? "Missing in build A" : "Missing in build B";
            return av;
        }

        av.buildAHash = m_policy.stripTimestamps ? a->strippedHash : a->contentHash;
        av.buildBHash = m_policy.stripTimestamps ? b->strippedHash : b->contentHash;
        av.sizeDeltaBytes = static_cast<int64_t>(b->sizeBytes) - static_cast<int64_t>(a->sizeBytes);

        // Check size drift
        if (a->sizeBytes > 0) {
            double pctDrift = std::abs(av.sizeDeltaBytes) * 100.0 / a->sizeBytes;
            if (pctDrift > m_policy.maxSizeDriftPct) {
                av.status = VerifyStatus::SizeMismatch;
                char buf[64];
                snprintf(buf, sizeof(buf), "Size drift %.2f%% exceeds %.2f%% limit",
                         pctDrift, m_policy.maxSizeDriftPct);
                av.detail = buf;
                return av;
            }
        }

        // Compare hashes
        if (av.buildAHash == av.buildBHash) {
            av.status = VerifyStatus::Reproducible;
            av.detail = "Hashes match";
        } else if (a->contentHash != b->contentHash &&
                   a->strippedHash == b->strippedHash) {
            av.status = VerifyStatus::TimestampDrift;
            av.detail = "Content differs only in timestamps/metadata";
        } else {
            av.status = VerifyStatus::NonReproducible;
            av.detail = "Hash mismatch";
        }
        return av;
    }

    bool IsExcluded(const std::string& path) const {
        for (auto& pat : m_policy.excludePaths) {
            if (path.find(pat) != std::string::npos)
                return true;
        }
        return false;
    }

    static std::string StatusLabel(VerifyStatus s) {
        switch (s) {
            case VerifyStatus::Reproducible:    return "OK";
            case VerifyStatus::NonReproducible: return "DIFF";
            case VerifyStatus::TimestampDrift:  return "TS";
            case VerifyStatus::MissingArtifact: return "MISS";
            case VerifyStatus::SizeMismatch:    return "SIZE";
            case VerifyStatus::Skipped:         return "SKIP";
        }
        return "?";
    }

    ReproducibilityPolicy m_policy;
};

} // namespace ExplorerLens

