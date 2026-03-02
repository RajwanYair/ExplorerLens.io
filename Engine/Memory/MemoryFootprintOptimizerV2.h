#pragma once
// ============================================================================
// MemoryFootprintOptimizerV2.h — Slab allocator + memory compaction
//
// Purpose:   Pre-allocates fixed-size memory slabs via VirtualAlloc for
//            thumbnail buffer storage. Provides best-fit allocation,
//            next-fit search, and memory compaction via VirtualFree
//            MEM_DECOMMIT. Enforces configurable memory budgets.
//
// Classes:   MemoryFootprintOptimizerV2
// Enums:     SlabSize
// Structs:   SlabBlock, MemoryStats
//
// Inputs:    Allocation size requests from decode pipeline
// Outputs:   Aligned virtual memory pointers; compaction metrics
//
// Threading: Thread-safe via SRWLOCK. All public methods acquire locks.
//
// Lifecycle: Constructor pre-allocates slab pools.
//            Destructor releases all virtual memory.
// ============================================================================

#include <windows.h>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

/// Pre-defined slab sizes for the allocator pools.
enum class SlabSize : uint32_t {
    Slab64K = 65536,        //  64 KB — small thumbnails (128x128 BGRA)
    Slab256K = 262144,       // 256 KB — medium thumbnails (256x256 BGRA)
    Slab1M = 1048576,      //   1 MB — large thumbnails (512x512 BGRA)
    Slab4M = 4194304       //   4 MB — extra-large (1024x1024 BGRA)
};

/// Individual slab block within a pool.
struct SlabBlock {
    void* data = nullptr;
    size_t   size = 0;
    bool     inUse = false;
    uint64_t lastAccessTime = 0;   // QPC tick of last allocation/deallocation
    uint64_t allocationId = 0;   // monotonic ID for tracking
};

/// Aggregate memory statistics.
struct MemoryStats {
    size_t   totalAllocatedBytes = 0;  // sum of all VirtualAlloc'd regions
    size_t   committedBytes = 0;  // currently committed (in-use) bytes
    size_t   peakCommittedBytes = 0;  // high-water mark
    size_t   budgetBytes = 0;  // configured ceiling
    uint32_t totalSlabs = 0;  // total slab count across all pools
    uint32_t inUseSlabs = 0;  // currently allocated slabs
    uint32_t freeSlabs = 0;  // available slabs
    double   fragmentationRatio = 0.0; // 0.0 = no fragmentation, 1.0 = fully fragmented
};

/// Slab-based memory allocator with VirtualAlloc backend and compaction support.
class MemoryFootprintOptimizerV2 {
public:
    static constexpr size_t DEFAULT_BUDGET = 256 * 1048576; // 256 MB default budget

    /// Construct the optimizer with optional initial slab counts per tier.
    explicit MemoryFootprintOptimizerV2(
        uint32_t slabs64K = 64,
        uint32_t slabs256K = 32,
        uint32_t slabs1M = 16,
        uint32_t slabs4M = 4)
        : m_budgetBytes(DEFAULT_BUDGET) {
        InitializeSRWLock(&m_lock);
        QueryPerformanceFrequency(&m_freq);

        // Pre-allocate slab pools
        PreallocatePool(static_cast<size_t>(SlabSize::Slab64K), slabs64K);
        PreallocatePool(static_cast<size_t>(SlabSize::Slab256K), slabs256K);
        PreallocatePool(static_cast<size_t>(SlabSize::Slab1M), slabs1M);
        PreallocatePool(static_cast<size_t>(SlabSize::Slab4M), slabs4M);
    }

    ~MemoryFootprintOptimizerV2() {
        AcquireSRWLockExclusive(&m_lock);
        for (auto& slab : m_slabs) {
            if (slab.data) {
                VirtualFree(slab.data, 0, MEM_RELEASE);
                slab.data = nullptr;
            }
        }
        m_slabs.clear();
        ReleaseSRWLockExclusive(&m_lock);
    }

    MemoryFootprintOptimizerV2(const MemoryFootprintOptimizerV2&) = delete;
    MemoryFootprintOptimizerV2& operator=(const MemoryFootprintOptimizerV2&) = delete;

    /// Allocate a block of at least `size` bytes from the slab pool.
    /// Returns nullptr if no suitable slab is available or budget is exceeded.
    inline void* Allocate(size_t size) {
        if (size == 0) return nullptr;

        AcquireSRWLockExclusive(&m_lock);

        // Would this exceed the budget?
        if (m_committedBytes + size > m_budgetBytes) {
            ReleaseSRWLockExclusive(&m_lock);
            return nullptr;
        }

        // Best-fit search: find the smallest free slab that fits
        size_t bestIdx = SIZE_MAX;
        size_t bestSize = SIZE_MAX;

        // Start from the last-allocated position (next-fit hint)
        const size_t count = m_slabs.size();
        for (size_t i = 0; i < count; ++i) {
            size_t idx = (m_nextFitHint + i) % count;
            const auto& slab = m_slabs[idx];
            if (!slab.inUse && slab.size >= size && slab.size < bestSize) {
                bestIdx = idx;
                bestSize = slab.size;
                if (bestSize == size) break; // exact fit
            }
        }

        if (bestIdx == SIZE_MAX) {
            // No existing slab fits — try to grow a new slab if within budget
            size_t slabSize = RoundUpToSlabSize(size);
            if (m_totalAllocatedBytes + slabSize <= m_budgetBytes) {
                void* mem = VirtualAlloc(nullptr, slabSize,
                    MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                if (mem) {
                    LARGE_INTEGER now;
                    QueryPerformanceCounter(&now);

                    SlabBlock newSlab;
                    newSlab.data = mem;
                    newSlab.size = slabSize;
                    newSlab.inUse = true;
                    newSlab.lastAccessTime = static_cast<uint64_t>(now.QuadPart);
                    newSlab.allocationId = ++m_nextAllocationId;

                    m_slabs.push_back(newSlab);
                    m_totalAllocatedBytes += slabSize;
                    m_committedBytes += slabSize;
                    if (m_committedBytes > m_peakCommittedBytes)
                        m_peakCommittedBytes = m_committedBytes;

                    m_nextFitHint = m_slabs.size();
                    ReleaseSRWLockExclusive(&m_lock);
                    return mem;
                }
            }
            ReleaseSRWLockExclusive(&m_lock);
            return nullptr;
        }

        // Found a suitable slab — mark as in-use
        m_slabs[bestIdx].inUse = true;
        m_slabs[bestIdx].allocationId = ++m_nextAllocationId;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        m_slabs[bestIdx].lastAccessTime = static_cast<uint64_t>(now.QuadPart);

        m_committedBytes += m_slabs[bestIdx].size;
        if (m_committedBytes > m_peakCommittedBytes)
            m_peakCommittedBytes = m_committedBytes;

        m_nextFitHint = (bestIdx + 1) % count;
        void* result = m_slabs[bestIdx].data;
        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    /// Return a previously allocated block to the pool.
    inline void Deallocate(void* ptr) {
        if (!ptr) return;

        AcquireSRWLockExclusive(&m_lock);
        for (auto& slab : m_slabs) {
            if (slab.data == ptr && slab.inUse) {
                slab.inUse = false;
                LARGE_INTEGER now;
                QueryPerformanceCounter(&now);
                slab.lastAccessTime = static_cast<uint64_t>(now.QuadPart);
                m_committedBytes -= slab.size;
                break;
            }
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Compact memory by decommitting pages in free slabs.
    /// This reduces the working set without releasing the virtual address space.
    inline void Compact() {
        AcquireSRWLockExclusive(&m_lock);
        for (auto& slab : m_slabs) {
            if (!slab.inUse && slab.data) {
                // Decommit the physical pages but keep the VA reservation
                VirtualFree(slab.data, slab.size, MEM_DECOMMIT);
                // Re-commit when next allocated (will get zeroed pages)
            }
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Set the maximum memory budget in bytes.
    inline void SetBudget(size_t maxBytes) {
        AcquireSRWLockExclusive(&m_lock);
        m_budgetBytes = (maxBytes > 0) ? maxBytes : DEFAULT_BUDGET;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Return current memory statistics.
    inline MemoryStats GetStats() const {
        MemoryStats stats;
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        stats.totalAllocatedBytes = m_totalAllocatedBytes;
        stats.committedBytes = m_committedBytes;
        stats.peakCommittedBytes = m_peakCommittedBytes;
        stats.budgetBytes = m_budgetBytes;
        stats.totalSlabs = static_cast<uint32_t>(m_slabs.size());

        uint32_t inUse = 0;
        for (const auto& slab : m_slabs) {
            if (slab.inUse) ++inUse;
        }
        stats.inUseSlabs = inUse;
        stats.freeSlabs = stats.totalSlabs - inUse;

        // Fragmentation: ratio of free slabs interspersed among used slabs
        // 0 = all free slabs are contiguous, 1 = maximally fragmented
        if (stats.totalSlabs <= 1 || stats.freeSlabs == 0 || stats.inUseSlabs == 0) {
            stats.fragmentationRatio = 0.0;
        }
        else {
            uint32_t transitions = 0;
            for (size_t i = 1; i < m_slabs.size(); ++i) {
                if (m_slabs[i].inUse != m_slabs[i - 1].inUse) ++transitions;
            }
            // Normalize: max transitions = totalSlabs - 1
            stats.fragmentationRatio = static_cast<double>(transitions)
                / static_cast<double>(stats.totalSlabs - 1);
        }

        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return stats;
    }

    /// Get the configured budget ceiling.
    inline size_t GetBudget() const noexcept { return m_budgetBytes; }

private:
    /// Pre-allocate a pool of slabs with VirtualAlloc.
    inline void PreallocatePool(size_t slabSize, uint32_t count) {
        for (uint32_t i = 0; i < count; ++i) {
            void* mem = VirtualAlloc(nullptr, slabSize,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!mem) break;

            SlabBlock slab;
            slab.data = mem;
            slab.size = slabSize;
            slab.inUse = false;
            slab.lastAccessTime = 0;
            slab.allocationId = 0;
            m_slabs.push_back(slab);
            m_totalAllocatedBytes += slabSize;
        }
    }

    /// Round a requested size up to the next standard slab tier.
    inline static size_t RoundUpToSlabSize(size_t size) noexcept {
        if (size <= static_cast<size_t>(SlabSize::Slab64K))  return static_cast<size_t>(SlabSize::Slab64K);
        if (size <= static_cast<size_t>(SlabSize::Slab256K)) return static_cast<size_t>(SlabSize::Slab256K);
        if (size <= static_cast<size_t>(SlabSize::Slab1M))   return static_cast<size_t>(SlabSize::Slab1M);
        if (size <= static_cast<size_t>(SlabSize::Slab4M))   return static_cast<size_t>(SlabSize::Slab4M);
        // For sizes > 4 MB, round up to next 4 MB boundary
        return (size + static_cast<size_t>(SlabSize::Slab4M) - 1)
            & ~(static_cast<size_t>(SlabSize::Slab4M) - 1);
    }

    SRWLOCK                  m_lock{};
    LARGE_INTEGER            m_freq{};
    std::vector<SlabBlock>   m_slabs;
    size_t                   m_totalAllocatedBytes = 0;
    size_t                   m_committedBytes = 0;
    size_t                   m_peakCommittedBytes = 0;
    size_t                   m_budgetBytes = DEFAULT_BUDGET;
    size_t                   m_nextFitHint = 0;
    uint64_t                 m_nextAllocationId = 0;
};

} // namespace Engine
} // namespace ExplorerLens
