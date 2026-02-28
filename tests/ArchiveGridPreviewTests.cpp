//==============================================================================
// ExplorerLens — Archive Content Grid Preview
// Tests archive format detection, grid layout engine, comic book layout,
// page count badges, archive grid config, and grid result summaries.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

// Header under test
#include "../Engine/Decoders/ArchiveGridPreview.h"

using namespace ExplorerLens::Engine::Decoders;

//==============================================================================
// Archive Format Detection Tests
//==============================================================================

TEST(ArchiveFormat, FormatNames)
{
    EXPECT_STREQ(ArchiveFormatName(ArchiveFormat::ZIP), "ZIP");
    EXPECT_STREQ(ArchiveFormatName(ArchiveFormat::RAR), "RAR");
    EXPECT_STREQ(ArchiveFormatName(ArchiveFormat::SevenZip), "7z");
    EXPECT_STREQ(ArchiveFormatName(ArchiveFormat::TAR), "TAR");
    EXPECT_STREQ(ArchiveFormatName(ArchiveFormat::CBZ), "CBZ");
    EXPECT_STREQ(ArchiveFormatName(ArchiveFormat::CBR), "CBR");
    EXPECT_STREQ(ArchiveFormatName(ArchiveFormat::CB7), "CB7");
}

TEST(ArchiveFormat, DetectFromExtension)
{
    EXPECT_EQ(DetectArchiveFormat(".zip"), ArchiveFormat::ZIP);
    EXPECT_EQ(DetectArchiveFormat(".rar"), ArchiveFormat::RAR);
    EXPECT_EQ(DetectArchiveFormat(".7z"),  ArchiveFormat::SevenZip);
    EXPECT_EQ(DetectArchiveFormat(".tar"), ArchiveFormat::TAR);
    EXPECT_EQ(DetectArchiveFormat(".cbz"), ArchiveFormat::CBZ);
    EXPECT_EQ(DetectArchiveFormat(".cbr"), ArchiveFormat::CBR);
    EXPECT_EQ(DetectArchiveFormat(".cb7"), ArchiveFormat::CB7);
    EXPECT_EQ(DetectArchiveFormat(".cbt"), ArchiveFormat::CBT);
    EXPECT_EQ(DetectArchiveFormat(".txt"), ArchiveFormat::Unknown);
}

TEST(ArchiveFormat, IsComicBook)
{
    EXPECT_TRUE(IsComicBookFormat(ArchiveFormat::CBZ));
    EXPECT_TRUE(IsComicBookFormat(ArchiveFormat::CBR));
    EXPECT_TRUE(IsComicBookFormat(ArchiveFormat::CB7));
    EXPECT_TRUE(IsComicBookFormat(ArchiveFormat::CBT));
    EXPECT_FALSE(IsComicBookFormat(ArchiveFormat::ZIP));
    EXPECT_FALSE(IsComicBookFormat(ArchiveFormat::RAR));
}

TEST(ArchiveFormat, SupportedExtensions)
{
    auto exts = SupportedArchiveExtensions();
    EXPECT_GE(exts.size(), 10u);
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".zip"), exts.end());
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".cbz"), exts.end());
}

//==============================================================================
// Archive Image Entry Tests
//==============================================================================

TEST(ArchiveEntry, CompressionRatio)
{
    ArchiveImageEntry entry;
    entry.compressedSize = 50000;
    entry.uncompressedSize = 100000;
    EXPECT_DOUBLE_EQ(entry.CompressionRatio(), 0.5);
}

TEST(ArchiveEntry, CompressionRatioZero)
{
    ArchiveImageEntry entry;
    EXPECT_DOUBLE_EQ(entry.CompressionRatio(), 1.0);
}

TEST(ArchiveEntry, SortKey)
{
    ArchiveImageEntry entry;
    entry.filename = "page_001.jpg";
    EXPECT_EQ(entry.SortKey(), "page_001.jpg");
}

//==============================================================================
// Grid Layout Mode Tests
//==============================================================================

TEST(GridLayout, ModeNames)
{
    EXPECT_STREQ(GridLayoutModeName(GridLayoutMode::Standard2x2), "Standard 2x2");
    EXPECT_STREQ(GridLayoutModeName(GridLayoutMode::CoverPlusThree), "Cover + 3");
    EXPECT_STREQ(GridLayoutModeName(GridLayoutMode::SingleCover), "Single Cover");
    EXPECT_STREQ(GridLayoutModeName(GridLayoutMode::StripHorizontal), "Horizontal Strip");
}

TEST(GridLayout, RecommendedForZIP)
{
    EXPECT_EQ(RecommendedGridLayout(ArchiveFormat::ZIP, 10), GridLayoutMode::Standard2x2);
}

TEST(GridLayout, RecommendedForCBZ)
{
    EXPECT_EQ(RecommendedGridLayout(ArchiveFormat::CBZ, 10), GridLayoutMode::CoverPlusThree);
}

TEST(GridLayout, RecommendedForSingleImage)
{
    EXPECT_EQ(RecommendedGridLayout(ArchiveFormat::ZIP, 1), GridLayoutMode::SingleCover);
}

TEST(GridLayout, RecommendedForNoImages)
{
    EXPECT_EQ(RecommendedGridLayout(ArchiveFormat::ZIP, 0), GridLayoutMode::SingleCover);
}

//==============================================================================
// Grid Layout Engine Tests
//==============================================================================

TEST(GridEngine, CanvasSize)
{
    GridLayoutEngine engine(512);
    EXPECT_EQ(engine.CanvasSize(), 512u);
}

TEST(GridEngine, Layout2x2)
{
    GridLayoutEngine engine(256);
    auto cells = engine.Layout2x2();
    EXPECT_EQ(cells.size(), 4u);
    // Verify all 4 cells have different indices
    for (uint32_t i = 0; i < 4; ++i)
        EXPECT_EQ(cells[i].imageIndex, i);
}

TEST(GridEngine, Layout2x2CellSizes)
{
    GridLayoutEngine engine(256);
    auto cells = engine.Layout2x2();
    for (auto& c : cells) {
        EXPECT_GT(c.width, 100.0f);   // Each cell > 100px
        EXPECT_GT(c.height, 100.0f);
        EXPECT_LT(c.width, 140.0f);   // Each cell < 140px
    }
}

TEST(GridEngine, CoverPlusThree)
{
    GridLayoutEngine engine(256);
    auto cells = engine.LayoutCoverPlusThree();
    EXPECT_EQ(cells.size(), 4u);
    EXPECT_TRUE(cells[0].isCover);
    // Cover should be wider than interior cells
    EXPECT_GT(cells[0].width, cells[1].width);
}

TEST(GridEngine, SingleCover)
{
    GridLayoutEngine engine(256);
    auto cells = engine.GenerateLayout(GridLayoutMode::SingleCover);
    EXPECT_EQ(cells.size(), 1u);
    EXPECT_EQ(cells[0].width, 256.0f);
    EXPECT_TRUE(cells[0].isCover);
}

TEST(GridEngine, HorizontalStrip)
{
    GridLayoutEngine engine(256);
    auto cells = engine.GenerateLayout(GridLayoutMode::StripHorizontal);
    EXPECT_EQ(cells.size(), 4u);
    EXPECT_EQ(cells[0].width, 64.0f);  // 256 / 4
}

TEST(GridEngine, GapSetting)
{
    GridLayoutEngine engine(256);
    engine.SetGap(4.0f);
    EXPECT_EQ(engine.Gap(), 4.0f);
}

TEST(GridEngine, CellCenter)
{
    GridCell cell{10.0f, 20.0f, 100.0f, 80.0f, 0, false};
    EXPECT_FLOAT_EQ(cell.CenterX(), 60.0f);
    EXPECT_FLOAT_EQ(cell.CenterY(), 60.0f);
}

TEST(GridEngine, CellArea)
{
    GridCell cell{0, 0, 128.0f, 128.0f, 0, false};
    EXPECT_FLOAT_EQ(cell.Area(), 16384.0f);
}

//==============================================================================
// Page Count Badge Tests
//==============================================================================

TEST(ArchiveBadge, BadgeText)
{
    ArchiveBadge badge;
    badge.imageCount = 42;
    EXPECT_EQ(badge.BadgeText(), "42 images");
}

TEST(ArchiveBadge, EmptyBadge)
{
    ArchiveBadge badge;
    EXPECT_EQ(badge.BadgeText(), "");
    EXPECT_FALSE(badge.ShouldShow());
}

TEST(ArchiveBadge, FormatBadge)
{
    ArchiveBadge badge;
    badge.format = ArchiveFormat::CBZ;
    EXPECT_EQ(badge.FormatBadge(), "CBZ");
}

TEST(ArchiveBadge, ShouldShow)
{
    ArchiveBadge badge;
    badge.imageCount = 0;
    EXPECT_FALSE(badge.ShouldShow());
    badge.imageCount = 5;
    EXPECT_TRUE(badge.ShouldShow());
}

//==============================================================================
// Archive Grid Config Tests
//==============================================================================

TEST(GridConfig, Default)
{
    auto config = ArchiveGridConfig::Default();
    EXPECT_TRUE(config.enabled);
    EXPECT_EQ(config.canvasSize, 256u);
    EXPECT_EQ(config.maxImagesToDecode, 4u);
    EXPECT_TRUE(config.showPageCountBadge);
    EXPECT_TRUE(config.useCoverLayoutForComics);
}

TEST(GridConfig, Disabled)
{
    auto config = ArchiveGridConfig::Disabled();
    EXPECT_FALSE(config.enabled);
}

TEST(GridConfig, HighQuality)
{
    auto config = ArchiveGridConfig::HighQuality();
    EXPECT_EQ(config.canvasSize, 512u);
    EXPECT_EQ(config.maxImagesToScan, 50u);
    EXPECT_GT(config.shadowSize, 2.0f);
}

//==============================================================================
// Archive Grid Result Tests
//==============================================================================

TEST(GridResult, DefaultFailed)
{
    ArchiveGridResult result;
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.HasContent());
}

TEST(GridResult, SuccessResult)
{
    ArchiveGridResult result;
    result.success = true;
    result.format = ArchiveFormat::CBZ;
    result.layoutMode = GridLayoutMode::CoverPlusThree;
    result.imagesFound = 42;
    result.imagesDecoded = 4;
    result.totalFiles = 45;
    result.decodeTimeMs = 35.5;
    EXPECT_TRUE(result.HasContent());
}

TEST(GridResult, Summary)
{
    ArchiveGridResult result;
    result.format = ArchiveFormat::ZIP;
    result.layoutMode = GridLayoutMode::Standard2x2;
    result.imagesFound = 20;
    result.imagesDecoded = 4;
    result.decodeTimeMs = 50.0;
    auto s = result.Summary();
    EXPECT_NE(s.find("ZIP"), std::string::npos);
    EXPECT_NE(s.find("4/20"), std::string::npos);
    EXPECT_NE(s.find("Standard 2x2"), std::string::npos);
    EXPECT_NE(s.find("50ms"), std::string::npos);
}

TEST(GridResult, ErrorResult)
{
    ArchiveGridResult result;
    result.success = false;
    result.errorMessage = "Archive corrupted";
    EXPECT_FALSE(result.HasContent());
    EXPECT_EQ(result.errorMessage, "Archive corrupted");
}

