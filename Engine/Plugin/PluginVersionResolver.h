// PluginVersionResolver.h — Semantic Version Resolution and Conflict Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Resolves dependency graphs for plugin packages using SemVer comparisons,
// detects version conflicts and missing dependencies before installation.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "PluginPackageManifest.h"

namespace ExplorerLens {
namespace Engine {

// ---- Semantic Version -------------------------------------------------------

struct SemVer {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
    std::string preRelease;  // "alpha.1", "rc.2", etc.
    std::string build;       // Build metadata (ignored in comparison)

    static bool Parse(const std::string& versionStr, SemVer& out);
    std::string ToString() const;

    bool operator<(const SemVer& o) const;
    bool operator==(const SemVer& o) const;
    bool operator<=(const SemVer& o) const { return *this < o || *this == o; }
    bool operator>=(const SemVer& o) const { return !(*this < o); }
};

// ---- Resolution Result ------------------------------------------------------

enum class ResolutionStatus {
    Resolved           = 0,
    ConflictDetected   = 1,   // Two plugins require incompatible versions of a dep
    MissingDependency  = 2,
    EngineVersionError = 3,
    CircularDependency = 4,
};

struct ViolationRecord {
    std::string pluginId;
    std::string dependencyId;
    std::string requiredRange;   // e.g. ">=1.0.0 <2.0.0"
    std::string installedVersion;
    ResolutionStatus reason = ResolutionStatus::MissingDependency;
};

struct ResolutionResult {
    ResolutionStatus        status = ResolutionStatus::Resolved;
    std::vector<std::string> installOrder;  // Topological order for install
    std::vector<ViolationRecord> violations;
};

// ---- PluginVersionResolver --------------------------------------------------

class PluginVersionResolver {
public:
    PluginVersionResolver();
    ~PluginVersionResolver();

    // Add a plugin manifest to the resolver's universe.
    void AddPlugin(const PluginPackageManifest& manifest);

    // Set the installed engine version for compatibility checks.
    void SetEngineVersion(const std::string& engineVersion);

    // Resolve all dependencies and order the install sequence.
    ResolutionResult Resolve() const;

    // Check if a specific version string satisfies a range spec.
    static bool Satisfies(const std::string& version, const std::string& rangeSpec);

    // Compare two version strings: -1 / 0 / +1.
    static int Compare(const std::string& a, const std::string& b);

    void Clear();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
