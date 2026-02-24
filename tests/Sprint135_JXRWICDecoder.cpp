// Sprint 135 — JPEG XR WIC Decoder Tests
#include <gtest/gtest.h>
#include "Decoders/JXRWICDecoder.h"
using namespace ExplorerLens::Decoders;

TEST(Sprint135_JXR, Extensions_Supported) {
    EXPECT_TRUE(JXRExtensions::IsSupported(".wdp"));
    EXPECT_TRUE(JXRExtensions::IsSupported(".hdp"));
    EXPECT_TRUE(JXRExtensions::IsSupported(".jxr"));
    EXPECT_FALSE(JXRExtensions::IsSupported(".jpg"));
}
TEST(Sprint135_JXR, ClassifyExtension) {
    EXPECT_EQ(JXRExtensions::ClassifyExtension(".wdp"), JXRFormat::WDP);
    EXPECT_EQ(JXRExtensions::ClassifyExtension(".hdp"), JXRFormat::HDP);
    EXPECT_EQ(JXRExtensions::ClassifyExtension(".jxr"), JXRFormat::JXR);
}
TEST(Sprint135_JXR, FormatName) {
    EXPECT_STREQ(JXRFormatName(JXRFormat::JXR), "JPEG XR (.jxr)");
}
TEST(Sprint135_JXR, PixelBytes) {
    EXPECT_EQ(JXRPixelBytes(JXRPixelFormat::BGR24), 3);
    EXPECT_EQ(JXRPixelBytes(JXRPixelFormat::BGRA32), 4);
    EXPECT_EQ(JXRPixelBytes(JXRPixelFormat::RGB128F), 16);
}
TEST(Sprint135_JXR, PixelFormatName) {
    EXPECT_STREQ(JXRPixelFormatName(JXRPixelFormat::BGRA32), "32bpp BGRA");
}
TEST(Sprint135_JXR, ImageInfo_Valid) {
    JXRImageInfo info;
    EXPECT_FALSE(info.IsValid());
    info.width = 100; info.height = 100;
    EXPECT_TRUE(info.IsValid());
}
TEST(Sprint135_JXR, ImageInfo_NeedsConversion) {
    JXRImageInfo info;
    info.pixelFormat = JXRPixelFormat::BGRA32;
    EXPECT_FALSE(info.NeedsConversion());
    info.pixelFormat = JXRPixelFormat::RGB48;
    EXPECT_TRUE(info.NeedsConversion());
}
TEST(Sprint135_JXR, DecodeOptions_Thumbnail) {
    auto opts = JXRDecodeOptions::Thumbnail(128);
    EXPECT_EQ(opts.targetWidth, 128u);
    EXPECT_TRUE(opts.useEmbeddedThumbnail);
}
TEST(Sprint135_JXR, Decoder_IsAvailable) {
    auto dec = JXRWICDecoder::Create();
    EXPECT_TRUE(dec.IsAvailable());
}
TEST(Sprint135_JXR, Decoder_ReadInfo) {
    auto dec = JXRWICDecoder::Create();
    auto info = dec.ReadInfo("test.jxr");
    EXPECT_TRUE(info.IsValid());
    EXPECT_EQ(info.format, JXRFormat::JXR);
}
TEST(Sprint135_JXR, Decoder_DecodeThumbnail) {
    auto dec = JXRWICDecoder::Create();
    auto result = dec.DecodeThumbnail("test.wdp", 256);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_GT(result.decodedWidth, 0u);
    EXPECT_GT(result.decodedHeight, 0u);
}
TEST(Sprint135_JXR, StatusName) {
    EXPECT_STREQ(JXRStatusName(JXRDecodeStatus::Success), "Success");
    EXPECT_STREQ(JXRStatusName(JXRDecodeStatus::WICNotAvailable), "WIC not available");
}
TEST(Sprint135_JXR, ImageInfo_EstimateSize) {
    JXRImageInfo info;
    info.width = 1920; info.height = 1080;
    info.pixelFormat = JXRPixelFormat::BGRA32;
    EXPECT_EQ(info.EstimateDecodedSize(), 1920u * 1080 * 4);
}

