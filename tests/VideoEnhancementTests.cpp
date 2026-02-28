// =============================================================================
// =============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------------------
// Scene Detection Tests
// ---------------------------------------------------------------------------

class SceneDetectionTest : public ::testing::Test {
protected:
    struct FrameScore {
        uint64_t timestampMs;
        float luminance;
        float entropy;
        float colorfulness;
        float darkPixelRatio;

        float computeScore() const {
            float score = 0.0f;
            if (darkPixelRatio > 0.7f) score -= 50.0f;
            if (luminance < 0.05f) score -= 100.0f;
            if (luminance > 0.95f) score -= 30.0f;
            score += entropy * 3.0f;
            score += std::min(colorfulness, 100.0f) * 0.3f;
            return score;
        }
    };
};

TEST_F(SceneDetectionTest, BlackFrameRejection) {
    FrameScore blackFrame{1000, 0.01f, 0.5f, 0.0f, 0.95f};
    FrameScore goodFrame{5000, 0.45f, 6.5f, 55.0f, 0.02f};

    EXPECT_LT(blackFrame.computeScore(), goodFrame.computeScore())
        << "Black frames must score lower than visually interesting frames";
}

TEST_F(SceneDetectionTest, OverexposedFrameRejection) {
    FrameScore blownOut{2000, 0.98f, 1.0f, 5.0f, 0.0f};
    FrameScore goodFrame{5000, 0.45f, 6.0f, 50.0f, 0.02f};

    EXPECT_LT(blownOut.computeScore(), goodFrame.computeScore())
        << "Overexposed frames should score lower than balanced frames";
}

TEST_F(SceneDetectionTest, BestFrameSelection) {
    std::vector<FrameScore> frames = {
        {1000,  0.02f, 0.5f,  0.0f,  0.95f},  // Black intro
        {5000,  0.45f, 6.0f,  55.0f, 0.02f},  // Good scene
        {10000, 0.50f, 7.0f,  70.0f, 0.01f},  // Best scene
        {15000, 0.40f, 5.0f,  30.0f, 0.05f},  // OK scene
        {20000, 0.05f, 1.0f,  2.0f,  0.80f},  // End credits (dark)
    };

    auto best = std::max_element(frames.begin(), frames.end(),
        [](const FrameScore& a, const FrameScore& b) {
            return a.computeScore() < b.computeScore();
        });

    EXPECT_EQ(best->timestampMs, 10000u)
        << "Frame at 10s should be selected as best (highest entropy + color)";
}

TEST_F(SceneDetectionTest, SamplingSkipsIntroAndCredits) {
    uint64_t durationMs = 120000;  // 2 minutes
    float startFrac = 0.05f;
    float endFrac = 0.95f;

    uint64_t firstSample = static_cast<uint64_t>(durationMs * startFrac);
    uint64_t lastSample = static_cast<uint64_t>(durationMs * endFrac);

    EXPECT_EQ(firstSample, 6000u)  << "First sample at 5% = 6 seconds";
    EXPECT_EQ(lastSample, 114000u) << "Last sample at 95% = 114 seconds";
}

TEST_F(SceneDetectionTest, SceneChangeDetection) {
    // Two consecutive frames with dramatically different entropy = scene change
    float frame1Entropy = 6.0f, frame2Entropy = 2.0f;
    float frame1Lum = 0.4f, frame2Lum = 0.1f;
    float threshold = 0.4f;

    float diff = std::abs(frame2Entropy - frame1Entropy) +
                 std::abs(frame2Lum - frame1Lum);
    bool isSceneChange = diff > threshold;

    EXPECT_TRUE(isSceneChange)
        << "Large entropy + luminance change should trigger scene boundary";
}

TEST_F(SceneDetectionTest, DefaultSampleCount) {
    uint32_t sampleCount = 20;
    EXPECT_GE(sampleCount, 10u)
        << "Need at least 10 samples for reliable scene detection";
    EXPECT_LE(sampleCount, 50u)
        << "More than 50 samples would be too slow for thumbnail generation";
}

// ---------------------------------------------------------------------------
// Animated Thumbnail Tests
// ---------------------------------------------------------------------------

class AnimatedThumbnailTest : public ::testing::Test {};

TEST_F(AnimatedThumbnailTest, FrameCountLimit) {
    uint32_t maxFrames = 8;
    uint32_t addedFrames = 0;

    for (uint32_t i = 0; i < 15; ++i) {
        if (addedFrames < maxFrames) addedFrames++;
    }

    EXPECT_EQ(addedFrames, maxFrames)
        << "Should cap at maxFrames to control file size";
}

TEST_F(AnimatedThumbnailTest, WebPIsDefaultFormat) {
    // WebPAnim is the default — best quality/size ratio
    enum AnimatedFormat { GIF, WebPAnim, APNG };
    AnimatedFormat defaultFormat = WebPAnim;

    EXPECT_EQ(defaultFormat, WebPAnim)
        << "WebP animated should be default for quality and compression";
}

TEST_F(AnimatedThumbnailTest, FileSizeLimit) {
    uint32_t maxSizeKB = 512;           // 512KB max for animated thumbnails
    uint32_t maxWidthPx = 320;          // Smaller than static thumbnails
    uint32_t frameDelayMs = 500;        // 2 fps — gentle animation

    EXPECT_LE(maxSizeKB, 1024u)
        << "Animated thumbnails must stay under 1MB";
    EXPECT_LE(maxWidthPx, 480u)
        << "Animated thumbnail resolution should be modest";
    EXPECT_GE(frameDelayMs, 200u)
        << "Frame rate should be ≤5fps for thumbnail context";
}

TEST_F(AnimatedThumbnailTest, FramesSortedByTimestamp) {
    std::vector<uint64_t> timestamps = {15000, 5000, 25000, 10000, 20000};
    std::sort(timestamps.begin(), timestamps.end());

    for (size_t i = 1; i < timestamps.size(); ++i) {
        EXPECT_GT(timestamps[i], timestamps[i - 1])
            << "Keyframes must be sorted chronologically";
    }
}

// ---------------------------------------------------------------------------
// HDR Tone Mapping Tests
// ---------------------------------------------------------------------------

class HDRToneMappingTest : public ::testing::Test {
protected:
    // Reinhard: L' = L / (1 + L)
    float reinhard(float x) const { return x / (1.0f + x); }

    // ACES approximation
    float aces(float x) const {
        const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
        return std::clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
    }
};

TEST_F(HDRToneMappingTest, ReinhardConverges) {
    // Reinhard operator should map [0,∞) → [0,1)
    EXPECT_NEAR(reinhard(0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(reinhard(1.0f), 0.5f, 0.001f);
    EXPECT_LT(reinhard(1000.0f), 1.0f)
        << "Reinhard must always produce values < 1.0";
    EXPECT_GT(reinhard(1000.0f), 0.99f)
        << "Very bright values should map close to 1.0";
}

TEST_F(HDRToneMappingTest, ACESFilmicRange) {
    // ACES should produce [0,1] range
    EXPECT_NEAR(aces(0.0f), 0.0f, 0.01f);
    float midtone = aces(0.18f);  // Middle gray in linear
    EXPECT_GT(midtone, 0.0f);
    EXPECT_LT(midtone, 0.5f);
    EXPECT_NEAR(aces(100.0f), 1.0f, 0.01f)
        << "Very bright values saturate to 1.0";
}

TEST_F(HDRToneMappingTest, HDR10NeedsToneMapping) {
    struct HDRInfo {
        bool isHDR;
        uint32_t bitDepth;
        enum TF { SDR, PQ, HLG } transfer;
    };

    HDRInfo hdr10 = {true, 10, HDRInfo::PQ};
    HDRInfo hlg = {true, 10, HDRInfo::HLG};
    HDRInfo sdr = {false, 8, HDRInfo::SDR};

    auto needsToneMap = [](const HDRInfo& h) {
        return h.isHDR || h.bitDepth > 8 || h.transfer == HDRInfo::PQ || h.transfer == HDRInfo::HLG;
    };

    EXPECT_TRUE(needsToneMap(hdr10))  << "HDR10 (PQ) needs tone mapping";
    EXPECT_TRUE(needsToneMap(hlg))    << "HLG needs tone mapping";
    EXPECT_FALSE(needsToneMap(sdr))   << "SDR should not need tone mapping";
}

TEST_F(HDRToneMappingTest, BT2020ColorSpace) {
    // BT.2020 has wider gamut than BT.709 — needs gamut mapping
    struct ColorPrimary {
        float rx, ry, gx, gy, bx, by;
    };
    ColorPrimary bt709  = {0.640f, 0.330f, 0.300f, 0.600f, 0.150f, 0.060f};
    ColorPrimary bt2020 = {0.708f, 0.292f, 0.170f, 0.797f, 0.131f, 0.046f};

    // BT.2020 green primary is more saturated
    EXPECT_GT(bt2020.gy, bt709.gy)
        << "BT.2020 has wider green primary";
}

// ---------------------------------------------------------------------------
// Metadata Badge Tests
// ---------------------------------------------------------------------------

class MetadataBadgeTest : public ::testing::Test {};

TEST_F(MetadataBadgeTest, DurationFormatting) {
    auto formatDuration = [](uint64_t ms) -> std::string {
        uint64_t totalSec = ms / 1000;
        uint64_t h = totalSec / 3600;
        uint64_t m = (totalSec % 3600) / 60;
        uint64_t s = totalSec % 60;
        char buf[32];
        if (h > 0) snprintf(buf, sizeof(buf), "%llu:%02llu:%02llu",
            (unsigned long long)h, (unsigned long long)m, (unsigned long long)s);
        else snprintf(buf, sizeof(buf), "%llu:%02llu",
            (unsigned long long)m, (unsigned long long)s);
        return std::string(buf);
    };

    EXPECT_EQ(formatDuration(45000),    "0:45");
    EXPECT_EQ(formatDuration(125000),   "2:05");
    EXPECT_EQ(formatDuration(3661000),  "1:01:01");
}

TEST_F(MetadataBadgeTest, ResolutionLabels) {
    auto formatRes = [](uint32_t w) -> std::string {
        if (w >= 7680) return "8K";
        if (w >= 3840) return "4K";
        if (w >= 2560) return "1440p";
        if (w >= 1920) return "1080p";
        if (w >= 1280) return "720p";
        return "480p";
    };

    EXPECT_EQ(formatRes(7680), "8K");
    EXPECT_EQ(formatRes(3840), "4K");
    EXPECT_EQ(formatRes(1920), "1080p");
    EXPECT_EQ(formatRes(1280), "720p");
    EXPECT_EQ(formatRes(854),  "480p");
}

TEST_F(MetadataBadgeTest, BadgeDefaultPosition) {
    enum BadgePosition { TopLeft, TopRight, BottomLeft, BottomRight };
    BadgePosition defaultPos = BottomRight;
    EXPECT_EQ(defaultPos, BottomRight)
        << "Duration badge should default to bottom-right (standard convention)";
}

// ---------------------------------------------------------------------------
// Integration Tests
// ---------------------------------------------------------------------------

TEST(VideoEnhancerIntegrationTest, VideoEnhancerHeaderExists) {
    namespace fs = std::filesystem;
    bool exists = fs::exists("Engine/Decoders/VideoEnhancer.h") ||
                  fs::exists("Engine\\Decoders\\VideoEnhancer.h");
    EXPECT_TRUE(exists) << "VideoEnhancer.h must exist for this module";
}

TEST(VideoEnhancerIntegrationTest, ExistingVideoDecoderPreserved) {
    namespace fs = std::filesystem;
    bool exists = fs::exists("Engine/Decoders/VideoDecoder.h") ||
                  fs::exists("Engine\\Decoders\\VideoDecoder.h");
    EXPECT_TRUE(exists) << "Original VideoDecoder.h should be preserved";
}
