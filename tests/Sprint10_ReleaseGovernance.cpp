// Sprint10_ReleaseGovernance.cpp
// Sprint 10: Release Governance & Packaging Tests
// Validates release infrastructure, version consistency, and packaging readiness

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

class ReleaseGovernanceTest : public ::testing::Test {
protected:
    std::string rootDir;
    
    void SetUp() override {
        // Navigate from tests/ up to project root
        rootDir = fs::current_path().string();
        // Try to find MASTER_PLAN.md to locate root
        auto searchDir = fs::current_path();
        for (int i = 0; i < 5; i++) {
            if (fs::exists(searchDir / "MASTER_PLAN.md")) {
                rootDir = searchDir.string();
                break;
            }
            searchDir = searchDir.parent_path();
        }
    }
    
    bool fileContains(const std::string& relPath, const std::string& needle) {
        auto fullPath = fs::path(rootDir) / relPath;
        if (!fs::exists(fullPath)) return false;
        std::ifstream f(fullPath.string());
        std::string content((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
        return content.find(needle) != std::string::npos;
    }
    
    bool fileExists(const std::string& relPath) {
        return fs::exists(fs::path(rootDir) / relPath);
    }
};

// =============================================================================
// Version Consistency Tests
// =============================================================================

TEST_F(ReleaseGovernanceTest, VersionInReadme) {
    EXPECT_TRUE(fileContains("README.md", "7.0.0"))
        << "README.md should reference version 7.0.0";
}

TEST_F(ReleaseGovernanceTest, VersionInMasterPlan) {
    EXPECT_TRUE(fileContains("MASTER_PLAN.md", "v7.0.0"))
        << "MASTER_PLAN.md should reference v7.0.0";
}

TEST_F(ReleaseGovernanceTest, VersionInChangelog) {
    EXPECT_TRUE(fileContains("CHANGELOG.md", "7.0.0"))
        << "CHANGELOG.md should reference version 7.0.0";
}

TEST_F(ReleaseGovernanceTest, VersionInImplementationStatus) {
    EXPECT_TRUE(fileContains(".github/standards/IMPLEMENTATION_STATUS.md", "v7.0.0"))
        << "IMPLEMENTATION_STATUS.md should reference v7.0.0";
}

// =============================================================================
// Packaging Infrastructure Tests
// =============================================================================

TEST_F(ReleaseGovernanceTest, WixInstallerDefinitionExists) {
    EXPECT_TRUE(fileExists("packaging/DarkThumbs.wxs"))
        << "WiX installer definition should exist";
}

TEST_F(ReleaseGovernanceTest, BuildInstallerScriptExists) {
    EXPECT_TRUE(fileExists("packaging/Build-Installer.ps1"))
        << "MSI build script should exist";
}

TEST_F(ReleaseGovernanceTest, PortableZipScriptExists) {
    EXPECT_TRUE(fileExists("packaging/Build-PortableZip.ps1"))
        << "Portable ZIP build script should exist";
}

TEST_F(ReleaseGovernanceTest, ChecksumScriptExists) {
    EXPECT_TRUE(fileExists("packaging/Generate-Checksums.ps1"))
        << "Checksum generation script should exist";
}

TEST_F(ReleaseGovernanceTest, MSIXManifestExists) {
    EXPECT_TRUE(fileExists("packaging/msix/AppxManifest.xml"))
        << "MSIX AppxManifest should exist";
}

TEST_F(ReleaseGovernanceTest, InnoSetupScriptExists) {
    EXPECT_TRUE(fileExists("packaging/inno/DarkThumbs-Installer.iss"))
        << "Inno Setup script should exist";
}

TEST_F(ReleaseGovernanceTest, NSISScriptExists) {
    EXPECT_TRUE(fileExists("packaging/nsis/DarkThumbs-Installer.nsi"))
        << "NSIS script should exist";
}

// =============================================================================
// Code Signing Infrastructure Tests  
// =============================================================================

TEST_F(ReleaseGovernanceTest, SignBinariesScriptExists) {
    EXPECT_TRUE(fileExists("build-scripts/Sign-Binaries.ps1"))
        << "Code signing script should exist";
}

TEST_F(ReleaseGovernanceTest, SignBinariesSupportsAzure) {
    EXPECT_TRUE(fileContains("build-scripts/Sign-Binaries.ps1", "AzureCodeSigning"))
        << "Code signing should support Azure Key Vault";
}

// =============================================================================
// CI Pipeline Tests
// =============================================================================

TEST_F(ReleaseGovernanceTest, CIBuildWorkflowExists) {
    EXPECT_TRUE(fileExists(".github/workflows/build.yml"))
        << "CI build workflow should exist";
}

TEST_F(ReleaseGovernanceTest, CIBuildV7WorkflowExists) {
    EXPECT_TRUE(fileExists(".github/workflows/build-v7.yml"))
        << "CI build v7 workflow should exist";
}

TEST_F(ReleaseGovernanceTest, CIBuildAndTestWorkflowExists) {
    EXPECT_TRUE(fileExists(".github/workflows/build-and-test.yml"))
        << "CI build-and-test workflow should exist";
}

TEST_F(ReleaseGovernanceTest, CIReleaseWorkflowExists) {
    EXPECT_TRUE(fileExists(".github/workflows/release.yml"))
        << "CI release workflow should exist";
}

TEST_F(ReleaseGovernanceTest, CICodeQualityWorkflowExists) {
    EXPECT_TRUE(fileExists(".github/workflows/code-quality.yml"))
        << "CI code quality workflow should exist";
}

TEST_F(ReleaseGovernanceTest, CIPerfRegressionWorkflowExists) {
    EXPECT_TRUE(fileExists(".github/workflows/performance-regression-gate.yml"))
        << "CI performance regression gate workflow should exist";
}

// =============================================================================
// Release Checklist & Validation Tests
// =============================================================================

TEST_F(ReleaseGovernanceTest, ReleaseChecklistScriptExists) {
    EXPECT_TRUE(fileExists("build-scripts/validation/Release-Checklist.ps1"))
        << "Release checklist script should exist";
}

TEST_F(ReleaseGovernanceTest, ReleasePipelineValidatorExists) {
    EXPECT_TRUE(fileExists("build-scripts/validation/Validate-Release-Pipeline.ps1"))
        << "Release pipeline validator should exist";
}

TEST_F(ReleaseGovernanceTest, ReleaseNotesExist) {
    EXPECT_TRUE(fileExists("docs/release-notes/RELEASE_NOTES_v7.0.0.md"))
        << "v7.0.0 release notes should exist";
}

TEST_F(ReleaseGovernanceTest, InstallerGuideExists) {
    EXPECT_TRUE(fileExists("docs/packaging/INSTALLER_GUIDE_V7.md"))
        << "Installer guide should exist";
}

// =============================================================================
// Build System Tests
// =============================================================================

TEST_F(ReleaseGovernanceTest, CMakeListsExists) {
    EXPECT_TRUE(fileExists("CMakeLists.txt"))
        << "Root CMakeLists.txt should exist";
}

TEST_F(ReleaseGovernanceTest, SolutionFileExists) {
    EXPECT_TRUE(fileExists("CBXShell.sln"))
        << "Visual Studio solution should exist";
}

TEST_F(ReleaseGovernanceTest, BuildAllScriptExists) {
    EXPECT_TRUE(fileExists("build-scripts/Build-All-And-Package.ps1"))
        << "Build-all-and-package script should exist";
}
