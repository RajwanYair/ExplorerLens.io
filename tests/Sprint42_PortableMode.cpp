//==============================================================================
// ExplorerLens — Sprint 42 Tests: Portable Mode & Thumbnail Overlay Badges
// Tests portable detection, INI config, cache paths, format badges,
// file-size badges, overlay config, deployment info.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../Engine/Utils/PortableMode.h"

using namespace ExplorerLens::Engine::Utils;

//==============================================================================
// Deployment Mode Tests
//==============================================================================

TEST(DeploymentMode, Names)
{
    EXPECT_STREQ(DeploymentModeName(DeploymentMode::Installed), "Installed");
    EXPECT_STREQ(DeploymentModeName(DeploymentMode::Portable), "Portable");
    EXPECT_STREQ(DeploymentModeName(DeploymentMode::Enterprise), "Enterprise (GPO)");
}

//==============================================================================
// Portable Detector Tests
//==============================================================================

TEST(PortableDetector, NotPortableWhenEmpty)
{
    PortableDetector det;
    EXPECT_FALSE(det.IsPortable());
    EXPECT_EQ(det.Detect(), DeploymentMode::Installed);
}

TEST(PortableDetector, PortableWhenIniExists)
{
    PortableDetector det;
    det.iniFilePath = "D:\\USB\\ExplorerLens\\portable.ini";
    EXPECT_TRUE(det.IsPortable());
    EXPECT_EQ(det.Detect(), DeploymentMode::Portable);
}

//==============================================================================
// INI Section Tests
//==============================================================================

TEST(INISection, GetDefault)
{
    INISection sec;
    sec.name = "General";
    EXPECT_EQ(sec.Get("Version", "0.0.0"), "0.0.0");
}

TEST(INISection, SetAndGet)
{
    INISection sec;
    sec.Set("Key1", "Value1");
    EXPECT_EQ(sec.Get("Key1"), "Value1");
    EXPECT_TRUE(sec.HasKey("Key1"));
    EXPECT_FALSE(sec.HasKey("Key2"));
}

TEST(INISection, GetInt)
{
    INISection sec;
    sec.Set("MaxSize", "256");
    EXPECT_EQ(sec.GetInt("MaxSize"), 256);
    EXPECT_EQ(sec.GetInt("Missing", 42), 42);
}

TEST(INISection, GetBoolVariants)
{
    INISection sec;
    sec.Set("A", "1");
    sec.Set("B", "true");
    sec.Set("C", "yes");
    sec.Set("D", "on");
    sec.Set("E", "0");
    sec.Set("F", "false");
    EXPECT_TRUE(sec.GetBool("A"));
    EXPECT_TRUE(sec.GetBool("B"));
    EXPECT_TRUE(sec.GetBool("C"));
    EXPECT_TRUE(sec.GetBool("D"));
    EXPECT_FALSE(sec.GetBool("E"));
    EXPECT_FALSE(sec.GetBool("F"));
}

TEST(INISection, KeyCount)
{
    INISection sec;
    EXPECT_EQ(sec.KeyCount(), 0u);
    sec.Set("A", "1");
    sec.Set("B", "2");
    EXPECT_EQ(sec.KeyCount(), 2u);
}

//==============================================================================
// Portable Config Tests
//==============================================================================

TEST(PortableConfig, DefaultTemplate)
{
    auto cfg = PortableConfig::DefaultTemplate();
    EXPECT_TRUE(cfg.loaded);
    EXPECT_GE(cfg.SectionCount(), 5u);

    auto* general = cfg.GetSection("General");
    ASSERT_NE(general, nullptr);
    EXPECT_EQ(general->Get("Version"), "7.0.0");
    EXPECT_EQ(general->Get("PortableMode"), "true");
}

TEST(PortableConfig, FormatsSection)
{
    auto cfg = PortableConfig::DefaultTemplate();
    auto* formats = cfg.GetSection("Formats");
    ASSERT_NE(formats, nullptr);
    EXPECT_TRUE(formats->GetBool("EnableJPEG"));
    EXPECT_TRUE(formats->GetBool("EnableHEIF"));
    EXPECT_FALSE(formats->GetBool("EnableVideo"));
}

TEST(PortableConfig, CacheSection)
{
    auto cfg = PortableConfig::DefaultTemplate();
    auto* cache = cfg.GetSection("Cache");
    ASSERT_NE(cache, nullptr);
    EXPECT_EQ(cache->GetInt("MaxSizeMB"), 256);
    EXPECT_EQ(cache->GetInt("MaxEntries"), 50000);
}

TEST(PortableConfig, EnsureSection)
{
    PortableConfig cfg;
    auto& sec = cfg.EnsureSection("NewSection");
    sec.Set("Key", "Val");
    EXPECT_EQ(cfg.SectionCount(), 1u);
    // Second call should return same section
    auto& sec2 = cfg.EnsureSection("NewSection");
    EXPECT_EQ(sec2.Get("Key"), "Val");
    EXPECT_EQ(cfg.SectionCount(), 1u);
}

TEST(PortableConfig, MissingSectionReturnsNull)
{
    PortableConfig cfg;
    EXPECT_EQ(cfg.GetSection("NonExistent"), nullptr);
}

//==============================================================================
// Portable Paths Tests
//==============================================================================

TEST(PortablePaths, FromDllDir)
{
    auto p = PortablePaths::FromDllDir("D:\\USB\\ExplorerLens");
    EXPECT_EQ(p.cacheDirectory, "D:\\USB\\ExplorerLens\\cache");
    EXPECT_EQ(p.configFile, "D:\\USB\\ExplorerLens\\portable.ini");
    EXPECT_EQ(p.logDirectory, "D:\\USB\\ExplorerLens\\logs");
    EXPECT_EQ(p.pluginDirectory, "D:\\USB\\ExplorerLens\\plugins");
}

TEST(PortablePaths, FromLocalAppData)
{
    auto p = PortablePaths::FromLocalAppData("C:\\Users\\test\\AppData\\Local");
    EXPECT_NE(p.cacheDirectory.find("ExplorerLens"), std::string::npos);
    EXPECT_NE(p.cacheDirectory.find("Cache"), std::string::npos);
    EXPECT_TRUE(p.configFile.empty()); // Uses registry
}

//==============================================================================
// Badge Position Tests
//==============================================================================

TEST(BadgePosition, Names)
{
    EXPECT_STREQ(BadgePositionName(BadgePosition::TopLeft), "Top-Left");
    EXPECT_STREQ(BadgePositionName(BadgePosition::BottomRight), "Bottom-Right");
    EXPECT_STREQ(BadgePositionName(BadgePosition::BottomLeft), "Bottom-Left");
    EXPECT_STREQ(BadgePositionName(BadgePosition::TopRight), "Top-Right");
}

//==============================================================================
// Format Badge Tests
//==============================================================================

TEST(FormatBadge, LabelForModernFormats)
{
    EXPECT_EQ(FormatBadge::LabelForExtension(".jxl"), "JXL");
    EXPECT_EQ(FormatBadge::LabelForExtension(".heif"), "HEIF");
    EXPECT_EQ(FormatBadge::LabelForExtension(".heic"), "HEIF");
    EXPECT_EQ(FormatBadge::LabelForExtension(".avif"), "AVIF");
    EXPECT_EQ(FormatBadge::LabelForExtension(".webp"), "WebP");
}

TEST(FormatBadge, LabelForProFormats)
{
    EXPECT_EQ(FormatBadge::LabelForExtension(".psd"), "PSD");
    EXPECT_EQ(FormatBadge::LabelForExtension(".hdr"), "HDR");
    EXPECT_EQ(FormatBadge::LabelForExtension(".exr"), "EXR");
    EXPECT_EQ(FormatBadge::LabelForExtension(".svg"), "SVG");
}

TEST(FormatBadge, LabelForRAW)
{
    EXPECT_EQ(FormatBadge::LabelForExtension(".dng"), "RAW");
    EXPECT_EQ(FormatBadge::LabelForExtension(".cr3"), "RAW");
    EXPECT_EQ(FormatBadge::LabelForExtension(".nef"), "RAW");
    EXPECT_EQ(FormatBadge::LabelForExtension(".arw"), "RAW");
}

TEST(FormatBadge, LabelForComics)
{
    EXPECT_EQ(FormatBadge::LabelForExtension(".cbz"), "LENS");
    EXPECT_EQ(FormatBadge::LabelForExtension(".cbr"), "LENS");
}

TEST(FormatBadge, LabelForMediaTypes)
{
    EXPECT_EQ(FormatBadge::LabelForExtension(".mp4"), "VID");
    EXPECT_EQ(FormatBadge::LabelForExtension(".mp3"), "AUD");
    EXPECT_EQ(FormatBadge::LabelForExtension(".ttf"), "FONT");
    EXPECT_EQ(FormatBadge::LabelForExtension(".obj"), "3D");
    EXPECT_EQ(FormatBadge::LabelForExtension(".epub"), "EPUB");
    EXPECT_EQ(FormatBadge::LabelForExtension(".pdf"), "PDF");
}

TEST(FormatBadge, LabelForCommonIsEmpty)
{
    // Common formats like JPEG/PNG don't need badges
    EXPECT_EQ(FormatBadge::LabelForExtension(".jpg"), "");
    EXPECT_EQ(FormatBadge::LabelForExtension(".png"), "");
    EXPECT_EQ(FormatBadge::LabelForExtension(".bmp"), "");
}

TEST(FormatBadge, EmptyBadge)
{
    FormatBadge b;
    EXPECT_TRUE(b.IsEmpty());
    b.formatLabel = "JXL";
    EXPECT_FALSE(b.IsEmpty());
}

//==============================================================================
// File Size Badge Tests
//==============================================================================

TEST(FileSizeBadge, SizeTextBytes)
{
    FileSizeBadge b;
    b.fileSize = 500;
    EXPECT_EQ(b.SizeText(), "500 B");
}

TEST(FileSizeBadge, SizeTextKB)
{
    FileSizeBadge b;
    b.fileSize = 15360; // 15 KB
    EXPECT_EQ(b.SizeText(), "15.0 KB");
}

TEST(FileSizeBadge, SizeTextMB)
{
    FileSizeBadge b;
    b.fileSize = 2621440; // 2.5 MB
    EXPECT_EQ(b.SizeText(), "2.5 MB");
}

TEST(FileSizeBadge, SizeTextGB)
{
    FileSizeBadge b;
    b.fileSize = 3221225472ULL; // 3.0 GB
    EXPECT_EQ(b.SizeText(), "3.0 GB");
}

TEST(FileSizeBadge, ShouldShow)
{
    FileSizeBadge b;
    EXPECT_FALSE(b.ShouldShow());
    b.fileSize = 1;
    EXPECT_TRUE(b.ShouldShow());
}

//==============================================================================
// Badge Overlay Config Tests
//==============================================================================

TEST(BadgeConfig, Default)
{
    auto cfg = BadgeOverlayConfig::Default();
    EXPECT_TRUE(cfg.showFormatBadge);
    EXPECT_FALSE(cfg.showSizeBadge);
    EXPECT_TRUE(cfg.HasAnyBadge());
}

TEST(BadgeConfig, Disabled)
{
    auto cfg = BadgeOverlayConfig::Disabled();
    EXPECT_FALSE(cfg.showFormatBadge);
    EXPECT_FALSE(cfg.showSizeBadge);
    EXPECT_FALSE(cfg.HasAnyBadge());
}

TEST(BadgeConfig, AllBadges)
{
    auto cfg = BadgeOverlayConfig::AllBadges();
    EXPECT_TRUE(cfg.showFormatBadge);
    EXPECT_TRUE(cfg.showSizeBadge);
}

//==============================================================================
// Deployment Info Tests
//==============================================================================

TEST(DeploymentInfo, Summary)
{
    DeploymentInfo info;
    info.mode = DeploymentMode::Portable;
    info.version = "7.0.0";
    info.configSource = "portable.ini";
    info.registeredWithShell = true;
    auto s = info.Summary();
    EXPECT_NE(s.find("7.0.0"), std::string::npos);
    EXPECT_NE(s.find("Portable"), std::string::npos);
    EXPECT_NE(s.find("portable.ini"), std::string::npos);
    EXPECT_NE(s.find("yes"), std::string::npos);
}

TEST(DeploymentInfo, InstalledSummary)
{
    DeploymentInfo info;
    info.configSource = "Registry";
    info.registeredWithShell = false;
    auto s = info.Summary();
    EXPECT_NE(s.find("Installed"), std::string::npos);
    EXPECT_NE(s.find("Registry"), std::string::npos);
    EXPECT_NE(s.find("no"), std::string::npos);
}

