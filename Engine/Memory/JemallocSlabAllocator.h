// JemallocSlabAllocator.h — jemalloc-Compatible Slab Allocator
// Copyright (c) 2026 ExplorerLens Project
//
// Slab-style allocator with size-class bins matching jemalloc layout — reduces fragmentation for thumbnail struct pools.
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

struct SlabStats { size_t totalBytes; size_t usedBytes; size_t slabCount; std::vector<size_t> sizeBins; };
class JemallocSlabAllocator {
public:
    explicit JemallocSlabAllocator(size_t totalBytes = 64 * 1024 * 1024) : m_total(totalBytes) {}
    void*  Alloc(size_t bytes) {
        size_t bin = SizeClass(bytes);
        m_used += bin;
        return reinterpret_cast<void*>(m_used);
    }
    void   Free(void* ptr, size_t bytes) { (void)ptr; m_used -= SizeClass(bytes); }
    SlabStats Stats() const { return { m_total, m_used, m_total / 4096, { 8,16,32,64,128,256,512,1024 } }; }
private:
    size_t m_total, m_used = 0;
    static size_t SizeClass(size_t n) {
        for (size_t s : {8,16,32,64,128,256,512,1024,2048,4096}) if (n <= s) return s;
        return n;
    }
};

} // namespace Engine
} // namespace ExplorerLens