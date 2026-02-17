//==============================================================================
// DarkThumbs — Sprint 10 Tests: Release Governance & Packaging
// Tests quality gates, code signing policy, packaging validators,
// CI pipeline registry, release manifest, and artifact management.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Header under test
#include "../Engine/Utils/ReleaseGovernance.h"

using namespace DarkThumbs::Engine::Release;

//==============================================================================
// Quality Gate Tests
//==============================================================================

TEST(QualityGate, DefaultPending)
{
    QualityGate gate{"TEST", "A test gate"};
    EXPECT_EQ(gate.status, GateStatus::Pending);
    EXPECT_FALSE(gate.IsPassed());
    EXPECT_FALSE(gate.IsFailed());
}

TEST(QualityGate, StatusNames)
{
    EXPECT_STREQ(GateStatusName(GateStatus::Pending), "Pending");
    EXPECT_STREQ(GateStatusName(GateStatus::Passed),  "Passed");
    EXPECT_STREQ(GateStatusName(GateStatus::Failed),  "Failed");
    EXPECT_STREQ(GateStatusName(GateStatus::Skipped), "Skipped");
}

TEST(QualityGate, SummaryFormat)
{
    QualityGate gate{"BUILD", "Build with no errors"};
    gate.status = GateStatus::Passed;
    auto s = gate.Summary();
    EXPECT_NE(s.find("[PASS]"), std::string::npos);
    EXPECT_NE(s.find("BUILD"), std::string::npos);
}

TEST(QualityGate, FailSummary)
{
    QualityGate gate{"SIGN", "Code signing"};
    gate.status = GateStatus::Failed;
    gate.detail = "Certificate expired";
    auto s = gate.Summary();
    EXPECT_NE(s.find("[FAIL]"), std::string::npos);
    EXPECT_NE(s.find("Certificate expired"), std::string::npos);
}

//==============================================================================
// Release Artifact Tests
//==============================================================================

TEST(Artifact, TypeNames)
{
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::DLL), "DLL");
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::EXE), "EXE");
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::MSI), "MSI");
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::ZIP), "ZIP");
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::PDB), "PDB");
    EXPECT_STREQ(ArtifactTypeName(ArtifactType::MSIX), "MSIX");
}

TEST(Artifact, ValidCheck)
{
    ReleaseArtifact a;
    EXPECT_FALSE(a.IsValid());
    a.name = "CBXShell.dll";
    a.sizeBytes = 2940 * 1024;
    EXPECT_TRUE(a.IsValid());
}

TEST(Artifact, SizeMB)
{
    ReleaseArtifact a;
    a.sizeBytes = 1024 * 1024;  // exactly 1 MB
    EXPECT_EQ(a.SizeMB(), "1.00 MB");
}

TEST(Artifact, SizeLarger)
{
    ReleaseArtifact a;
    a.sizeBytes = 2940 * 1024;  // ~2.87 MB (CBXShell.dll typical)
    auto s = a.SizeMB();
    EXPECT_NE(s.find("MB"), std::string::npos);
}

//==============================================================================
// Release Checklist Tests
//==============================================================================

TEST(Checklist, DefaultGateCount)
{
    ReleaseChecklist cl;
    EXPECT_EQ(cl.TotalGates(), 14u);
}

TEST(Checklist, AllPending)
{
    ReleaseChecklist cl;
    EXPECT_EQ(cl.PendingGates(), 14u);
    EXPECT_EQ(cl.PassedGates(), 0u);
    EXPECT_EQ(cl.FailedGates(), 0u);
}

TEST(Checklist, SetPassGate)
{
    ReleaseChecklist cl;
    cl.SetGate("BUILD_SUCCESS", GateStatus::Passed, "0 errors, 0 warnings");
    EXPECT_EQ(cl.PassedGates(), 1u);
    EXPECT_EQ(cl.PendingGates(), 13u);
}

TEST(Checklist, SetFailGate)
{
    ReleaseChecklist cl;
    cl.SetGate("BINARY_SIGNED", GateStatus::Failed, "No certificate");
    EXPECT_EQ(cl.FailedGates(), 1u);
    EXPECT_TRUE(cl.HasBlockers());
}

TEST(Checklist, AllPassed)
{
    ReleaseChecklist cl;
    for (auto& g : cl.Gates())
        cl.SetGate(g.id, GateStatus::Passed);
    EXPECT_TRUE(cl.AllPassed());
    EXPECT_FALSE(cl.HasBlockers());
    EXPECT_DOUBLE_EQ(cl.PassRate(), 100.0);
}

TEST(Checklist, MixedResults)
{
    ReleaseChecklist cl;
    cl.SetGate("BUILD_SUCCESS", GateStatus::Passed);
    cl.SetGate("TEST_PASS", GateStatus::Passed);
    cl.SetGate("BINARY_SIGNED", GateStatus::Failed);
    EXPECT_FALSE(cl.AllPassed());
    EXPECT_TRUE(cl.HasBlockers());
    // 2 passed, 1 failed => 66.67%
    EXPECT_GT(cl.PassRate(), 66.0);
    EXPECT_LT(cl.PassRate(), 67.0);
}

TEST(Checklist, SkippedCountsAsPass)
{
    ReleaseChecklist cl;
    for (auto& g : cl.Gates())
        cl.SetGate(g.id, GateStatus::Skipped);
    EXPECT_TRUE(cl.AllPassed());
}

TEST(Checklist, GetFailed)
{
    ReleaseChecklist cl;
    cl.SetGate("MSI_INSTALL", GateStatus::Failed, "WiX not found");
    cl.SetGate("PORTABLE_ZIP", GateStatus::Failed, "Missing files");
    auto failed = cl.GetFailed();
    EXPECT_EQ(failed.size(), 2u);
}

TEST(Checklist, Report)
{
    ReleaseChecklist cl;
    cl.SetGate("BUILD_SUCCESS", GateStatus::Passed);
    auto md = cl.GenerateReport();
    EXPECT_NE(md.find("Release Checklist Report"), std::string::npos);
    EXPECT_NE(md.find("BUILD_SUCCESS"), std::string::npos);
    EXPECT_NE(md.find("Pass Rate"), std::string::npos);
}

//==============================================================================
// Code Signing Policy Tests
//==============================================================================

TEST(Signing, DefaultNotConfigured)
{
    CodeSigningPolicy policy;
    EXPECT_FALSE(policy.IsConfigured());
    EXPECT_EQ(policy.method, SigningMethod::None);
}

TEST(Signing, MethodNames)
{
    EXPECT_STREQ(SigningMethodName(SigningMethod::None), "None");
    EXPECT_STREQ(SigningMethodName(SigningMethod::EV), "EV Certificate");
    EXPECT_STREQ(SigningMethodName(SigningMethod::AzureKeyVault), "Azure Key Vault");
}

TEST(Signing, EVConfigured)
{
    CodeSigningPolicy policy;
    policy.method = SigningMethod::EV;
    EXPECT_TRUE(policy.IsConfigured());
}

TEST(Signing, RequiredBinaries)
{
    CodeSigningPolicy policy;
    EXPECT_EQ(policy.RequiredCount(), 3u);
    EXPECT_TRUE(policy.RequiresSigning("CBXShell.dll"));
    EXPECT_TRUE(policy.RequiresSigning("CBXManager.exe"));
    EXPECT_TRUE(policy.RequiresSigning("PluginHost.exe"));
    EXPECT_FALSE(policy.RequiresSigning("readme.txt"));
}

TEST(Signing, TimestampDefaults)
{
    CodeSigningPolicy policy;
    EXPECT_TRUE(policy.timestampEnabled);
    EXPECT_EQ(policy.hashAlgorithm, "SHA256");
    EXPECT_NE(policy.timestampUrl.find("digicert"), std::string::npos);
}

//==============================================================================
// Packaging Validator Tests
//==============================================================================

TEST(Packaging, PackageTypeNames)
{
    EXPECT_STREQ(PackageTypeName(PackageType::MSI), "MSI");
    EXPECT_STREQ(PackageTypeName(PackageType::PortableZip), "Portable ZIP");
    EXPECT_STREQ(PackageTypeName(PackageType::MSIX), "MSIX");
}

TEST(Packaging, RequiredFiles)
{
    auto files = PackagingValidator::RequiredFiles();
    EXPECT_GE(files.size(), 6u);
}

TEST(Packaging, MSIValidation)
{
    PackagingValidator pv;
    auto result = pv.ValidateMSI();
    EXPECT_TRUE(result.IsReady());
    EXPECT_EQ(result.type, PackageType::MSI);
    EXPECT_GT(result.packageSize, 0u);
}

TEST(Packaging, PortableZipValidation)
{
    PackagingValidator pv;
    auto result = pv.ValidatePortableZip();
    EXPECT_TRUE(result.IsReady());
    EXPECT_EQ(result.type, PackageType::PortableZip);
}

TEST(Packaging, MSIXValidation)
{
    PackagingValidator pv;
    auto result = pv.ValidateMSIX();
    EXPECT_TRUE(result.IsReady());
    EXPECT_EQ(result.type, PackageType::MSIX);
}

TEST(Packaging, PortableZipNoInstallNeeded)
{
    PackageValidation pv;
    pv.type = PackageType::PortableZip;
    pv.canBuild = true;
    pv.containsAllFiles = true;
    // installClean/uninstallClean not needed for portable
    EXPECT_TRUE(pv.IsReady());
}

//==============================================================================
// CI Pipeline Tests
//==============================================================================

TEST(CIPipeline, RegistryPopulated)
{
    CIPipelineRegistry reg;
    EXPECT_EQ(reg.TotalPipelines(), 6u);
}

TEST(CIPipeline, RequiredPipelines)
{
    CIPipelineRegistry reg;
    EXPECT_GE(reg.RequiredPipelines(), 3u);
}

TEST(CIPipeline, AllEnabled)
{
    CIPipelineRegistry reg;
    EXPECT_EQ(reg.EnabledPipelines(), reg.TotalPipelines());
}

TEST(CIPipeline, TotalSteps)
{
    CIPipelineRegistry reg;
    EXPECT_GE(reg.TotalSteps(), 25u);
}

TEST(CIPipeline, BuildPipelineSteps)
{
    CIPipelineRegistry reg;
    auto& pipelines = reg.AllPipelines();
    ASSERT_GE(pipelines.size(), 1u);
    EXPECT_EQ(pipelines[0].workflowName, "build");
    EXPECT_TRUE(pipelines[0].isRequired);
    EXPECT_GE(pipelines[0].StepCount(), 4u);
}

//==============================================================================
// Release Manifest Tests
//==============================================================================

TEST(Manifest, DefaultVersion)
{
    ReleaseManifest manifest;
    EXPECT_EQ(manifest.version, "7.0.0");
    EXPECT_EQ(manifest.platform, "x64");
    EXPECT_EQ(manifest.configuration, "Release");
}

TEST(Manifest, TotalSize)
{
    ReleaseManifest manifest;
    manifest.artifacts.push_back({"CBXShell.dll", "x64/Release/", ArtifactType::DLL, 2940*1024, true, true, ""});
    manifest.artifacts.push_back({"CBXManager.exe", "x64/Release/", ArtifactType::EXE, 400*1024, true, true, ""});
    EXPECT_EQ(manifest.TotalArtifacts(), 2u);
    EXPECT_GT(manifest.TotalSizeBytes(), 3000000u);
}

TEST(Manifest, NotReadyByDefault)
{
    ReleaseManifest manifest;
    EXPECT_FALSE(manifest.IsReleaseReady());
}

TEST(Manifest, ReadyWhenComplete)
{
    ReleaseManifest manifest;
    manifest.artifacts.push_back({"CBXShell.dll", "", ArtifactType::DLL, 2940*1024, true, true, ""});
    for (auto& g : manifest.checklist.Gates())
        manifest.checklist.SetGate(g.id, GateStatus::Passed);
    manifest.signingPolicy.method = SigningMethod::EV;
    EXPECT_TRUE(manifest.IsReleaseReady());
}

TEST(Manifest, MarkdownOutput)
{
    ReleaseManifest manifest;
    manifest.buildDate = "2025-07-11";
    manifest.commitHash = "abc1234";
    manifest.artifacts.push_back({"CBXShell.dll", "", ArtifactType::DLL, 2940*1024, true, true, ""});
    auto md = manifest.GenerateManifestMarkdown();
    EXPECT_NE(md.find("v7.0.0"), std::string::npos);
    EXPECT_NE(md.find("CBXShell.dll"), std::string::npos);
    EXPECT_NE(md.find("abc1234"), std::string::npos);
}
