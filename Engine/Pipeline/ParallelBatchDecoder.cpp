//==============================================================================
// ParallelBatchDecoder — Sprint 189 Implementation
// Thread pool decoder with format-aware parallelism
//==============================================================================

#include "ParallelBatchDecoder.h"
#include <algorithm>
#include <cctype>

namespace DarkThumbs { namespace Engine {

//------------------------------------------------------------------------------
// Construction / Destruction
//------------------------------------------------------------------------------

ParallelBatchDecoder::ParallelBatchDecoder()
    : m_config()
{
}

ParallelBatchDecoder::ParallelBatchDecoder(const BatchDecoderConfig& config)
    : m_config(config)
{
}

ParallelBatchDecoder::~ParallelBatchDecoder()
{
    Shutdown();
}

//------------------------------------------------------------------------------
// Lifecycle
//------------------------------------------------------------------------------

bool ParallelBatchDecoder::Initialize()
{
    if (m_running.load())
        return true;

    if (m_config.workerThreads == 0)
        m_config.workerThreads = 2;
    if (m_config.maxBatchSize == 0)
        m_config.maxBatchSize = 100;

    m_running.store(true);
    return true;
}

void ParallelBatchDecoder::Shutdown()
{
    if (!m_running.load())
        return;

    m_running.store(false);

    std::lock_guard<std::mutex> lock(m_batchMutex);
    for (auto& [batchId, results] : m_batchResults) {
        for (auto& item : results) {
            if (item.status == BatchItemStatus::Pending ||
                item.status == BatchItemStatus::Decoding) {
                item.status = BatchItemStatus::Cancelled;
                m_stats.cancelledItems++;
            }
        }
    }
}

//------------------------------------------------------------------------------
// Batch Operations
//------------------------------------------------------------------------------

uint64_t ParallelBatchDecoder::SubmitBatch(
    const std::vector<std::wstring>& files,
    BatchPriority priority,
    BatchProgressCallback progressCb,
    BatchCompleteCallback completeCb)
{
    if (!m_running.load() || files.empty())
        return 0;

    if (files.size() > m_config.maxBatchSize)
        return 0;

    uint64_t batchId = m_nextBatchId.fetch_add(1);

    // Create batch item results
    std::vector<BatchItemResult> results;
    results.reserve(files.size());

    for (const auto& file : files) {
        BatchItemResult item;
        item.filePath = file;
        item.status = BatchItemStatus::Pending;
        
        // Extract format name from extension
        auto ext = GetExtension(file);
        item.formatName = ext.empty() ? L"unknown" : ext;
        
        results.push_back(std::move(item));
    }

    {
        std::lock_guard<std::mutex> lock(m_batchMutex);
        m_batchResults[batchId] = std::move(results);
    }

    m_stats.totalBatches++;
    m_stats.totalItems += files.size();

    // In production, dispatch to thread pool here
    // For now, items remain in Pending state awaiting worker threads

    return batchId;
}

bool ParallelBatchDecoder::CancelBatch(uint64_t batchId)
{
    std::lock_guard<std::mutex> lock(m_batchMutex);
    auto it = m_batchResults.find(batchId);
    if (it == m_batchResults.end())
        return false;

    for (auto& item : it->second) {
        if (item.status == BatchItemStatus::Pending) {
            item.status = BatchItemStatus::Cancelled;
            m_stats.cancelledItems++;
        }
    }
    return true;
}

std::vector<BatchItemResult> ParallelBatchDecoder::GetBatchResults(uint64_t batchId) const
{
    std::lock_guard<std::mutex> lock(m_batchMutex);
    auto it = m_batchResults.find(batchId);
    if (it == m_batchResults.end())
        return {};
    return it->second;
}

bool ParallelBatchDecoder::WaitForBatch(uint64_t batchId, uint32_t timeoutMs)
{
    auto deadline = std::chrono::steady_clock::now() +
                    std::chrono::milliseconds(timeoutMs);

    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(m_batchMutex);
            auto it = m_batchResults.find(batchId);
            if (it == m_batchResults.end())
                return false;

            bool allDone = true;
            for (const auto& item : it->second) {
                if (item.status == BatchItemStatus::Pending ||
                    item.status == BatchItemStatus::Decoding) {
                    allDone = false;
                    break;
                }
            }
            if (allDone)
                return true;
        }
        std::this_thread::yield();
    }
    return false;
}

//------------------------------------------------------------------------------
// Format Classification
//------------------------------------------------------------------------------

ParallelismLevel ParallelBatchDecoder::GetParallelismForFormat(
    const std::wstring& extension) const
{
    // Check user-configured overrides first
    for (const auto& rule : m_config.formatRules) {
        if (rule.formatCategory == extension)
            return rule.level;
    }
    return ClassifyFormat(extension);
}

ParallelismLevel ParallelBatchDecoder::ClassifyFormat(const std::wstring& extension)
{
    // Archive formats — serial extraction required
    if (extension == L".zip" || extension == L".rar" || extension == L".7z" ||
        extension == L".cbz" || extension == L".cbr" || extension == L".cb7" ||
        extension == L".tar" || extension == L".gz") {
        return ParallelismLevel::SerialOnly;
    }

    // GPU-dependent formats — limited by GPU resources
    if (extension == L".dds" || extension == L".ktx" || extension == L".ktx2" ||
        extension == L".vtf") {
        return ParallelismLevel::LimitedParallel;
    }

    // External library formats — library-specific limits
    if (extension == L".cr2" || extension == L".nef" || extension == L".arw" ||
        extension == L".dng" || extension == L".orf" || extension == L".rw2" ||
        extension == L".heif" || extension == L".heic" || extension == L".avif") {
        return ParallelismLevel::LimitedParallel;
    }

    // Standard image formats — fully parallel
    return ParallelismLevel::FullParallel;
}

//------------------------------------------------------------------------------
// Statistics
//------------------------------------------------------------------------------

BatchStats ParallelBatchDecoder::GetStats() const
{
    return m_stats;
}

bool ParallelBatchDecoder::IsRunning() const
{
    return m_running.load();
}

//------------------------------------------------------------------------------
// Static Helpers
//------------------------------------------------------------------------------

const wchar_t* ParallelBatchDecoder::GetParallelismName(ParallelismLevel level)
{
    switch (level) {
        case ParallelismLevel::FullParallel:    return L"FullParallel";
        case ParallelismLevel::LimitedParallel: return L"LimitedParallel";
        case ParallelismLevel::SerialOnly:      return L"SerialOnly";
        case ParallelismLevel::Adaptive:        return L"Adaptive";
        default:                                return L"Unknown";
    }
}

const wchar_t* ParallelBatchDecoder::GetBatchStatusName(BatchItemStatus status)
{
    switch (status) {
        case BatchItemStatus::Pending:   return L"Pending";
        case BatchItemStatus::Decoding:  return L"Decoding";
        case BatchItemStatus::Completed: return L"Completed";
        case BatchItemStatus::Failed:    return L"Failed";
        case BatchItemStatus::Skipped:   return L"Skipped";
        case BatchItemStatus::Cancelled: return L"Cancelled";
        default:                         return L"Unknown";
    }
}

const wchar_t* ParallelBatchDecoder::GetBatchPriorityName(BatchPriority priority)
{
    switch (priority) {
        case BatchPriority::Immediate:  return L"Immediate";
        case BatchPriority::Background: return L"Background";
        case BatchPriority::CacheWarm:  return L"CacheWarm";
        default:                        return L"Unknown";
    }
}

//------------------------------------------------------------------------------
// Private Implementation
//------------------------------------------------------------------------------

std::unordered_map<std::wstring, std::vector<size_t>>
ParallelBatchDecoder::GroupByFormat(const std::vector<std::wstring>& files) const
{
    std::unordered_map<std::wstring, std::vector<size_t>> groups;
    for (size_t i = 0; i < files.size(); ++i) {
        auto ext = GetExtension(files[i]);
        groups[ext].push_back(i);
    }
    return groups;
}

std::wstring ParallelBatchDecoder::GetExtension(const std::wstring& path)
{
    auto pos = path.rfind(L'.');
    if (pos == std::wstring::npos || pos == path.size() - 1)
        return L"";

    std::wstring ext = path.substr(pos);
    // Lowercase
    for (auto& c : ext) {
        c = static_cast<wchar_t>(std::tolower(static_cast<int>(c)));
    }
    return ext;
}

}} // namespace DarkThumbs::Engine
