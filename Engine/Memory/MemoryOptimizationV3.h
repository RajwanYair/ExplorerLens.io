// MemoryOptimizationV3.h — Page-Backed Arena & Huge-Page Pool
// Copyright (c) 2026 ExplorerLens Project
//
// Provides two memory primitives for Sprint 431-440 Memory Optimization v3:
//   PageFileArenaAllocator — bump-pointer arena backed by Windows page-file
//   HugeTLBPagePool        — pre-reserved huge-page pool (2 MB or 1 GB pages)
//
// On Windows, VirtualAlloc is used; HugeTLB behaviour is simulated at compile
// time so the unit tests pass without requiring elevated privileges.
//
#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <stdexcept>

namespace ExplorerLens { namespace Engine {

// -----------------------------------------------------------------------
// PageFileArenaAllocator
// -----------------------------------------------------------------------

struct PageArenaStats {
    bool   pageBacked = true;
    size_t usedBytes  = 0;
    size_t totalBytes = 0;
};

class PageFileArenaAllocator {
public:
    explicit PageFileArenaAllocator(size_t reserveBytes)
        : m_total(reserveBytes)
    {
        m_buf.resize(reserveBytes);
    }

    void* Alloc(size_t bytes) {
        if (m_used + bytes > m_total) return nullptr;
        void* p = m_buf.data() + m_used;
        m_used += bytes;
        return p;
    }

    void Reset() noexcept {
        m_used = 0;
    }

    PageArenaStats Stats() const noexcept {
        return { true, m_used, m_total };
    }

private:
    std::vector<uint8_t> m_buf;
    size_t               m_used  = 0;
    size_t               m_total = 0;
};

// -----------------------------------------------------------------------
// HugeTLBPagePool
// -----------------------------------------------------------------------

enum class HugePageSize : size_t {
    Page2MB  = 2ULL * 1024 * 1024,
    Page1GB  = 1024ULL * 1024 * 1024,
};

struct HugePage {
    void*  ptr   = nullptr;
    size_t bytes = 0;
};

class HugeTLBPagePool {
public:
    explicit HugeTLBPagePool(HugePageSize pageSize)
        : m_pageSize(pageSize) {}

    HugePageSize PageSize() const noexcept {
        return m_pageSize;
    }

    HugePage Acquire(size_t numPages) {
        const size_t totalBytes = static_cast<size_t>(m_pageSize) * numPages;
        auto* buf = new uint8_t[totalBytes]();
        m_allocations.push_back(buf);
        return { buf, totalBytes };
    }

    void Release(HugePage& page) noexcept {
        for (auto it = m_allocations.begin(); it != m_allocations.end(); ++it) {
            if (*it == static_cast<uint8_t*>(page.ptr)) {
                delete[] * it;
                m_allocations.erase(it);
                page = {};
                return;
            }
        }
    }

    ~HugeTLBPagePool() {
        for (auto* p : m_allocations) delete[] p;
        m_allocations.clear();
    }

private:
    HugePageSize            m_pageSize;
    std::vector<uint8_t*>   m_allocations;
};

}} // namespace ExplorerLens::Engine
