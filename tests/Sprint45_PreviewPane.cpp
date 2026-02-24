//==============================================================================
// ExplorerLens — Sprint 45 Tests: Preview Pane & Rich Tooltip Integration
// Tests preview mode, EXIF fields, image dimensions, camera info,
// GPS info, file metadata, tooltip content, property columns, config.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../Engine/Shell/PreviewPaneHandler.h"

using namespace ExplorerLens::Engine::Shell;

//==============================================================================
// Preview Mode Tests
//==============================================================================

TEST(PreviewMode, Names)
{
    EXPECT_STREQ(PreviewModeName(PreviewMode::Thumbnail), "Thumbnail");
    EXPECT_STREQ(PreviewModeName(PreviewMode::FullImage), "Full Image");
    EXPECT_STREQ(PreviewModeName(PreviewMode::EXIF), "EXIF Metadata");
    EXPECT_STREQ(PreviewModeName(PreviewMode::SideBySide), "Side-by-Side");
    EXPECT_STREQ(PreviewModeName(PreviewMode::Unsupported), "Unsupported");
}

//==============================================================================
// EXIF Field Tests
//==============================================================================

TEST(EXIFField, FormattedValueNoUnit)
{
    EXIFField f;
    f.value = "Canon";
    EXPECT_EQ(f.FormattedValue(), "Canon");
}

TEST(EXIFField, FormattedValueWithUnit)
{
    EXIFField f;
    f.value = "24";
    f.unit = "mm";
    EXPECT_EQ(f.FormattedValue(), "24 mm");
}

TEST(EXIFField, IsEmpty)
{
    EXIFField f;
    EXPECT_TRUE(f.IsEmpty());
    f.value = "test";
    EXPECT_FALSE(f.IsEmpty());
}

//==============================================================================
// Image Dimensions Tests
//==============================================================================

TEST(ImageDimensions, PixelCount)
{
    ImageDimensions d;
    d.width = 1920;
    d.height = 1080;
    EXPECT_EQ(d.PixelCount(), 2073600u);
}

TEST(ImageDimensions, MegaPixels)
{
    ImageDimensions d;
    d.width = 4000;
    d.height = 3000;
    EXPECT_NEAR(d.MegaPixels(), 12.0, 0.01);
}

TEST(ImageDimensions, ResolutionText)
{
    ImageDimensions d;
    d.width = 1920;
    d.height = 1080;
    EXPECT_EQ(d.ResolutionText(), "1920 x 1080");
}

TEST(ImageDimensions, DetailedText)
{
    ImageDimensions d;
    d.width = 4000;
    d.height = 3000;
    d.bitDepth = 16;
    d.hasAlpha = true;
    d.isAnimated = true;
    d.frameCount = 30;
    auto s = d.DetailedText();
    EXPECT_NE(s.find("4000 x 3000"), std::string::npos);
    EXPECT_NE(s.find("16-bit"), std::string::npos);
    EXPECT_NE(s.find("alpha"), std::string::npos);
    EXPECT_NE(s.find("30 frames"), std::string::npos);
}

TEST(ImageDimensions, AspectRatio)
{
    ImageDimensions d;
    d.width = 1920;
    d.height = 1080;
    EXPECT_NEAR(d.AspectRatio(), 16.0 / 9.0, 0.01);
}

TEST(ImageDimensions, Orientation)
{
    ImageDimensions landscape;
    landscape.width = 1920; landscape.height = 1080;
    EXPECT_TRUE(landscape.IsLandscape());
    EXPECT_FALSE(landscape.IsPortrait());
    EXPECT_FALSE(landscape.IsSquare());

    ImageDimensions portrait;
    portrait.width = 1080; portrait.height = 1920;
    EXPECT_TRUE(portrait.IsPortrait());
    EXPECT_FALSE(portrait.IsLandscape());

    ImageDimensions square;
    square.width = 1000; square.height = 1000;
    EXPECT_TRUE(square.IsSquare());
}

//==============================================================================
// Camera Info Tests
//==============================================================================

TEST(CameraInfo, NoCamera)
{
    CameraInfo c;
    EXPECT_FALSE(c.HasCamera());
    EXPECT_FALSE(c.HasLens());
    EXPECT_EQ(c.CameraName(), "");
}

TEST(CameraInfo, FullCamera)
{
    CameraInfo c;
    c.make = "Canon";
    c.model = "EOS R5";
    c.lens = "RF 24-70mm F2.8";
    EXPECT_TRUE(c.HasCamera());
    EXPECT_TRUE(c.HasLens());
    EXPECT_EQ(c.CameraName(), "Canon EOS R5");
}

TEST(CameraInfo, MakeOnly)
{
    CameraInfo c;
    c.make = "Sony";
    EXPECT_EQ(c.CameraName(), "Sony");
}

TEST(CameraInfo, ModelOnly)
{
    CameraInfo c;
    c.model = "iPhone 16 Pro";
    EXPECT_EQ(c.CameraName(), "iPhone 16 Pro");
}

TEST(CameraInfo, ExposureDescription)
{
    CameraInfo c;
    c.aperture     = 2.8;
    c.exposureTime = 1.0 / 250.0;
    c.isoSpeed     = 400;
    c.focalLength  = 50.0;
    auto desc = c.ExposureDescription();
    EXPECT_NE(desc.find("f/2.8"), std::string::npos);
    EXPECT_NE(desc.find("1/250s"), std::string::npos);
    EXPECT_NE(desc.find("ISO 400"), std::string::npos);
    EXPECT_NE(desc.find("50mm"), std::string::npos);
}

TEST(CameraInfo, LongExposure)
{
    CameraInfo c;
    c.exposureTime = 30.0;
    auto desc = c.ExposureDescription();
    EXPECT_NE(desc.find("30.0s"), std::string::npos);
}

//==============================================================================
// GPS Info Tests
//==============================================================================

TEST(GPSInfo, NoLocation)
{
    GPSInfo g;
    EXPECT_EQ(g.LocationText(), "");
}

TEST(GPSInfo, WithLocation)
{
    GPSInfo g;
    g.hasLocation = true;
    g.latitude = 37.774929;
    g.longitude = -122.419418;
    auto s = g.LocationText();
    EXPECT_NE(s.find("37.774929"), std::string::npos);
    EXPECT_NE(s.find("-122.419418"), std::string::npos);
}

TEST(GPSInfo, WithAltitude)
{
    GPSInfo g;
    g.hasLocation = true;
    g.latitude = 48.858844;
    g.longitude = 2.294351;
    g.hasAltitude = true;
    g.altitude = 330.0;
    auto s = g.LocationText();
    EXPECT_NE(s.find("330"), std::string::npos);
    EXPECT_NE(s.find("m)"), std::string::npos);
}

//==============================================================================
// File Metadata Tests
//==============================================================================

TEST(FileMetadata, FileSizeHumanBytes)
{
    FileMetadata m;
    m.fileSize = 500;
    EXPECT_EQ(m.FileSizeHuman(), "500 B");
}

TEST(FileMetadata, FileSizeHumanKB)
{
    FileMetadata m;
    m.fileSize = 15360;
    EXPECT_NE(m.FileSizeHuman().find("KB"), std::string::npos);
}

TEST(FileMetadata, FileSizeHumanMB)
{
    FileMetadata m;
    m.fileSize = 5 * 1024 * 1024;
    EXPECT_NE(m.FileSizeHuman().find("MB"), std::string::npos);
}

TEST(FileMetadata, FileSizeHumanGB)
{
    FileMetadata m;
    m.fileSize = 3ULL * 1024 * 1024 * 1024;
    EXPECT_NE(m.FileSizeHuman().find("GB"), std::string::npos);
}

TEST(FileMetadata, CompressionRatio)
{
    FileMetadata m;
    m.dimensions.width = 1000;
    m.dimensions.height = 1000;
    m.dimensions.channels = 3;
    m.dimensions.bitDepth = 8;
    m.fileSize = 300000; // 300KB for 3MB raw = 10:1
    EXPECT_NEAR(m.CompressionRatio(), 10.0, 0.1);
}

//==============================================================================
// Tooltip Content Tests
//==============================================================================

TEST(TooltipContent, Empty)
{
    TooltipContent t;
    EXPECT_TRUE(t.IsEmpty());
    EXPECT_EQ(t.FieldCount(), 0u);
}

TEST(TooltipContent, AddFields)
{
    TooltipContent t;
    t.title = "test.jpg";
    t.AddField("Size", "1.5 MB");
    t.AddField("Dimensions", "1920 x 1080");
    t.AddField("Empty", "");  // Should not be added
    EXPECT_FALSE(t.IsEmpty());
    EXPECT_EQ(t.FieldCount(), 2u);
}

TEST(TooltipContent, AsPlainText)
{
    TooltipContent t;
    t.title = "photo.heic";
    t.subtitle = "HEIF  4000 x 3000";
    t.AddField("Size", "2.5 MB");
    auto text = t.AsPlainText();
    EXPECT_NE(text.find("photo.heic"), std::string::npos);
    EXPECT_NE(text.find("HEIF"), std::string::npos);
    EXPECT_NE(text.find("Size: 2.5 MB"), std::string::npos);
}

TEST(TooltipContent, FromMetadata)
{
    FileMetadata meta;
    meta.fileName = "sunset.jxl";
    meta.formatName = "JPEG XL";
    meta.fileSize = 2 * 1024 * 1024;
    meta.dimensions.width = 6000;
    meta.dimensions.height = 4000;
    meta.camera.make = "Canon";
    meta.camera.model = "EOS R5";
    meta.camera.aperture = 5.6;
    meta.camera.isoSpeed = 200;
    meta.colorSpace = "Display P3";
    meta.isHDR = true;

    auto tip = TooltipContent::FromMetadata(meta);
    EXPECT_EQ(tip.title, "sunset.jxl");
    EXPECT_NE(tip.subtitle.find("JPEG XL"), std::string::npos);
    EXPECT_GE(tip.FieldCount(), 4u);
}

TEST(TooltipContent, FromMetadataMinimal)
{
    FileMetadata meta;
    meta.fileName = "icon.png";
    meta.formatName = "PNG";
    meta.fileSize = 512;
    meta.dimensions.width = 32;
    meta.dimensions.height = 32;

    auto tip = TooltipContent::FromMetadata(meta);
    EXPECT_EQ(tip.title, "icon.png");
    EXPECT_GE(tip.FieldCount(), 2u); // Size + Dimensions
}

//==============================================================================
// Property Column Tests
//==============================================================================

TEST(PropertyColumn, DefaultColumns)
{
    auto cols = PropertyColumn::DefaultColumns();
    EXPECT_GE(cols.size(), 5u);
    EXPECT_EQ(cols[0].id, "ExplorerLens.Format");
    EXPECT_EQ(cols[0].label, "Image Format");
    EXPECT_TRUE(cols[0].visible);
}

TEST(PropertyColumn, VisibleCount)
{
    auto cols = PropertyColumn::DefaultColumns();
    auto count = PropertyColumn::VisibleCount(cols);
    EXPECT_GE(count, 2u);
    EXPECT_LT(count, cols.size()); // Not all are visible
}

//==============================================================================
// Preview Pane Config Tests
//==============================================================================

TEST(PreviewPaneConfig, Default)
{
    auto c = PreviewPaneConfig::Default();
    EXPECT_EQ(c.defaultMode, PreviewMode::FullImage);
    EXPECT_TRUE(c.showEXIFOverlay);
    EXPECT_TRUE(c.enableZoom);
    EXPECT_DOUBLE_EQ(c.maxZoomFactor, 8.0);
}

TEST(PreviewPaneConfig, Minimal)
{
    auto c = PreviewPaneConfig::Minimal();
    EXPECT_FALSE(c.showEXIFOverlay);
    EXPECT_FALSE(c.enableZoom);
    EXPECT_FALSE(c.showHistogram);
}

TEST(PreviewPaneConfig, Photographer)
{
    auto c = PreviewPaneConfig::Photographer();
    EXPECT_TRUE(c.showEXIFOverlay);
    EXPECT_TRUE(c.showHistogram);
    EXPECT_TRUE(c.showColorProfile);
    EXPECT_DOUBLE_EQ(c.maxZoomFactor, 16.0);
}

