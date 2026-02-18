// Sprint 132 — Explorer Work Scheduler Tests
#include <gtest/gtest.h>
#include "Pipeline/ExplorerWorkScheduler.h"
using namespace DarkThumbs::Pipeline;

TEST(Sprint132_Scheduler, Submit_ReturnsId) {
    auto sched = ExplorerWorkScheduler::Create();
    uint64_t id = sched.Submit("test.jpg", 0);
    EXPECT_GT(id, 0u);
}
TEST(Sprint132_Scheduler, Submit_QueueSizeGrows) {
    auto sched = ExplorerWorkScheduler::Create();
    sched.Submit("a.jpg", 0);
    sched.Submit("b.jpg", 1);
    EXPECT_EQ(sched.QueueSize(), 2u);
}
TEST(Sprint132_Scheduler, Dequeue_HighestPriority) {
    auto sched = ExplorerWorkScheduler::Create();
    ViewportState vs{5, 10, 3, 5};
    sched.UpdateViewport(vs);
    sched.Submit("far.jpg", 50);     // Low priority
    sched.Submit("visible.jpg", 7);  // Critical priority
    auto item = sched.Dequeue();
    EXPECT_EQ(item.filePath, "visible.jpg");
    EXPECT_EQ(item.priority, WorkPriority::Critical);
}
TEST(Sprint132_Scheduler, Cancel_SkipsOnDequeue) {
    auto sched = ExplorerWorkScheduler::Create();
    uint64_t id = sched.Submit("cancel_me.jpg");
    sched.Cancel(id);
    auto item = sched.Dequeue();
    EXPECT_EQ(item.id, 0u);  // empty item
}
TEST(Sprint132_Scheduler, ViewportState_IsInViewport) {
    ViewportState vs{10, 20, 5, 5};
    EXPECT_TRUE(vs.IsInViewport(15));
    EXPECT_FALSE(vs.IsInViewport(5));
}
TEST(Sprint132_Scheduler, ViewportState_PrefetchZone) {
    ViewportState vs{10, 20, 5, 5};
    EXPECT_TRUE(vs.IsInPrefetchZone(7));   // before viewport
    EXPECT_TRUE(vs.IsInPrefetchZone(23));  // after viewport
    EXPECT_FALSE(vs.IsInPrefetchZone(15)); // in viewport
}
TEST(Sprint132_Scheduler, ViewportState_PriorityForIndex) {
    ViewportState vs{10, 20, 5, 5};
    EXPECT_EQ(vs.PriorityForIndex(15), WorkPriority::Critical);
    EXPECT_EQ(vs.PriorityForIndex(7), WorkPriority::High);
    EXPECT_EQ(vs.PriorityForIndex(50), WorkPriority::Low);
}
TEST(Sprint132_Scheduler, OnScroll_UpdatesViewport) {
    auto sched = ExplorerWorkScheduler::Create();
    sched.OnScroll(20, 30);
    auto vs = sched.GetViewport();
    EXPECT_EQ(vs.firstVisibleIndex, 20);
    EXPECT_EQ(vs.lastVisibleIndex, 30);
}
TEST(Sprint132_Scheduler, Stats_CompletionRate) {
    auto sched = ExplorerWorkScheduler::Create();
    sched.Submit("a.jpg");
    auto item = sched.Dequeue();
    sched.Complete(item);
    auto stats = sched.GetStats();
    EXPECT_EQ(stats.totalCompleted, 1u);
    EXPECT_GT(stats.CompletionRate(), 0.0);
}
TEST(Sprint132_Scheduler, Config_LowLatency) {
    auto c = SchedulerConfig::LowLatency();
    EXPECT_EQ(c.maxConcurrency, 2u);
    EXPECT_EQ(c.scrollDebounceMs, 30u);
}
TEST(Sprint132_Scheduler, ClearQueue) {
    auto sched = ExplorerWorkScheduler::Create();
    sched.Submit("a.jpg");
    sched.Submit("b.jpg");
    sched.ClearQueue();
    EXPECT_EQ(sched.QueueSize(), 0u);
}
TEST(Sprint132_Scheduler, PriorityName_Valid) {
    EXPECT_STREQ(PriorityName(WorkPriority::Critical), "Critical");
    EXPECT_STREQ(PriorityName(WorkPriority::Low), "Low");
}
TEST(Sprint132_Scheduler, WorkItem_Timing) {
    ThumbnailWorkItem item;
    EXPECT_DOUBLE_EQ(item.LatencyMs(), 0.0);
    EXPECT_DOUBLE_EQ(item.QueueWaitMs(), 0.0);
}
