#include <gtest/gtest.h>
#include "Decoders/EBookCoverExtractor.h"
using namespace ExplorerLens::Decoders;

TEST(Sprint136_EBook, Extensions_AllSupported) {
    EXPECT_TRUE(EBookExtensions::IsSupported(".epub"));
    EXPECT_TRUE(EBookExtensions::IsSupported(".mobi"));
    EXPECT_TRUE(EBookExtensions::IsSupported(".azw3"));
    EXPECT_TRUE(EBookExtensions::IsSupported(".fb2"));
    EXPECT_TRUE(EBookExtensions::IsSupported(".djvu"));
    EXPECT_FALSE(EBookExtensions::IsSupported(".pdf"));
}
TEST(Sprint136_EBook, Extensions_NewFormats) {
    EXPECT_TRUE(EBookExtensions::IsNewFormat(".mobi"));
    EXPECT_TRUE(EBookExtensions::IsNewFormat(".azw3"));
    EXPECT_FALSE(EBookExtensions::IsNewFormat(".epub"));
}
TEST(Sprint136_EBook, ClassifyExtension) {
    EXPECT_EQ(EBookExtensions::ClassifyExtension(".mobi"), EBookFormat::MOBI);
    EXPECT_EQ(EBookExtensions::ClassifyExtension(".azw3"), EBookFormat::AZW3);
    EXPECT_EQ(EBookExtensions::ClassifyExtension(".fb2"), EBookFormat::FB2);
    EXPECT_EQ(EBookExtensions::ClassifyExtension(".djvu"), EBookFormat::DJVU);
}
TEST(Sprint136_EBook, FormatName) {
    EXPECT_STREQ(EBookFormatName(EBookFormat::MOBI), "Mobipocket");
    EXPECT_STREQ(EBookFormatName(EBookFormat::AZW3), "Kindle KF8");
    EXPECT_STREQ(EBookFormatName(EBookFormat::FB2), "FictionBook 2");
}
TEST(Sprint136_EBook, MOBIHeader_HasUsableCover) {
    MOBIHeaderInfo hdr;
    hdr.hasCover = true; hdr.isDRM = false;
    EXPECT_TRUE(hdr.HasUsableCover());
    hdr.isDRM = true;
    EXPECT_FALSE(hdr.HasUsableCover());
}
TEST(Sprint136_EBook, CoverStatusName) {
    EXPECT_STREQ(CoverStatusName(CoverExtractionStatus::Success), "Success");
    EXPECT_STREQ(CoverStatusName(CoverExtractionStatus::DRMProtected), "DRM-protected file");
}
TEST(Sprint136_EBook, ExtractCover_MOBI) {
    auto ext = EBookCoverExtractor::Create();
    auto result = ext.ExtractCover("test.mobi");
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.sourceFormat, EBookFormat::MOBI);
}
TEST(Sprint136_EBook, ExtractCover_FB2) {
    auto ext = EBookCoverExtractor::Create();
    auto result = ext.ExtractCover("test.fb2");
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.sourceFormat, EBookFormat::FB2);
}
TEST(Sprint136_EBook, ExtractCover_DjVu) {
    auto ext = EBookCoverExtractor::Create();
    auto result = ext.ExtractCover("book.djvu");
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.sourceFormat, EBookFormat::DJVU);
}
TEST(Sprint136_EBook, ExtractCover_Unsupported) {
    auto ext = EBookCoverExtractor::Create();
    auto result = ext.ExtractCover("document.pdf");
    EXPECT_EQ(result.status, CoverExtractionStatus::UnsupportedFormat);
}
TEST(Sprint136_EBook, CanExtract) {
    auto ext = EBookCoverExtractor::Create();
    EXPECT_TRUE(ext.CanExtract(".mobi"));
    EXPECT_FALSE(ext.CanExtract(".txt"));
}
TEST(Sprint136_EBook, CoverResult_HasImage) {
    CoverImageResult r;
    EXPECT_FALSE(r.HasImage());
    r.imageData.push_back(0xFF);
    EXPECT_TRUE(r.HasImage());
}

