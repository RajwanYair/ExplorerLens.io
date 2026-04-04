//==============================================================================
// ExplorerLens Engine — Batch Processing & Queue Management
//
// Provides priority-based job scheduling, parallel decode queue management,
// progress tracking with callbacks, cancellation/pause/resume, rate limiting,
// and batch result aggregation for large-scale thumbnail generation.
//==============================================================================
#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens::Engine::Pipeline {

//==============================================================================
// Job Priority
//==============================================================================

enum class JobPriority : uint8_t {
    Critical = 0,  // Currently visible thumbnails — immediate
    High = 1,      // User-initiated regeneration
    Normal = 2,    // Background pre-generation
    Low = 3,       // Batch scan / duplicate detection
    Idle = 4       // Speculative pre-fetch
};

inline const char* JobPriorityName(JobPriority p)
{
    switch (p) {
        case JobPriority::Critical:
            return "Critical";
        case JobPriority::High:
            return "High";
        case JobPriority::Normal:
            return "Normal";
        case JobPriority::Low:
            return "Low";
        case JobPriority::Idle:
            return "Idle";
        default:
            return "Unknown";
    }
}

//==============================================================================
// Job Status
//==============================================================================

enum class JobStatus : uint8_t {
    Queued,
    Running,
    Completed,
    Failed,
    Cancelled,
    Paused
};

inline const char* JobStatusName(JobStatus s)
{
    switch (s) {
        case JobStatus::Queued:
            return "Queued";
        case JobStatus::Running:
            return "Running";
        case JobStatus::Completed:
            return "Completed";
        case JobStatus::Failed:
            return "Failed";
        case JobStatus::Cancelled:
            return "Cancelled";
        case JobStatus::Paused:
            return "Paused";
        default:
            return "Unknown";
    }
}

inline bool IsTerminalStatus(JobStatus s)
{
    return s == JobStatus::Completed || s == JobStatus::Failed || s == JobStatus::Cancelled;
}

//==============================================================================
// Thumbnail Job — Single decode work item
//==============================================================================

struct ThumbnailJob
{
    uint64_t jobId = 0;
    std::string filePath;
    std::string outputPath;  // Empty = cache only
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    JobPriority priority = JobPriority::Normal;
    JobStatus status = JobStatus::Queued;
    std::string errorMessage;
    double decodeTimeMs = 0.0;
    uint64_t fileSizeBytes = 0;

    bool IsComplete() const
    {
        return IsTerminalStatus(status);
    }
    bool IsSuccess() const
    {
        return status == JobStatus::Completed;
    }
    bool IsFailed() const
    {
        return status == JobStatus::Failed;
    }

    // For priority queue: lower priority value = higher priority
    bool operator>(const ThumbnailJob& other) const
    {
        return static_cast<uint8_t>(priority) > static_cast<uint8_t>(other.priority);
    }
};

//==============================================================================
// Batch Request — Collection of jobs with shared settings
//==============================================================================

struct BatchRequest
{
    uint64_t batchId = 0;
    std::string sourceDirectory;
    std::string outputDirectory;
    std::vector<std::string> filePaths;
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    JobPriority defaultPriority = JobPriority::Normal;
    bool recursive = false;
    bool overwriteExisting = false;

    size_t FileCount() const
    {
        return filePaths.size();
    }
    bool IsEmpty() const
    {
        return filePaths.empty();
    }
};

//==============================================================================
// Progress Info — Real-time batch progress data
//==============================================================================

struct ProgressInfo
{
    uint64_t batchId = 0;
    size_t totalJobs = 0;
    size_t completedJobs = 0;
    size_t failedJobs = 0;
    size_t cancelledJobs = 0;
    size_t activeJobs = 0;
    double elapsedMs = 0.0;

    double PercentComplete() const
    {
        if (totalJobs == 0)
            return 0.0;
        return (static_cast<double>(completedJobs + failedJobs + cancelledJobs) / static_cast<double>(totalJobs))
               * 100.0;
    }

    size_t RemainingJobs() const
    {
        size_t done = completedJobs + failedJobs + cancelledJobs;
        return (done < totalJobs) ? (totalJobs - done) : 0;
    }

    double EstimatedRemainingMs() const
    {
        size_t done = completedJobs + failedJobs + cancelledJobs;
        if (done == 0 || elapsedMs <= 0.0)
            return 0.0;
        double msPerJob = elapsedMs / static_cast<double>(done);
        return msPerJob * static_cast<double>(RemainingJobs());
    }

    double ThroughputPerSecond() const
    {
        if (elapsedMs <= 0.0)
            return 0.0;
        return (static_cast<double>(completedJobs) / elapsedMs) * 1000.0;
    }

    bool IsFinished() const
    {
        return RemainingJobs() == 0 && activeJobs == 0;
    }
};

//==============================================================================
// Progress Callback
//==============================================================================

using ProgressCallback = std::function<void(const ProgressInfo&)>;
using JobCompleteCallback = std::function<void(const ThumbnailJob&)>;

//==============================================================================
// Batch Result — Final summary of a completed batch
//==============================================================================

struct BatchResult
{
    uint64_t batchId = 0;
    size_t totalJobs = 0;
    size_t succeeded = 0;
    size_t failed = 0;
    size_t cancelled = 0;
    double totalTimeMs = 0.0;
    double avgDecodeTimeMs = 0.0;
    uint64_t totalBytesProcessed = 0;

    double SuccessRate() const
    {
        if (totalJobs == 0)
            return 0.0;
        return (static_cast<double>(succeeded) / static_cast<double>(totalJobs)) * 100.0;
    }

    double ThroughputPerSecond() const
    {
        if (totalTimeMs <= 0.0)
            return 0.0;
        return (static_cast<double>(succeeded) / totalTimeMs) * 1000.0;
    }

    std::string Summary() const
    {
        std::ostringstream ss;
        ss << "Batch #" << batchId << ": " << succeeded << "/" << totalJobs << " succeeded"
           << " (" << std::fixed << std::setprecision(1) << SuccessRate() << "%), " << failed << " failed, "
           << cancelled << " cancelled, " << std::setprecision(0) << totalTimeMs << " ms total, "
           << std::setprecision(1) << ThroughputPerSecond() << " img/s";
        return ss.str();
    }
};

//==============================================================================
// Rate Limiter — Throttle job submission to prevent resource exhaustion
//==============================================================================

struct RateLimitConfig
{
    uint32_t maxConcurrentJobs = 4;                  // Thread pool size
    uint32_t maxQueueDepth = 10000;                  // Max pending jobs
    uint32_t maxJobsPerSecond = 0;                   // 0 = unlimited
    uint64_t maxMemoryBytes = 512 * 1024 * 1024ULL;  // 512 MB working set limit
    uint32_t jobTimeoutMs = 30000;                   // 30s timeout per job

    static RateLimitConfig Default()
    {
        return {};
    }

    static RateLimitConfig Conservative()
    {
        RateLimitConfig c;
        c.maxConcurrentJobs = 2;
        c.maxQueueDepth = 1000;
        c.maxJobsPerSecond = 10;
        c.maxMemoryBytes = 256 * 1024 * 1024ULL;
        return c;
    }

    static RateLimitConfig Aggressive()
    {
        RateLimitConfig c;
        c.maxConcurrentJobs = 8;
        c.maxQueueDepth = 50000;
        c.maxJobsPerSecond = 0;
        c.maxMemoryBytes = 1024 * 1024 * 1024ULL;
        return c;
    }

    bool IsWithinLimits(size_t currentQueueDepth, size_t currentActive) const
    {
        if (currentQueueDepth >= maxQueueDepth)
            return false;
        if (currentActive >= maxConcurrentJobs)
            return false;
        return true;
    }
};

//==============================================================================
// Job Priority Queue — Min-heap ordered by priority then submission time
//==============================================================================

class JobQueue
{
  public:
    void Push(const ThumbnailJob& job)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(job);
    }

    bool TryPop(ThumbnailJob& job)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return false;
        job = m_queue.top();
        m_queue.pop();
        return true;
    }

    size_t Size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    bool Empty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty())
            m_queue.pop();
    }

  private:
    // Min-heap: lower priority value = higher priority = dequeued first
    struct JobCompare
    {
        bool operator()(const ThumbnailJob& a, const ThumbnailJob& b) const
        {
            if (a.priority != b.priority)
                return static_cast<uint8_t>(a.priority) > static_cast<uint8_t>(b.priority);
            return a.jobId > b.jobId;  // FIFO within same priority
        }
    };

    mutable std::mutex m_mutex;
    std::priority_queue<ThumbnailJob, std::vector<ThumbnailJob>, JobCompare> m_queue;
};

//==============================================================================
// Batch Processor — Orchestrates job scheduling and execution
//==============================================================================

class BatchProcessor
{
  public:
    BatchProcessor() = default;
    explicit BatchProcessor(const RateLimitConfig& config) : m_rateConfig(config) {}

    //--- Job submission ---

    uint64_t SubmitJob(ThumbnailJob job)
    {
        job.jobId = m_nextJobId++;
        job.status = JobStatus::Queued;
        m_queue.Push(job);
        m_totalSubmitted++;
        return job.jobId;
    }

    uint64_t SubmitBatch(const BatchRequest& request)
    {
        uint64_t batchId = m_nextBatchId++;
        for (const auto& path : request.filePaths) {
            ThumbnailJob job;
            job.filePath = path;
            job.outputPath = request.outputDirectory;
            job.targetWidth = request.targetWidth;
            job.targetHeight = request.targetHeight;
            job.priority = request.defaultPriority;
            SubmitJob(job);
        }
        return batchId;
    }

    //--- Queue inspection ---

    size_t QueueDepth() const
    {
        return m_queue.Size();
    }
    size_t TotalSubmitted() const
    {
        return m_totalSubmitted.load();
    }
    size_t TotalCompleted() const
    {
        return m_totalCompleted.load();
    }
    size_t TotalFailed() const
    {
        return m_totalFailed.load();
    }
    size_t ActiveJobs() const
    {
        return m_activeJobs.load();
    }

    bool HasPendingWork() const
    {
        return !m_queue.Empty() || m_activeJobs.load() > 0;
    }

    //--- Control ---

    void Pause()
    {
        m_paused.store(true);
    }
    void Resume()
    {
        m_paused.store(false);
    }
    bool IsPaused() const
    {
        return m_paused.load();
    }

    void Cancel()
    {
        m_cancelled.store(true);
        m_queue.Clear();
    }
    bool IsCancelled() const
    {
        return m_cancelled.load();
    }

    void Reset()
    {
        m_cancelled.store(false);
        m_paused.store(false);
        m_queue.Clear();
        m_totalSubmitted.store(0);
        m_totalCompleted.store(0);
        m_totalFailed.store(0);
        m_activeJobs.store(0);
    }

    //--- Callbacks ---

    void SetProgressCallback(ProgressCallback cb)
    {
        m_progressCb = std::move(cb);
    }
    void SetJobCompleteCallback(JobCompleteCallback cb)
    {
        m_jobCompleteCb = std::move(cb);
    }

    //--- Simulated Processing (for testing) ---

    bool ProcessNextJob(ThumbnailJob& job)
    {
        if (m_paused.load() || m_cancelled.load())
            return false;
        if (!m_rateConfig.IsWithinLimits(m_queue.Size(), m_activeJobs.load())) {
            // Rate limited — can still pop if under concurrent limit
            if (m_activeJobs.load() >= m_rateConfig.maxConcurrentJobs)
                return false;
        }

        if (!m_queue.TryPop(job))
            return false;

        job.status = JobStatus::Running;
        m_activeJobs++;
        return true;
    }

    void CompleteJob(ThumbnailJob& job, bool success, const std::string& error = "")
    {
        if (success) {
            job.status = JobStatus::Completed;
            m_totalCompleted++;
        } else {
            job.status = JobStatus::Failed;
            job.errorMessage = error;
            m_totalFailed++;
        }
        m_activeJobs--;

        if (m_jobCompleteCb) {
            m_jobCompleteCb(job);
        }
    }

    //--- Build result snapshot ---

    BatchResult GetResult(uint64_t batchId = 0) const
    {
        BatchResult r;
        r.batchId = batchId;
        r.totalJobs = m_totalSubmitted.load();
        r.succeeded = m_totalCompleted.load();
        r.failed = m_totalFailed.load();
        return r;
    }

    ProgressInfo GetProgress(uint64_t batchId = 0) const
    {
        ProgressInfo p;
        p.batchId = batchId;
        p.totalJobs = m_totalSubmitted.load();
        p.completedJobs = m_totalCompleted.load();
        p.failedJobs = m_totalFailed.load();
        p.activeJobs = m_activeJobs.load();
        return p;
    }

    //--- Config ---

    const RateLimitConfig& GetRateConfig() const
    {
        return m_rateConfig;
    }
    void SetRateConfig(const RateLimitConfig& config)
    {
        m_rateConfig = config;
    }

  private:
    JobQueue m_queue;
    RateLimitConfig m_rateConfig;
    ProgressCallback m_progressCb;
    JobCompleteCallback m_jobCompleteCb;

    std::atomic<uint64_t> m_nextJobId{1};
    std::atomic<uint64_t> m_nextBatchId{1};
    std::atomic<size_t> m_totalSubmitted{0};
    std::atomic<size_t> m_totalCompleted{0};
    std::atomic<size_t> m_totalFailed{0};
    std::atomic<size_t> m_activeJobs{0};
    std::atomic<bool> m_paused{false};
    std::atomic<bool> m_cancelled{false};
};

//==============================================================================
// Batch Processing Config — High-level presets
//==============================================================================

struct BatchProcessingConfig
{
    RateLimitConfig rateLimit;
    uint32_t progressIntervalMs = 500;  // How often to fire progress callback
    bool stopOnFirstError = false;
    bool retryFailedJobs = false;
    uint32_t maxRetries = 3;

    static BatchProcessingConfig Default()
    {
        return {};
    }

    static BatchProcessingConfig LowResource()
    {
        BatchProcessingConfig c;
        c.rateLimit = RateLimitConfig::Conservative();
        c.progressIntervalMs = 1000;
        return c;
    }

    static BatchProcessingConfig HighPerformance()
    {
        BatchProcessingConfig c;
        c.rateLimit = RateLimitConfig::Aggressive();
        c.progressIntervalMs = 250;
        return c;
    }
};

}  // namespace ExplorerLens::Engine::Pipeline
