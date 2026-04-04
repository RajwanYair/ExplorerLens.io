// MultiFormatBatchDecoder.h — Batch Decode Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates batch decoding of multiple files across different formats,
// routing each to the optimal decoder and managing parallel execution.
//
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class MultiBatchItemStatus : uint8_t {
    Pending = 0,
    Decoding = 1,
    Success = 2,
    Failed = 3,
    Skipped = 4,
    CacheHit = 5
};

struct BatchDecodeItem
{
    std::wstring filePath;
    std::string decoderName;
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    MultiBatchItemStatus status = MultiBatchItemStatus::Pending;
    double decodeMs = 0.0;
    std::string errorMessage;
    uint32_t priority = 0;
};

struct MultiBatchDecodeConfig
{
    uint32_t maxConcurrent = 4;
    uint32_t timeoutPerItemMs = 5000;
    uint32_t maxBatchSize = 100;
    bool skipOnError = true;
    bool checkCacheFirst = true;
    bool priorityOrdering = true;
};

struct BatchDecodeResult
{
    uint32_t totalItems = 0;
    uint32_t successCount = 0;
    uint32_t failedCount = 0;
    uint32_t skippedCount = 0;
    uint32_t cacheHitCount = 0;
    double totalMs = 0.0;
    double avgItemMs = 0.0;
    double throughputPerSec = 0.0;
};

class MultiFormatBatchDecoder
{
  public:
    void Configure(const MultiBatchDecodeConfig& config)
    {
        m_config = config;
    }

    BatchDecodeResult Summarize(const std::vector<BatchDecodeItem>& items) const
    {
        BatchDecodeResult r;
        r.totalItems = static_cast<uint32_t>(items.size());
        for (const auto& item : items) {
            switch (item.status) {
                case MultiBatchItemStatus::Success:
                    r.successCount++;
                    break;
                case MultiBatchItemStatus::Failed:
                    r.failedCount++;
                    break;
                case MultiBatchItemStatus::Skipped:
                    r.skippedCount++;
                    break;
                case MultiBatchItemStatus::CacheHit:
                    r.cacheHitCount++;
                    break;
                default:
                    break;
            }
            r.totalMs += item.decodeMs;
        }
        uint32_t processed = r.successCount + r.failedCount;
        r.avgItemMs = processed > 0 ? r.totalMs / processed : 0.0;
        r.throughputPerSec = r.totalMs > 0 ? processed * 1000.0 / r.totalMs : 0.0;
        return r;
    }

    void SortByPriority(std::vector<BatchDecodeItem>& items) const
    {
        if (!m_config.priorityOrdering)
            return;
        std::sort(items.begin(), items.end(),
                  [](const BatchDecodeItem& a, const BatchDecodeItem& b) { return a.priority > b.priority; });
    }

    bool ShouldSkip(const BatchDecodeItem& item) const
    {
        return item.status == MultiBatchItemStatus::Failed && m_config.skipOnError;
    }

    MultiBatchDecodeConfig GetConfig() const
    {
        return m_config;
    }

  private:
    MultiBatchDecodeConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
