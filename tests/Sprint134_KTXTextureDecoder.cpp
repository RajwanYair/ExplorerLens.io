// Sprint 134 — KTX/KTX2 Texture Decoder Tests
#include <gtest/gtest.h>
#include "Decoders/KTXTextureDecoder.h"
using namespace ExplorerLens::Decoders;

TEST(Sprint134_KTX, Extensions_Supported) {
    EXPECT_TRUE(KTXExtensions::IsSupported(".ktx"));
    EXPECT_TRUE(KTXExtensions::IsSupported(".ktx2"));
    EXPECT_FALSE(KTXExtensions::IsSupported(".png"));
}
TEST(Sprint134_KTX, VersionFromExtension) {
    EXPECT_EQ(KTXExtensions::VersionFromExtension(".ktx"), KTXVersion::KTX1);
    EXPECT_EQ(KTXExtensions::VersionFromExtension(".ktx2"), KTXVersion::KTX2);
    EXPECT_EQ(KTXExtensions::VersionFromExtension(".jpg"), KTXVersion::Unknown);
}
TEST(Sprint134_KTX, CompressionName) {
    EXPECT_STREQ(CompressionName(TextureCompression::BC7_RGBA), "BC7 (RGBA)");
    EXPECT_STREQ(CompressionName(TextureCompression::ASTC_4x4), "ASTC 4x4");
}
TEST(Sprint134_KTX, IsBlockCompressed) {
    EXPECT_TRUE(IsBlockCompressed(TextureCompression::BC1_RGB));
    EXPECT_FALSE(IsBlockCompressed(TextureCompression::Uncompressed));
}
TEST(Sprint134_KTX, TextureInfo_Valid) {
    KTXTextureInfo info;
    EXPECT_FALSE(info.IsValid());
    info.width = 256; info.height = 256; info.version = KTXVersion::KTX2;
    EXPECT_TRUE(info.IsValid());
}
TEST(Sprint134_KTX, TextureInfo_BestMip) {
    KTXTextureInfo info;
    info.width = 2048; info.height = 2048; info.mipLevels = 11;
    uint32_t mip = info.BestMipForThumbnail(256);
    EXPECT_GE(mip, 2u);
}
TEST(Sprint134_KTX, TextureInfo_EstimateCompressedSize) {
    KTXTextureInfo info;
    info.width = 1024; info.height = 1024;
    info.compression = TextureCompression::BC1_RGB;
    size_t size = info.EstimateCompressedSize();
    EXPECT_EQ(size, 256u * 256 * 8);  // (1024/4)*(1024/4)*8
}
TEST(Sprint134_KTX, SupercompressionName) {
    EXPECT_STREQ(SupercompressionName(KTXSupercompression::Zstd), "Zstandard");
}
TEST(Sprint134_KTX, Decoder_IsAvailable) {
    auto dec = KTXTextureDecoder::Create();
    EXPECT_TRUE(dec.IsAvailable());
}
TEST(Sprint134_KTX, Decoder_ReadInfo) {
    auto dec = KTXTextureDecoder::Create();
    auto info = dec.ReadInfo("test.ktx2");
    EXPECT_TRUE(info.IsValid());
    EXPECT_EQ(info.version, KTXVersion::KTX2);
}
TEST(Sprint134_KTX, Decoder_DecodeThumbnail) {
    auto dec = KTXTextureDecoder::Create();
    auto result = dec.DecodeThumbnail("test.ktx2", 256);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_GT(result.decodedWidth, 0u);
}
TEST(Sprint134_KTX, TextureInfo_Cubemap) {
    KTXTextureInfo info;
    info.faces = 6; info.isCubemap = true;
    EXPECT_TRUE(info.isCubemap);
    EXPECT_EQ(info.faces, 6u);
}
TEST(Sprint134_KTX, TextureInfo_HasMipmaps) {
    KTXTextureInfo info;
    info.mipLevels = 1;
    EXPECT_FALSE(info.HasMipmaps());
    info.mipLevels = 8;
    EXPECT_TRUE(info.HasMipmaps());
}

