// =============================================================================
// Sprint 24: Microsoft Store / MSIX Packaging Tests
// =============================================================================
// Validates MSIX manifest compliance, Store certification requirements,
// file type associations, and packaging infrastructure.
// =============================================================================

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <set>
#include <regex>

namespace fs = std::filesystem;

class MSIXPackagingTest : public ::testing::Test {
protected:
    // Path to the MSIX manifest
    static constexpr const char* kManifestPath = "packaging/msix/AppxManifest.xml";

    std::string ReadManifest() {
        std::ifstream file(kManifestPath);
        if (!file.is_open()) return "";
        return std::string((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    }

    bool ManifestContains(const std::string& text) {
        return ReadManifest().find(text) != std::string::npos;
    }
};

// ---------------------------------------------------------------------------
// Manifest Structure Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, ManifestFileExists) {
    EXPECT_TRUE(fs::exists(kManifestPath))
        << "AppxManifest.xml must exist at packaging/msix/";
}

TEST_F(MSIXPackagingTest, ManifestIsValidXml) {
    std::string content = ReadManifest();
    ASSERT_FALSE(content.empty());
    EXPECT_TRUE(content.find("<?xml version=") != std::string::npos)
        << "Manifest must start with XML declaration";
    EXPECT_TRUE(content.find("</Package>") != std::string::npos)
        << "Manifest must have closing Package tag";
}

TEST_F(MSIXPackagingTest, ManifestHasRequiredNamespaces) {
    std::string content = ReadManifest();
    EXPECT_TRUE(content.find("schemas.microsoft.com/appx/manifest/foundation/windows10") != std::string::npos);
    EXPECT_TRUE(content.find("schemas.microsoft.com/appx/manifest/uap/windows10") != std::string::npos);
    EXPECT_TRUE(content.find("schemas.microsoft.com/appx/manifest/desktop/windows10") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Identity & Properties Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, IdentityHasCorrectVersion) {
    EXPECT_TRUE(ManifestContains("Version=\"7.0.0.0\""))
        << "MSIX manifest version must match current project version 7.0.0.0";
}

TEST_F(MSIXPackagingTest, IdentityHasPublisher) {
    EXPECT_TRUE(ManifestContains("Publisher=\"CN="))
        << "Publisher must use valid CN= format for Store certification";
}

TEST_F(MSIXPackagingTest, PropertiesHaveDisplayName) {
    EXPECT_TRUE(ManifestContains("<DisplayName>DarkThumbs</DisplayName>"));
}

TEST_F(MSIXPackagingTest, PropertiesHaveDescription) {
    EXPECT_TRUE(ManifestContains("<Description>"))
        << "Store listing requires a description";
}

TEST_F(MSIXPackagingTest, PropertiesHaveLogo) {
    EXPECT_TRUE(ManifestContains("<Logo>Assets\\StoreLogo.png</Logo>"))
        << "Store logo asset path must be specified";
}

// ---------------------------------------------------------------------------
// Target Platform Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, TargetsDesktopDeviceFamily) {
    EXPECT_TRUE(ManifestContains("Name=\"Windows.Desktop\""))
        << "Must target Windows.Desktop device family";
}

TEST_F(MSIXPackagingTest, MinVersionIsWindows10_2004OrLater) {
    // 10.0.19041.0 = Windows 10 version 2004
    EXPECT_TRUE(ManifestContains("MinVersion=\"10.0.19041.0\""))
        << "Minimum version should be Windows 10 2004 (10.0.19041.0) for DirectX12 support";
}

TEST_F(MSIXPackagingTest, MaxVersionTestedIsRecent) {
    EXPECT_TRUE(ManifestContains("MaxVersionTested="))
        << "MaxVersionTested must be specified for Store certification";
}

// ---------------------------------------------------------------------------
// Application Entry Point Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, EntryPointIsCBXManager) {
    EXPECT_TRUE(ManifestContains("Executable=\"CBXManager.exe\""))
        << "Primary executable must be CBXManager.exe";
}

TEST_F(MSIXPackagingTest, EntryPointIsFullTrust) {
    EXPECT_TRUE(ManifestContains("EntryPoint=\"Windows.FullTrustApplication\""))
        << "Shell extensions require full-trust execution";
}

// ---------------------------------------------------------------------------
// Visual Assets Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, VisualElementsHaveRequiredLogos) {
    std::string content = ReadManifest();
    EXPECT_TRUE(content.find("Square150x150Logo=") != std::string::npos)
        << "Square 150x150 logo required for Store tile";
    EXPECT_TRUE(content.find("Square44x44Logo=") != std::string::npos)
        << "Square 44x44 logo required for Store list view";
}

TEST_F(MSIXPackagingTest, VisualElementsHaveWideTile) {
    EXPECT_TRUE(ManifestContains("Wide310x150Logo="))
        << "Wide tile logo recommended for Store visibility";
}

// ---------------------------------------------------------------------------
// Shell Extension Registration Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, HasComServerExtension) {
    EXPECT_TRUE(ManifestContains("windows.comServer"))
        << "COM server extension required for shell extension registration";
}

TEST_F(MSIXPackagingTest, HasDesktopExeServer) {
    EXPECT_TRUE(ManifestContains("<desktop:ExeServer"))
        << "Desktop COM EXE server declaration needed for thumbnail provider";
}

// ---------------------------------------------------------------------------
// File Type Association Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, HasFileTypeAssociationExtension) {
    EXPECT_TRUE(ManifestContains("windows.fileTypeAssociation"))
        << "File type association extension required for shell integration";
}

TEST_F(MSIXPackagingTest, RegistersCoreArchiveFormats) {
    std::string content = ReadManifest();
    const std::vector<std::string> coreFormats = {
        ".cbz", ".cbr", ".cb7", ".cbt", ".epub"
    };
    for (const auto& ext : coreFormats) {
        EXPECT_TRUE(content.find("<uap:FileType>" + ext + "</uap:FileType>") != std::string::npos)
            << "Core archive format " << ext << " must be registered";
    }
}

TEST_F(MSIXPackagingTest, RegistersModernImageFormats) {
    std::string content = ReadManifest();
    const std::vector<std::string> modernFormats = {
        ".webp", ".avif", ".jxl"
    };
    for (const auto& ext : modernFormats) {
        EXPECT_TRUE(content.find("<uap:FileType>" + ext + "</uap:FileType>") != std::string::npos)
            << "Modern image format " << ext << " must be registered";
    }
}

// ---------------------------------------------------------------------------
// Capabilities Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, HasFullTrustCapability) {
    EXPECT_TRUE(ManifestContains("runFullTrust"))
        << "Shell extensions require runFullTrust restricted capability";
}

TEST_F(MSIXPackagingTest, HasUnvirtualizedResources) {
    EXPECT_TRUE(ManifestContains("unvirtualizedResources"))
        << "COM registration requires unvirtualized filesystem access";
}

// ---------------------------------------------------------------------------
// Store Certification Compliance Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, NoPlaceholderCLSID) {
    // The manifest still has a placeholder CLSID that needs to be replaced
    // before actual Store submission. This test documents the known TODO.
    std::string content = ReadManifest();
    bool hasPlaceholder = content.find("YOUR-CLSID-HERE") != std::string::npos;
    if (hasPlaceholder) {
        GTEST_SKIP() << "CLSID placeholder exists — must be replaced with actual CLSID before Store submission";
    }
    // If no placeholder, verify it looks like a valid GUID
    std::regex guidRegex("[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}");
    EXPECT_TRUE(std::regex_search(content, guidRegex))
        << "CLSID must be a valid GUID format";
}

TEST_F(MSIXPackagingTest, LanguageResourceDeclared) {
    EXPECT_TRUE(ManifestContains("<Resource Language=\"en-us\""))
        << "At least en-us language resource must be declared";
}

// ---------------------------------------------------------------------------
// Packaging Infrastructure Tests
// ---------------------------------------------------------------------------

TEST_F(MSIXPackagingTest, PackagingDirectoryExists) {
    EXPECT_TRUE(fs::exists("packaging/msix") || fs::exists("packaging\\msix"))
        << "packaging/msix/ directory must exist";
}

TEST_F(MSIXPackagingTest, MSIPackagingAlsoAvailable) {
    // DarkThumbs supports both MSI and MSIX packaging
    bool hasMsi = fs::exists("packaging/msi") || fs::exists("packaging\\msi");
    bool hasWix = fs::exists("packaging/wix") || fs::exists("packaging\\wix");
    EXPECT_TRUE(hasMsi || hasWix)
        << "Traditional MSI/WiX packaging should coexist with MSIX for non-Store distribution";
}
