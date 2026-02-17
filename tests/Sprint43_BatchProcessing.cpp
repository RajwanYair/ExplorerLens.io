//==============================================================================
// DarkThumbs — Sprint 43 Tests: Batch Processing & Queue Management
// Tests job priority queue, batch processor, progress tracking,
// rate limiting, pause/cancel, result aggregation.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../Engine/Pipeline/BatchProcessor.h"

using namespace DarkThumbs::Engine::Pipeline;

//==============================================================================
// Job Priority Tests
//==============================================================================

TEST(JobPriority, Names)
{
    EXPECT_STREQ(JobPriorityName(JobPriority::Critical), "Critical");
    EXPECT_STREQ(JobPriorityName(JobPriority::High), "High");
    EXPECT_STREQ(JobPriorityName(JobPriority::Normal), "Normal");
    EXPECT_STREQ(JobPriorityName(JobPriority::Low), "Low");
    EXPECT_STREQ(JobPriorityName(JobPriority::Idle), "Idle");
}

TEST(JobPriority, Ordering)
{
    EXPECT_LT(static_cast<uint8_t>(JobPriority::Critical),
              static_cast<uint8_t>(JobPriority::Normal));
    EXPECT_LT(static_cast<uint8_t>(JobPriority::Normal),
              static_cast<uint8_t>(JobPriority::Idle));
}

//==============================================================================
// Job Status Tests
//==============================================================================

TEST(JobStatus, Names)
{
    EXPECT_STREQ(JobStatusName(JobStatus::Queued), "Queued");
    EXPECT_STREQ(JobStatusName(JobStatus::Running), "Running");
    EXPECT_STREQ(JobStatusName(JobStatus::Completed), "Completed");
    EXPECT_STREQ(JobStatusName(JobStatus::Failed), "Failed");
    EXPECT_STREQ(JobStatusName(JobStatus::Cancelled), "Cancelled");
    EXPECT_STREQ(JobStatusName(JobStatus::Paused), "Paused");
}

TEST(JobStatus, TerminalStates)
{
    EXPECT_TRUE(IsTerminalStatus(JobStatus::Completed));
    EXPECT_TRUE(IsTerminalStatus(JobStatus::Failed));
    EXPECT_TRUE(IsTerminalStatus(JobStatus::Cancelled));
    EXPECT_FALSE(IsTerminalStatus(JobStatus::Queued));
    EXPECT_FALSE(IsTerminalStatus(JobStatus::Running));
    EXPECT_FALSE(IsTerminalStatus(JobStatus::Paused));
}

//==============================================================================
// Thumbnail Job Tests
//==============================================================================

TEST(ThumbnailJob, DefaultState)
{
    ThumbnailJob job;
    EXPECT_EQ(job.jobId, 0u);
    EXPECT_EQ(job.priority, JobPriority::Normal);
    EXPECT_EQ(job.status, JobStatus::Queued);
    EXPECT_FALSE(job.IsComplete());
    EXPECT_FALSE(job.IsSuccess());
}

TEST(ThumbnailJob, CompletedState)
{
    ThumbnailJob job;
    job.status = JobStatus::Completed;
    EXPECT_TRUE(job.IsComplete());
    EXPECT_TRUE(job.IsSuccess());
    EXPECT_FALSE(job.IsFailed());
}

TEST(ThumbnailJob, FailedState)
{
    ThumbnailJob job;
    job.status = JobStatus::Failed;
    job.errorMessage = "Decoder error";
    EXPECT_TRUE(job.IsComplete());
    EXPECT_FALSE(job.IsSuccess());
    EXPECT_TRUE(job.IsFailed());
}

TEST(ThumbnailJob, PriorityComparison)
{
    ThumbnailJob critical;
    critical.priority = JobPriority::Critical;
    ThumbnailJob idle;
    idle.priority = JobPriority::Idle;
    // Greater-than means lower priority (for min-heap)
    EXPECT_TRUE(idle > critical);
    EXPECT_FALSE(critical > idle);
}

//==============================================================================
// Batch Request Tests
//==============================================================================

TEST(BatchRequest, Empty)
{
    BatchRequest req;
    EXPECT_TRUE(req.IsEmpty());
    EXPECT_EQ(req.FileCount(), 0u);
}

TEST(BatchRequest, WithFiles)
{
    BatchRequest req;
    req.filePaths = {"a.jpg", "b.png", "c.heic"};
    EXPECT_FALSE(req.IsEmpty());
    EXPECT_EQ(req.FileCount(), 3u);
}

//==============================================================================
// Progress Info Tests
//==============================================================================

TEST(ProgressInfo, ZeroProgress)
{
    ProgressInfo p;
    EXPECT_DOUBLE_EQ(p.PercentComplete(), 0.0);
    EXPECT_EQ(p.RemainingJobs(), 0u);
    EXPECT_FALSE(p.IsFinished()); // totalJobs=0, activeJobs=0 → finished
}

TEST(ProgressInfo, HalfComplete)
{
    ProgressInfo p;
    p.totalJobs = 100;
    p.completedJobs = 50;
    EXPECT_DOUBLE_EQ(p.PercentComplete(), 50.0);
    EXPECT_EQ(p.RemainingJobs(), 50u);
    EXPECT_FALSE(p.IsFinished());
}

TEST(ProgressInfo, AllDone)
{
    ProgressInfo p;
    p.totalJobs     = 10;
    p.completedJobs = 8;
    p.failedJobs    = 2;
    p.activeJobs    = 0;
    EXPECT_DOUBLE_EQ(p.PercentComplete(), 100.0);
    EXPECT_EQ(p.RemainingJobs(), 0u);
    EXPECT_TRUE(p.IsFinished());
}

TEST(ProgressInfo, Throughput)
{
    ProgressInfo p;
    p.completedJobs = 100;
    p.elapsedMs = 2000.0;
    EXPECT_DOUBLE_EQ(p.ThroughputPerSecond(), 50.0);
}

TEST(ProgressInfo, EstimatedRemaining)
{
    ProgressInfo p;
    p.totalJobs     = 200;
    p.completedJobs = 100;
    p.elapsedMs     = 5000.0;
    // 50ms per job, 100 remaining → 5000ms
    EXPECT_DOUBLE_EQ(p.EstimatedRemainingMs(), 5000.0);
}

//==============================================================================
// Batch Result Tests
//==============================================================================

TEST(BatchResult, SuccessRate)
{
    BatchResult r;
    r.totalJobs = 100;
    r.succeeded = 95;
    r.failed = 5;
    EXPECT_DOUBLE_EQ(r.SuccessRate(), 95.0);
}

TEST(BatchResult, Throughput)
{
    BatchResult r;
    r.succeeded = 200;
    r.totalTimeMs = 4000.0;
    EXPECT_DOUBLE_EQ(r.ThroughputPerSecond(), 50.0);
}

TEST(BatchResult, Summary)
{
    BatchResult r;
    r.batchId = 1;
    r.totalJobs = 10;
    r.succeeded = 9;
    r.failed = 1;
    auto s = r.Summary();
    EXPECT_NE(s.find("Batch #1"), std::string::npos);
    EXPECT_NE(s.find("9/10"), std::string::npos);
    EXPECT_NE(s.find("90.0%"), std::string::npos);
}

//==============================================================================
// Rate Limit Config Tests
//==============================================================================

TEST(RateLimitConfig, DefaultValues)
{
    auto c = RateLimitConfig::Default();
    EXPECT_EQ(c.maxConcurrentJobs, 4u);
    EXPECT_EQ(c.maxQueueDepth, 10000u);
    EXPECT_EQ(c.maxJobsPerSecond, 0u);
}

TEST(RateLimitConfig, ConservativeValues)
{
    auto c = RateLimitConfig::Conservative();
    EXPECT_EQ(c.maxConcurrentJobs, 2u);
    EXPECT_EQ(c.maxJobsPerSecond, 10u);
}

TEST(RateLimitConfig, AggressiveValues)
{
    auto c = RateLimitConfig::Aggressive();
    EXPECT_EQ(c.maxConcurrentJobs, 8u);
    EXPECT_GT(c.maxMemoryBytes, 512u * 1024u * 1024u);
}

TEST(RateLimitConfig, WithinLimits)
{
    auto c = RateLimitConfig::Default();
    EXPECT_TRUE(c.IsWithinLimits(0, 0));
    EXPECT_TRUE(c.IsWithinLimits(100, 3));
    EXPECT_FALSE(c.IsWithinLimits(100, 4)); // At concurrent limit
    EXPECT_FALSE(c.IsWithinLimits(10000, 0)); // At queue limit
}

//==============================================================================
// Job Queue Tests
//==============================================================================

TEST(JobQueue, EmptyQueue)
{
    JobQueue q;
    EXPECT_TRUE(q.Empty());
    EXPECT_EQ(q.Size(), 0u);
    ThumbnailJob job;
    EXPECT_FALSE(q.TryPop(job));
}

TEST(JobQueue, PushAndPop)
{
    JobQueue q;
    ThumbnailJob j;
    j.jobId = 1;
    j.filePath = "test.jpg";
    q.Push(j);
    EXPECT_EQ(q.Size(), 1u);

    ThumbnailJob popped;
    EXPECT_TRUE(q.TryPop(popped));
    EXPECT_EQ(popped.jobId, 1u);
    EXPECT_EQ(popped.filePath, "test.jpg");
    EXPECT_TRUE(q.Empty());
}

TEST(JobQueue, PriorityOrdering)
{
    JobQueue q;

    ThumbnailJob low;
    low.jobId = 1;
    low.priority = JobPriority::Low;
    q.Push(low);

    ThumbnailJob critical;
    critical.jobId = 2;
    critical.priority = JobPriority::Critical;
    q.Push(critical);

    ThumbnailJob normal;
    normal.jobId = 3;
    normal.priority = JobPriority::Normal;
    q.Push(normal);

    ThumbnailJob popped;
    EXPECT_TRUE(q.TryPop(popped));
    EXPECT_EQ(popped.priority, JobPriority::Critical);

    EXPECT_TRUE(q.TryPop(popped));
    EXPECT_EQ(popped.priority, JobPriority::Normal);

    EXPECT_TRUE(q.TryPop(popped));
    EXPECT_EQ(popped.priority, JobPriority::Low);
}

TEST(JobQueue, Clear)
{
    JobQueue q;
    ThumbnailJob j;
    q.Push(j);
    q.Push(j);
    q.Push(j);
    EXPECT_EQ(q.Size(), 3u);
    q.Clear();
    EXPECT_TRUE(q.Empty());
}

//==============================================================================
// Batch Processor Tests
//==============================================================================

TEST(BatchProcessor, SubmitJob)
{
    BatchProcessor bp;
    ThumbnailJob job;
    job.filePath = "test.jpg";
    auto id = bp.SubmitJob(job);
    EXPECT_GE(id, 1u);
    EXPECT_EQ(bp.QueueDepth(), 1u);
    EXPECT_EQ(bp.TotalSubmitted(), 1u);
}

TEST(BatchProcessor, SubmitBatch)
{
    BatchProcessor bp;
    BatchRequest req;
    req.filePaths = {"a.jpg", "b.png", "c.heic"};
    auto batchId = bp.SubmitBatch(req);
    EXPECT_GE(batchId, 1u);
    EXPECT_EQ(bp.QueueDepth(), 3u);
    EXPECT_EQ(bp.TotalSubmitted(), 3u);
}

TEST(BatchProcessor, ProcessJob)
{
    BatchProcessor bp;
    ThumbnailJob job;
    job.filePath = "test.jpg";
    bp.SubmitJob(job);

    ThumbnailJob next;
    EXPECT_TRUE(bp.ProcessNextJob(next));
    EXPECT_EQ(next.status, JobStatus::Running);
    EXPECT_EQ(bp.ActiveJobs(), 1u);
}

TEST(BatchProcessor, CompleteJobSuccess)
{
    BatchProcessor bp;
    ThumbnailJob job;
    job.filePath = "test.jpg";
    bp.SubmitJob(job);

    ThumbnailJob next;
    bp.ProcessNextJob(next);
    bp.CompleteJob(next, true);

    EXPECT_EQ(next.status, JobStatus::Completed);
    EXPECT_EQ(bp.TotalCompleted(), 1u);
    EXPECT_EQ(bp.ActiveJobs(), 0u);
}

TEST(BatchProcessor, CompleteJobFailure)
{
    BatchProcessor bp;
    ThumbnailJob job;
    bp.SubmitJob(job);

    ThumbnailJob next;
    bp.ProcessNextJob(next);
    bp.CompleteJob(next, false, "Corrupt file");

    EXPECT_EQ(next.status, JobStatus::Failed);
    EXPECT_EQ(next.errorMessage, "Corrupt file");
    EXPECT_EQ(bp.TotalFailed(), 1u);
}

TEST(BatchProcessor, PauseAndResume)
{
    BatchProcessor bp;
    ThumbnailJob job;
    bp.SubmitJob(job);

    bp.Pause();
    EXPECT_TRUE(bp.IsPaused());

    ThumbnailJob next;
    EXPECT_FALSE(bp.ProcessNextJob(next)); // Should not dequeue when paused

    bp.Resume();
    EXPECT_FALSE(bp.IsPaused());
    EXPECT_TRUE(bp.ProcessNextJob(next)); // Should work after resume
}

TEST(BatchProcessor, Cancel)
{
    BatchProcessor bp;
    ThumbnailJob job;
    bp.SubmitJob(job);
    bp.SubmitJob(job);
    bp.SubmitJob(job);

    bp.Cancel();
    EXPECT_TRUE(bp.IsCancelled());
    EXPECT_EQ(bp.QueueDepth(), 0u); // Queue cleared

    ThumbnailJob next;
    EXPECT_FALSE(bp.ProcessNextJob(next)); // Cannot process when cancelled
}

TEST(BatchProcessor, Reset)
{
    BatchProcessor bp;
    ThumbnailJob job;
    bp.SubmitJob(job);
    bp.Cancel();
    bp.Reset();

    EXPECT_FALSE(bp.IsCancelled());
    EXPECT_FALSE(bp.IsPaused());
    EXPECT_EQ(bp.TotalSubmitted(), 0u);
    EXPECT_EQ(bp.QueueDepth(), 0u);
}

TEST(BatchProcessor, HasPendingWork)
{
    BatchProcessor bp;
    EXPECT_FALSE(bp.HasPendingWork());

    ThumbnailJob job;
    bp.SubmitJob(job);
    EXPECT_TRUE(bp.HasPendingWork());
}

TEST(BatchProcessor, JobCompleteCallback)
{
    BatchProcessor bp;
    int callbackCount = 0;
    bp.SetJobCompleteCallback([&](const ThumbnailJob&) {
        callbackCount++;
    });

    ThumbnailJob job;
    bp.SubmitJob(job);
    ThumbnailJob next;
    bp.ProcessNextJob(next);
    bp.CompleteJob(next, true);
    EXPECT_EQ(callbackCount, 1);
}

TEST(BatchProcessor, GetProgress)
{
    BatchProcessor bp;
    ThumbnailJob job;
    bp.SubmitJob(job);
    bp.SubmitJob(job);

    auto p = bp.GetProgress(42);
    EXPECT_EQ(p.batchId, 42u);
    EXPECT_EQ(p.totalJobs, 2u);
    EXPECT_EQ(p.completedJobs, 0u);
}

TEST(BatchProcessor, GetResult)
{
    BatchProcessor bp;
    ThumbnailJob job;
    bp.SubmitJob(job);
    ThumbnailJob next;
    bp.ProcessNextJob(next);
    bp.CompleteJob(next, true);

    auto r = bp.GetResult(1);
    EXPECT_EQ(r.batchId, 1u);
    EXPECT_EQ(r.succeeded, 1u);
}

//==============================================================================
// Batch Processing Config Tests
//==============================================================================

TEST(BatchProcessingConfig, Default)
{
    auto c = BatchProcessingConfig::Default();
    EXPECT_EQ(c.progressIntervalMs, 500u);
    EXPECT_FALSE(c.stopOnFirstError);
    EXPECT_FALSE(c.retryFailedJobs);
}

TEST(BatchProcessingConfig, LowResource)
{
    auto c = BatchProcessingConfig::LowResource();
    EXPECT_EQ(c.rateLimit.maxConcurrentJobs, 2u);
    EXPECT_EQ(c.progressIntervalMs, 1000u);
}

TEST(BatchProcessingConfig, HighPerformance)
{
    auto c = BatchProcessingConfig::HighPerformance();
    EXPECT_EQ(c.rateLimit.maxConcurrentJobs, 8u);
    EXPECT_EQ(c.progressIntervalMs, 250u);
}
