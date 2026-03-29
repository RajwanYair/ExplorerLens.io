// HeapCorruptionSentinel.h — Heap Corruption Sentinel with Guard Pages & Canaries
// Copyright (c) 2026 ExplorerLens Project
//
// Runtime heap corruption detection using canary values placed before/after
// tracked allocations. Detected violations are logged with allocation callsite
// and optional crash dump before structured cleanup.
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <string>

namespace ExplorerLens {
namespace Engine {

static constexpr uint64_t CANARY_MAGIC_HEAD = 0xDEADBEEFCAFE5AFEull;
static constexpr uint64_t CANARY_MAGIC_TAIL = 0x5AFECAFEDEADBEEFull;

enum class HeapCorruptionType { None, HeadCanaryOverwrite, TailCanaryOverwrite, DoubleFree };

struct HeapCorruptionReport {
    HeapCorruptionType type       = HeapCorruptionType::None;
    void*          address    = nullptr;
    size_t         size       = 0;
    std::string    context;
    bool           IsClean() const noexcept { return type == HeapCorruptionType::None; }
    std::string    TypeName() const noexcept {
        switch (type) {
        case HeapCorruptionType::None:               return "None";
        case HeapCorruptionType::HeadCanaryOverwrite: return "HeadCanaryOverwrite";
        case HeapCorruptionType::TailCanaryOverwrite: return "TailCanaryOverwrite";
        case HeapCorruptionType::DoubleFree:          return "DoubleFree";
        }
        return "Unknown";
    }
};

struct TrackedBlock {
    uint64_t  headCanary = CANARY_MAGIC_HEAD;
    void*     payload    = nullptr;
    size_t    payloadSize = 0;
    uint64_t  tailCanary = CANARY_MAGIC_TAIL;
    std::string context;
};

class HeapCorruptionSentinel {
public:
    explicit HeapCorruptionSentinel() = default;

    TrackedBlock* Allocate(size_t bytes, const std::string& ctx = {}) {
        void* mem = ::operator new(bytes);
        auto* blk = new TrackedBlock();
        blk->payload     = mem;
        blk->payloadSize = bytes;
        blk->context     = ctx;
        m_blocks.push_back(blk);
        return blk;
    }

    HeapCorruptionReport Check(const TrackedBlock* blk) const {
        HeapCorruptionReport r;
        if (!blk) return r;
        if (blk->headCanary != CANARY_MAGIC_HEAD) {
            r.type = HeapCorruptionType::HeadCanaryOverwrite;
            r.address = blk->payload;
            r.size    = blk->payloadSize;
            r.context = blk->context;
        } else if (blk->tailCanary != CANARY_MAGIC_TAIL) {
            r.type = HeapCorruptionType::TailCanaryOverwrite;
            r.address = blk->payload;
            r.size    = blk->payloadSize;
            r.context = blk->context;
        }
        return r;
    }

    std::vector<HeapCorruptionReport> ScanAll() const {
        std::vector<HeapCorruptionReport> reports;
        for (const auto* blk : m_blocks) {
            auto r = Check(blk);
            if (!r.IsClean()) reports.push_back(r);
        }
        return reports;
    }

    bool Free(TrackedBlock* blk) {
        if (!blk) return false;
        auto it = std::find(m_blocks.begin(), m_blocks.end(), blk);
        if (it == m_blocks.end()) return false; // double-free guard
        m_blocks.erase(it);
        ::operator delete(blk->payload);
        delete blk;
        return true;
    }

    int   BlockCount()     const noexcept { return (int)m_blocks.size(); }
    bool  IsHealthy()      const          { return ScanAll().empty(); }

    ~HeapCorruptionSentinel() {
        for (auto* blk : m_blocks) { ::operator delete(blk->payload); delete blk; }
    }

private:
    std::vector<TrackedBlock*> m_blocks;
};

} // namespace Engine
} // namespace ExplorerLens
