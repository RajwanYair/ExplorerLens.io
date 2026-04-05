// DirectStorageBatchScheduler.cpp — Batched DirectStorage I/O request scheduler
// Copyright (c) 2026 ExplorerLens Project

#include "DirectStorageBatchScheduler.h"

namespace ExplorerLens { namespace Engine {

DirectStorageBatchScheduler& DirectStorageBatchScheduler::Instance()
{
    static DirectStorageBatchScheduler instance;
    return instance;
}

void DirectStorageBatchScheduler::AddItem(const DSBatchItem& item)
{
    m_pending.push_back(item);
}

bool DirectStorageBatchScheduler::Flush()
{
    if (m_pending.empty()) { return true; }

    m_results.clear();
    m_results.reserve(m_pending.size());

    for (const DSBatchItem& item : m_pending)
    {
        DSBatchResult r{};
        r.itemId            = item.itemId;
        r.success           = !item.filePath.empty();
        r.totalLatencyMs    = r.success ? 8.5f : 0.0f;   // simulated DS latency
        r.compressedBytes   = r.success ? 512'000 : 0;
        r.decompressedBytes = r.success ? 2'048'000 : 0;
        m_results.push_back(r);
    }

    m_pending.clear();
    return true;
}

void DirectStorageBatchScheduler::Reset()
{
    m_pending.clear();
    m_results.clear();
}

}} // namespace ExplorerLens::Engine
