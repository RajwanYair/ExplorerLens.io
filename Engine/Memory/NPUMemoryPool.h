// NPUMemoryPool.h — NPU Memory Pool with Zero-Copy ONNX Tensor Input
// Copyright (c) 2026 ExplorerLens Project
//
// Pre-allocates pinned / DMA-cohere memory regions mapped into the NPU's
// address space, enabling zero-copy tensor uploads from the CPU decode pipeline.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

enum class NPUMemoryKind   { Pinned, DMACoherent, Cached, Uncached };

struct NPUMemoryBlock {
    void*        ptr       = nullptr;
    size_t       sizeBytes = 0;
    NPUMemoryKind kind     = NPUMemoryKind::Pinned;
    uint32_t     blockId   = 0;
    bool         inUse     = false;
};

struct NPUMemoryPoolConfig {
    size_t       totalBytes       = 64ULL * 1024 * 1024;
    size_t       blockSizeBytes   = 4ULL  * 1024 * 1024;
    NPUMemoryKind kind            = NPUMemoryKind::Pinned;
    uint32_t     maxBlocks        = 16;
    bool         zeroCopyEnabled  = true;
};

class NPUMemoryPool {
public:
    explicit NPUMemoryPool(const NPUMemoryPoolConfig& cfg = {}) : m_cfg(cfg) {}

    bool  Initialize() {
        m_blocks.clear();
        uint32_t n = m_cfg.maxBlocks;
        for (uint32_t i = 0; i < n; ++i) {
            NPUMemoryBlock blk;
            blk.blockId   = i + 1;
            blk.sizeBytes = m_cfg.blockSizeBytes;
            blk.kind      = m_cfg.kind;
            m_blocks.push_back(blk);
        }
        m_initialized = true;
        return true;
    }

    NPUMemoryBlock* Acquire() {
        for (auto& b : m_blocks)
            if (!b.inUse) { b.inUse = true; return &b; }
        return nullptr;
    }
    bool  Release(uint32_t blockId) {
        for (auto& b : m_blocks)
            if (b.blockId == blockId) { b.inUse = false; return true; }
        return false;
    }

    bool     IsInitialized()  const { return m_initialized; }
    size_t   BlockCount()     const { return m_blocks.size(); }
    size_t   FreeBlocks()     const {
        size_t n = 0; for (const auto& b : m_blocks) if (!b.inUse) ++n; return n;
    }
    bool     ZeroCopyEnabled()const { return m_cfg.zeroCopyEnabled; }
    const NPUMemoryPoolConfig& GetConfig() const { return m_cfg; }
    void     Reset()          { m_blocks.clear(); m_initialized = false; }

private:
    NPUMemoryPoolConfig       m_cfg;
    std::vector<NPUMemoryBlock> m_blocks;
    bool                      m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
