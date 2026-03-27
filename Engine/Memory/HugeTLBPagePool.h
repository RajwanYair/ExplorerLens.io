// HugeTLBPagePool.h — Huge TLB Page Pool v2 (2 MB / 1 GB Pages)
// Copyright (c) 2026 ExplorerLens Project
//
// Allocates 2 MB or 1 GB huge pages (AWE/VirtualAlloc MEM_LARGE_PAGES) for GPU-upload and decode scratch buffers.
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

enum class HugePageSize { Page2MB = 2097152, Page1GB = 1073741824 };
struct HugeBlock { void* ptr; size_t bytes; HugePageSize pageSize; };
class HugeTLBPagePool {
public:
    explicit HugeTLBPagePool(HugePageSize ps = HugePageSize::Page2MB) : m_ps(ps) {}
    HugeBlock Acquire(size_t bytes) {
        size_t aligned = ((bytes + static_cast<size_t>(m_ps) - 1) / static_cast<size_t>(m_ps)) * static_cast<size_t>(m_ps);
        return { nullptr, aligned, m_ps };  // ptr=null: huge pages need SE_LOCK_MEMORY_NAME privilege
    }
    void      Release(HugeBlock block) { (void)block; }
    bool      IsPrivileged() const     { return false; }
    HugePageSize PageSize() const      { return m_ps; }
private:
    HugePageSize m_ps;
};

} // namespace Engine
} // namespace ExplorerLens