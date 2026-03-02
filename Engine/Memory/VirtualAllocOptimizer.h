#pragma once
// ============================================================================
// VirtualAllocOptimizer.h
//
// Purpose:
//   Smart VirtualAlloc wrapper providing reserve/commit-on-demand memory
//   management with guard-page support.  Thumbnails often need large
//   contiguous buffers whose actual usage varies at runtime; this class
//   reserves large virtual ranges but only commits pages when accessed,
//   keeping the working set small.
//
// Classes:
//   VirtualAllocOptimizer — manages virtual regions (reserve, commit,
//                           decommit, release) with optional guard pages
//                           and vectored exception handler auto-commit.
//
// Key Types:
//   VirtualRegion — describes one reserved/committed range
//   CommitStats   — aggregate statistics for all managed regions
//
// Inputs:
//   Reserve / Commit / Decommit / Release        — lifecycle management
//   SetGuardPage                                  — detect-on-access
//   ReserveWithAutoCommit                         — VEH auto-commit
//
// Outputs:
//   GetStats()          — totals across all tracked regions
//   GetActiveRegions()  — snapshot of current regions
//
// Thread Safety:
//   All public methods are serialized with a Win32 SRWLOCK (exclusive).
//
// Dependencies:
//   Windows API + C++ standard library only (header-only, no external libs).
// ============================================================================

#include <windows.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ── Region descriptor ────────────────────────────────────────────────────────

struct VirtualRegion {
    void* base = nullptr;
    size_t  reserved = 0;
    size_t  committed = 0;
    DWORD   protect = PAGE_READWRITE;
    bool    autoCommit = false;  // true when VEH auto-commit is active
};

// ── Aggregate statistics ─────────────────────────────────────────────────────

struct CommitStats {
    uint32_t regionsTracked = 0;
    size_t   totalReserved = 0;
    size_t   totalCommitted = 0;
    double   commitRatio = 0.0;   // committed / reserved
    uint64_t guardPageFaultsHandled = 0;
};

// ── Main class ───────────────────────────────────────────────────────────────

class VirtualAllocOptimizer {
public:
    VirtualAllocOptimizer() noexcept {
        InitializeSRWLock(&m_lock);

        // Query system page info once
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        m_pageSize = si.dwPageSize;
        m_allocationGranularity = si.dwAllocationGranularity;

        // Install vectored exception handler for auto-commit guard pages
        m_vehHandle = AddVectoredExceptionHandler(
            1 /*first handler*/, &VirtualAllocOptimizer::VehHandler);
    }

    ~VirtualAllocOptimizer() {
        // Release all regions
        AcquireSRWLockExclusive(&m_lock);
        for (auto& region : m_regions) {
            if (region.base) {
                VirtualFree(region.base, 0, MEM_RELEASE);
                region.base = nullptr;
            }
        }
        m_regions.clear();
        ReleaseSRWLockExclusive(&m_lock);

        // Remove VEH
        if (m_vehHandle) {
            RemoveVectoredExceptionHandler(m_vehHandle);
            m_vehHandle = nullptr;
        }
    }

    VirtualAllocOptimizer(const VirtualAllocOptimizer&) = delete;
    VirtualAllocOptimizer& operator=(const VirtualAllocOptimizer&) = delete;

    // ── Reserve ──────────────────────────────────────────────────

    VirtualRegion Reserve(size_t size, DWORD protect = PAGE_READWRITE) {
        VirtualRegion region{};
        size_t aligned = AlignUp(size, m_allocationGranularity);

        void* base = VirtualAlloc(nullptr, aligned, MEM_RESERVE, protect);
        if (!base) return region;

        region.base = base;
        region.reserved = aligned;
        region.committed = 0;
        region.protect = protect;
        region.autoCommit = false;

        AcquireSRWLockExclusive(&m_lock);
        m_regions.push_back(region);
        ReleaseSRWLockExclusive(&m_lock);

        return region;
    }

    // ── Commit a sub-range ───────────────────────────────────────

    bool Commit(VirtualRegion& region, size_t offset, size_t size) {
        if (!region.base || size == 0) return false;

        size_t alignedOffset = AlignDown(offset, m_pageSize);
        size_t alignedEnd = AlignUp(offset + size, m_pageSize);
        size_t commitSize = alignedEnd - alignedOffset;

        // Clamp to region bounds
        if (alignedOffset + commitSize > region.reserved) {
            commitSize = region.reserved - alignedOffset;
        }

        void* addr = static_cast<uint8_t*>(region.base) + alignedOffset;
        void* result = VirtualAlloc(addr, commitSize,
            MEM_COMMIT, region.protect);
        if (!result) return false;

        // Track committed range (approximate — overlaps are idempotent)
        AcquireSRWLockExclusive(&m_lock);
        region.committed += commitSize;
        // Update our stored copy
        for (auto& r : m_regions) {
            if (r.base == region.base) {
                r.committed = region.committed;
                break;
            }
        }
        ReleaseSRWLockExclusive(&m_lock);

        return true;
    }

    // ── Decommit a sub-range ─────────────────────────────────────

    bool Decommit(VirtualRegion& region, size_t offset, size_t size) {
        if (!region.base || size == 0) return false;

        size_t alignedOffset = AlignDown(offset, m_pageSize);
        size_t alignedEnd = AlignUp(offset + size, m_pageSize);
        size_t decommitSize = alignedEnd - alignedOffset;

        if (alignedOffset + decommitSize > region.reserved) {
            decommitSize = region.reserved - alignedOffset;
        }

        void* addr = static_cast<uint8_t*>(region.base) + alignedOffset;
        BOOL ok = VirtualFree(addr, decommitSize, MEM_DECOMMIT);
        if (!ok) return false;

        AcquireSRWLockExclusive(&m_lock);
        if (region.committed >= decommitSize) {
            region.committed -= decommitSize;
        }
        else {
            region.committed = 0;
        }
        for (auto& r : m_regions) {
            if (r.base == region.base) {
                r.committed = region.committed;
                break;
            }
        }
        ReleaseSRWLockExclusive(&m_lock);

        return true;
    }

    // ── Release ──────────────────────────────────────────────────

    void Release(VirtualRegion& region) {
        if (!region.base) return;

        AcquireSRWLockExclusive(&m_lock);
        m_regions.erase(
            std::remove_if(m_regions.begin(), m_regions.end(),
                [&](const VirtualRegion& r) { return r.base == region.base; }),
            m_regions.end());
        ReleaseSRWLockExclusive(&m_lock);

        VirtualFree(region.base, 0, MEM_RELEASE);
        region.base = nullptr;
        region.reserved = 0;
        region.committed = 0;
    }

    // ── Guard page ───────────────────────────────────────────────

    bool SetGuardPage(VirtualRegion& region, size_t offset) {
        if (!region.base) return false;

        size_t alignedOffset = AlignDown(offset, m_pageSize);
        if (alignedOffset >= region.reserved) return false;

        void* addr = static_cast<uint8_t*>(region.base) + alignedOffset;

        // Commit the page first if not already committed
        VirtualAlloc(addr, m_pageSize, MEM_COMMIT, region.protect);

        DWORD oldProtect = 0;
        BOOL ok = VirtualProtect(addr, m_pageSize,
            region.protect | PAGE_GUARD, &oldProtect);
        return ok != FALSE;
    }

    // ── Reserve with auto-commit on guard-page fault ─────────────

    VirtualRegion ReserveWithAutoCommit(size_t maxSize,
        size_t initialCommit) {
        VirtualRegion region = Reserve(maxSize);
        if (!region.base) return region;

        // Commit initial range
        if (initialCommit > 0) {
            size_t toCommit = (std::min)(initialCommit, region.reserved);
            Commit(region, 0, toCommit);
        }

        // Place a guard page right after the committed range
        size_t guardOffset = AlignUp(initialCommit, m_pageSize);
        if (guardOffset < region.reserved) {
            // Commit one guard page
            void* guardAddr = static_cast<uint8_t*>(region.base) + guardOffset;
            VirtualAlloc(guardAddr, m_pageSize, MEM_COMMIT, PAGE_READWRITE);

            // Apply PAGE_GUARD
            DWORD oldProtect = 0;
            VirtualProtect(guardAddr, m_pageSize,
                PAGE_READWRITE | PAGE_GUARD, &oldProtect);
        }

        region.autoCommit = true;

        // Update our stored copy
        AcquireSRWLockExclusive(&m_lock);
        for (auto& r : m_regions) {
            if (r.base == region.base) {
                r.autoCommit = true;
                break;
            }
        }
        ReleaseSRWLockExclusive(&m_lock);

        return region;
    }

    // ── Statistics ───────────────────────────────────────────────

    CommitStats GetStats() {
        AcquireSRWLockExclusive(&m_lock);
        CommitStats stats{};
        stats.regionsTracked = static_cast<uint32_t>(m_regions.size());
        for (const auto& r : m_regions) {
            stats.totalReserved += r.reserved;
            stats.totalCommitted += r.committed;
        }
        stats.commitRatio = stats.totalReserved > 0
            ? static_cast<double>(stats.totalCommitted) /
            static_cast<double>(stats.totalReserved)
            : 0.0;
        stats.guardPageFaultsHandled =
            s_guardFaultsHandled.load(std::memory_order_relaxed);
        ReleaseSRWLockExclusive(&m_lock);
        return stats;
    }

    std::vector<VirtualRegion> GetActiveRegions() {
        AcquireSRWLockExclusive(&m_lock);
        auto copy = m_regions;
        ReleaseSRWLockExclusive(&m_lock);
        return copy;
    }

    // ── System page info ─────────────────────────────────────────

    size_t GetPageSize()             const noexcept { return m_pageSize; }
    size_t GetAllocationGranularity() const noexcept { return m_allocationGranularity; }

private:
    // ── Alignment helpers ────────────────────────────────────────

    static size_t AlignUp(size_t value, size_t alignment) noexcept {
        if (alignment == 0) return value;
        return (value + alignment - 1) & ~(alignment - 1);
    }

    static size_t AlignDown(size_t value, size_t alignment) noexcept {
        if (alignment == 0) return value;
        return value & ~(alignment - 1);
    }

    // ── Vectored exception handler (auto-commit) ─────────────────

    static LONG CALLBACK VehHandler(PEXCEPTION_POINTERS pExInfo) {
        if (pExInfo->ExceptionRecord->ExceptionCode != STATUS_GUARD_PAGE_VIOLATION) {
            return EXCEPTION_CONTINUE_SEARCH;
        }

        // A guard page was hit — the system automatically removes the
        // PAGE_GUARD attribute.  We commit the next page and set a new
        // guard page one page ahead to allow further growth.
        void* faultAddr =
            reinterpret_cast<void*>(pExInfo->ExceptionRecord->ExceptionInformation[1]);

        // Round to page boundary
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        uintptr_t pageBase = reinterpret_cast<uintptr_t>(faultAddr)
            & ~(static_cast<uintptr_t>(si.dwPageSize) - 1);
        uintptr_t nextPage = pageBase + si.dwPageSize;

        // Commit next page and set its guard flag
        void* committed = VirtualAlloc(reinterpret_cast<void*>(nextPage),
            si.dwPageSize, MEM_COMMIT,
            PAGE_READWRITE | PAGE_GUARD);
        if (committed) {
            s_guardFaultsHandled.fetch_add(1, std::memory_order_relaxed);
        }

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    // ── Data members ─────────────────────────────────────────────

    SRWLOCK                     m_lock{};
    std::vector<VirtualRegion>  m_regions;
    PVOID                       m_vehHandle = nullptr;

    size_t m_pageSize = 4096;
    size_t m_allocationGranularity = 65536;

    inline static std::atomic<uint64_t> s_guardFaultsHandled{ 0 };
};

} // namespace Engine
} // namespace ExplorerLens
