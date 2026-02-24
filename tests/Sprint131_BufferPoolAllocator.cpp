// Sprint 131 — Buffer Pool & Slab Allocator Tests
#include <gtest/gtest.h>
#include "Memory/BufferPoolAllocator.h"
using namespace ExplorerLens::Memory;

TEST(Sprint131_BufferPool, ClassifyDimension_Tiny) {
    EXPECT_EQ(ClassifyDimension(32, 64), SlabClass::Tiny);
}
TEST(Sprint131_BufferPool, ClassifyDimension_Medium) {
    EXPECT_EQ(ClassifyDimension(200, 256), SlabClass::Medium);
}
TEST(Sprint131_BufferPool, ClassifyDimension_Huge) {
    EXPECT_EQ(ClassifyDimension(2048, 2048), SlabClass::Huge);
}
TEST(Sprint131_BufferPool, SlabClassBufferSize_Values) {
    EXPECT_EQ(SlabClassBufferSize(SlabClass::Tiny), 64u * 64 * 4);
    EXPECT_EQ(SlabClassBufferSize(SlabClass::Large), 512u * 512 * 4);
}
TEST(Sprint131_BufferPool, AcquireRelease_Lifecycle) {
    auto pool = BufferPool::Create();
    auto buf = pool.Acquire(100, 100);
    EXPECT_TRUE(buf.IsValid());
    EXPECT_EQ(buf.slabClass, SlabClass::Small);
    EXPECT_TRUE(pool.Release(buf));
    EXPECT_FALSE(buf.IsValid());
}
TEST(Sprint131_BufferPool, BufferReuse) {
    auto pool = BufferPool::Create();
    auto buf1 = pool.Acquire(100, 100);
    uint8_t* ptr1 = buf1.data;
    pool.Release(buf1);
    auto buf2 = pool.Acquire(100, 100);
    EXPECT_EQ(buf2.data, ptr1);  // reused from free list
    pool.Release(buf2);
}
TEST(Sprint131_BufferPool, DrainAll) {
    auto pool = BufferPool::Create();
    auto buf = pool.Acquire(400, 400);
    pool.Release(buf);
    pool.DrainAll();
    auto stats = pool.GetClassStats(SlabClass::Large);
    EXPECT_EQ(stats.currentFree, 0u);
}
TEST(Sprint131_BufferPool, Config_LowMemory) {
    auto c = BufferPoolConfig::LowMemory();
    EXPECT_EQ(c.maxFreePerClass, 4u);
    EXPECT_EQ(c.globalMemoryLimitBytes, 16u * 1024 * 1024);
}
TEST(Sprint131_BufferPool, Config_HighThroughput) {
    auto c = BufferPoolConfig::HighThroughput();
    EXPECT_EQ(c.maxFreePerClass, 32u);
}
TEST(Sprint131_BufferPool, Stats_Summary) {
    auto pool = BufferPool::Create();
    pool.Acquire(50, 50);
    auto stats = pool.GetStats();
    EXPECT_EQ(stats.totalAcquires, 1u);
    EXPECT_FALSE(stats.Summary().empty());
}
TEST(Sprint131_BufferPool, PoolCount) {
    auto pool = BufferPool::Create();
    EXPECT_EQ(pool.PoolCount(), static_cast<size_t>(SlabClass::COUNT));
}
TEST(Sprint131_BufferPool, PooledBuffer_Clear) {
    auto pool = BufferPool::Create();
    auto buf = pool.Acquire(32, 32);
    buf.data[0] = 0xFF;
    buf.Clear();
    EXPECT_EQ(buf.data[0], 0);
    pool.Release(buf);
}
TEST(Sprint131_BufferPool, SlabClassName_Valid) {
    EXPECT_STREQ(SlabClassName(SlabClass::Medium), "Medium (≤256x256)");
}

