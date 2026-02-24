#pragma once
// Archive Memory Compactor
// Post-decode buffer compaction, slab recycling, large-archive eviction policy.
// Works together with the existing cache layer to reduce working-set on low-RAM machines.

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens::Memory {

// ─── Slab descriptor ─────────────────────────────────────────────────────────

enum class SlabState : uint32_t {
    Free        = 0,
    Active      = 1,   // in-use by decoder
    Evictable   = 2,   // decoded, not in active viewport
    Pinned      = 3,   // must stay resident (e.g., shown in Explorer)
};

struct MemorySlab {
    uint64_t    slabId          { 0 };
    size_t      sizeBytes       { 0 };
    SlabState   state           { SlabState::Free };
    std::string ownerDecoder;           // e.g., "ArchiveDecoder"
    uint64_t    lastAccessTick  { 0 };  // monotonic counter
    bool        isLargeAlloc    { false };  // true if > 16 MB

    bool CanEvict() const { return state == SlabState::Evictable; }
};

// ─── Compact report ───────────────────────────────────────────────────────────

struct CompactionReport {
    uint32_t    slabsExamined   { 0 };
    uint32_t    slabsEvicted    { 0 };
    uint32_t    slabsCompacted  { 0 };
    size_t      bytesBefore     { 0 };
    size_t      bytesAfter      { 0 };
    double      compactionMs    { 0.0 };

    size_t BytesReclaimed() const { return bytesBefore > bytesAfter ? bytesBefore - bytesAfter : 0; }
    double ReductionPercent() const {
        return bytesBefore > 0 ? 100.0 * BytesReclaimed() / bytesBefore : 0.0;
    }
};

// ─── Eviction policy ─────────────────────────────────────────────────────────

enum class EvictionPolicy : uint32_t {
    LRU         = 0,   // least recently used
    LFU         = 1,   // least frequently used
    SizeLargest = 2,   // largest slabs first
    OlderThan   = 3,   // evict anything not accessed for N ticks
};

struct EvictionConfig {
    EvictionPolicy  policy          { EvictionPolicy::LRU };
    uint64_t        maxAgeTicksLRU  { 1000 };
    size_t          targetUsageBytes{ 128ULL * 1024 * 1024 };  // 128 MB target
    bool            spareOnePinned  { true };

    static EvictionConfig Default() {
        EvictionConfig c;
        c.policy = EvictionPolicy::LRU;
        c.targetUsageBytes = 128ULL * 1024 * 1024;
        return c;
    }

    static EvictionConfig AggressiveLowRAM() {
        EvictionConfig c;
        c.policy = EvictionPolicy::SizeLargest;
        c.targetUsageBytes = 64ULL * 1024 * 1024;   // 64 MB target on low-RAM
        return c;
    }
};

// ─── Archive memory compactor ────────────────────────────────────────────────

class ArchiveMemoryCompactor {
public:
    explicit ArchiveMemoryCompactor(EvictionConfig config = EvictionConfig::Default())
        : m_config(std::move(config)), m_currentTick(0) {}

    void TrackSlab(MemorySlab slab) {
        slab.lastAccessTick = ++m_currentTick;
        m_slabs.push_back(std::move(slab));
    }

    void TouchSlab(uint64_t slabId) {
        for (auto& s : m_slabs)
            if (s.slabId == slabId) { s.lastAccessTick = ++m_currentTick; return; }
    }

    size_t TotalActiveBytes() const {
        size_t total = 0;
        for (const auto& s : m_slabs)
            if (s.state == SlabState::Active || s.state == SlabState::Evictable)
                total += s.sizeBytes;
        return total;
    }

    CompactionReport Compact() {
        CompactionReport report;
        report.bytesBefore = TotalActiveBytes();
        report.slabsExamined = static_cast<uint32_t>(m_slabs.size());

        // Mark oldest active slabs as evictable
        for (auto& s : m_slabs)
            if (s.state == SlabState::Active &&
                (m_currentTick - s.lastAccessTick) > m_config.maxAgeTicksLRU)
                s.state = SlabState::Evictable;

        // Evict if over budget
        std::vector<MemorySlab> survivors;
        for (auto& s : m_slabs) {
            bool evict = s.CanEvict() && TotalActiveBytes() > m_config.targetUsageBytes;
            if (evict) {
                ++report.slabsEvicted;
            } else {
                survivors.push_back(s);
            }
        }
        m_slabs = std::move(survivors);
        report.bytesAfter = TotalActiveBytes();
        return report;
    }

    const EvictionConfig& Config() const { return m_config; }

private:
    EvictionConfig          m_config;
    std::vector<MemorySlab> m_slabs;
    uint64_t                m_currentTick;
};

} // namespace ExplorerLens::Memory

