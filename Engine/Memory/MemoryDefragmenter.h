#pragma once
// ============================================================================
// MemoryDefragmenter.h
//
// Purpose:
//   Defragments thumbnail buffer memory by compacting live allocations toward
//   lower addresses and coalescing adjacent free blocks.  This reduces
//   fragmentation that accumulates when heterogeneous image sizes are decoded
//   and released in arbitrary order.
//
// Classes:
//   MemoryDefragmenter — manages a list of memory regions, compacts live
//                         blocks, coalesces free blocks, and optionally runs
//                         background auto-defrag.
//
// Key Types:
//   MemoryRegion   — descriptor for one managed block (base, size, in-use)
//   DefragResult   — statistics from a single defrag pass
//   DefragStats    — cumulative lifetime statistics
//
// Inputs:
//   RegisterRegion / UnregisterRegion — add/remove regions for management
//   MarkFree                          — mark a region as reclaimable
//   SetRelocationCallback             — pointer-fixup callback
//
// Outputs:
//   Defragment()           — compacts memory, returns DefragResult
//   GetFragmentationRatio  — 0.0 (perfect) to 1.0 (fully fragmented)
//   GetStats()             — cumulative counters
//
// Thread Safety:
//   All public methods are serialized with a Win32 SRWLOCK (exclusive).
//
// Dependencies:
//   Windows API + C++ standard library only (header-only, no external libs).
// ============================================================================

#include <windows.h>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

// ── Region descriptor ────────────────────────────────────────────────────────

struct MemoryRegion {
    void* base = nullptr;
    size_t   size = 0;
    bool     inUse = true;
    uint64_t tag = 0;       // caller-defined tag for identification
};

// ── Defrag pass result ───────────────────────────────────────────────────────

struct DefragResult {
    uint64_t bytesMoved = 0;
    uint32_t regionsCompacted = 0;
    uint32_t fragmentsBefore = 0;
    uint32_t fragmentsAfter = 0;
    uint64_t durationUs = 0;
};

// ── Cumulative stats ─────────────────────────────────────────────────────────

struct DefragStats {
    uint32_t totalRegions = 0;
    uint32_t freeRegions = 0;
    double   fragmentationRatio = 0.0;
    uint32_t defragRuns = 0;
    uint64_t totalBytesMoved = 0;
};

// ── Main class ───────────────────────────────────────────────────────────────

class MemoryDefragmenter {
public:
    MemoryDefragmenter() noexcept {
        InitializeSRWLock(&m_lock);
    }

    ~MemoryDefragmenter() {
        DisableAutoDefrag();
    }

    MemoryDefragmenter(const MemoryDefragmenter&) = delete;
    MemoryDefragmenter& operator=(const MemoryDefragmenter&) = delete;

    // ── Region management ────────────────────────────────────────

    void RegisterRegion(void* base, size_t size, uint64_t tag = 0) {
        if (!base || size == 0) return;
        AcquireSRWLockExclusive(&m_lock);
        MemoryRegion r;
        r.base = base;
        r.size = size;
        r.inUse = true;
        r.tag = tag;
        m_regions.push_back(r);
        ReleaseSRWLockExclusive(&m_lock);
    }

    void UnregisterRegion(void* base) {
        if (!base) return;
        AcquireSRWLockExclusive(&m_lock);
        m_regions.erase(
            std::remove_if(m_regions.begin(), m_regions.end(),
                [base](const MemoryRegion& r) { return r.base == base; }),
            m_regions.end());
        ReleaseSRWLockExclusive(&m_lock);
    }

    void MarkFree(void* base) {
        if (!base) return;
        AcquireSRWLockExclusive(&m_lock);
        for (auto& r : m_regions) {
            if (r.base == base) {
                r.inUse = false;
                break;
            }
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Relocation callback ──────────────────────────────────────

    void SetRelocationCallback(
        std::function<void(void* oldBase, void* newBase, size_t size)> fn) {
        AcquireSRWLockExclusive(&m_lock);
        m_relocationCb = std::move(fn);
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Core defragmentation ─────────────────────────────────────

    DefragResult Defragment() {
        AcquireSRWLockExclusive(&m_lock);
        DefragResult result{};

        auto startTime = std::chrono::high_resolution_clock::now();

        // Sort regions by base address (ascending)
        std::sort(m_regions.begin(), m_regions.end(),
            [](const MemoryRegion& a, const MemoryRegion& b) {
                return reinterpret_cast<uintptr_t>(a.base) <
                    reinterpret_cast<uintptr_t>(b.base);
            });

        // Count free fragments before compaction
        result.fragmentsBefore = CountFreeFragmentsLocked();

        // Phase 1: Compact — slide live regions down into free gaps
        // We identify contiguous runs and move live blocks to fill free gaps.
        size_t writeIdx = 0;
        for (size_t readIdx = 0; readIdx < m_regions.size(); ++readIdx) {
            if (!m_regions[readIdx].inUse) continue;

            // Find the lowest free region that is before this live region
            size_t freeIdx = FindFirstFreeBefore(readIdx);
            if (freeIdx < m_regions.size()) {
                // Move data from live region into the free region's space
                MemoryRegion& freeReg = m_regions[freeIdx];
                MemoryRegion& liveReg = m_regions[readIdx];

                if (freeReg.size >= liveReg.size) {
                    void* oldBase = liveReg.base;
                    void* newBase = freeReg.base;
                    size_t moveSize = liveReg.size;

                    // Use memmove for safe overlapping copies
                    std::memmove(newBase, oldBase, moveSize);
                    result.bytesMoved += moveSize;
                    result.regionsCompacted++;

                    // Notify callback
                    if (m_relocationCb) {
                        m_relocationCb(oldBase, newBase, moveSize);
                    }

                    // Update region descriptors
                    size_t remainingFree = freeReg.size - liveReg.size;
                    void* remainingBase = static_cast<uint8_t*>(freeReg.base) + liveReg.size;

                    // Live region now occupies the free region's base
                    liveReg.base = newBase;

                    // Shrink or remove the free region
                    if (remainingFree > 0) {
                        freeReg.base = remainingBase;
                        freeReg.size = remainingFree;
                    }
                    else {
                        freeReg.size = 0; // mark for removal
                    }

                    // The old location becomes free
                    // We represent this by marking the original region area
                    // (already handled by the free region adjustment above in
                    //  a contiguous arena model)
                }
            }
        }

        // Phase 2: Coalesce adjacent free blocks
        CoalesceFreeLocked();

        // Remove zero-size regions
        m_regions.erase(
            std::remove_if(m_regions.begin(), m_regions.end(),
                [](const MemoryRegion& r) { return r.size == 0; }),
            m_regions.end());

        result.fragmentsAfter = CountFreeFragmentsLocked();

        auto endTime = std::chrono::high_resolution_clock::now();
        result.durationUs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                endTime - startTime).count());

        m_defragRuns++;
        m_totalBytesMoved += result.bytesMoved;

        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    // ── Fragmentation ratio ──────────────────────────────────────

    double GetFragmentationRatio() {
        AcquireSRWLockExclusive(&m_lock);
        double ratio = ComputeFragmentationLocked();
        ReleaseSRWLockExclusive(&m_lock);
        return ratio;
    }

    // ── Auto-defrag ──────────────────────────────────────────────

    void EnableAutoDefrag(double fragmentationThreshold = 0.3,
        uint32_t checkIntervalMs = 10000) {
        DisableAutoDefrag();

        m_autoDefragEnabled.store(true, std::memory_order_release);
        m_autoDefragThreshold = fragmentationThreshold;
        m_autoDefragIntervalMs = checkIntervalMs;

        m_autoDefragThread = std::thread([this]() {
            while (m_autoDefragEnabled.load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(m_autoDefragIntervalMs));

                if (!m_autoDefragEnabled.load(std::memory_order_acquire))
                    break;

                double frag = GetFragmentationRatio();
                if (frag > m_autoDefragThreshold) {
                    Defragment();
                }
            }
            });
    }

    void DisableAutoDefrag() {
        m_autoDefragEnabled.store(false, std::memory_order_release);
        if (m_autoDefragThread.joinable()) {
            m_autoDefragThread.join();
        }
    }

    // ── Statistics ───────────────────────────────────────────────

    DefragStats GetStats() {
        AcquireSRWLockExclusive(&m_lock);
        DefragStats stats{};
        stats.totalRegions = static_cast<uint32_t>(m_regions.size());
        stats.freeRegions = 0;
        for (const auto& r : m_regions) {
            if (!r.inUse) stats.freeRegions++;
        }
        stats.fragmentationRatio = ComputeFragmentationLocked();
        stats.defragRuns = m_defragRuns;
        stats.totalBytesMoved = m_totalBytesMoved;
        ReleaseSRWLockExclusive(&m_lock);
        return stats;
    }

    // ── Accessors ────────────────────────────────────────────────

    uint32_t GetRegionCount() {
        AcquireSRWLockExclusive(&m_lock);
        uint32_t count = static_cast<uint32_t>(m_regions.size());
        ReleaseSRWLockExclusive(&m_lock);
        return count;
    }

    uint32_t GetFreeRegionCount() {
        AcquireSRWLockExclusive(&m_lock);
        uint32_t count = 0;
        for (const auto& r : m_regions) {
            if (!r.inUse) count++;
        }
        ReleaseSRWLockExclusive(&m_lock);
        return count;
    }

    size_t GetTotalFreeBytes() {
        AcquireSRWLockExclusive(&m_lock);
        size_t total = 0;
        for (const auto& r : m_regions) {
            if (!r.inUse) total += r.size;
        }
        ReleaseSRWLockExclusive(&m_lock);
        return total;
    }

    size_t GetTotalUsedBytes() {
        AcquireSRWLockExclusive(&m_lock);
        size_t total = 0;
        for (const auto& r : m_regions) {
            if (r.inUse) total += r.size;
        }
        ReleaseSRWLockExclusive(&m_lock);
        return total;
    }

private:
    // ── Internal helpers (caller must hold lock) ─────────────────

    uint32_t CountFreeFragmentsLocked() const {
        uint32_t count = 0;
        for (const auto& r : m_regions) {
            if (!r.inUse && r.size > 0) count++;
        }
        return count;
    }

    size_t FindFirstFreeBefore(size_t liveIdx) const {
        // Find the first free region with base address < live region's base
        uintptr_t liveAddr = reinterpret_cast<uintptr_t>(m_regions[liveIdx].base);
        size_t bestIdx = m_regions.size(); // sentinel = not found
        uintptr_t bestAddr = UINTPTR_MAX;

        for (size_t i = 0; i < m_regions.size(); ++i) {
            if (i == liveIdx) continue;
            if (m_regions[i].inUse) continue;
            if (m_regions[i].size == 0) continue;

            uintptr_t freeAddr = reinterpret_cast<uintptr_t>(m_regions[i].base);
            if (freeAddr < liveAddr && freeAddr < bestAddr) {
                bestAddr = freeAddr;
                bestIdx = i;
            }
        }
        return bestIdx;
    }

    void CoalesceFreeLocked() {
        // Sort by base address first (should already be sorted, but ensure)
        std::sort(m_regions.begin(), m_regions.end(),
            [](const MemoryRegion& a, const MemoryRegion& b) {
                return reinterpret_cast<uintptr_t>(a.base) <
                    reinterpret_cast<uintptr_t>(b.base);
            });

        // Merge adjacent free blocks
        for (size_t i = 0; i + 1 < m_regions.size(); ) {
            if (!m_regions[i].inUse && !m_regions[i + 1].inUse) {
                uintptr_t endOfI = reinterpret_cast<uintptr_t>(m_regions[i].base)
                    + m_regions[i].size;
                uintptr_t startOfNext = reinterpret_cast<uintptr_t>(
                    m_regions[i + 1].base);

                if (endOfI == startOfNext) {
                    // Merge: extend region i, remove region i+1
                    m_regions[i].size += m_regions[i + 1].size;
                    m_regions.erase(m_regions.begin() +
                        static_cast<ptrdiff_t>(i + 1));
                    continue; // re-check at same index
                }
            }
            ++i;
        }
    }

    double ComputeFragmentationLocked() const {
        size_t totalFree = 0;
        size_t largestFree = 0;

        for (const auto& r : m_regions) {
            if (!r.inUse) {
                totalFree += r.size;
                largestFree = (std::max)(largestFree, r.size);
            }
        }

        if (totalFree == 0) return 0.0;
        return 1.0 - (static_cast<double>(largestFree) /
            static_cast<double>(totalFree));
    }

    // ── Data members ─────────────────────────────────────────────

    SRWLOCK                m_lock{};
    std::vector<MemoryRegion> m_regions;

    std::function<void(void*, void*, size_t)> m_relocationCb;

    uint32_t m_defragRuns = 0;
    uint64_t m_totalBytesMoved = 0;

    // Auto-defrag
    std::atomic<bool> m_autoDefragEnabled{ false };
    double            m_autoDefragThreshold = 0.3;
    uint32_t          m_autoDefragIntervalMs = 10000;
    std::thread       m_autoDefragThread;
};

} // namespace Engine
} // namespace ExplorerLens
