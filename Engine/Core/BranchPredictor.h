// BranchPredictor.h — Branch prediction hints and branch-free utilities
// Copyright (c) 2026 ExplorerLens Project
//
// Provides LIKELY/UNLIKELY macros using C++20 [[likely]]/[[unlikely]] attributes,
// branch-free binary search (SortedLookup), hot path markers, and compile-time
// format dispatch table for the decode pipeline.
//
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// C++20 Branch Prediction Hints
// ============================================================================

/// LIKELY/UNLIKELY macros using C++20 attributes
/// Usage: if (LENS_LIKELY(ptr != nullptr)) { ... }
///        if (LENS_UNLIKELY(errorCode)) { ... }
#define LENS_LIKELY(cond) (cond) [[likely]]
#define LENS_UNLIKELY(cond) (cond) [[unlikely]]

/// Hot path marker — annotates a branch as the common case
/// Usage: if (condition) LENS_HOT_PATH { ... }
#define LENS_HOT_PATH [[likely]]

/// Cold path marker — annotates a branch as rare/error handling
/// Usage: if (errorCondition) LENS_COLD_PATH { handleError(); }
#define LENS_COLD_PATH [[unlikely]]

/// Force-inline hint for hot path functions
#define LENS_FORCE_INLINE __forceinline

// ============================================================================
// Branch-Free Binary Search
// ============================================================================

/// Branch-free binary search result
struct LookupResult
{
    uint32_t index = 0;
    bool found = false;
};

/// Branch-free sorted lookup — performs binary search using conditional moves
/// instead of unpredictable branches. Best for small-to-medium sorted arrays.
template <typename KeyT, size_t MaxSize>
class SortedLookup
{
  public:
    SortedLookup() = default;

    /// Insert a key-value pair (must call Sort() after all inserts)
    bool Insert(KeyT key, uint32_t value)
    {
        if (m_count >= MaxSize)
            return false;
        m_entries[m_count] = {key, value};
        m_count++;
        m_sorted = false;
        return true;
    }

    /// Sort entries (required after inserts, before lookups)
    void Sort()
    {
        if (m_count <= 1) {
            m_sorted = true;
            return;
        }
        // Simple insertion sort — optimal for small arrays
        for (size_t i = 1; i < m_count; ++i) {
            auto temp = m_entries[i];
            size_t j = i;
            while (j > 0 && m_entries[j - 1].key > temp.key) {
                m_entries[j] = m_entries[j - 1];
                --j;
            }
            m_entries[j] = temp;
        }
        m_sorted = true;
    }

    /// Branch-free binary search — uses arithmetic instead of branches
    LookupResult Find(KeyT needle) const
    {
        if (m_count == 0)
            return {0, false};

        size_t lo = 0;
        size_t hi = m_count;

        // Branch-free binary search: use arithmetic selection
        while (hi - lo > 1) {
            size_t mid = lo + (hi - lo) / 2;
            // Branchless: lo = (needle >= m_entries[mid].key) ? mid : lo;
            lo = (needle >= m_entries[mid].key) ? mid : lo;
            hi = (needle < m_entries[mid].key) ? mid : hi;
        }

        bool match = (lo < m_count && m_entries[lo].key == needle);
        return {match ? m_entries[lo].value : 0, match};
    }

    /// Get number of entries
    size_t Count() const
    {
        return m_count;
    }

    /// Check if sorted
    bool IsSorted() const
    {
        return m_sorted;
    }

    /// Clear all entries
    void Clear()
    {
        m_count = 0;
        m_sorted = true;
    }

  private:
    struct Entry
    {
        KeyT key{};
        uint32_t value = 0;
    };

    std::array<Entry, MaxSize> m_entries{};
    size_t m_count = 0;
    bool m_sorted = true;
};

// ============================================================================
// Compile-Time Format Dispatch Table
// ============================================================================

/// Format dispatch entry — maps a format ID to a decoder function pointer
struct FormatDispatchEntry
{
    uint32_t formatId = 0;
    bool (*decoderAvailable)() = nullptr;
    const char* formatName = nullptr;
};

/// Compile-time format dispatch table
template <size_t N>
class FormatDispatchTable
{
  public:
    constexpr FormatDispatchTable() = default;

    /// Register a format at compile time
    constexpr void Register(size_t idx, uint32_t formatId, bool (*available)(), const char* name)
    {
        if (idx < N) {
            m_entries[idx] = {formatId, available, name};
            if (idx >= m_count)
                m_count = idx + 1;
        }
    }

    /// Lookup decoder availability by format ID (linear scan — small tables)
    bool IsAvailable(uint32_t formatId) const
    {
        for (size_t i = 0; i < m_count; ++i) {
            if (m_entries[i].formatId == formatId) {
                return m_entries[i].decoderAvailable ? m_entries[i].decoderAvailable() : false;
            }
        }
        return false;
    }

    /// Get format name by ID
    const char* GetName(uint32_t formatId) const
    {
        for (size_t i = 0; i < m_count; ++i) {
            if (m_entries[i].formatId == formatId) {
                return m_entries[i].formatName ? m_entries[i].formatName : "";
            }
        }
        return "";
    }

    /// Get entry count
    constexpr size_t Count() const
    {
        return m_count;
    }

    /// Get table capacity
    static constexpr size_t Capacity()
    {
        return N;
    }

  private:
    std::array<FormatDispatchEntry, N> m_entries{};
    size_t m_count = 0;
};

// ============================================================================
// Profile-Guided Hot Path Utilities
// ============================================================================

/// Simple hit counter for identifying hot/cold paths at runtime
class HotPathCounter
{
  public:
    HotPathCounter() = default;

    /// Record a hit on a labelled path
    LENS_FORCE_INLINE void RecordHit(size_t pathIndex)
    {
        if (pathIndex < MAX_PATHS) {
            m_counts[pathIndex]++;
        }
    }

    /// Get hit count for a path
    uint64_t GetCount(size_t pathIndex) const
    {
        return (pathIndex < MAX_PATHS) ? m_counts[pathIndex] : 0;
    }

    /// Get the hottest path index
    size_t HottestPath() const
    {
        size_t best = 0;
        for (size_t i = 1; i < MAX_PATHS; ++i) {
            if (m_counts[i] > m_counts[best]) {
                best = i;
            }
        }
        return best;
    }

    /// Get total hits across all paths
    uint64_t TotalHits() const
    {
        uint64_t total = 0;
        for (size_t i = 0; i < MAX_PATHS; ++i) {
            total += m_counts[i];
        }
        return total;
    }

    /// Reset all counters
    void Reset()
    {
        for (auto& c : m_counts)
            c = 0;
    }

    static constexpr size_t MAX_PATHS = 32;

  private:
    uint64_t m_counts[MAX_PATHS]{};
};

/// Branchless minimum of two unsigned values
LENS_FORCE_INLINE uint32_t BranchlessMin(uint32_t a, uint32_t b)
{
    // Uses arithmetic: if a < b, mask = 0xFFFFFFFF, else 0
    // Result = a & mask | b & ~mask
    uint32_t diff = a - b;
    uint32_t mask = static_cast<uint32_t>(static_cast<int32_t>(diff) >> 31);
    return (a & mask) | (b & ~mask);
}

/// Branchless maximum of two unsigned values
LENS_FORCE_INLINE uint32_t BranchlessMax(uint32_t a, uint32_t b)
{
    uint32_t diff = a - b;
    uint32_t mask = static_cast<uint32_t>(static_cast<int32_t>(diff) >> 31);
    return (b & mask) | (a & ~mask);
}

/// Branchless clamp
LENS_FORCE_INLINE uint32_t BranchlessClamp(uint32_t val, uint32_t lo, uint32_t hi)
{
    return BranchlessMin(BranchlessMax(val, lo), hi);
}

}  // namespace Engine
}  // namespace ExplorerLens
