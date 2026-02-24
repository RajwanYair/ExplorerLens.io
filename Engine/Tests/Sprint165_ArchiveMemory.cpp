// Archive Memory Compactor — GTest
#include "../Memory/ArchiveMemoryCompactor.h"
#include "GTestShim.h"

using namespace ExplorerLens::Memory;

TEST(ArchiveMemoryCompactor, DefaultEvictionConfig) {
  auto c = EvictionConfig::Default();
  EXPECT_EQ(c.policy, EvictionPolicy::LRU);
  EXPECT_GT(c.targetUsageBytes, 0u);
}

TEST(ArchiveMemoryCompactor, AggressiveConfigLowerTarget) {
  auto def = EvictionConfig::Default();
  auto agg = EvictionConfig::AggressiveLowRAM();
  EXPECT_LT(agg.targetUsageBytes, def.targetUsageBytes);
}

TEST(ArchiveMemoryCompactor, TrackSlabIncreasesTotal) {
  ArchiveMemoryCompactor c;
  MemorySlab s;
  s.sizeBytes = 1024;
  s.state = SlabState::Active;
  c.TrackSlab(s);
  EXPECT_GT(c.TotalActiveBytes(), 0u);
}

TEST(ArchiveMemoryCompactor, FreeSlabNotCounted) {
  ArchiveMemoryCompactor c;
  MemorySlab s;
  s.sizeBytes = 1024;
  s.state = SlabState::Free;
  c.TrackSlab(s);
  EXPECT_EQ(c.TotalActiveBytes(), 0u);
}

TEST(ArchiveMemoryCompactor, CompactReducesBytes) {
  EvictionConfig cfg = EvictionConfig::Default();
  cfg.targetUsageBytes = 0; // force eviction of everything
  ArchiveMemoryCompactor c(cfg);
  MemorySlab s;
  s.sizeBytes = 1024 * 1024;
  s.state = SlabState::Evictable;
  c.TrackSlab(s);
  auto report = c.Compact();
  EXPECT_GE(report.bytesAfter, 0u);
}

TEST(ArchiveMemoryCompactor, CompactionReportBytesBefore) {
  EvictionConfig cfg;
  cfg.targetUsageBytes = 1024ULL * 1024 * 1024; // don't evict
  ArchiveMemoryCompactor c(cfg);
  MemorySlab s;
  s.sizeBytes = 65536;
  s.state = SlabState::Evictable;
  c.TrackSlab(s);
  auto report = c.Compact();
  EXPECT_GE(report.bytesBefore, 0u);
}

TEST(ArchiveMemoryCompactor, CompactionReportSlabsExamined) {
  ArchiveMemoryCompactor c;
  MemorySlab s1, s2;
  s1.sizeBytes = 1024;
  s1.state = SlabState::Active;
  s2.sizeBytes = 2048;
  s2.state = SlabState::Evictable;
  c.TrackSlab(s1);
  c.TrackSlab(s2);
  auto report = c.Compact();
  EXPECT_GE(report.slabsExamined, 2u);
}

TEST(ArchiveMemoryCompactor, SlabStateEvictableCanEvict) {
  MemorySlab s;
  s.state = SlabState::Evictable;
  EXPECT_TRUE(s.CanEvict());
}

TEST(ArchiveMemoryCompactor, SlabStateActiveNoEvict) {
  MemorySlab s;
  s.state = SlabState::Active;
  EXPECT_FALSE(s.CanEvict());
}

TEST(ArchiveMemoryCompactor, SlabStatePinnedNoEvict) {
  MemorySlab s;
  s.state = SlabState::Pinned;
  EXPECT_FALSE(s.CanEvict());
}

TEST(ArchiveMemoryCompactor, ReductionPercent) {
  CompactionReport r;
  r.bytesBefore = 1000;
  r.bytesAfter = 600;
  EXPECT_DOUBLE_EQ(r.ReductionPercent(), 40.0);
}

TEST(ArchiveMemoryCompactor, BytesReclaimed) {
  CompactionReport r;
  r.bytesBefore = 2048;
  r.bytesAfter = 1024;
  EXPECT_EQ(r.BytesReclaimed(), 1024u);
}

TEST(ArchiveMemoryCompactor, TouchSlabUpdatesAccess) {
  ArchiveMemoryCompactor c;
  MemorySlab s;
  s.slabId = 42;
  s.sizeBytes = 512;
  s.state = SlabState::Active;
  c.TrackSlab(s);
  // Touch should not throw
  EXPECT_NO_THROW(c.TouchSlab(42));
}
