// SecureAllocator.h — Security-Hardened Memory Allocator
// Copyright (c) 2026 ExplorerLens Project
//
// STL-compatible allocator that zero-fills memory on deallocation to prevent
// data leakage, validates allocation size limits (max 256MB), and tracks
// total allocation count for leak detection. Designed for use in a COM DLL
// running inside explorer.exe where security is critical.
//
#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>

namespace ExplorerLens {
namespace Engine {

/// Global allocation tracker for SecureAllocator instances.
/// Thread-safe via atomics.
class SecureAllocationTracker {
public:
    static SecureAllocationTracker& Instance() {
        static SecureAllocationTracker inst;
        return inst;
    }

    /// Record an allocation
    void TrackAlloc(size_t bytes) noexcept {
        m_allocationCount.fetch_add(1, std::memory_order_relaxed);
        m_totalAllocatedBytes.fetch_add(bytes, std::memory_order_relaxed);
    }

    /// Record a deallocation
    void TrackDealloc(size_t bytes) noexcept {
        m_deallocationCount.fetch_add(1, std::memory_order_relaxed);
        m_totalDeallocatedBytes.fetch_add(bytes, std::memory_order_relaxed);
    }

    /// Current number of live allocations
    int64_t LiveAllocationCount() const noexcept {
        return static_cast<int64_t>(m_allocationCount.load(std::memory_order_relaxed)) -
            static_cast<int64_t>(m_deallocationCount.load(std::memory_order_relaxed));
    }

    /// Total number of allocations ever made
    uint64_t TotalAllocations() const noexcept {
        return m_allocationCount.load(std::memory_order_relaxed);
    }

    /// Total number of deallocations ever made
    uint64_t TotalDeallocations() const noexcept {
        return m_deallocationCount.load(std::memory_order_relaxed);
    }

    /// Total bytes currently live
    int64_t LiveBytes() const noexcept {
        return static_cast<int64_t>(m_totalAllocatedBytes.load(std::memory_order_relaxed)) -
            static_cast<int64_t>(m_totalDeallocatedBytes.load(std::memory_order_relaxed));
    }

    /// Snapshot the current allocation count (for leak detection)
    uint64_t Snapshot() const noexcept {
        return m_allocationCount.load(std::memory_order_acquire);
    }

    /// Snapshot the current deallocation count
    uint64_t SnapshotDeallocations() const noexcept {
        return m_deallocationCount.load(std::memory_order_acquire);
    }

    /// Check if allocations have leaked since a snapshot
    /// Returns true if there are more net allocations now than at snapshot time
    bool HasLeakedSince(uint64_t allocSnapshot, uint64_t deallocSnapshot) const noexcept {
        uint64_t currentAllocs = m_allocationCount.load(std::memory_order_acquire);
        uint64_t currentDeallocs = m_deallocationCount.load(std::memory_order_acquire);
        int64_t netAtSnapshot = static_cast<int64_t>(allocSnapshot) - static_cast<int64_t>(deallocSnapshot);
        int64_t netNow = static_cast<int64_t>(currentAllocs) - static_cast<int64_t>(currentDeallocs);
        return netNow > netAtSnapshot;
    }

    /// Reset all counters (for testing)
    void Reset() noexcept {
        m_allocationCount.store(0, std::memory_order_relaxed);
        m_deallocationCount.store(0, std::memory_order_relaxed);
        m_totalAllocatedBytes.store(0, std::memory_order_relaxed);
        m_totalDeallocatedBytes.store(0, std::memory_order_relaxed);
    }

private:
    SecureAllocationTracker() = default;

    std::atomic<uint64_t> m_allocationCount{ 0 };
    std::atomic<uint64_t> m_deallocationCount{ 0 };
    std::atomic<uint64_t> m_totalAllocatedBytes{ 0 };
    std::atomic<uint64_t> m_totalDeallocatedBytes{ 0 };
};

/// Maximum allocation size: 256 MB
static constexpr size_t SECURE_ALLOC_MAX_BYTES = 256ULL * 1024ULL * 1024ULL;

/// STL-compatible secure allocator.
/// - Zero-fills memory on deallocation (prevents data leakage)
/// - Rejects allocations exceeding 256 MB
/// - Tracks allocations via SecureAllocationTracker
template <typename T>
class SecureAllocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    SecureAllocator() noexcept = default;

    template <typename U>
    SecureAllocator(const SecureAllocator<U>&) noexcept {}

    T* allocate(size_type n) {
        if (n == 0) return nullptr;

        // Overflow check
        if (n > (std::numeric_limits<size_type>::max)() / sizeof(T)) {
            throw std::bad_alloc();
        }

        size_type totalBytes = n * sizeof(T);

        // Size limit enforcement
        if (totalBytes > SECURE_ALLOC_MAX_BYTES) {
            throw std::bad_alloc();
        }

        void* ptr = ::operator new(totalBytes);
        if (!ptr) {
            throw std::bad_alloc();
        }

        // Zero-fill on allocation for safety
        std::memset(ptr, 0, totalBytes);

        SecureAllocationTracker::Instance().TrackAlloc(totalBytes);
        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, size_type n) noexcept {
        if (ptr && n > 0) {
            size_type totalBytes = n * sizeof(T);

            // Secure zero-fill before freeing (prevent data leakage)
            // Use volatile to prevent compiler from optimizing away the memset
            volatile unsigned char* vptr = reinterpret_cast<volatile unsigned char*>(ptr);
            for (size_type i = 0; i < totalBytes; ++i) {
                vptr[i] = 0;
            }

            SecureAllocationTracker::Instance().TrackDealloc(totalBytes);
            ::operator delete(ptr);
        }
    }

    template <typename U>
    bool operator==(const SecureAllocator<U>&) const noexcept { return true; }

    template <typename U>
    bool operator!=(const SecureAllocator<U>&) const noexcept { return false; }
};

} // namespace Engine
} // namespace ExplorerLens
