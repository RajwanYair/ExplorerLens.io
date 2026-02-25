// CIHardeningConfig.h — CI/CD Pipeline Hardening Configuration
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 379
// Copyright (c) 2026 ExplorerLens Project
//
// Defines CI pipeline configuration for hardened builds: reproducible
// builds, artifact signing, SBOM generation, dependency pinning,
// and security scanning integration.

#pragma once

#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// CI pipeline stage
enum class CIStage : uint8_t {
    Checkout       = 0,
    DependencyPin  = 1,
    Configure      = 2,
    Build          = 3,
    UnitTest       = 4,
    IntegrationTest = 5,
    StaticAnalysis = 6,
    SecurityScan   = 7,
    ArtifactSign   = 8,
    SBOMGenerate   = 9,
    Package        = 10,
    Publish        = 11,
    StageCount     = 12
};

/// Security scan tool
enum class SecurityScanner : uint8_t {
    None           = 0,
    MSDefender     = 1,   ///< Windows Defender for CI
    CppCheck       = 2,   ///< Static analysis
    PVSStudio      = 3,   ///< Deep static analysis
    Snyk           = 4,   ///< Dependency vulnerability scan
    Trivy          = 5,   ///< Container/artifact scanning
    CodeQL         = 6    ///< GitHub CodeQL
};

/// Build reproducibility settings
struct ReproducibleBuildConfig {
    bool stripTimestamps = true;         ///< Remove __DATE__, __TIME__
    bool deterministicMode = true;       ///< /Brepro for MSVC
    bool lockDependencies = true;        ///< Pin all dependency versions
    bool hashSourceFiles = true;         ///< SHA-256 of all source
    bool embedBuildId = true;            ///< Embed unique build ID in binary
    const char* buildIdFormat = "LENS-{version}-{date}-{commit:8}";
};

/// Artifact signing configuration
struct ArtifactSigningConfig {
    bool signBinaries = true;            ///< Authenticode sign .dll/.exe
    bool signInstaller = true;           ///< Sign MSI/MSIX
    bool timestampSign = true;           ///< RFC 3161 timestamp
    const char* timestampServer = "http://timestamp.digicert.com";
    const char* hashAlgorithm = "SHA256";
    const char* certSubject = "ExplorerLens";
    bool dualSign = false;               ///< SHA1 + SHA256 dual signing
};

/// CI hardening configuration
class CIHardeningConfig {
public:
    static CIHardeningConfig& Instance() {
        static CIHardeningConfig inst;
        return inst;
    }

    /// Get stage name
    static const char* StageName(CIStage s) {
        switch (s) {
            case CIStage::Checkout:        return "checkout";
            case CIStage::DependencyPin:   return "dependency-pin";
            case CIStage::Configure:       return "configure";
            case CIStage::Build:           return "build";
            case CIStage::UnitTest:        return "unit-test";
            case CIStage::IntegrationTest: return "integration-test";
            case CIStage::StaticAnalysis:  return "static-analysis";
            case CIStage::SecurityScan:    return "security-scan";
            case CIStage::ArtifactSign:    return "artifact-sign";
            case CIStage::SBOMGenerate:    return "sbom-generate";
            case CIStage::Package:         return "package";
            case CIStage::Publish:         return "publish";
            default:                        return "unknown";
        }
    }

    /// Check if a stage is enabled
    bool IsStageEnabled(CIStage stage) const {
        return (m_enabledStages & (1u << static_cast<uint32_t>(stage))) != 0;
    }

    /// Enable/disable a stage
    void SetStageEnabled(CIStage stage, bool enabled) {
        uint32_t bit = 1u << static_cast<uint32_t>(stage);
        if (enabled) m_enabledStages |= bit;
        else m_enabledStages &= ~bit;
    }

    /// MSVC compiler hardening flags
    static const char* GetHardenedCompileFlags() {
        return "/sdl "       // Security Development Lifecycle checks
               "/GS "        // Buffer security check
               "/guard:cf "  // Control Flow Guard
               "/guard:ehcont " // EH continuation metadata
               "/Qspectre "  // Spectre mitigation
               "/DYNAMICBASE " // ASLR
               "/CETCOMPAT"; // CET Shadow Stack
    }

    /// MSVC linker hardening flags
    static const char* GetHardenedLinkFlags() {
        return "/NXCOMPAT "       // DEP
               "/DYNAMICBASE "    // ASLR
               "/HIGHENTROPYVA "  // High-entropy ASLR
               "/GUARD:CF "       // CFG
               "/CETCOMPAT "      // CET
               "/INTEGRITYCHECK"; // Force integrity check
    }

    /// CMake flags for hardened build
    static const char* GetCMakeHardenFlags() {
        return "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL "
               "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON "
               "-DCMAKE_BUILD_TYPE=Release";
    }

    ReproducibleBuildConfig reproducible;
    ArtifactSigningConfig signing;

    /// Security scanners to run
    SecurityScanner primaryScanner = SecurityScanner::CodeQL;
    SecurityScanner secondaryScanner = SecurityScanner::CppCheck;

    /// Maximum allowed vulnerabilities by severity
    uint32_t maxCriticalVulns = 0;   ///< Build fails if > 0
    uint32_t maxHighVulns = 0;       ///< Build fails if > 0
    uint32_t maxMediumVulns = 5;     ///< Warning threshold
    uint32_t maxLowVulns = 20;       ///< Info only

    /// Dependency pinning
    bool pinVcpkgBaseline = true;
    bool pinSDKVersion = true;
    const char* pinnedWindowsSDK = "10.0.26100.0";
    const char* pinnedMSVCToolset = "14.50.35717";

private:
    CIHardeningConfig()
        : m_enabledStages(0xFFFFFFFF) {} // All stages enabled by default

    uint32_t m_enabledStages;
};

} // namespace Engine
} // namespace ExplorerLens
