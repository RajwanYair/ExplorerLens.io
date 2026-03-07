// StackAllocator.h — Fast Linear Stack-Based Memory Allocator
// Copyright (c) 2026 ExplorerLens Project
//
// Provides O(1) allocation and batch deallocation for short-lived decode
// scratch buffers using a pre-allocated stack with LIFO semantics.
//
#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>

namespace ExplorerLens {
namespace Engine {

class StackAllocator {
public:
    using Marker = size_t;

    explicit StackAllocator(size_t capacityBytes = 1024 * 1024)
        : m_capacity(capacityBytes), m_offset(0), m_peakUsage(0), m_allocCount(0) {
        m_buffer = new uint8_t[m_capacity];
    }

    ~StackAllocator() { delete[] m_buffer; }

    StackAllocator(const StackAllocator&) = delete;
    StackAllocator& operator=(const StackAllocator&) = delete;

    void* Allocate(size_t bytes, size_t alignment = 16) {
        size_t aligned = (m_offset + alignment - 1) & ~(alignment - 1);
        if (aligned + bytes > m_capacity) return nullptr;
        void* ptr = m_buffer + aligned;
        m_offset = aligned + bytes;
        if (m_offset > m_peakUsage) m_peakUsage = m_offset;
        m_allocCount++;
        return ptr;
    }

    Marker GetMarker() const { return m_offset; }

    void FreeToMarker(Marker marker) {
        assert(marker <= m_offset);
        m_offset = marker;
    }

    void Reset() { m_offset = 0; }

    size_t Capacity() const { return m_capacity; }
    size_t Used() const { return m_offset; }
    size_t Available() const { return m_capacity - m_offset; }
    size_t PeakUsage() const { return m_peakUsage; }
    uint64_t AllocationCount() const { return m_allocCount; }

private:
    uint8_t* m_buffer = nullptr;
    size_t m_capacity;
    size_t m_offset;
    size_t m_peakUsage;
    uint64_t m_allocCount;
};

class ScopedStackMark {
public:
    explicit ScopedStackMark(StackAllocator& alloc)
        : m_alloc(alloc), m_marker(alloc.GetMarker()) {
    }
    ~ScopedStackMark() { m_alloc.FreeToMarker(m_marker); }
    ScopedStackMark(const ScopedStackMark&) = delete;
    ScopedStackMark& operator=(const ScopedStackMark&) = delete;
private:
    StackAllocator& m_alloc;
    StackAllocator::Marker m_marker;
};

} // namespace Engine
} // namespace ExplorerLens
