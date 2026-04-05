// DirectStorageBatchScheduler.h — Batched DirectStorage I/O request scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Coalesces multiple thumbnail decode requests into a single DirectStorage
// batch submission to maximise NVMe queue depth utilisation and minimise
// per-request round-trip overhead. After I/O completes, decompression is
// routed through GPUDecompressOrchestrator before the decode pipeline runs.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct DSBatchItem {
    std::wstring filePath;
    uint32_t     thumbSize   = 256;
    uint32_t     itemId      = 0;
};

struct DSBatchResult {
    uint32_t itemId            = 0;
    bool     success           = false;
    float    totalLatencyMs    = 0.0f;
    uint32_t compressedBytes   = 0;
    uint32_t decompressedBytes = 0;
};

class DirectStorageBatchScheduler {
public:
    static DirectStorageBatchScheduler& Instance();

    void   AddItem(const DSBatchItem& item);
    bool   Flush();
    void   Reset();

    const std::vector<DSBatchResult>& GetResults() const noexcept { return m_results; }
    uint32_t PendingCount()                        const noexcept { return static_cast<uint32_t>(m_pending.size()); }
    bool     IsNvmeDirectStorageEnabled()          const noexcept { return m_nvmeEnabled; }

private:
    std::vector<DSBatchItem>   m_pending;
    std::vector<DSBatchResult> m_results;
    bool                       m_nvmeEnabled = true;
};

}} // namespace ExplorerLens::Engine
