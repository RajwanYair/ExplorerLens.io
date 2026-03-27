// ThreadLocalContextPool.h — Thread-Local Decode Context Pool
// Copyright (c) 2026 ExplorerLens Project
//
// Pool of per-thread decode contexts (scratch buffers, decoders, metrics) avoiding cross-thread contention.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct DecodeContext {
    std::vector<uint8_t> scratchBuffer;
    uint32_t             decoderFlags = 0;
    uint64_t             totalDecodes = 0;
    void Reset() { scratchBuffer.clear(); decoderFlags = 0; }
};
class ThreadLocalContextPool {
public:
    DecodeContext& Acquire()    { return m_ctx; }
    void           Release()    { m_ctx.Reset(); }
    uint64_t       TotalDecodes() const { return m_ctx.totalDecodes; }
    void           SetScratchSize(size_t bytes) { m_ctx.scratchBuffer.resize(bytes); }
private:
    static thread_local DecodeContext m_ctx;
};
inline thread_local DecodeContext ThreadLocalContextPool::m_ctx{};

} // namespace Engine
} // namespace ExplorerLens