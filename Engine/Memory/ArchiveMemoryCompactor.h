#pragma once
// ============================================================================
// ArchiveMemoryCompactor.h
//
// Purpose:
//   Specialized memory compactor for archive extraction buffers.  When an
//   archive is opened, each entry is extracted into a separate buffer.  Over
//   time, as individual entries are freed, the arena becomes fragmented.
//   This class manages a large VirtualAlloc arena, allocates sub-blocks for
//   extracted entries, and compacts live (non-pinned) buffers to reclaim
//   contiguous free space.
//
// Classes:
//   ArchiveMemoryCompactor — arena-based allocator with compaction support.
//
// Key Types:
//   ExtractedBuffer — metadata for one extraction buffer (archive ID, entry
//                     index, data pointer, size, pin state)
//   CompactResult   — outcome of a compaction pass
//   CompactorStats  — lifetime statistics
//
// Inputs:
//   AllocateBuffer / FreeBuffer — buffer lifecycle
//   PinBuffer / UnpinBuffer     — prevent/allow compaction movement
//
// Outputs:
//   Compact()     — move live buffers to fill gaps
//   GetStats()    — arena utilisation and fragmentation metrics
//
// Thread Safety:
//   All public methods are serialized with a Win32 SRWLOCK (exclusive).
//
// Arena Growth:
//   The arena is backed by VirtualAlloc.  It is reserved up to a configurable
//   limit and committed in 16 MB chunks on demand.
//
// Dependencies:
//   Windows API + C++ standard library only (header-only, no external libs).
// ============================================================================

#include <windows.h>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

// ── Buffer descriptor ────────────────────────────────────────────────────────

struct ExtractedBuffer {
    uint64_t archiveId = 0;
    uint32_t entryIndex = 0;
    void* data = nullptr;
    size_t   size = 0;
    bool     pinned = false;
    bool     alive = true;   // false after FreeBuffer
};

// ── Compact result ───────────────────────────────────────────────────────────

struct CompactResult {
    uint32_t buffersMoved = 0;
    uint64_t bytesMoved = 0;
    uint32_t gapsFilled = 0;
    uint32_t fragmentsBefore = 0;
    uint32_t fragmentsAfter = 0;
    uint64_t durationUs = 0;
};

// ── Statistics ───────────────────────────────────────────────────────────────

struct CompactorStats {
    size_t   arenaReserved = 0;
    size_t   arenaCommitted = 0;
    size_t   usedBytes = 0;
    size_t   freeBytes = 0;
    uint32_t bufferCount = 0;
    double   fragmentationRatio = 0.0;
    uint32_t compactionsPerformed = 0;
};

// ── Constants ────────────────────────────────────────────────────────────────

static constexpr size_t kArenaChunkSize = 16ULL * 1024 * 1024; // 16 MB
static constexpr size_t kDefaultArenaLimit = 512ULL * 1024 * 1024; // 512 MB
static constexpr double kAutoCompactThreshold = 0.40; // 40% fragmentation

// ── Main class ───────────────────────────────────────────────────────────────

class ArchiveMemoryCompactor {
public:
    explicit ArchiveMemoryCompactor(size_t arenaLimit = kDefaultArenaLimit) noexcept
        : m_arenaLimit(arenaLimit) {
        InitializeSRWLock(&m_lock);

        // Query page size
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        m_pageSize = si.dwPageSize;

        // Reserve the full arena (no physical commit yet)
        m_arenaBase = static_cast<uint8_t*>(
            VirtualAlloc(nullptr, m_arenaLimit, MEM_RESERVE, PAGE_READWRITE));
        m_arenaReserved = m_arenaBase ? m_arenaLimit : 0;
    }

    ~ArchiveMemoryCompactor() {
        AcquireSRWLockExclusive(&m_lock);
        if (m_arenaBase) {
            VirtualFree(m_arenaBase, 0, MEM_RELEASE);
            m_arenaBase = nullptr;
        }
        m_buffers.clear();
        ReleaseSRWLockExclusive(&m_lock);
    }

    ArchiveMemoryCompactor(const ArchiveMemoryCompactor&) = delete;
    ArchiveMemoryCompactor& operator=(const ArchiveMemoryCompactor&) = delete;

    // ── Allocation ───────────────────────────────────────────────

    ExtractedBuffer* AllocateBuffer(uint64_t archiveId,
        uint32_t entryIndex,
        size_t   size) {
        if (size == 0 || !m_arenaBase) return nullptr;

        AcquireSRWLockExclusive(&m_lock);

        // Align size to 16-byte boundary for cache-line friendliness
        size_t alignedSize = AlignUp(size, 16);

        // Find a free gap in the arena (first-fit)
        void* slot = FindFreeSlotLocked(alignedSize);
        if (!slot) {
            // Try to grow committed region
            if (!GrowArenaLocked(alignedSize)) {
                ReleaseSRWLockExclusive(&m_lock);
                return nullptr;
            }
            slot = FindFreeSlotLocked(alignedSize);
            if (!slot) {
                // Allocate at the end of the used range
                slot = m_arenaBase + m_arenaUsedEnd;
                m_arenaUsedEnd += alignedSize;
            }
        }

        // Ensure pages are committed
        EnsureCommittedLocked(slot, alignedSize);

        ExtractedBuffer buf;
        buf.archiveId = archiveId;
        buf.entryIndex = entryIndex;
        buf.data = slot;
        buf.size = alignedSize;
        buf.pinned = false;
        buf.alive = true;

        m_buffers.push_back(buf);
        ExtractedBuffer* ptr = &m_buffers.back();

        m_totalUsed += alignedSize;

        // Auto-compact check
        double frag = ComputeFragmentationLocked();
        bool needsCompact = frag > kAutoCompactThreshold;

        ReleaseSRWLockExclusive(&m_lock);

        if (needsCompact) {
            Compact();
        }

        return ptr;
    }

    // ── Free ─────────────────────────────────────────────────────

    void FreeBuffer(ExtractedBuffer* buf) {
        if (!buf) return;
        AcquireSRWLockExclusive(&m_lock);
        buf->alive = false;
        if (m_totalUsed >= buf->size) {
            m_totalUsed -= buf->size;
        }
        // Zero the memory to avoid stale data
        if (buf->data && buf->size > 0) {
            std::memset(buf->data, 0, buf->size);
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Pin / Unpin ──────────────────────────────────────────────

    void PinBuffer(ExtractedBuffer* buf) {
        if (!buf) return;
        AcquireSRWLockExclusive(&m_lock);
        buf->pinned = true;
        ReleaseSRWLockExclusive(&m_lock);
    }

    void UnpinBuffer(ExtractedBuffer* buf) {
        if (!buf) return;
        AcquireSRWLockExclusive(&m_lock);
        buf->pinned = false;
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Compaction ───────────────────────────────────────────────

    CompactResult Compact() {
        AcquireSRWLockExclusive(&m_lock);
        CompactResult result{};

        auto startTime = std::chrono::high_resolution_clock::now();

        // Remove dead entries
        m_buffers.erase(
            std::remove_if(m_buffers.begin(), m_buffers.end(),
                [](const ExtractedBuffer& b) { return !b.alive; }),
            m_buffers.end());

        // Sort live buffers by arena offset (ascending)
        std::sort(m_buffers.begin(), m_buffers.end(),
            [](const ExtractedBuffer& a, const ExtractedBuffer& b) {
                return reinterpret_cast<uintptr_t>(a.data) <
                    reinterpret_cast<uintptr_t>(b.data);
            });

        result.fragmentsBefore = CountGapsLocked();

        // Compact: slide non-pinned buffers toward the arena base
        uint8_t* writePtr = m_arenaBase;

        for (auto& buf : m_buffers) {
            if (buf.pinned) {
                // Pinned buffer stays in place; advance writePtr past it
                uint8_t* bufEnd = static_cast<uint8_t*>(buf.data) + buf.size;
                if (bufEnd > writePtr) {
                    writePtr = bufEnd;
                }
                continue;
            }

            uint8_t* currentPos = static_cast<uint8_t*>(buf.data);
            if (currentPos != writePtr) {
                // Move buffer toward lower addresses using memmove
                std::memmove(writePtr, currentPos, buf.size);
                buf.data = writePtr;
                result.bytesMoved += buf.size;
                result.buffersMoved++;
                result.gapsFilled++;
            }
            writePtr += buf.size;
        }

        // Update arena used end
        if (!m_buffers.empty()) {
            auto& last = m_buffers.back();
            m_arenaUsedEnd = static_cast<size_t>(
                static_cast<uint8_t*>(last.data) + last.size - m_arenaBase);
        }
        else {
            m_arenaUsedEnd = 0;
        }

        // Recalculate total used
        m_totalUsed = 0;
        for (const auto& buf : m_buffers) {
            m_totalUsed += buf.size;
        }

        result.fragmentsAfter = CountGapsLocked();
        m_compactionsPerformed++;

        auto endTime = std::chrono::high_resolution_clock::now();
        result.durationUs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                endTime - startTime).count());

        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    // ── Arena limit ──────────────────────────────────────────────

    void SetArenaLimit(size_t maxBytes) {
        AcquireSRWLockExclusive(&m_lock);
        m_arenaLimit = maxBytes;
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Statistics ───────────────────────────────────────────────

    CompactorStats GetStats() {
        AcquireSRWLockExclusive(&m_lock);
        CompactorStats stats{};
        stats.arenaReserved = m_arenaReserved;
        stats.arenaCommitted = m_arenaCommitted;
        stats.usedBytes = m_totalUsed;
        stats.freeBytes = m_arenaCommitted > m_totalUsed
            ? m_arenaCommitted - m_totalUsed : 0;
        stats.bufferCount = 0;
        for (const auto& b : m_buffers) {
            if (b.alive) stats.bufferCount++;
        }
        stats.fragmentationRatio = ComputeFragmentationLocked();
        stats.compactionsPerformed = m_compactionsPerformed;
        ReleaseSRWLockExclusive(&m_lock);
        return stats;
    }

private:
    // ── Helpers (caller must hold lock) ──────────────────────────

    static size_t AlignUp(size_t value, size_t alignment) noexcept {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    void EnsureCommittedLocked(void* addr, size_t size) {
        uintptr_t start = reinterpret_cast<uintptr_t>(addr);
        uintptr_t end = start + size;
        uintptr_t commitEnd = reinterpret_cast<uintptr_t>(m_arenaBase) +
            m_arenaCommitted;

        if (end > commitEnd) {
            size_t needed = static_cast<size_t>(end - commitEnd);
            size_t commitSize = AlignUp(needed, kArenaChunkSize);
            commitSize = (std::min)(commitSize,
                m_arenaReserved - m_arenaCommitted);
            if (commitSize > 0) {
                void* commitAddr = m_arenaBase + m_arenaCommitted;
                void* result = VirtualAlloc(commitAddr, commitSize,
                    MEM_COMMIT, PAGE_READWRITE);
                if (result) {
                    m_arenaCommitted += commitSize;
                }
            }
        }
    }

    bool GrowArenaLocked(size_t needed) {
        size_t commitSize = (std::max)(kArenaChunkSize,
            AlignUp(needed, m_pageSize));
        if (m_arenaCommitted + commitSize > m_arenaReserved) {
            commitSize = m_arenaReserved - m_arenaCommitted;
        }
        if (commitSize == 0) return false;

        void* result = VirtualAlloc(m_arenaBase + m_arenaCommitted,
            commitSize, MEM_COMMIT, PAGE_READWRITE);
        if (!result) return false;

        m_arenaCommitted += commitSize;
        return true;
    }

    void* FindFreeSlotLocked(size_t size) {
        // First-fit scan: find a gap between live buffers
        if (m_buffers.empty()) {
            if (m_arenaCommitted >= size) {
                m_arenaUsedEnd = size;
                return m_arenaBase;
            }
            return nullptr;
        }

        // Sort by offset
        std::sort(m_buffers.begin(), m_buffers.end(),
            [](const ExtractedBuffer& a, const ExtractedBuffer& b) {
                return reinterpret_cast<uintptr_t>(a.data) <
                    reinterpret_cast<uintptr_t>(b.data);
            });

        // Check gap before first buffer
        uint8_t* firstStart = static_cast<uint8_t*>(m_buffers.front().data);
        if (m_buffers.front().alive) {
            size_t gapBefore = static_cast<size_t>(firstStart - m_arenaBase);
            if (gapBefore >= size) return m_arenaBase;
        }

        // Check gaps between buffers
        for (size_t i = 0; i + 1 < m_buffers.size(); ++i) {
            if (!m_buffers[i].alive) continue;
            uint8_t* endI = static_cast<uint8_t*>(m_buffers[i].data)
                + m_buffers[i].size;
            uint8_t* startNext = nullptr;
            for (size_t j = i + 1; j < m_buffers.size(); ++j) {
                if (m_buffers[j].alive) {
                    startNext = static_cast<uint8_t*>(m_buffers[j].data);
                    break;
                }
            }
            if (startNext && static_cast<size_t>(startNext - endI) >= size) {
                return endI;
            }
        }

        // Check gap after last buffer
        uint8_t* lastEnd = nullptr;
        for (auto it = m_buffers.rbegin(); it != m_buffers.rend(); ++it) {
            if (it->alive) {
                lastEnd = static_cast<uint8_t*>(it->data) + it->size;
                break;
            }
        }
        if (lastEnd) {
            size_t remaining = m_arenaCommitted -
                static_cast<size_t>(lastEnd - m_arenaBase);
            if (remaining >= size) {
                size_t newEnd = static_cast<size_t>(lastEnd - m_arenaBase) + size;
                if (newEnd > m_arenaUsedEnd) m_arenaUsedEnd = newEnd;
                return lastEnd;
            }
        }

        return nullptr;
    }

    uint32_t CountGapsLocked() const {
        uint32_t gaps = 0;
        uint8_t* expected = m_arenaBase;
        for (const auto& buf : m_buffers) {
            if (!buf.alive) continue;
            uint8_t* bufStart = static_cast<uint8_t*>(buf.data);
            if (bufStart > expected) {
                gaps++;
            }
            expected = bufStart + buf.size;
        }
        return gaps;
    }

    double ComputeFragmentationLocked() const {
        if (m_arenaUsedEnd == 0 || m_totalUsed == 0) return 0.0;
        size_t totalSpan = m_arenaUsedEnd;
        size_t freeInSpan = totalSpan > m_totalUsed
            ? totalSpan - m_totalUsed : 0;
        if (freeInSpan == 0) return 0.0;

        // Find largest contiguous free block
        size_t largestGap = 0;
        uint8_t* expected = m_arenaBase;
        for (const auto& buf : m_buffers) {
            if (!buf.alive) continue;
            uint8_t* bufStart = static_cast<uint8_t*>(buf.data);
            if (bufStart > expected) {
                size_t gap = static_cast<size_t>(bufStart - expected);
                largestGap = (std::max)(largestGap, gap);
            }
            expected = bufStart + buf.size;
        }
        // Gap after last buffer
        uint8_t* arenaEnd = m_arenaBase + m_arenaUsedEnd;
        if (arenaEnd > expected) {
            size_t gap = static_cast<size_t>(arenaEnd - expected);
            largestGap = (std::max)(largestGap, gap);
        }

        return 1.0 - (static_cast<double>(largestGap) /
            static_cast<double>(freeInSpan));
    }

    // ── Data members ─────────────────────────────────────────────

    SRWLOCK m_lock{};

    uint8_t* m_arenaBase = nullptr;
    size_t   m_arenaReserved = 0;
    size_t   m_arenaCommitted = 0;
    size_t   m_arenaUsedEnd = 0;   // high-water mark within arena
    size_t   m_arenaLimit = kDefaultArenaLimit;
    size_t   m_totalUsed = 0;
    size_t   m_pageSize = 4096;

    std::vector<ExtractedBuffer> m_buffers;

    uint32_t m_compactionsPerformed = 0;
};

} // namespace Engine
} // namespace ExplorerLens
