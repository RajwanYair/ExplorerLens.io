#include <gtest/gtest.h>
#include "Decoders/JPEG2000Decoder.h"
using namespace ExplorerLens::Decoders;

TEST(JP2, Extensions_Supported) {
    EXPECT_TRUE(JP2Extensions::IsSupported(".jp2"));
    EXPECT_TRUE(JP2Extensions::IsSupported(".j2k"));
    EXPECT_TRUE(JP2Extensions::IsSupported(".jph"));
    EXPECT_FALSE(JP2Extensions::IsSupported(".jpg"));
}
TEST(JP2, Extensions_CaseInsensitive) {
    EXPECT_TRUE(JP2Extensions::IsSupported(".JP2"));
    EXPECT_TRUE(JP2Extensions::IsSupported(".J2K"));
}
TEST(JP2, ClassifyExtension) {
    EXPECT_EQ(JP2Extensions::ClassifyExtension(".jp2"), JP2Format::JP2);
    EXPECT_EQ(JP2Extensions::ClassifyExtension(".j2k"), JP2Format::J2K);
    EXPECT_EQ(JP2Extensions::ClassifyExtension(".jpx"), JP2Format::JPX);
    EXPECT_EQ(JP2Extensions::ClassifyExtension(".jph"), JP2Format::JPH);
    EXPECT_EQ(JP2Extensions::ClassifyExtension(".bmp"), JP2Format::Unknown);
}
TEST(JP2, FormatName) {
    EXPECT_STREQ(JP2FormatName(JP2Format::JP2), "JPEG 2000 (.jp2)");
    EXPECT_STREQ(JP2FormatName(JP2Format::JPH), "JPEG 2000 HTJ2K (.jph)");
}
TEST(JP2, ImageInfo_Valid) {
    JP2ImageInfo info;
    EXPECT_FALSE(info.IsValid());
    info.width = 100; info.height = 100; info.numComponents = 3;
    EXPECT_TRUE(info.IsValid());
}
TEST(JP2, ImageInfo_ReductionLevel) {
    JP2ImageInfo info;
    info.width = 4096; info.height = 4096;
    info.numResolutionLevels = 6;
    uint32_t level = info.BestReductionLevel(256, 256);
    EXPECT_GE(level, 3u);  // 4096 → 2048 → 1024 → 512 → stop (≈256 threshold)
}
TEST(JP2, ImageInfo_EstimateSize) {
    JP2ImageInfo info;
    info.width = 1024; info.height = 1024; info.numComponents = 3; info.bitsPerComponent = 8;
    EXPECT_EQ(info.EstimateDecodedSize(), 1024u * 1024 * 3);
}
TEST(JP2, ImageInfo_EstimateReducedSize) {
    JP2ImageInfo info;
    info.width = 1024; info.height = 1024; info.numComponents = 3; info.bitsPerComponent = 8;
    size_t reduced = info.EstimateReducedSize(2);
    EXPECT_EQ(reduced, 256u * 256 * 3);
}
TEST(JP2, DecodeOptions_Thumbnail) {
    auto opts = JP2DecodeOptions::Thumbnail(128);
    EXPECT_EQ(opts.maxWidth, 128u);
    EXPECT_TRUE(opts.autoSelectLevel);
}
TEST(JP2, Decoder_IsAvailable) {
    auto dec = JPEG2000Decoder::Create();
    EXPECT_TRUE(dec.IsAvailable());
}
TEST(JP2, Decoder_ReadInfo) {
    auto dec = JPEG2000Decoder::Create();
    auto info = dec.ReadInfo("test.jp2");
    EXPECT_TRUE(info.IsValid());
    EXPECT_EQ(info.format, JP2Format::JP2);
}
TEST(JP2, Decoder_DecodeThumbnail) {
    auto dec = JPEG2000Decoder::Create();
    auto result = dec.DecodeThumbnail("test.jp2", 256);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_GT(result.decodedWidth, 0u);
}
TEST(JP2, StatusName) {
    EXPECT_STREQ(JP2StatusName(JP2DecodeStatus::Success), "Success");
    EXPECT_STREQ(JP2StatusName(JP2DecodeStatus::CorruptData), "Corrupt codestream data");
}
TEST(JP2, StaticExtCheck) {
    EXPECT_TRUE(JPEG2000Decoder::IsJP2Extension(".jp2"));
    EXPECT_FALSE(JPEG2000Decoder::IsJP2Extension(".png"));
}

