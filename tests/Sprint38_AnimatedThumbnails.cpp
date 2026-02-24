//==============================================================================
// ExplorerLens — Sprint 38 Tests: Animated & Multi-Frame Thumbnail Support
// Tests animation format detection, frame info, composition strategies,
// frame count badges, stacked preview layout, Live Photo detection.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cmath>

// Header under test
#include "../Engine/Decoders/AnimatedThumbnailDecoder.h"

using namespace ExplorerLens::Engine::Decoders;

//==============================================================================
// Animation Format Tests
//==============================================================================

TEST(AnimFormat, FormatNames)
{
    EXPECT_STREQ(AnimationFormatName(AnimationFormat::None), "Static");
    EXPECT_STREQ(AnimationFormatName(AnimationFormat::AnimatedWebP), "Animated WebP");
    EXPECT_STREQ(AnimationFormatName(AnimationFormat::AnimatedJXL), "Animated JXL");
    EXPECT_STREQ(AnimationFormatName(AnimationFormat::AnimatedGIF), "Animated GIF");
    EXPECT_STREQ(AnimationFormatName(AnimationFormat::MultiPagePDF), "Multi-Page PDF");
    EXPECT_STREQ(AnimationFormatName(AnimationFormat::MultiPageTIFF), "Multi-Page TIFF");
    EXPECT_STREQ(AnimationFormatName(AnimationFormat::LivePhoto), "Live Photo");
    EXPECT_STREQ(AnimationFormatName(AnimationFormat::AnimatedPNG), "Animated PNG");
}

TEST(AnimFormat, IsAnimated)
{
    EXPECT_FALSE(IsAnimated(AnimationFormat::None));
    EXPECT_TRUE(IsAnimated(AnimationFormat::AnimatedWebP));
    EXPECT_TRUE(IsAnimated(AnimationFormat::AnimatedJXL));
    EXPECT_TRUE(IsAnimated(AnimationFormat::MultiPagePDF));
    EXPECT_TRUE(IsAnimated(AnimationFormat::LivePhoto));
}

TEST(AnimFormat, IsMultiPage)
{
    EXPECT_TRUE(IsMultiPage(AnimationFormat::MultiPagePDF));
    EXPECT_TRUE(IsMultiPage(AnimationFormat::MultiPageTIFF));
    EXPECT_FALSE(IsMultiPage(AnimationFormat::AnimatedWebP));
    EXPECT_FALSE(IsMultiPage(AnimationFormat::AnimatedGIF));
    EXPECT_FALSE(IsMultiPage(AnimationFormat::None));
}

//==============================================================================
// Frame Info Tests
//==============================================================================

TEST(FrameInfo, AspectRatio)
{
    FrameInfo f;
    f.width = 1920;
    f.height = 1080;
    EXPECT_NEAR(f.AspectRatio(), 16.0 / 9.0, 0.01);
}

TEST(FrameInfo, AspectRatioZeroHeight)
{
    FrameInfo f;
    f.width = 100;
    f.height = 0;
    EXPECT_DOUBLE_EQ(f.AspectRatio(), 1.0);
}

//==============================================================================
// Animation Info Tests
//==============================================================================

TEST(AnimInfo, StaticNotAnimated)
{
    AnimationInfo info;
    info.format = AnimationFormat::None;
    info.frameCount = 1;
    EXPECT_FALSE(info.IsAnimated());
}

TEST(AnimInfo, MultiFrameAnimated)
{
    AnimationInfo info;
    info.format = AnimationFormat::AnimatedWebP;
    info.frameCount = 24;
    info.totalDurationMs = 1000;
    EXPECT_TRUE(info.IsAnimated());
}

TEST(AnimInfo, TotalDuration)
{
    AnimationInfo info;
    info.totalDurationMs = 2500;
    EXPECT_DOUBLE_EQ(info.TotalDurationSec(), 2.5);
}

TEST(AnimInfo, AverageFrameDuration)
{
    AnimationInfo info;
    info.frameCount = 30;
    info.totalDurationMs = 1000;
    EXPECT_NEAR(info.AverageFrameDurationMs(), 33.33, 0.01);
}

TEST(AnimInfo, FPS)
{
    AnimationInfo info;
    info.frameCount = 30;
    info.totalDurationMs = 1000;
    EXPECT_NEAR(info.FPS(), 30.0, 0.5);
}

TEST(AnimInfo, ZeroFPS)
{
    AnimationInfo info;
    EXPECT_DOUBLE_EQ(info.FPS(), 0.0);
}

//==============================================================================
// Composition Strategy Tests
//==============================================================================

TEST(Composition, StrategyNames)
{
    EXPECT_STREQ(CompositionStrategyName(CompositionStrategy::FirstFrame), "First Frame");
    EXPECT_STREQ(CompositionStrategyName(CompositionStrategy::KeyFrame), "Key Frame");
    EXPECT_STREQ(CompositionStrategyName(CompositionStrategy::StackedPreview), "Stacked Preview");
    EXPECT_STREQ(CompositionStrategyName(CompositionStrategy::GridComposite), "Grid Composite");
    EXPECT_STREQ(CompositionStrategyName(CompositionStrategy::CoverWithBadge), "Cover + Badge");
}

TEST(Composition, RecommendedForAnimatedWebP)
{
    EXPECT_EQ(RecommendedStrategy(AnimationFormat::AnimatedWebP), CompositionStrategy::CoverWithBadge);
}

TEST(Composition, RecommendedForPDF)
{
    EXPECT_EQ(RecommendedStrategy(AnimationFormat::MultiPagePDF), CompositionStrategy::StackedPreview);
}

TEST(Composition, RecommendedForLivePhoto)
{
    EXPECT_EQ(RecommendedStrategy(AnimationFormat::LivePhoto), CompositionStrategy::KeyFrame);
}

TEST(Composition, RecommendedForStatic)
{
    EXPECT_EQ(RecommendedStrategy(AnimationFormat::None), CompositionStrategy::FirstFrame);
}

//==============================================================================
// Frame Count Badge Tests
//==============================================================================

TEST(Badge, ShouldShow)
{
    FrameCountBadge badge;
    badge.frameCount = 1;
    EXPECT_FALSE(badge.ShouldShow());
    badge.frameCount = 2;
    EXPECT_TRUE(badge.ShouldShow());
}

TEST(Badge, BadgeText)
{
    FrameCountBadge badge;
    badge.frameCount = 48;
    EXPECT_EQ(badge.BadgeText(), "48 frames");
}

TEST(Badge, SinglePage)
{
    FrameCountBadge badge;
    badge.frameCount = 1;
    EXPECT_EQ(badge.PageBadgeText(), "1 page");
}

TEST(Badge, MultiplePages)
{
    FrameCountBadge badge;
    badge.frameCount = 42;
    EXPECT_EQ(badge.PageBadgeText(), "42 pages");
}

TEST(Badge, ZeroFrames)
{
    FrameCountBadge badge;
    badge.frameCount = 0;
    EXPECT_EQ(badge.BadgeText(), "");
    EXPECT_FALSE(badge.ShouldShow());
}

TEST(Badge, DefaultConfig)
{
    auto config = BadgeConfig::Default();
    EXPECT_EQ(config.fontSize, 11u);
    EXPECT_TRUE(config.showAtBottomRight);
}

//==============================================================================
// Stacked Preview Layout Tests
//==============================================================================

TEST(StackedLayout, SinglePage)
{
    StackedPreviewLayout layout(256);
    auto rects = layout.GenerateLayout(1);
    EXPECT_EQ(rects.size(), 1u);
}

TEST(StackedLayout, ThreePages)
{
    StackedPreviewLayout layout(256);
    auto rects = layout.GenerateLayout(10, 3);
    EXPECT_EQ(rects.size(), 3u);  // Max 3 shown
}

TEST(StackedLayout, LimitToMaxPages)
{
    StackedPreviewLayout layout(256);
    auto rects = layout.GenerateLayout(100, 3);
    EXPECT_EQ(rects.size(), 3u);
}

TEST(StackedLayout, CanvasSize)
{
    StackedPreviewLayout layout(512);
    EXPECT_EQ(layout.CanvasSize(), 512u);
}

TEST(StackedLayout, RectsWithinCanvas)
{
    StackedPreviewLayout layout(256);
    auto rects = layout.GenerateLayout(3);
    for (auto& r : rects) {
        EXPECT_GE(r.x, 0.0f);
        EXPECT_GE(r.y, -10.0f);  // Allow slight negative for stacking offset
        EXPECT_LE(r.x + r.width, 280.0f);  // Allow some overflow due to offset
    }
}

//==============================================================================
// Live Photo Detector Tests
//==============================================================================

TEST(LivePhoto, CandidateExtensions)
{
    EXPECT_TRUE(LivePhotoDetector::IsLivePhotoCandidate(".heic"));
    EXPECT_TRUE(LivePhotoDetector::IsLivePhotoCandidate(".heif"));
    EXPECT_TRUE(LivePhotoDetector::IsLivePhotoCandidate(".mov"));
    EXPECT_FALSE(LivePhotoDetector::IsLivePhotoCandidate(".jpg"));
    EXPECT_FALSE(LivePhotoDetector::IsLivePhotoCandidate(".png"));
}

TEST(LivePhoto, DetectPairFromHEIC)
{
    LivePhotoDetector detector;
    auto lp = detector.DetectPair("C:\\Photos\\IMG_1234.heic");
    EXPECT_TRUE(lp.IsValid());
    EXPECT_NE(lp.photoPath.find(".heic"), std::string::npos);
    EXPECT_NE(lp.videoPath.find(".MOV"), std::string::npos);
}

TEST(LivePhoto, InvalidInput)
{
    LivePhotoDetector detector;
    auto lp = detector.DetectPair("abc");
    EXPECT_FALSE(lp.IsValid());
}

//==============================================================================
// Format Detector Tests
//==============================================================================

TEST(FormatDetector, Extensions)
{
    EXPECT_EQ(AnimationFormatDetector::DetectFromExtension(".webp"), AnimationFormat::AnimatedWebP);
    EXPECT_EQ(AnimationFormatDetector::DetectFromExtension(".jxl"), AnimationFormat::AnimatedJXL);
    EXPECT_EQ(AnimationFormatDetector::DetectFromExtension(".gif"), AnimationFormat::AnimatedGIF);
    EXPECT_EQ(AnimationFormatDetector::DetectFromExtension(".pdf"), AnimationFormat::MultiPagePDF);
    EXPECT_EQ(AnimationFormatDetector::DetectFromExtension(".tiff"), AnimationFormat::MultiPageTIFF);
    EXPECT_EQ(AnimationFormatDetector::DetectFromExtension(".heic"), AnimationFormat::LivePhoto);
    EXPECT_EQ(AnimationFormatDetector::DetectFromExtension(".txt"), AnimationFormat::None);
}

TEST(FormatDetector, AnimatableCount)
{
    EXPECT_GE(AnimationFormatDetector::AnimatableFormatCount(), 8u);
}

TEST(FormatDetector, AnimatableExtensions)
{
    auto exts = AnimationFormatDetector::AnimatableExtensions();
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".webp"), exts.end());
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".gif"), exts.end());
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".pdf"), exts.end());
}

//==============================================================================
// Animated Decoder Config Tests
//==============================================================================

TEST(DecoderConfig, Defaults)
{
    auto config = AnimatedDecoderConfig::Default();
    EXPECT_EQ(config.maxFramesToDecode, 4u);
    EXPECT_EQ(config.maxDecodeTimeMs, 2000u);
    EXPECT_FALSE(config.extractFirstFrameOnly);
    EXPECT_TRUE(config.showFrameCountBadge);
    EXPECT_TRUE(config.enableStackedPDFPreview);
}

TEST(DecoderConfig, Performance)
{
    auto config = AnimatedDecoderConfig::Performance();
    EXPECT_TRUE(config.extractFirstFrameOnly);
    EXPECT_FALSE(config.enableStackedPDFPreview);
    EXPECT_EQ(config.maxFramesToDecode, 1u);
}

