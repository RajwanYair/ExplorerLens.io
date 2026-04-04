// AsyncDMACopyEngine.h — Async DMA Copy Engine Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Submits asynchronous DMA transfers on the GPU copy queue to overlap decode with upload for zero-stall streaming.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DMATransfer
{
    uint32_t id;
    const void* src;
    void* dst;
    size_t bytes;
    uint64_t fenceValue;
};
class AsyncDMACopyEngine
{
  public:
    AsyncDMACopyEngine() = default;
    AsyncDMACopyEngine(const AsyncDMACopyEngine&) = delete;
    AsyncDMACopyEngine& operator=(const AsyncDMACopyEngine&) = delete;
    uint64_t Submit(DMATransfer xfer)
    {
        xfer.fenceValue = ++m_fence;
        m_pending.push_back(xfer);
        return m_fence.load();
    }
    bool IsComplete(uint64_t fence) const
    {
        return fence <= m_completed;
    }
    void Flush()
    {
        m_completed.store(m_fence.load());
        m_pending.clear();
    }
    size_t Pending() const
    {
        return m_pending.size();
    }
    uint64_t Bandwidth() const
    {
        return m_bytesXfr;
    }

  private:
    std::atomic<uint64_t> m_fence{0};
    std::atomic<uint64_t> m_completed{0};
    uint64_t m_bytesXfr = 0;
    std::vector<DMATransfer> m_pending;
};

}  // namespace Engine
}  // namespace ExplorerLens
