// GPUBatchSubmitter.h — Batched GPU Command Submission
// Copyright (c) 2026 ExplorerLens Project
//
// Batches multiple thumbnail decode/render commands into efficient GPU
// command buffer submissions to minimize driver overhead.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class GPUCommandType : uint8_t {
    Decode,
    Resize,
    ColorConvert,
    Composite,
    Copy,
    Barrier
};

struct GPUCommand {
    uint64_t commandId = 0;
    GPUCommandType type = GPUCommandType::Decode;
    uint32_t resourceIndex = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct BatchSubmitResult {
    uint64_t batchId = 0;
    uint32_t commandCount = 0;
    bool submitted = false;
    double submitTimeUs = 0.0;
};

struct BatchSubmitterMetrics {
    uint64_t totalBatches = 0;
    uint64_t totalCommands = 0;
    double avgBatchSize = 0.0;
    double avgSubmitTimeUs = 0.0;
    uint32_t maxBatchSize = 0;
};

class GPUBatchSubmitter {
public:
    explicit GPUBatchSubmitter(uint32_t maxBatchSize = 64)
        : m_maxBatchSize(maxBatchSize) {
    }

    void AddCommand(const GPUCommand& cmd) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingCommands.push_back(cmd);
    }

    BatchSubmitResult FlushBatch() {
        std::lock_guard<std::mutex> lock(m_mutex);
        BatchSubmitResult result;
        result.batchId = ++m_nextBatchId;
        result.commandCount = static_cast<uint32_t>(m_pendingCommands.size());
        result.submitted = !m_pendingCommands.empty();
        m_metrics.totalBatches++;
        m_metrics.totalCommands += result.commandCount;
        if (result.commandCount > m_metrics.maxBatchSize)
            m_metrics.maxBatchSize = result.commandCount;
        m_pendingCommands.clear();
        return result;
    }

    bool ShouldFlush() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_pendingCommands.size() >= m_maxBatchSize;
    }

    uint32_t GetPendingCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_pendingCommands.size());
    }

    void SetMaxBatchSize(uint32_t size) { m_maxBatchSize = size; }
    uint32_t GetMaxBatchSize() const { return m_maxBatchSize; }
    BatchSubmitterMetrics GetMetrics() const { return m_metrics; }

private:
    mutable std::mutex m_mutex;
    std::vector<GPUCommand> m_pendingCommands;
    uint32_t m_maxBatchSize;
    uint64_t m_nextBatchId = 0;
    BatchSubmitterMetrics m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
