// Archive Memory Compactor — Tests
#include "../Memory/ArchiveMemoryCompactor.h"
#include "GTestShim.h"

using namespace ExplorerLens::Engine;

TEST(ArchiveMemoryCompactor, DefaultConstruction)
{
    ArchiveMemoryCompactor c;
    auto stats = c.GetStats();
    EXPECT_EQ(stats.bufferCount, 0u);
    EXPECT_EQ(stats.usedBytes, 0u);
}

TEST(ArchiveMemoryCompactor, AllocateBuffer)
{
    ArchiveMemoryCompactor c;
    auto* buf = c.AllocateBuffer(1, 0, 1024);
    EXPECT_TRUE(buf != nullptr);
    EXPECT_EQ(buf->archiveId, 1u);
    EXPECT_EQ(buf->size, 1024u);
    EXPECT_TRUE(buf->alive);
    auto stats = c.GetStats();
    EXPECT_EQ(stats.bufferCount, 1u);
}

TEST(ArchiveMemoryCompactor, FreeBuffer)
{
    ArchiveMemoryCompactor c;
    auto* buf = c.AllocateBuffer(1, 0, 2048);
    EXPECT_TRUE(buf != nullptr);
    c.FreeBuffer(buf);
    EXPECT_FALSE(buf->alive);
}

TEST(ArchiveMemoryCompactor, PinUnpinBuffer)
{
    ArchiveMemoryCompactor c;
    auto* buf = c.AllocateBuffer(1, 0, 4096);
    EXPECT_TRUE(buf != nullptr);
    EXPECT_FALSE(buf->pinned);
    c.PinBuffer(buf);
    EXPECT_TRUE(buf->pinned);
    c.UnpinBuffer(buf);
    EXPECT_FALSE(buf->pinned);
}

TEST(ArchiveMemoryCompactor, CompactResult)
{
    ArchiveMemoryCompactor c;
    auto* b1 = c.AllocateBuffer(1, 0, 1024);
    auto* b2 = c.AllocateBuffer(1, 1, 2048);
    (void)b2;
    c.FreeBuffer(b1);
    auto result = c.Compact();
    EXPECT_GE(result.buffersMoved, 0u);
}

TEST(ArchiveMemoryCompactor, GetStats)
{
    ArchiveMemoryCompactor c;
    c.AllocateBuffer(1, 0, 512);
    c.AllocateBuffer(1, 1, 1024);
    auto stats = c.GetStats();
    EXPECT_EQ(stats.bufferCount, 2u);
    EXPECT_GE(stats.arenaReserved, 0u);
}

TEST(ArchiveMemoryCompactor, MultipleAllocations)
{
    ArchiveMemoryCompactor c;
    for (uint32_t i = 0; i < 10; ++i) {
        auto* buf = c.AllocateBuffer(1, i, 256);
        EXPECT_TRUE(buf != nullptr);
    }
    auto stats = c.GetStats();
    EXPECT_EQ(stats.bufferCount, 10u);
}

TEST(ArchiveMemoryCompactor, ExtractedBufferDefaults)
{
    ExtractedBuffer buf;
    EXPECT_EQ(buf.archiveId, 0u);
    EXPECT_EQ(buf.data, nullptr);
    EXPECT_TRUE(buf.alive);
    EXPECT_FALSE(buf.pinned);
}

TEST(ArchiveMemoryCompactor, CompactResultDefaults)
{
    CompactResult result;
    EXPECT_EQ(result.buffersMoved, 0u);
    EXPECT_EQ(result.bytesMoved, 0u);
}

TEST(ArchiveMemoryCompactor, CompactorStatsDefaults)
{
    CompactorStats stats;
    EXPECT_EQ(stats.bufferCount, 0u);
    EXPECT_EQ(stats.fragmentationRatio, 0.0);
}
