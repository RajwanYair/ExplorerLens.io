// AIThumbnailBatchProcessor.h — Batch-mode AI thumbnail generation queue
// Copyright (c) 2026 ExplorerLens Project
//
// Processes large batches of AI thumbnail synthesis requests in priority-ordered
// queues with configurable concurrency limits. Routes work to NPUThumbnailSynthesizer
// and falls back to CPU inference for unsupported formats. Tracks per-batch
// throughput and latency for adaptive throttling.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class BatchPriority : uint8_t
{
    Low      = 0,
    Normal   = 1,
    High     = 2,
    Realtime = 3,
};

struct BatchRequest
{
    std::string   filePath;
    uint32_t      requestId   = 0;
    BatchPriority priority    = BatchPriority::Normal;
};

struct BatchResult
{
    uint32_t              requestId      = 0;
    bool                  success        = false;
    float                 inferenceMs    = 0.0f;
    std::vector<uint8_t>  pixels;
};

struct BatchProcessorStats
{
    uint64_t  totalProcessed    = 0;
    uint64_t  totalFailed       = 0;
    float     avgInferenceMs    = 0.0f;
    float     throughputImgSec  = 0.0f;
};

class AIThumbnailBatchProcessor
{
public:
    AIThumbnailBatchProcessor();
    ~AIThumbnailBatchProcessor();

    AIThumbnailBatchProcessor(const AIThumbnailBatchProcessor&)            = delete;
    AIThumbnailBatchProcessor& operator=(const AIThumbnailBatchProcessor&) = delete;

    bool                       Initialize(uint32_t maxConcurrency = 4);
    void                       Shutdown();
    uint32_t                   Enqueue(const BatchRequest& req);
    bool                       ProcessBatch(std::vector<BatchResult>& outResults);
    void                       ClearQueue();
    BatchProcessorStats        GetStats()   const noexcept { return m_stats; }
    uint32_t                   QueueDepth() const noexcept { return static_cast<uint32_t>(m_queue.size()); }

    static AIThumbnailBatchProcessor& Instance() noexcept;

private:
    std::vector<BatchRequest>  m_queue;
    BatchProcessorStats        m_stats;
    uint32_t                   m_maxConcurrency = 4;
    uint32_t                   m_nextId         = 1;
    bool                       m_initialized    = false;
    static AIThumbnailBatchProcessor s_instance;
};

}} // namespace ExplorerLens::Engine
