#pragma once
// Sprint 159 — ARM64 CI Integration
// CI matrix entry descriptor, workflow metadata, and documentation completeness
// checks for ARM64 build/test/deploy stages.

#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs::Platform {

// ─── CI stage types ───────────────────────────────────────────────────────────

enum class CIStage : uint32_t {
    Checkout    = 0,
    Setup       = 1,
    Build       = 2,
    UnitTest    = 3,
    Deploy      = 4,
    Validate    = 5,
};

inline std::string ToString(CIStage s) {
    switch (s) {
        case CIStage::Checkout:  return "checkout";
        case CIStage::Setup:     return "setup";
        case CIStage::Build:     return "build";
        case CIStage::UnitTest:  return "unit-test";
        case CIStage::Deploy:    return "deploy";
        case CIStage::Validate:  return "validate";
        default: return "unknown";
    }
}

// ─── CI matrix dimension ─────────────────────────────────────────────────────

struct CIMatrixDimension {
    std::string name;
    std::vector<std::string> values;
};

struct CIMatrixEntry {
    std::string     architecture;   // e.g., "ARM64"
    std::string     configuration;  // "Release" / "Debug"
    std::string     runner;         // "windows-2025" / "self-hosted"
    std::string     vsVersion;      // "18.0"
    bool            runTests        { false };
    bool            uploadArtifacts { true };

    static CIMatrixEntry ARM64BuildOnly() {
        return { "ARM64", "Release", "windows-2025", "18.0", false, true };
    }

    static CIMatrixEntry ARM64Full() {
        return { "ARM64", "Release", "self-hosted-arm64", "18.0", true, true };
    }
};

// ─── GitHub Actions workflow descriptor ──────────────────────────────────────

struct WorkflowJobDescriptor {
    std::string                     workflowFile    { ".github/workflows/arm64.yml" };
    std::string                     jobName         { "build-arm64" };
    std::vector<CIMatrixEntry>      matrix;
    std::vector<CIStage>            stages;
    bool                            continueOnError { false };
    uint32_t                        timeoutMinutes  { 60 };

    static WorkflowJobDescriptor Default() {
        WorkflowJobDescriptor d;
        d.matrix = { CIMatrixEntry::ARM64BuildOnly() };
        d.stages = {
            CIStage::Checkout,
            CIStage::Setup,
            CIStage::Build,
            CIStage::UnitTest,
        };
        return d;
    }

    std::string ToYAMLFragment() const {
        std::string yaml = "  " + jobName + ":\n";
        yaml += "    runs-on: " + (matrix.empty() ? "windows-2025" : matrix[0].runner) + "\n";
        yaml += "    timeout-minutes: " + std::to_string(timeoutMinutes) + "\n";
        yaml += "    steps:\n";
        for (const auto& s : stages)
            yaml += "      - name: " + ToString(s) + "\n";
        return yaml;
    }
};

// ─── ARM64 documentation manifest ────────────────────────────────────────────

struct ARM64DocsManifest {
    struct DocEntry {
        std::string filePath;
        std::string description;
        bool        required { true };
    };

    std::vector<DocEntry>   entries;

    static ARM64DocsManifest Required() {
        ARM64DocsManifest m;
        m.entries = {
            { "docs/ARM64_SUPPORT.md",            "ARM64 status, limitations, library matrix",  true  },
            { "cmake/toolchain-windows-arm64.cmake", "CMake ARM64 toolchain file",             true  },
            { ".github/workflows/arm64.yml",      "ARM64 CI workflow",                         true  },
            { "docs/development/sprints-v8/SPRINT_159.md", "Sprint 159 execution record",       true  },
        };
        return m;
    }

    bool AllRequiredPresent(const std::vector<std::string>& existingFiles) const {
        for (const auto& e : entries) {
            if (!e.required) continue;
            bool found = false;
            for (const auto& f : existingFiles)
                if (f == e.filePath) { found = true; break; }
            if (!found) return false;
        }
        return true;
    }
};

// ─── ARM64 CI integration summary ────────────────────────────────────────────

struct ARM64CIIntegration {
    WorkflowJobDescriptor   workflow;
    ARM64DocsManifest       docsManifest;
    bool                    masterPlanUpdated   { false };

    bool IsComplete(const std::vector<std::string>& existingFiles) const {
        return masterPlanUpdated && docsManifest.AllRequiredPresent(existingFiles);
    }

    static ARM64CIIntegration CreateDefault() {
        ARM64CIIntegration ci;
        ci.workflow = WorkflowJobDescriptor::Default();
        ci.docsManifest = ARM64DocsManifest::Required();
        ci.masterPlanUpdated = true;
        return ci;
    }
};

} // namespace DarkThumbs::Platform
