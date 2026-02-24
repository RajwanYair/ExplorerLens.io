//==============================================================================
// Sprint 49 Tests — Release Packaging & Distribution
//==============================================================================
#include <gtest/gtest.h>
#include "../Engine/Release/ReleasePackaging.h"

using namespace ExplorerLens::Engine::Release;

// =============================================================================
// PackageType Tests
// =============================================================================

TEST(PackageTypeTest, Names) {
    EXPECT_STREQ(PackageTypeName(PackageType::MSI),         "MSI");
    EXPECT_STREQ(PackageTypeName(PackageType::PortableZIP), "PortableZIP");
    EXPECT_STREQ(PackageTypeName(PackageType::MSIX),        "MSIX");
    EXPECT_STREQ(PackageTypeName(PackageType::NuGet),       "NuGet");
    EXPECT_STREQ(PackageTypeName(PackageType::Symbols),     "Symbols");
}

TEST(PackageTypeTest, Extensions) {
    EXPECT_STREQ(PackageExtension(PackageType::MSI),         ".msi");
    EXPECT_STREQ(PackageExtension(PackageType::PortableZIP), ".zip");
    EXPECT_STREQ(PackageExtension(PackageType::MSIX),        ".msix");
    EXPECT_STREQ(PackageExtension(PackageType::NuGet),       ".nupkg");
    EXPECT_STREQ(PackageExtension(PackageType::Symbols),     ".symbols.zip");
}

// =============================================================================
// Architecture Tests
// =============================================================================

TEST(ArchitectureTest, Names) {
    EXPECT_STREQ(ArchitectureName(Architecture::x64),   "x64");
    EXPECT_STREQ(ArchitectureName(Architecture::x86),   "x86");
    EXPECT_STREQ(ArchitectureName(Architecture::ARM64), "ARM64");
}

// =============================================================================
// Version Tests
// =============================================================================

TEST(VersionTest, ToString) {
    Version v{7, 0, 0, 0, ""};
    EXPECT_EQ(v.ToString(), "7.0.0");
}

TEST(VersionTest, ToStringWithBuild) {
    Version v{7, 1, 2, 100, ""};
    EXPECT_EQ(v.ToString(), "7.1.2.100");
}

TEST(VersionTest, ToStringWithPreRelease) {
    Version v{7, 0, 0, 0, "beta.1"};
    EXPECT_EQ(v.ToString(), "7.0.0-beta.1");
}

TEST(VersionTest, FileVersion) {
    Version v{7, 1, 2, 100, ""};
    EXPECT_EQ(v.FileVersion(), "7.1.2.100");
}

TEST(VersionTest, IsPreRelease) {
    Version stable{7, 0, 0, 0, ""};
    Version beta{7, 0, 0, 0, "beta"};
    EXPECT_FALSE(stable.IsPreRelease());
    EXPECT_TRUE(beta.IsPreRelease());
}

TEST(VersionTest, IsNewerThanMajor) {
    Version v8{8, 0, 0, 0, ""};
    Version v7{7, 0, 0, 0, ""};
    EXPECT_TRUE(v8.IsNewerThan(v7));
    EXPECT_FALSE(v7.IsNewerThan(v8));
}

TEST(VersionTest, IsNewerThanMinor) {
    Version v71{7, 1, 0, 0, ""};
    Version v70{7, 0, 0, 0, ""};
    EXPECT_TRUE(v71.IsNewerThan(v70));
}

TEST(VersionTest, IsNewerThanPreRelease) {
    Version stable{7, 0, 0, 0, ""};
    Version beta{7, 0, 0, 0, "beta"};
    EXPECT_TRUE(stable.IsNewerThan(beta));
    EXPECT_FALSE(beta.IsNewerThan(stable));
}

TEST(VersionTest, IsNewerThanSame) {
    Version v{7, 0, 0, 0, ""};
    EXPECT_FALSE(v.IsNewerThan(v));
}

TEST(VersionTest, Parse) {
    auto v = Version::Parse("7.1.2");
    EXPECT_EQ(v.major, 7);
    EXPECT_EQ(v.minor, 1);
    EXPECT_EQ(v.patch, 2);
}

TEST(VersionTest, ParseWithBuild) {
    auto v = Version::Parse("7.1.2.100");
    EXPECT_EQ(v.build, 100);
}

TEST(VersionTest, Current) {
    auto v = Version::Current();
    EXPECT_EQ(v.major, 7);
    EXPECT_FALSE(v.IsPreRelease());
}

// =============================================================================
// Artifact Tests
// =============================================================================

TEST(ArtifactTest, HasChecksum) {
    Artifact a;
    EXPECT_FALSE(a.HasChecksum());
    a.sha256 = "abc123";
    EXPECT_TRUE(a.HasChecksum());
}

TEST(ArtifactTest, IsValid) {
    Artifact a;
    EXPECT_FALSE(a.IsValid());
    a.filename = "ExplorerLens-7.0.0-x64.msi";
    a.sizeBytes = 5000000;
    EXPECT_TRUE(a.IsValid());
}

TEST(ArtifactTest, SizeHumanMB) {
    Artifact a;
    a.sizeBytes = 5 * 1024 * 1024;
    EXPECT_NE(a.SizeHuman().find("MB"), std::string::npos);
}

TEST(ArtifactTest, SizeHumanKB) {
    Artifact a;
    a.sizeBytes = 512 * 1024;
    EXPECT_NE(a.SizeHuman().find("KB"), std::string::npos);
}

TEST(ArtifactTest, SizeHumanBytes) {
    Artifact a;
    a.sizeBytes = 42;
    EXPECT_NE(a.SizeHuman().find("B"), std::string::npos);
}

// =============================================================================
// MSI Validation Tests
// =============================================================================

TEST(MSIValidationTest, AllPassed) {
    auto r = MSIValidationResult::AllPassed();
    EXPECT_TRUE(r.IsValid());
    EXPECT_EQ(r.PassedChecks(), 6u);
}

TEST(MSIValidationTest, Empty) {
    auto r = MSIValidationResult::Empty();
    EXPECT_FALSE(r.IsValid());
    EXPECT_EQ(r.PassedChecks(), 0u);
}

TEST(MSIValidationTest, PartialPass) {
    MSIValidationResult r;
    r.hasProductCode = true;
    r.hasVersion = true;
    r.hasManufacturer = true;
    EXPECT_FALSE(r.IsValid());
    EXPECT_EQ(r.PassedChecks(), 3u);
    EXPECT_EQ(r.TotalChecks(), 6u);
}

// =============================================================================
// SBOM Tests
// =============================================================================

TEST(SBOMEntryTest, FullName) {
    SBOMEntry e;
    e.name = "libjxl";
    e.version = "0.11.1";
    EXPECT_EQ(e.FullName(), "libjxl@0.11.1");
}

TEST(SBOMEntryTest, FullNameNoVersion) {
    SBOMEntry e;
    e.name = "libjxl";
    EXPECT_EQ(e.FullName(), "libjxl");
}

TEST(SBOMEntryTest, HasLicenseAndURL) {
    SBOMEntry e;
    EXPECT_FALSE(e.HasLicense());
    EXPECT_FALSE(e.HasURL());
    e.license = "MIT";
    e.url = "https://example.com";
    EXPECT_TRUE(e.HasLicense());
    EXPECT_TRUE(e.HasURL());
}

TEST(SBOMTest, ExplorerLensSBOM) {
    auto sbom = SBOM::ExplorerLensSBOM();
    EXPECT_EQ(sbom.product, "ExplorerLens");
    EXPECT_GT(sbom.entries.size(), 10u);
    EXPECT_GT(sbom.DirectDependencyCount(), 0u);
}

TEST(SBOMTest, TransitiveDependencies) {
    auto sbom = SBOM::ExplorerLensSBOM();
    EXPECT_GT(sbom.TransitiveDependencyCount(), 0u);
    EXPECT_EQ(sbom.DirectDependencyCount() + sbom.TransitiveDependencyCount(),
              sbom.entries.size());
}

TEST(SBOMTest, AllHaveLicenses) {
    auto sbom = SBOM::ExplorerLensSBOM();
    EXPECT_TRUE(sbom.AllHaveLicenses());
}

TEST(SBOMTest, ToText) {
    auto sbom = SBOM::ExplorerLensSBOM();
    auto text = sbom.ToText();
    EXPECT_NE(text.find("Software Bill of Materials"), std::string::npos);
    EXPECT_NE(text.find("ExplorerLens"), std::string::npos);
    EXPECT_NE(text.find("libjxl"), std::string::npos);
    EXPECT_NE(text.find("transitive"), std::string::npos);
}

// =============================================================================
// UpdateChannel Tests
// =============================================================================

TEST(UpdateChannelTest, Names) {
    EXPECT_STREQ(UpdateChannelName(UpdateChannel::Stable),  "Stable");
    EXPECT_STREQ(UpdateChannelName(UpdateChannel::Beta),    "Beta");
    EXPECT_STREQ(UpdateChannelName(UpdateChannel::Nightly), "Nightly");
    EXPECT_STREQ(UpdateChannelName(UpdateChannel::Canary),  "Canary");
}

// =============================================================================
// UpdateManifest Tests
// =============================================================================

TEST(UpdateManifestTest, HasDownloadInfo) {
    UpdateManifest m;
    EXPECT_FALSE(m.HasDownloadInfo());
    m.downloadUrl = "https://example.com/ExplorerLens.msi";
    m.sizeBytes = 5000000;
    EXPECT_TRUE(m.HasDownloadInfo());
}

TEST(UpdateManifestTest, HasChecksum) {
    UpdateManifest m;
    EXPECT_FALSE(m.HasChecksum());
    m.sha256 = "abc123";
    EXPECT_TRUE(m.HasChecksum());
}

TEST(UpdateManifestTest, ToJSON) {
    UpdateManifest m;
    m.version = {7, 1, 0, 0, ""};
    m.channel = UpdateChannel::Stable;
    m.downloadUrl = "https://example.com/ExplorerLens.msi";
    m.sizeBytes = 5000000;
    m.isRequired = false;
    auto json = m.ToJSON();
    EXPECT_NE(json.find("\"version\": \"7.1.0\""), std::string::npos);
    EXPECT_NE(json.find("\"channel\": \"Stable\""), std::string::npos);
    EXPECT_NE(json.find("\"required\": false"), std::string::npos);
}

// =============================================================================
// SignatureInfo Tests
// =============================================================================

TEST(SignatureInfoTest, FullyVerified) {
    SignatureInfo s;
    s.isValid = true;
    s.isTrusted = true;
    s.isExpired = false;
    EXPECT_TRUE(s.IsFullyVerified());
    EXPECT_EQ(s.StatusText(), "Verified");
}

TEST(SignatureInfoTest, InvalidSignature) {
    SignatureInfo s;
    s.isValid = false;
    EXPECT_FALSE(s.IsFullyVerified());
    EXPECT_EQ(s.StatusText(), "Invalid Signature");
}

TEST(SignatureInfoTest, UntrustedCert) {
    SignatureInfo s;
    s.isValid = true;
    s.isTrusted = false;
    EXPECT_EQ(s.StatusText(), "Untrusted Certificate");
}

TEST(SignatureInfoTest, ExpiredCert) {
    SignatureInfo s;
    s.isValid = true;
    s.isTrusted = true;
    s.isExpired = true;
    EXPECT_EQ(s.StatusText(), "Expired Certificate");
}

// =============================================================================
// ReleaseConfig Tests
// =============================================================================

TEST(ReleaseConfigTest, Default) {
    auto cfg = ReleaseConfig::Default();
    EXPECT_TRUE(cfg.signBinaries);
    EXPECT_TRUE(cfg.generateSBOM);
    EXPECT_TRUE(cfg.generateMSI);
    EXPECT_TRUE(cfg.generateZIP);
    EXPECT_FALSE(cfg.generateMSIX);
    EXPECT_EQ(cfg.version.major, 7);
}

TEST(ReleaseConfigTest, CI) {
    auto cfg = ReleaseConfig::CI();
    EXPECT_FALSE(cfg.signBinaries);
    EXPECT_FALSE(cfg.generateMSIX);
    EXPECT_TRUE(cfg.generateMSI);
}

TEST(ReleaseConfigTest, Full) {
    auto cfg = ReleaseConfig::Full();
    EXPECT_TRUE(cfg.signBinaries);
    EXPECT_TRUE(cfg.generateMSIX);
}

TEST(ReleaseConfigTest, PackagesToBuild) {
    auto cfg = ReleaseConfig::Default();
    auto pkgs = cfg.PackagesToBuild();
    // MSI + ZIP + Symbols = 3
    EXPECT_EQ(pkgs.size(), 3u);
}

TEST(ReleaseConfigTest, PackagesToBuildFull) {
    auto cfg = ReleaseConfig::Full();
    auto pkgs = cfg.PackagesToBuild();
    // MSI + ZIP + MSIX + Symbols = 4
    EXPECT_EQ(pkgs.size(), 4u);
}

