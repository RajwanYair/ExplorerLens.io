// JPEG 2000 Full Decoder — GTest
#include "../Decoders/JPEG2000Decoder.h"
#include "GTestShim.h"

using namespace ExplorerLens::Decoders;

TEST(JPEG2000Decoder, IsAvailable) {
  auto dec = JPEG2000Decoder::Create();
  EXPECT_TRUE(dec.IsAvailable());
}

TEST(JPEG2000Decoder, SupportedExtensionsNotEmpty) {
  EXPECT_TRUE(JPEG2000Decoder::IsJP2Extension(".jp2"));
  EXPECT_TRUE(JPEG2000Decoder::IsJP2Extension(".j2k"));
  EXPECT_TRUE(JPEG2000Decoder::IsJP2Extension(".jpx"));
}

TEST(JPEG2000Decoder, UnsupportedExtensionFalse) {
  EXPECT_FALSE(JPEG2000Decoder::IsJP2Extension(".png"));
  EXPECT_FALSE(JPEG2000Decoder::IsJP2Extension(".jpg"));
}

TEST(JPEG2000Decoder, ReadInfoReturnsValidInfo) {
  auto dec = JPEG2000Decoder::Create();
  auto info = dec.ReadInfo("test.jp2");
  EXPECT_TRUE(info.IsValid());
}

TEST(JPEG2000Decoder, ReadInfoBitDepth8) {
  auto dec = JPEG2000Decoder::Create();
  auto info = dec.ReadInfo("test.jp2");
  EXPECT_EQ(info.bitsPerComponent, 8u);
}

TEST(JPEG2000Decoder, ReadInfoHasResolutionLevels) {
  auto dec = JPEG2000Decoder::Create();
  auto info = dec.ReadInfo("test.j2k");
  EXPECT_GT(info.numResolutionLevels, 0u);
}

TEST(JPEG2000Decoder, DecodeOptionsThumbnailSize) {
  auto opt = JP2DecodeOptions::Thumbnail(256);
  EXPECT_EQ(opt.maxWidth, 256u);
  EXPECT_EQ(opt.maxHeight, 256u);
}

TEST(JPEG2000Decoder, DecodeOptionsThumbnailMemLimit) {
  auto opt = JP2DecodeOptions::Thumbnail(256);
  EXPECT_LE(opt.memoryLimitBytes, 128ULL * 1024 * 1024);
}

TEST(JPEG2000Decoder, DecodeOptionsFull) {
  auto opt = JP2DecodeOptions::FullResolution();
  EXPECT_EQ(opt.reductionLevel, 0u);
}

TEST(JPEG2000Decoder, DecodeThumbnailSuccess) {
  auto dec = JPEG2000Decoder::Create();
  auto res = dec.DecodeThumbnail("test.jp2", 256);
  EXPECT_EQ(res.status, JP2DecodeStatus::Success);
}

TEST(JPEG2000Decoder, DecodeThumbnailDecodeTime) {
  auto dec = JPEG2000Decoder::Create();
  auto res = dec.DecodeThumbnail("test.jp2", 256);
  EXPECT_GT(res.decodeTimeMs, 0.0);
}

TEST(JPEG2000Decoder, BestReductionLevelMatchesTarget) {
  JP2ImageInfo info;
  info.width = 4096;
  info.height = 3072;
  info.numComponents = 3;
  info.bitsPerComponent = 8;
  info.numResolutionLevels = 6;
  uint32_t lvl = info.BestReductionLevel(256, 256);
  (void)lvl;
  EXPECT_GT(lvl, 0u);
}

TEST(JPEG2000Decoder, ClassifyExtensionJ2K) {
  EXPECT_EQ(JP2Extensions::ClassifyExtension(".j2k"), JP2Format::J2K);
}

TEST(JPEG2000Decoder, StatusNameNotEmpty) {
  EXPECT_STRNE(JP2StatusName(JP2DecodeStatus::Success), "");
}

TEST(JPEG2000Decoder, EstimateDecodedSizePositive) {
  JP2ImageInfo info;
  info.width = 1024;
  info.height = 1024;
  info.numComponents = 3;
  info.bitsPerComponent = 8;
  info.numResolutionLevels = 1;
  EXPECT_GT(info.EstimateDecodedSize(), 0u);
}

TEST(JPEG2000Decoder, LibraryNotAvailableStatus) {
  // Test the not-available code path
  JPEG2000Decoder dec;
  // Force not available to test the code path (header-only test)
  // JustVerify status name exists
  EXPECT_STRNE(JP2StatusName(JP2DecodeStatus::LibraryNotAvailable), "");
}

TEST(JPEG2000Decoder, DecodedWidthHeightPositive) {
  auto dec = JPEG2000Decoder::Create();
  auto res = dec.DecodeThumbnail("test.jp2", 256);
  EXPECT_GT(res.decodedWidth, 0u);
  EXPECT_GT(res.decodedHeight, 0u);
}
