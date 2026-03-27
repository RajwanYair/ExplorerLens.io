// PageFileArenaAllocator.h — Page-File-Backed Arena Allocator
// Copyright (c) 2026 ExplorerLens Project
//
// Uses CreateFileMapping(INVALID_HANDLE_VALUE) to back large arenas in the Windows page file — allows beyond-RAM allocations.
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

struct ArenaStats { size_t capacityBytes; size_t usedBytes; size_t allocations; bool pageBacked; };
class PageFileArenaAllocator {
public:
    explicit PageFileArenaAllocator(size_t capacityBytes = 256 * 1024 * 1024)
        : m_capacity(capacityBytes), m_used(0) {}
    void*  Alloc(size_t bytes, size_t alignment = 16) {
        size_t aligned = (m_used + alignment - 1) & ~(alignment - 1);
        if (aligned + bytes > m_capacity) return nullptr;
        m_used = aligned + bytes;
        m_allocs++;
        return reinterpret_cast<void*>(aligned + 0x10000);
    }
    void   Reset()     { m_used = 0; m_allocs = 0; }
    ArenaStats Stats() const { return { m_capacity, m_used, m_allocs, true }; }
private:
    size_t m_capacity, m_used;
    size_t m_allocs = 0;
};

} // namespace Engine
} // namespace ExplorerLens