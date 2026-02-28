//==============================================================================
// ExplorerLens — Color Space Awareness & HDR Tone Mapping
// Tests ICC profile extraction, gamut mapping, tone mapping operators,
// color accuracy metrics, WCS config, and per-format color defaults.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cmath>

#include "../Engine/Decoders/ColorSpaceManager.h"

using namespace ExplorerLens::Engine::Decoders;

//==============================================================================
// Color Space Identification
//==============================================================================

TEST(ColorSpace, NameStrings)
{
    EXPECT_STREQ(ColorSpaceName(ColorSpace::sRGB), "sRGB");
    EXPECT_STREQ(ColorSpaceName(ColorSpace::DisplayP3), "Display P3");
    EXPECT_STREQ(ColorSpaceName(ColorSpace::AdobeRGB), "Adobe RGB (1998)");
    EXPECT_STREQ(ColorSpaceName(ColorSpace::ProPhotoRGB), "ProPhoto RGB");
    EXPECT_STREQ(ColorSpaceName(ColorSpace::Rec709), "Rec.709");
    EXPECT_STREQ(ColorSpaceName(ColorSpace::Rec2020), "Rec.2020");
    EXPECT_STREQ(ColorSpaceName(ColorSpace::ACES), "ACES");
    EXPECT_STREQ(ColorSpaceName(ColorSpace::LinearRGB), "Linear sRGB");
    EXPECT_STREQ(ColorSpaceName(ColorSpace::Unknown), "Unknown");
}

TEST(ColorSpace, WideGamutDetection)
{
    EXPECT_TRUE(IsWideGamut(ColorSpace::DisplayP3));
    EXPECT_TRUE(IsWideGamut(ColorSpace::AdobeRGB));
    EXPECT_TRUE(IsWideGamut(ColorSpace::ProPhotoRGB));
    EXPECT_TRUE(IsWideGamut(ColorSpace::Rec2020));
    EXPECT_FALSE(IsWideGamut(ColorSpace::sRGB));
    EXPECT_FALSE(IsWideGamut(ColorSpace::Rec709));
}

TEST(ColorSpace, HDRDetection)
{
    EXPECT_TRUE(IsHDR(ColorSpace::Rec2020));
    EXPECT_TRUE(IsHDR(ColorSpace::ACES));
    EXPECT_TRUE(IsHDR(ColorSpace::LinearRGB));
    EXPECT_FALSE(IsHDR(ColorSpace::sRGB));
    EXPECT_FALSE(IsHDR(ColorSpace::DisplayP3));
}

//==============================================================================
// ICC Profile Tests
//==============================================================================

TEST(ICCProfile, EmptyProfileInvalid)
{
    ICCProfile profile;
    EXPECT_FALSE(profile.IsValid());
    EXPECT_EQ(profile.SizeBytes(), 0u);
}

TEST(ICCProfile, ValidProfile)
{
    ICCProfile profile;
    profile.rawData.resize(256, 0);
    profile.description = "sRGB IEC61966-2.1";
    EXPECT_TRUE(profile.IsValid());
    EXPECT_EQ(profile.SizeBytes(), 256u);
}

TEST(ICCProfile, IdentifyKnownSpaces)
{
    ICCProfile p;
    p.description = "sRGB IEC61966-2.1";
    EXPECT_EQ(p.IdentifyColorSpace(), ColorSpace::sRGB);

    p.description = "Display P3";
    EXPECT_EQ(p.IdentifyColorSpace(), ColorSpace::DisplayP3);

    p.description = "Adobe RGB (1998)";
    EXPECT_EQ(p.IdentifyColorSpace(), ColorSpace::AdobeRGB);

    p.description = "ProPhoto RGB";
    EXPECT_EQ(p.IdentifyColorSpace(), ColorSpace::ProPhotoRGB);

    p.description = "ITU-R BT.2020";
    EXPECT_EQ(p.IdentifyColorSpace(), ColorSpace::Rec2020);
}

TEST(ICCProfile, UnknownProfileIsCustom)
{
    ICCProfile p;
    p.description = "My Custom Camera Profile";
    EXPECT_EQ(p.IdentifyColorSpace(), ColorSpace::Custom);
}

//==============================================================================
// ICC Extraction Tests
//==============================================================================

TEST(ICCExtractor, SupportedFormats)
{
    auto formats = ICCSupportedFormats();
    EXPECT_GE(formats.size(), 10u);
    EXPECT_NE(std::find(formats.begin(), formats.end(), "JPEG"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "PNG"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "HEIF"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "EXR"), formats.end());
}

TEST(ICCExtractor, UnsupportedFormatReturnsError)
{
    ICCProfileExtractor extractor;
    auto result = extractor.Extract("test.bmp", "BMP");
    EXPECT_FALSE(result.found);
    EXPECT_NE(result.error.find("Unsupported"), std::string::npos);
}

TEST(ICCExtractor, FormatSupportCheck)
{
    ICCProfileExtractor extractor;
    EXPECT_TRUE(extractor.SupportsFormat("JPEG"));
    EXPECT_TRUE(extractor.SupportsFormat("TIFF"));
    EXPECT_FALSE(extractor.SupportsFormat("BMP"));
    EXPECT_FALSE(extractor.SupportsFormat("GIF"));
}

//==============================================================================
// Transfer Function Tests
//==============================================================================

TEST(TransferFunctions, SRGBRoundTrip)
{
    // sRGB → Linear → sRGB should be identity
    for (double v : {0.0, 0.1, 0.5, 0.8, 1.0}) {
        double linear = SRGBToLinear(v);
        double back = LinearToSRGB(linear);
        EXPECT_NEAR(back, v, 1e-10);
    }
}

TEST(TransferFunctions, SRGBLinearAt0)
{
    EXPECT_DOUBLE_EQ(SRGBToLinear(0.0), 0.0);
    EXPECT_DOUBLE_EQ(LinearToSRGB(0.0), 0.0);
}

TEST(TransferFunctions, PQAtZero)
{
    EXPECT_DOUBLE_EQ(PQToLinear(0.0), 0.0);
}

TEST(TransferFunctions, HLGAtZero)
{
    EXPECT_DOUBLE_EQ(HLGToLinear(0.0), 0.0);
}

TEST(TransferFunctions, HLGMonotonic)
{
    double prev = 0;
    for (double v = 0.1; v <= 1.0; v += 0.1) {
        double result = HLGToLinear(v);
        EXPECT_GT(result, prev);
        prev = result;
    }
}

//==============================================================================
// Gamut Mapping Tests
//==============================================================================

TEST(GamutMapper, IdentityForSameSpace)
{
    GamutMapper mapper(ColorSpace::sRGB, ColorSpace::sRGB);
    EXPECT_FALSE(mapper.NeedsConversion());
    auto result = mapper.MapPixel(0.5, 0.3, 0.7);
    EXPECT_NEAR(result[0], 0.5, 1e-10);
    EXPECT_NEAR(result[1], 0.3, 1e-10);
    EXPECT_NEAR(result[2], 0.7, 1e-10);
}

TEST(GamutMapper, DisplayP3ToSRGBMatrix)
{
    GamutMapper mapper(ColorSpace::DisplayP3, ColorSpace::sRGB);
    EXPECT_TRUE(mapper.NeedsConversion());
    auto matrix = mapper.GetMatrix();
    // First row should sum close to 1 (white preservation)
    double rowSum = matrix.m[0][0] + matrix.m[0][1] + matrix.m[0][2];
    EXPECT_NEAR(rowSum, 1.0, 0.01);
}

TEST(GamutMapper, ClipMethod)
{
    GamutMapper mapper(ColorSpace::Rec2020, ColorSpace::sRGB, GamutMappingMethod::Clip);
    auto result = mapper.MapPixel(2.0, -0.5, 0.5);
    EXPECT_LE(result[0], 1.0);
    EXPECT_GE(result[1], 0.0);
}

TEST(GamutMapper, MethodNames)
{
    EXPECT_STREQ(GamutMappingMethodName(GamutMappingMethod::Clip), "Clip");
    EXPECT_STREQ(GamutMappingMethodName(GamutMappingMethod::Perceptual), "Perceptual");
    EXPECT_STREQ(GamutMappingMethodName(GamutMappingMethod::RelativeColorimetric), "Relative Colorimetric");
}

TEST(GamutMapper, SourceTargetAccessors)
{
    GamutMapper mapper(ColorSpace::AdobeRGB, ColorSpace::sRGB, GamutMappingMethod::Saturation);
    EXPECT_EQ(mapper.Source(), ColorSpace::AdobeRGB);
    EXPECT_EQ(mapper.Target(), ColorSpace::sRGB);
    EXPECT_EQ(mapper.Method(), GamutMappingMethod::Saturation);
}

//==============================================================================
// Tone Mapping Operator Tests
//==============================================================================

TEST(ToneMapper, OperatorNames)
{
    EXPECT_STREQ(ToneMappingOperatorName(ToneMappingOperator::Reinhard), "Reinhard");
    EXPECT_STREQ(ToneMappingOperatorName(ToneMappingOperator::ACES_Filmic), "ACES Filmic");
    EXPECT_STREQ(ToneMappingOperatorName(ToneMappingOperator::Hable), "Hable (Uncharted 2)");
    EXPECT_STREQ(ToneMappingOperatorName(ToneMappingOperator::Lottes), "Lottes (AMD)");
}

TEST(ToneMapper, ReinhardZero)
{
    ToneMapper tm(ToneMappingOperator::Reinhard);
    EXPECT_DOUBLE_EQ(tm.ToneMap(0.0), 0.0);
}

TEST(ToneMapper, ReinhardMonotonic)
{
    ToneMapper tm(ToneMappingOperator::Reinhard);
    double prev = 0;
    for (double v = 0.1; v <= 10.0; v += 0.5) {
        double result = tm.ToneMap(v);
        EXPECT_GT(result, prev);
        EXPECT_LE(result, 1.0);
        prev = result;
    }
}

TEST(ToneMapper, ACESFilmicRange)
{
    ToneMapper tm(ToneMappingOperator::ACES_Filmic);
    EXPECT_GE(tm.ToneMap(0.0), 0.0);
    EXPECT_LE(tm.ToneMap(100.0), 1.0);
    EXPECT_GT(tm.ToneMap(1.0), 0.0);
}

TEST(ToneMapper, HableRange)
{
    ToneMapper tm(ToneMappingOperator::Hable);
    double result = tm.ToneMap(1.0);
    EXPECT_GT(result, 0.0);
    EXPECT_LE(result, 1.0);
}

TEST(ToneMapper, ExposureControl)
{
    ToneMapper tm(ToneMappingOperator::Reinhard, 2.0);
    EXPECT_EQ(tm.Exposure(), 2.0);
    double bright = tm.ToneMap(0.5);
    tm.SetExposure(0.5);
    double dim = tm.ToneMap(0.5);
    EXPECT_GT(bright, dim);
}

TEST(ToneMapper, ToneMapRGB)
{
    ToneMapper tm(ToneMappingOperator::Reinhard);
    auto result = tm.ToneMapRGB(0.5, 1.0, 2.0);
    EXPECT_EQ(result.size(), 3u);
    EXPECT_LT(result[0], result[1]);
    EXPECT_LT(result[1], result[2]);
}

TEST(ToneMapper, LinearClamp)
{
    ToneMapper tm(ToneMappingOperator::Linear_Clamp);
    EXPECT_DOUBLE_EQ(tm.ToneMap(0.5), 0.5);
    EXPECT_DOUBLE_EQ(tm.ToneMap(2.0), 1.0);
    EXPECT_DOUBLE_EQ(tm.ToneMap(-1.0), 0.0);
}

//==============================================================================
// HDR Metadata Tests
//==============================================================================

TEST(HDRMetadata, DefaultIsHDR)
{
    HDRMetadata meta;
    EXPECT_TRUE(meta.IsHDR()); // Default 1000 nits
    EXPECT_GT(meta.DynamicRange(), 1.0);
}

TEST(HDRMetadata, SDRIsNotHDR)
{
    HDRMetadata meta;
    meta.maxContentLightLevel = 80.0;
    EXPECT_FALSE(meta.IsHDR());
}

TEST(HDRMetadata, PQFlagMakesHDR)
{
    HDRMetadata meta;
    meta.maxContentLightLevel = 50.0;
    meta.hasPQ = true;
    EXPECT_TRUE(meta.IsHDR());
}

//==============================================================================
// Color Accuracy (dE2000) Tests
//==============================================================================

TEST(ColorAccuracy, IdenticalColorsZeroDelta)
{
    LABColor a{50.0, 20.0, -10.0};
    EXPECT_DOUBLE_EQ(DeltaE2000(a, a), 0.0);
}

TEST(ColorAccuracy, DifferentColorsPositiveDelta)
{
    LABColor a{50.0, 25.0, 10.0};
    LABColor b{55.0, 30.0, 15.0};
    EXPECT_GT(DeltaE2000(a, b), 0.0);
}

TEST(ColorAccuracy, ResultSummary)
{
    ColorAccuracyResult result;
    result.dE2000_mean = 1.2;
    result.dE2000_max = 3.5;
    result.dE2000_p95 = 2.1;
    result.sampleCount = 100;
    result.passesThreshold = true;
    auto s = result.Summary();
    EXPECT_NE(s.find("PASS"), std::string::npos);
    EXPECT_NE(s.find("100"), std::string::npos);
}

TEST(ColorAccuracy, FailResult)
{
    ColorAccuracyResult result;
    result.passesThreshold = false;
    EXPECT_NE(result.Summary().find("FAIL"), std::string::npos);
}

//==============================================================================
// Pipeline Config Tests
//==============================================================================

TEST(ColorConfig, DefaultEnabled)
{
    auto cfg = ColorPipelineConfig::Default();
    EXPECT_TRUE(cfg.enableColorManagement);
    EXPECT_TRUE(cfg.extractICCProfiles);
    EXPECT_TRUE(cfg.applyGamutMapping);
    EXPECT_TRUE(cfg.applyToneMapping);
    EXPECT_EQ(cfg.outputSpace, ColorSpace::sRGB);
}

TEST(ColorConfig, DisabledConfig)
{
    auto cfg = ColorPipelineConfig::Disabled();
    EXPECT_FALSE(cfg.enableColorManagement);
    EXPECT_FALSE(cfg.extractICCProfiles);
}

TEST(ColorConfig, HDRPreset)
{
    auto cfg = ColorPipelineConfig::HDR();
    EXPECT_EQ(cfg.toneMapper, ToneMappingOperator::ACES_Filmic);
    EXPECT_GT(cfg.hdrExposure, 1.0);
}

TEST(ColorConfig, HighAccuracyPreset)
{
    auto cfg = ColorPipelineConfig::HighAccuracy();
    EXPECT_EQ(cfg.gamutMethod, GamutMappingMethod::RelativeColorimetric);
    EXPECT_LT(cfg.maxDeltaE, 2.0);
}

//==============================================================================
// Per-Format Defaults
//==============================================================================

TEST(FormatDefaults, HEIFIsP3)
{
    EXPECT_EQ(DefaultColorSpaceForFormat("HEIF"), ColorSpace::DisplayP3);
    EXPECT_EQ(DefaultColorSpaceForFormat("HEIC"), ColorSpace::DisplayP3);
}

TEST(FormatDefaults, EXRIsLinear)
{
    EXPECT_EQ(DefaultColorSpaceForFormat("EXR"), ColorSpace::LinearRGB);
}

TEST(FormatDefaults, PSDIsAdobeRGB)
{
    EXPECT_EQ(DefaultColorSpaceForFormat("PSD"), ColorSpace::AdobeRGB);
}

TEST(FormatDefaults, DNGIsProPhoto)
{
    EXPECT_EQ(DefaultColorSpaceForFormat("DNG"), ColorSpace::ProPhotoRGB);
}

TEST(FormatDefaults, JPEGIsSRGB)
{
    EXPECT_EQ(DefaultColorSpaceForFormat("JPEG"), ColorSpace::sRGB);
}

TEST(FormatDefaults, UnknownFormatIsSRGB)
{
    EXPECT_EQ(DefaultColorSpaceForFormat("UNKNOWN"), ColorSpace::sRGB);
}

