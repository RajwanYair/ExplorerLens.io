#pragma once
// Batch Processing Engine — multi-file batch operations with progress
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Type of batch operation
enum class BatchOperation : uint32_t {
    GenerateThumbnails = 0,
    ConvertFormats = 1,
    ValidateFiles = 2,
    CleanCache = 3,
    ExportMetadata = 4,
    COUNT = 5
};

/// Status of a batch job
enum class BatchStatus : uint32_t {
    Queued = 0,
    Running = 1,
    Paused = 2,
    Completed = 3,
    Failed = 4,
    Cancelled = 5,
    COUNT = 6
};

/// Progress of a batch job
struct BatchProgress
{
    uint32_t totalFiles = 0;
    uint32_t processedFiles = 0;
    uint32_t failedFiles = 0;
    double progressPercent = 0.0;
    double elapsedMs = 0.0;
    double estimatedRemainingMs = 0.0;
};

/// A batch job descriptor
struct BatchJob
{
    std::wstring name;
    BatchOperation operation = BatchOperation::GenerateThumbnails;
    BatchStatus status = BatchStatus::Queued;
    std::vector<std::wstring> inputFiles;
    BatchProgress progress;
};

/// Manages batch file operations
class BatchProcessingEngine
{
  public:
    BatchProcessingEngine();

    static const wchar_t* GetOperationName(BatchOperation op);
    static const wchar_t* GetStatusName(BatchStatus status);
    static uint32_t GetOperationCount()
    {
        return static_cast<uint32_t>(BatchOperation::COUNT);
    }

    /// Create a new batch job
    size_t CreateJob(const std::wstring& name, BatchOperation op, const std::vector<std::wstring>& files);
    /// Get a job by index
    const BatchJob& GetJobByIndex(size_t index) const;
    /// Get total job count
    size_t GetJobCount() const
    {
        return m_jobs.size();
    }
    /// Run a job (simulated)
    bool RunJob(size_t index);
    /// Cancel a job
    bool CancelJob(size_t index);

    using ProgressCallback = std::function<void(const BatchProgress&)>;
    void SetProgressCallback(ProgressCallback cb)
    {
        m_progressCb = std::move(cb);
    }

  private:
    std::vector<BatchJob> m_jobs;
    ProgressCallback m_progressCb;
    static BatchJob s_emptyJob;
};

}  // namespace Engine
}  // namespace ExplorerLens
