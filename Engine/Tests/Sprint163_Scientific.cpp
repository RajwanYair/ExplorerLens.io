// Scientific Format Plugin — GTest
#include "../Decoders/ScientificFormatPlugin.h"
#include "GTestShim.h"

using namespace ExplorerLens::Decoders;

TEST(ScientificFormatPlugin, SupportedExtensionsNotEmpty) {
  auto exts = ScientificFormatPlugin::SupportedExtensions();
  EXPECT_GE(exts.size(), 3u);
}

TEST(ScientificFormatPlugin, FITSDetection) {
  EXPECT_EQ(DetectScientificFormat(".fits"), ScientificFormat::FITS);
  EXPECT_EQ(DetectScientificFormat(".fit"), ScientificFormat::FITS);
  EXPECT_EQ(DetectScientificFormat(".fts"), ScientificFormat::FITS);
}

TEST(ScientificFormatPlugin, NIfTIDetection) {
  EXPECT_EQ(DetectScientificFormat(".nii"), ScientificFormat::NIfTI);
}

TEST(ScientificFormatPlugin, UnknownExtension) {
  EXPECT_EQ(DetectScientificFormat(".tiff"), ScientificFormat::Unknown);
}

TEST(ScientificFormatPlugin, FITSHeaderValid) {
  FITSHeader h;
  h.naxis1 = 1024;
  h.naxis2 = 1024;
  EXPECT_TRUE(h.IsValid());
}

TEST(ScientificFormatPlugin, FITSHeaderInvalidWhenZero) {
  FITSHeader h;
  EXPECT_FALSE(h.IsValid());
}

TEST(ScientificFormatPlugin, FITSHeaderSingleFrame) {
  FITSHeader h;
  h.naxis = 2;
  h.naxis1 = 100;
  h.naxis2 = 100;
  EXPECT_EQ(h.FrameCount(), 1u);
}

TEST(ScientificFormatPlugin, NIfTIHeaderMiddleSlice) {
  NIfTIHeader h;
  h.dimZ = 100;
  EXPECT_EQ(h.MiddleSlice(), 50u);
}

TEST(ScientificFormatPlugin, LUTToStringNotEmpty) {
  EXPECT_FALSE(ToString(ScientificLUT::Viridis).empty());
}

TEST(ScientificFormatPlugin, Viridis5StopLUT) {
  auto lut = GetViridisLUT5();
  EXPECT_EQ(lut.size(), 5u);
}

TEST(ScientificFormatPlugin, DecodeSuccess) {
  ScientificDecodeRequest req;
  req.size = 1024;
  req.format = ScientificFormat::FITS;
  req.targetW = 256;
  req.targetH = 256;
  auto r = ScientificFormatPlugin::Decode(req);
  EXPECT_TRUE(r.success);
}

TEST(ScientificFormatPlugin, DecodeFailEmptyData) {
  ScientificDecodeRequest req;
  req.size = 0;
  auto r = ScientificFormatPlugin::Decode(req);
  EXPECT_FALSE(r.success);
}

TEST(ScientificFormatPlugin, IsPluginBacked) {
  EXPECT_TRUE(ScientificFormatPlugin::IsPluginBacked());
}

TEST(ScientificFormatPlugin, DecodeOutputDimensions) {
  ScientificDecodeRequest req;
  req.size = 512;
  req.format = ScientificFormat::NIfTI;
  req.targetW = 128;
  req.targetH = 128;
  auto r = ScientificFormatPlugin::Decode(req);
  EXPECT_EQ(r.widthPx, 128u);
  EXPECT_EQ(r.heightPx, 128u);
}
