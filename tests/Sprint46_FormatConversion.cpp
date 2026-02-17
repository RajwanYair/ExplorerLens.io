//==============================================================================
// DarkThumbs — Sprint 46 Tests: Format Conversion & Export Pipeline
// Tests output formats, quality presets, resize modes, metadata handling,
// conversion profiles, conversion results, batch results, compatibility.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../Engine/Codec/FormatConverter.h"

using namespace DarkThumbs::Engine::Codec;

//==============================================================================
// Output Format Tests
//==============================================================================

TEST(OutputFormat, Names)
{
    EXPECT_STREQ(OutputFormatName(OutputFormat::JPEG), "JPEG");
    EXPECT_STREQ(OutputFormatName(OutputFormat::PNG), "PNG");
    EXPECT_STREQ(OutputFormatName(OutputFormat::WebP), "WebP");
    EXPECT_STREQ(OutputFormatName(OutputFormat::JXL), "JPEG XL");
    EXPECT_STREQ(OutputFormatName(OutputFormat::HEIF), "HEIF");
    EXPECT_STREQ(OutputFormatName(OutputFormat::AVIF), "AVIF");
    EXPECT_STREQ(OutputFormatName(OutputFormat::QOI), "QOI");
}

TEST(OutputFormat, Extensions)
{
    EXPECT_STREQ(OutputFormatExtension(OutputFormat::JPEG), ".jpg");
    EXPECT_STREQ(OutputFormatExtension(OutputFormat::PNG), ".png");
    EXPECT_STREQ(OutputFormatExtension(OutputFormat::JXL), ".jxl");
    EXPECT_STREQ(OutputFormatExtension(OutputFormat::AVIF), ".avif");
}

TEST(OutputFormat, LosslessSupport)
{
    EXPECT_FALSE(SupportsLossless(OutputFormat::JPEG));
    EXPECT_TRUE(SupportsLossless(OutputFormat::PNG));
    EXPECT_TRUE(SupportsLossless(OutputFormat::WebP));
    EXPECT_TRUE(SupportsLossless(OutputFormat::JXL));
    EXPECT_TRUE(SupportsLossless(OutputFormat::QOI));
}

TEST(OutputFormat, AlphaSupport)
{
    EXPECT_FALSE(SupportsAlpha(OutputFormat::JPEG));
    EXPECT_TRUE(SupportsAlpha(OutputFormat::PNG));
    EXPECT_TRUE(SupportsAlpha(OutputFormat::WebP));
    EXPECT_TRUE(SupportsAlpha(OutputFormat::JXL));
    EXPECT_FALSE(SupportsAlpha(OutputFormat::BMP));
}

TEST(OutputFormat, AnimationSupport)
{
    EXPECT_TRUE(SupportsAnimation(OutputFormat::WebP));
    EXPECT_TRUE(SupportsAnimation(OutputFormat::JXL));
    EXPECT_TRUE(SupportsAnimation(OutputFormat::AVIF));
    EXPECT_FALSE(SupportsAnimation(OutputFormat::JPEG));
    EXPECT_FALSE(SupportsAnimation(OutputFormat::PNG));
}

TEST(OutputFormat, HDRSupport)
{
    EXPECT_TRUE(SupportsHDR(OutputFormat::JXL));
    EXPECT_TRUE(SupportsHDR(OutputFormat::AVIF));
    EXPECT_FALSE(SupportsHDR(OutputFormat::JPEG));
    EXPECT_FALSE(SupportsHDR(OutputFormat::PNG));
}

//==============================================================================
// Quality Preset Tests
//==============================================================================

TEST(QualityPreset, Names)
{
    EXPECT_STREQ(QualityPresetName(QualityPreset::Lossless), "Lossless");
    EXPECT_STREQ(QualityPresetName(QualityPreset::Maximum), "Maximum");
    EXPECT_STREQ(QualityPresetName(QualityPreset::High), "High");
    EXPECT_STREQ(QualityPresetName(QualityPreset::Medium), "Medium");
    EXPECT_STREQ(QualityPresetName(QualityPreset::Low), "Low");
    EXPECT_STREQ(QualityPresetName(QualityPreset::Thumbnail), "Thumbnail");
}

TEST(QualityPreset, Values)
{
    EXPECT_EQ(QualityPresetValue(QualityPreset::Lossless), 100);
    EXPECT_EQ(QualityPresetValue(QualityPreset::Maximum), 95);
    EXPECT_EQ(QualityPresetValue(QualityPreset::High), 85);
    EXPECT_EQ(QualityPresetValue(QualityPreset::Medium), 75);
    EXPECT_GT(QualityPresetValue(QualityPreset::Low),
              QualityPresetValue(QualityPreset::Thumbnail));
}

//==============================================================================
// Resize Mode Tests
//==============================================================================

TEST(ResizeMode, Names)
{
    EXPECT_STREQ(ResizeModeName(ResizeMode::None), "None");
    EXPECT_STREQ(ResizeModeName(ResizeMode::FitWithin), "Fit Within");
    EXPECT_STREQ(ResizeModeName(ResizeMode::ExactSize), "Exact Size");
    EXPECT_STREQ(ResizeModeName(ResizeMode::FillCrop), "Fill & Crop");
    EXPECT_STREQ(ResizeModeName(ResizeMode::Percentage), "Percentage");
}

//==============================================================================
// Metadata Handling Tests
//==============================================================================

TEST(MetadataHandling, Names)
{
    EXPECT_STREQ(MetadataHandlingName(MetadataHandling::Preserve), "Preserve All");
    EXPECT_STREQ(MetadataHandlingName(MetadataHandling::Strip), "Strip All");
    EXPECT_STREQ(MetadataHandlingName(MetadataHandling::Essential), "Essential Only");
    EXPECT_STREQ(MetadataHandlingName(MetadataHandling::Anonymize), "Anonymize");
}

//==============================================================================
// Conversion Profile Tests
//==============================================================================

TEST(ConversionProfile, EffectiveQuality)
{
    ConversionProfile p;
    p.quality = QualityPreset::High;
    EXPECT_EQ(p.EffectiveQuality(), 85);

    p.qualityOverride = 92;
    EXPECT_EQ(p.EffectiveQuality(), 92);
}

TEST(ConversionProfile, NeedsResize)
{
    ConversionProfile p;
    EXPECT_FALSE(p.NeedsResize());
    p.resize = ResizeMode::FitWithin;
    EXPECT_TRUE(p.NeedsResize());
}

TEST(ConversionProfile, WebOptimized)
{
    auto p = ConversionProfile::WebOptimized();
    EXPECT_EQ(p.name, "Web Optimized");
    EXPECT_EQ(p.format, OutputFormat::WebP);
    EXPECT_EQ(p.resize, ResizeMode::FitWithin);
    EXPECT_EQ(p.maxWidth, 2048u);
    EXPECT_EQ(p.metadata, MetadataHandling::Essential);
}

TEST(ConversionProfile, ArchiveQuality)
{
    auto p = ConversionProfile::ArchiveQuality();
    EXPECT_EQ(p.format, OutputFormat::JXL);
    EXPECT_EQ(p.quality, QualityPreset::Lossless);
    EXPECT_EQ(p.metadata, MetadataHandling::Preserve);
}

TEST(ConversionProfile, SocialMedia)
{
    auto p = ConversionProfile::SocialMedia();
    EXPECT_EQ(p.format, OutputFormat::JPEG);
    EXPECT_EQ(p.metadata, MetadataHandling::Anonymize);
    EXPECT_EQ(p.maxWidth, 4096u);
}

TEST(ConversionProfile, ThumbnailExport)
{
    auto p = ConversionProfile::ThumbnailExport();
    EXPECT_EQ(p.quality, QualityPreset::Thumbnail);
    EXPECT_EQ(p.maxWidth, 256u);
    EXPECT_EQ(p.metadata, MetadataHandling::Strip);
}

//==============================================================================
// Conversion Result Tests
//==============================================================================

TEST(ConversionResult, SizeReduction)
{
    ConversionResult r;
    r.inputSize = 1000000;
    r.outputSize = 300000;
    EXPECT_NEAR(r.SizeReduction(), 70.0, 0.1);
}

TEST(ConversionResult, CompressionRatio)
{
    ConversionResult r;
    r.inputSize = 3000000;
    r.outputSize = 300000;
    EXPECT_NEAR(r.CompressionRatio(), 10.0, 0.1);
}

TEST(ConversionResult, IsSmallerOutput)
{
    ConversionResult r;
    r.inputSize = 1000;
    r.outputSize = 500;
    EXPECT_TRUE(r.IsSmallerOutput());

    r.outputSize = 1500;
    EXPECT_FALSE(r.IsSmallerOutput());
}

TEST(ConversionResult, ZeroSizes)
{
    ConversionResult r;
    EXPECT_DOUBLE_EQ(r.SizeReduction(), 0.0);
    EXPECT_DOUBLE_EQ(r.CompressionRatio(), 0.0);
    EXPECT_FALSE(r.IsSmallerOutput());
}

//==============================================================================
// Batch Conversion Result Tests
//==============================================================================

TEST(BatchConversionResult, SuccessRate)
{
    BatchConversionResult b;
    b.totalFiles = 100;
    b.succeeded = 95;
    b.failed = 5;
    EXPECT_NEAR(b.SuccessRate(), 95.0, 0.1);
}

TEST(BatchConversionResult, OverallSizeReduction)
{
    BatchConversionResult b;
    b.totalInputSize = 10000000;
    b.totalOutputSize = 3000000;
    EXPECT_NEAR(b.OverallSizeReduction(), 70.0, 0.1);
}

TEST(BatchConversionResult, Throughput)
{
    BatchConversionResult b;
    b.succeeded = 100;
    b.totalTimeMs = 5000.0;
    EXPECT_NEAR(b.ThroughputPerSecond(), 20.0, 0.1);
}

TEST(BatchConversionResult, Summary)
{
    BatchConversionResult b;
    b.totalFiles = 50;
    b.succeeded = 48;
    b.failed = 2;
    b.totalInputSize = 100000000;
    b.totalOutputSize = 30000000;
    auto s = b.Summary();
    EXPECT_NE(s.find("48/50"), std::string::npos);
    EXPECT_NE(s.find("96.0%"), std::string::npos);
    EXPECT_NE(s.find("2 failed"), std::string::npos);
    EXPECT_NE(s.find("70.0%"), std::string::npos);
}

//==============================================================================
// Format Compatibility Tests
//==============================================================================

TEST(FormatCompatibility, CanConvertImages)
{
    EXPECT_TRUE(FormatCompatibility::CanConvertTo(".jpg", OutputFormat::WebP));
    EXPECT_TRUE(FormatCompatibility::CanConvertTo(".heic", OutputFormat::JPEG));
    EXPECT_TRUE(FormatCompatibility::CanConvertTo(".jxl", OutputFormat::PNG));
    EXPECT_TRUE(FormatCompatibility::CanConvertTo(".psd", OutputFormat::JPEG));
    EXPECT_TRUE(FormatCompatibility::CanConvertTo(".dng", OutputFormat::TIFF));
    EXPECT_TRUE(FormatCompatibility::CanConvertTo(".svg", OutputFormat::PNG));
}

TEST(FormatCompatibility, CaseInsensitive)
{
    EXPECT_TRUE(FormatCompatibility::CanConvertTo(".JPG", OutputFormat::PNG));
    EXPECT_TRUE(FormatCompatibility::CanConvertTo(".Heic", OutputFormat::JPEG));
}

TEST(FormatCompatibility, UnsupportedFormats)
{
    EXPECT_FALSE(FormatCompatibility::CanConvertTo(".doc", OutputFormat::JPEG));
    EXPECT_FALSE(FormatCompatibility::CanConvertTo(".mp4", OutputFormat::PNG));
    EXPECT_FALSE(FormatCompatibility::CanConvertTo("", OutputFormat::JPEG));
}

TEST(FormatCompatibility, SupportedOutputFormats)
{
    auto formats = FormatCompatibility::SupportedOutputFormats();
    EXPECT_EQ(formats.size(), 9u);
}

TEST(FormatCompatibility, ModernFormats)
{
    auto formats = FormatCompatibility::ModernFormats();
    EXPECT_EQ(formats.size(), 4u);
}

TEST(FormatCompatibility, LosslessFormats)
{
    auto formats = FormatCompatibility::LosslessFormats();
    EXPECT_GE(formats.size(), 5u);
    // JPEG should not be in lossless list
    for (auto f : formats) {
        EXPECT_NE(f, OutputFormat::JPEG);
    }
}
