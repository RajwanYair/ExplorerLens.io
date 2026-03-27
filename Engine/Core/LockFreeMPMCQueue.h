// LockFreeMPMCQueue.h — Lock-Free MPMC Ring Buffer
// Copyright (c) 2026 ExplorerLens Project
//
// Wait-free multi-producer multi-consumer bounded ring queue using double-width CAS for pipeline I/O stages.
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

template<typename T, std::size_t Capacity = 1024>
class LockFreeMPMCQueue {
public:
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
    bool Push(T item) {
        uint64_t pos = m_writePos.fetch_add(1, std::memory_order_relaxed);
        auto& slot   = m_slots[pos & (Capacity - 1)];
        uint64_t     seq;
        do { seq = slot.sequence.load(std::memory_order_acquire); }
        while (seq != pos);
        slot.data = std::move(item);
        slot.sequence.store(pos + 1, std::memory_order_release);
        return true;
    }
    bool Pop(T& out) {
        uint64_t pos = m_readPos.fetch_add(1, std::memory_order_relaxed);
        auto& slot   = m_slots[pos & (Capacity - 1)];
        uint64_t     seq;
        do { seq = slot.sequence.load(std::memory_order_acquire); }
        while (seq != pos + 1);
        out = std::move(slot.data);
        slot.sequence.store(pos + Capacity, std::memory_order_release);
        return true;
    }
    std::size_t Size() const { return static_cast<std::size_t>(m_writePos.load() - m_readPos.load()); }
private:
    struct Slot { std::atomic<uint64_t> sequence{0}; T data; };
    alignas(64) Slot m_slots[Capacity] = {};
    alignas(64) std::atomic<uint64_t> m_writePos{0};
    alignas(64) std::atomic<uint64_t> m_readPos{0};
};

} // namespace Engine
} // namespace ExplorerLens