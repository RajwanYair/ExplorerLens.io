#pragma once
//==============================================================================
// ParallelBatchDecoder
// Thread pool decoder with per-format parallelism controls
//
// Architecture:
// 1. Accept batch of file paths for thumbnail generation
// 2. Group by format for optimal parallelism (some decoders not thread-safe)
// 3. Dispatch to configurable thread pool with priority ordering
// 4. Collect results with progress callbacks
//
// Thread safety rules per format:
// - Image formats (JPEG/PNG/BMP/etc): Fully parallel
// - Archive formats (ZIP/RAR/7z): Serial per archive
// - GPU-accelerated (DDS/KTX): Limited by GPU resources
// - External libraries (LibRaw/HEIF): Library-specific limits
//==============================================================================

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Parallelism strategy per format category
enum class ParallelismLevel : uint8_t {
    FullParallel,     ///< No restrictions (most image formats)
    LimitedParallel,  ///< Max N concurrent (GPU/external lib)
    SerialOnly,       ///< One at a time (archive extraction)
    Adaptive          ///< Adjust based on system load
};

/// Batch decode priority
enum class BatchPriority : uint8_t {
    Immediate = 0,   ///< User-triggered, highest priority
    Background = 1,  ///< Background pre-fetch
    CacheWarm = 2    ///< Cache warming, lowest priority
};

/// Status of a single item in a batch
enum class BatchItemStatus : uint8_t {
    Pending,
    Decoding,
    Completed,
    Failed,
    Skipped,
    Cancelled
};

/// Result for a single batch item
struct BatchItemResult
{
    std::wstring filePath;
    BatchItemStatus status = BatchItemStatus::Pending;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> pixelData;
    double decodeTimeMs = 0.0;
    std::wstring errorMessage;
    std::wstring formatName;
};

/// Progress callback
/// @param completed Number of items completed so far
/// @param total Total items in batch
/// @param currentFile File currently being processed
using BatchProgressCallback = std::function<void(uint32_t completed, uint32_t total, const std::wstring& currentFile)>;

/// Batch completion callback
using BatchCompleteCallback = std::function<void(const std::vector<BatchItemResult>& results)>;

/// Format parallelism configuration
struct FormatParallelism
{
    std::wstring formatCategory;
    ParallelismLevel level = ParallelismLevel::FullParallel;
    uint32_t maxConcurrent = 0;  ///< 0 = unlimited (for FullParallel)
};

/// Batch decoder configuration
struct BatchDecoderConfig
{
    uint32_t workerThreads = 4;        ///< Size of thread pool
    uint32_t maxBatchSize = 1000;      ///< Maximum files per batch
    uint32_t thumbnailSize = 256;      ///< Default thumbnail dimension
    uint32_t perItemTimeoutMs = 5000;  ///< Per-file timeout
    uint32_t batchTimeoutMs = 60000;   ///< Total batch timeout
    bool enableProgressCallbacks = true;
    bool skipOnError = true;                     ///< Continue batch on individual failures
    bool enableFormatGrouping = true;            ///< Group files by format for optimal I/O
    std::vector<FormatParallelism> formatRules;  ///< Per-format parallelism overrides
};

/// Batch statistics
struct BatchStats
{
    uint64_t totalBatches = 0;
    uint64_t totalItems = 0;
    uint64_t completedItems = 0;
    uint64_t failedItems = 0;
    uint64_t skippedItems = 0;
    uint64_t cancelledItems = 0;
    double avgItemTimeMs = 0.0;
    double totalBatchTimeMs = 0.0;
    double throughputItemsPerSec = 0.0;
    uint32_t peakConcurrency = 0;
};

//==============================================================================
// ParallelBatchDecoder
//==============================================================================
class ParallelBatchDecoder
{
  public:
    ParallelBatchDecoder();
    explicit ParallelBatchDecoder(const BatchDecoderConfig& config);
    ~ParallelBatchDecoder();

    ParallelBatchDecoder(const ParallelBatchDecoder&) = delete;
    ParallelBatchDecoder& operator=(const ParallelBatchDecoder&) = delete;

    /// Initialize thread pool
    bool Initialize();

    /// Shutdown and cancel pending work
    void Shutdown();

    /// Submit a batch of files for parallel decode
    /// @return Batch ID or 0 on failure
    uint64_t SubmitBatch(const std::vector<std::wstring>& files, BatchPriority priority = BatchPriority::Immediate,
                         BatchProgressCallback progressCb = nullptr, BatchCompleteCallback completeCb = nullptr);

    /// Cancel a batch
    bool CancelBatch(uint64_t batchId);

    /// Get results for a completed batch
    std::vector<BatchItemResult> GetBatchResults(uint64_t batchId) const;

    /// Wait for a batch to complete
    bool WaitForBatch(uint64_t batchId, uint32_t timeoutMs);

    /// Get parallelism level for a file extension
    ParallelismLevel GetParallelismForFormat(const std::wstring& extension) const;

    /// Get overall statistics
    BatchStats GetStats() const;

    /// Check if running
    bool IsRunning() const;

    /// Get configuration
    const BatchDecoderConfig& GetConfig() const
    {
        return m_config;
    }

    /// Static helpers
    static const wchar_t* GetParallelismName(ParallelismLevel level);
    static const wchar_t* GetBatchStatusName(BatchItemStatus status);
    static const wchar_t* GetBatchPriorityName(BatchPriority priority);

    /// Classify a file extension into a parallelism category
    static ParallelismLevel ClassifyFormat(const std::wstring& extension);

  private:
    /// Group files by format for optimal dispatch
    std::unordered_map<std::wstring, std::vector<size_t>> GroupByFormat(const std::vector<std::wstring>& files) const;

    /// Extract extension from file path
    static std::wstring GetExtension(const std::wstring& path);

    BatchDecoderConfig m_config;
    std::atomic<bool> m_running{false};
    std::atomic<uint64_t> m_nextBatchId{1};
    mutable std::mutex m_batchMutex;
    std::unordered_map<uint64_t, std::vector<BatchItemResult>> m_batchResults;
    BatchStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
