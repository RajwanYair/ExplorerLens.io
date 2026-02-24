//==============================================================================
// ExplorerLens — Sprint 37 Tests: Context Menu & Shell UX Integration
// Tests context menu actions, shell property handler, batch operations,
// clipboard/export results, and supported extension detection.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Header under test
#include "../Engine/Shell/ContextMenuHandler.h"

using namespace ExplorerLens::Engine::Shell;

//==============================================================================
// Context Menu Action Tests
//==============================================================================

TEST(ContextAction, ActionNames)
{
    EXPECT_STREQ(ContextMenuActionName(ContextMenuAction::RegenerateThumbnail), "Regenerate Thumbnail");
    EXPECT_STREQ(ContextMenuActionName(ContextMenuAction::CopyToClipboard), "Copy Thumbnail to Clipboard");
    EXPECT_STREQ(ContextMenuActionName(ContextMenuAction::ExportAsPNG), "Export Thumbnail as PNG");
    EXPECT_STREQ(ContextMenuActionName(ContextMenuAction::RegenerateFolder), "Regenerate All Thumbnails");
    EXPECT_STREQ(ContextMenuActionName(ContextMenuAction::ShowProperties), "Show Format Properties");
}

TEST(ContextAction, VerbStrings)
{
    EXPECT_STREQ(ContextMenuVerb(ContextMenuAction::RegenerateThumbnail), "explorerlens.regenerate");
    EXPECT_STREQ(ContextMenuVerb(ContextMenuAction::CopyToClipboard), "explorerlens.copy");
    EXPECT_STREQ(ContextMenuVerb(ContextMenuAction::ExportAsPNG), "explorerlens.export");
    EXPECT_STREQ(ContextMenuVerb(ContextMenuAction::RegenerateFolder), "explorerlens.regenerateall");
}

//==============================================================================
// Context Menu Handler Tests
//==============================================================================

TEST(ContextMenu, DefaultItems)
{
    ContextMenuHandler handler;
    EXPECT_EQ(handler.ItemCount(), 4u);
}

TEST(ContextMenu, FileMenuItems)
{
    ContextMenuHandler handler;
    auto items = handler.GetFileMenuItems();
    EXPECT_EQ(items.size(), 3u);  // Regenerate, Copy, Export (not folder-only)
    for (auto& item : items) {
        EXPECT_FALSE(item.requiresFolder);
        EXPECT_TRUE(item.IsEnabled());
    }
}

TEST(ContextMenu, FolderMenuItems)
{
    ContextMenuHandler handler;
    auto items = handler.GetFolderMenuItems();
    EXPECT_EQ(items.size(), 1u);  // Only "Regenerate All"
    EXPECT_TRUE(items[0].requiresFolder);
}

TEST(ContextMenu, GetItemByAction)
{
    ContextMenuHandler handler;
    auto* item = handler.GetItem(ContextMenuAction::CopyToClipboard);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->action, ContextMenuAction::CopyToClipboard);
    EXPECT_NE(item->displayText.find("Clipboard"), std::string::npos);
}

TEST(ContextMenu, GetItemNotFound)
{
    ContextMenuHandler handler;
    auto* item = handler.GetItem(ContextMenuAction::ShowProperties);
    EXPECT_EQ(item, nullptr);  // ShowProperties not in default set
}

//==============================================================================
// Supported Extension Tests
//==============================================================================

TEST(Extensions, CommonImageFormats)
{
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".jpg"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".jpeg"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".png"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".bmp"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".gif"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".tiff"));
}

TEST(Extensions, ModernFormats)
{
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".webp"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".jxl"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".heif"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".heic"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".avif"));
}

TEST(Extensions, RAWFormats)
{
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".nef"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".cr2"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".cr3"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".arw"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".dng"));
}

TEST(Extensions, ArchiveFormats)
{
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".zip"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".rar"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".7z"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".cbz"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".cbr"));
}

TEST(Extensions, Unsupported)
{
    EXPECT_FALSE(ContextMenuHandler::IsSupportedExtension(".txt"));
    EXPECT_FALSE(ContextMenuHandler::IsSupportedExtension(".doc"));
    EXPECT_FALSE(ContextMenuHandler::IsSupportedExtension(".exe"));
    EXPECT_FALSE(ContextMenuHandler::IsSupportedExtension(".mp3"));
}

TEST(Extensions, CaseInsensitive)
{
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".JPG"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".Png"));
    EXPECT_TRUE(ContextMenuHandler::IsSupportedExtension(".HEIF"));
}

//==============================================================================
// Shell Property Handler Tests
//==============================================================================

TEST(FileProperties, DimensionsString)
{
    FileProperties fp;
    fp.width = 4000;
    fp.height = 3000;
    EXPECT_EQ(fp.DimensionsString(), "4000 x 3000");
}

TEST(FileProperties, FileSizeBytes)
{
    FileProperties fp;
    fp.fileSizeBytes = 512;
    EXPECT_EQ(fp.FileSizeString(), "512 B");
}

TEST(FileProperties, FileSizeKB)
{
    FileProperties fp;
    fp.fileSizeBytes = 150 * 1024;
    EXPECT_EQ(fp.FileSizeString(), "150 KB");
}

TEST(FileProperties, FileSizeMB)
{
    FileProperties fp;
    fp.fileSizeBytes = 5 * 1024 * 1024;
    auto s = fp.FileSizeString();
    EXPECT_NE(s.find("5.0 MB"), std::string::npos);
}

TEST(FileProperties, PropertyList)
{
    FileProperties fp;
    fp.format = "JPEG XL";
    fp.codec = "libjxl 0.11.1";
    fp.width = 3840;
    fp.height = 2160;
    fp.bitsPerPixel = 24;
    fp.hasAlpha = false;
    fp.decodeTimeMs = 12.5;
    fp.fileSizeBytes = 2 * 1024 * 1024;
    fp.colorSpace = "sRGB";

    auto props = fp.ToPropertyList();
    EXPECT_GE(props.size(), 7u);

    // Find format property
    bool foundFormat = false;
    for (auto& p : props) {
        if (p.propertyName == "Format") {
            EXPECT_EQ(p.value, "JPEG XL");
            foundFormat = true;
        }
    }
    EXPECT_TRUE(foundFormat);
}

TEST(FileProperties, AnimatedProperties)
{
    FileProperties fp;
    fp.format = "WebP";
    fp.width = 256; fp.height = 256;
    fp.bitsPerPixel = 32;
    fp.isAnimated = true;
    fp.frameCount = 48;
    fp.codec = "libwebp";
    fp.fileSizeBytes = 500 * 1024;

    auto props = fp.ToPropertyList();
    bool foundFrames = false;
    for (auto& p : props) {
        if (p.propertyName == "Frames") {
            EXPECT_EQ(p.value, "48");
            foundFrames = true;
        }
    }
    EXPECT_TRUE(foundFrames);
}

//==============================================================================
// Batch Operation Tests
//==============================================================================

TEST(BatchState, StateNames)
{
    EXPECT_STREQ(BatchStateName(BatchOperationState::Pending), "Pending");
    EXPECT_STREQ(BatchStateName(BatchOperationState::Running), "Running");
    EXPECT_STREQ(BatchStateName(BatchOperationState::Completed), "Completed");
    EXPECT_STREQ(BatchStateName(BatchOperationState::Cancelled), "Cancelled");
}

TEST(BatchProgress, ProgressPercent)
{
    BatchProgress bp;
    bp.totalFiles = 100;
    bp.processed = 50;
    EXPECT_DOUBLE_EQ(bp.ProgressPercent(), 50.0);
}

TEST(BatchProgress, EmptyProgress)
{
    BatchProgress bp;
    EXPECT_DOUBLE_EQ(bp.ProgressPercent(), 100.0);
}

TEST(BatchProgress, SuccessRate)
{
    BatchProgress bp;
    bp.processed = 10;
    bp.succeeded = 8;
    bp.failed = 2;
    EXPECT_DOUBLE_EQ(bp.SuccessRate(), 80.0);
}

TEST(BatchManager, StartBatch)
{
    BatchOperationManager mgr;
    auto progress = mgr.StartBatch(50);
    EXPECT_EQ(progress.totalFiles, 50u);
    EXPECT_EQ(progress.state, BatchOperationState::Running);
}

TEST(BatchManager, ProcessFiles)
{
    BatchOperationManager mgr;
    mgr.StartBatch(3);
    mgr.RecordSuccess();
    mgr.RecordSuccess();
    mgr.RecordFailure();
    auto& p = mgr.Progress();
    EXPECT_EQ(p.processed, 3u);
    EXPECT_EQ(p.succeeded, 2u);
    EXPECT_EQ(p.failed, 1u);
    EXPECT_EQ(p.state, BatchOperationState::Completed);
}

TEST(BatchManager, Cancel)
{
    BatchOperationManager mgr;
    mgr.StartBatch(100);
    mgr.RecordSuccess();
    mgr.Cancel();
    EXPECT_EQ(mgr.Progress().state, BatchOperationState::Cancelled);
    EXPECT_TRUE(mgr.Progress().IsComplete());
}

TEST(BatchManager, StatusString)
{
    BatchOperationManager mgr;
    mgr.StartBatch(10);
    mgr.RecordSuccess();
    mgr.RecordSuccess();
    auto s = mgr.Progress().StatusString();
    EXPECT_NE(s.find("2/10"), std::string::npos);
    EXPECT_NE(s.find("20%"), std::string::npos);
}

//==============================================================================
// Clipboard & Export Result Tests
//==============================================================================

TEST(Clipboard, DefaultResult)
{
    ClipboardResult r;
    EXPECT_FALSE(r.success);
    EXPECT_EQ(r.format, "PNG");
}

TEST(Clipboard, SuccessResult)
{
    ClipboardResult r;
    r.success = true;
    r.width = 256;
    r.height = 256;
    r.dataSizeBytes = 256 * 256 * 4;
    EXPECT_TRUE(r.success);
    EXPECT_GT(r.dataSizeBytes, 0u);
}

TEST(Export, DefaultResult)
{
    ExportResult r;
    EXPECT_FALSE(r.success);
    EXPECT_EQ(r.format, "PNG");
}

TEST(Export, SuccessResult)
{
    ExportResult r;
    r.success = true;
    r.outputPath = "C:\\Export\\thumbnail.png";
    r.fileSizeBytes = 48000;
    EXPECT_TRUE(r.success);
    EXPECT_NE(r.outputPath.find("thumbnail.png"), std::string::npos);
}

