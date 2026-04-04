// CacheLinePadding.h — False-sharing prevention wrappers
// Copyright (c) 2026 ExplorerLens Project
//
// Ensures hot data structures avoid false sharing by aligning to cache line
// boundaries. Provides CacheAligned<T> wrapper, PaddedAtomic<T> for atomic
// counters on separate cache lines, and CacheLineArray<T,N> for arrays with
// per-element cache line alignment.
//
#pragma once

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4324)  // structure was padded due to alignment specifier
#endif

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

namespace ExplorerLens {
namespace Cache {

/// Hardware cache line size (64 bytes on x86/x64, common on ARM64)
static constexpr size_t CACHE_LINE_BYTES = 64;

// ============================================================================
// CacheAligned<T> — Aligns any type to a cache line boundary
// ============================================================================

/// Wraps a value type so it occupies its own cache line(s), preventing
/// false sharing when multiple threads access adjacent data.
template <typename T>
struct alignas(CACHE_LINE_BYTES) CacheAligned
{
    T value{};

    CacheAligned() = default;
    explicit CacheAligned(const T& val) : value(val) {}
    explicit CacheAligned(T&& val) : value(std::move(val)) {}

    // Implicit conversion operators
    operator T&()
    {
        return value;
    }
    operator const T&() const
    {
        return value;
    }

    T& Get()
    {
        return value;
    }
    const T& Get() const
    {
        return value;
    }

    T* operator->()
    {
        return &value;
    }
    const T* operator->() const
    {
        return &value;
    }

    /// Check alignment at runtime
    bool IsProperlyAligned() const
    {
        return (reinterpret_cast<uintptr_t>(this) % CACHE_LINE_BYTES) == 0;
    }
};

// Static assertions for CacheAligned
static_assert(alignof(CacheAligned<int>) == CACHE_LINE_BYTES, "CacheAligned must have cache-line alignment");
static_assert(sizeof(CacheAligned<int>) >= CACHE_LINE_BYTES, "CacheAligned must occupy at least one cache line");

// ============================================================================
// PaddedAtomic<T> — Atomic counter on its own cache line
// ============================================================================

/// An atomic variable padded to occupy an entire cache line, preventing
/// false sharing between frequently updated atomic counters.
template <typename T>
struct alignas(CACHE_LINE_BYTES) PaddedAtomic
{
    std::atomic<T> value{};

    PaddedAtomic() = default;
    explicit PaddedAtomic(T initial) : value(initial) {}

    // Atomic operations forwarded
    T Load(std::memory_order order = std::memory_order_seq_cst) const
    {
        return value.load(order);
    }

    void Store(T val, std::memory_order order = std::memory_order_seq_cst)
    {
        value.store(val, order);
    }

    T FetchAdd(T delta, std::memory_order order = std::memory_order_seq_cst)
    {
        return value.fetch_add(delta, order);
    }

    T FetchSub(T delta, std::memory_order order = std::memory_order_seq_cst)
    {
        return value.fetch_sub(delta, order);
    }

    bool CompareExchange(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst)
    {
        return value.compare_exchange_strong(expected, desired, order);
    }

    // Pre-increment
    T operator++()
    {
        return value.fetch_add(1) + 1;
    }
    // Post-increment
    T operator++(int)
    {
        return value.fetch_add(1);
    }
    // Pre-decrement
    T operator--()
    {
        return value.fetch_sub(1) - 1;
    }
    // Post-decrement
    T operator--(int)
    {
        return value.fetch_sub(1);
    }

    /// Check that this atomic is on its own cache line
    bool IsProperlyAligned() const
    {
        return (reinterpret_cast<uintptr_t>(this) % CACHE_LINE_BYTES) == 0;
    }
};

// Static assertions for PaddedAtomic
static_assert(alignof(PaddedAtomic<uint64_t>) == CACHE_LINE_BYTES, "PaddedAtomic must have cache-line alignment");
static_assert(sizeof(PaddedAtomic<uint64_t>) >= CACHE_LINE_BYTES, "PaddedAtomic must occupy at least one cache line");

// ============================================================================
// CacheLineArray<T, N> — Array with per-element cache line alignment
// ============================================================================

/// An array where each element is aligned to its own cache line.
/// Ideal for per-thread counters, per-core accumulators, etc.
template <typename T, size_t N>
class CacheLineArray
{
  public:
    CacheLineArray() = default;

    /// Initialize all elements to a value
    explicit CacheLineArray(const T& initVal)
    {
        for (size_t i = 0; i < N; ++i) {
            m_elements[i].value = initVal;
        }
    }

    /// Element access (returns reference to the aligned value)
    T& operator[](size_t index)
    {
        return m_elements[index].value;
    }

    const T& operator[](size_t index) const
    {
        return m_elements[index].value;
    }

    /// Get pointer to aligned element
    T* At(size_t index)
    {
        return (index < N) ? &m_elements[index].value : nullptr;
    }

    const T* At(size_t index) const
    {
        return (index < N) ? &m_elements[index].value : nullptr;
    }

    /// Number of elements
    static constexpr size_t Size()
    {
        return N;
    }

    /// Total memory footprint in bytes
    static constexpr size_t FootprintBytes()
    {
        return sizeof(CacheLineArray);
    }

    /// Bytes per element (includes padding)
    static constexpr size_t BytesPerElement()
    {
        return sizeof(CacheAligned<T>);
    }

    /// Verify all elements are cache-line aligned
    bool AllAligned() const
    {
        for (size_t i = 0; i < N; ++i) {
            if (!m_elements[i].IsProperlyAligned())
                return false;
        }
        return true;
    }

    /// Fill all elements with a value
    void Fill(const T& val)
    {
        for (size_t i = 0; i < N; ++i) {
            m_elements[i].value = val;
        }
    }

  private:
    CacheAligned<T> m_elements[N];
};

// ============================================================================
// Utility: False-Sharing Detector
// ============================================================================

/// Compile-time check: are two members potentially sharing a cache line?
/// Usage: static_assert(!MayFalseShare(offsetof(S, a), sizeof(a),
///                                     offsetof(S, b), sizeof(b)));
constexpr bool MayFalseShare(size_t offsetA, size_t sizeA, size_t offsetB, size_t /*sizeB*/)
{
    size_t endA = offsetA + sizeA;
    size_t lineA = offsetA / CACHE_LINE_BYTES;
    size_t lineEndA = (endA > 0) ? (endA - 1) / CACHE_LINE_BYTES : 0;
    size_t lineB = offsetB / CACHE_LINE_BYTES;
    // If B starts on the same or adjacent cache line as A ends, may false share
    return (lineB >= lineA && lineB <= lineEndA);
}

/// Helper struct for diagnosing cache line occupancy
struct CacheLineInfo
{
    size_t objectSize = 0;
    size_t alignment = 0;
    size_t cacheLinesUsed = 0;
    size_t wastedBytes = 0;

    template <typename T>
    static CacheLineInfo Analyze()
    {
        CacheLineInfo info;
        info.objectSize = sizeof(T);
        info.alignment = alignof(T);
        info.cacheLinesUsed = (sizeof(T) + CACHE_LINE_BYTES - 1) / CACHE_LINE_BYTES;
        info.wastedBytes = info.cacheLinesUsed * CACHE_LINE_BYTES - sizeof(T);
        return info;
    }
};

}  // namespace Cache
}  // namespace ExplorerLens

#ifdef _MSC_VER
    #pragma warning(pop)
#endif
