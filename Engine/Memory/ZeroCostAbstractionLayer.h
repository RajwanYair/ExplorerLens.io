// ZeroCostAbstractionLayer.h — Compile-Time Memory Layout Optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Compile-time memory layout optimization. Provides constexpr utilities for
// struct packing validation, alignment analysis, and cache-line optimization.
//
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

namespace ExplorerLens {
namespace Engine {

static constexpr size_t CACHE_LINE_SIZE = 64;
static constexpr size_t L1_CACHE_SIZE = 32768;
static constexpr size_t L2_CACHE_SIZE = 262144;

template <typename T>
struct LayoutInfo
{
    static constexpr size_t Size = sizeof(T);
    static constexpr size_t Alignment = alignof(T);
    static constexpr bool FitsCacheLine = sizeof(T) <= CACHE_LINE_SIZE;
    static constexpr size_t CacheLinesUsed = (sizeof(T) + CACHE_LINE_SIZE - 1) / CACHE_LINE_SIZE;
    static constexpr bool IsTriviallyCopyable = std::is_trivially_copyable_v<T>;
    static constexpr bool IsTriviallyDestructible = std::is_trivially_destructible_v<T>;
    static constexpr bool IsStandardLayout = std::is_standard_layout_v<T>;
    static constexpr size_t WastedBytes = CacheLinesUsed * CACHE_LINE_SIZE - sizeof(T);
    static constexpr double Efficiency = static_cast<double>(sizeof(T)) / (CacheLinesUsed * CACHE_LINE_SIZE);
};

template <size_t Size, size_t Alignment = CACHE_LINE_SIZE>
struct alignas(Alignment) AlignedStorage
{
    uint8_t data[Size];

    inline uint8_t* Ptr()
    {
        return data;
    }
    inline const uint8_t* Ptr() const
    {
        return data;
    }

    template <typename T>
    inline T* As()
    {
        static_assert(sizeof(T) <= Size, "Type does not fit in AlignedStorage");
        static_assert(Alignment % alignof(T) == 0, "Alignment mismatch");
        return reinterpret_cast<T*>(data);
    }

    template <typename T>
    inline const T* As() const
    {
        static_assert(sizeof(T) <= Size, "Type does not fit in AlignedStorage");
        return reinterpret_cast<const T*>(data);
    }
};

template <typename T, size_t N>
struct alignas(CACHE_LINE_SIZE) CacheAlignedArray
{
    T elements[N];

    static constexpr size_t Count = N;
    static constexpr size_t TotalSize = sizeof(T) * N;
    static constexpr size_t ElementsPerCacheLine = CACHE_LINE_SIZE / sizeof(T);

    inline T& operator[](size_t index)
    {
        return elements[index];
    }
    inline const T& operator[](size_t index) const
    {
        return elements[index];
    }
    inline T* Data()
    {
        return elements;
    }
    inline const T* Data() const
    {
        return elements;
    }
    inline constexpr size_t Size() const
    {
        return N;
    }
};

template <typename T>
struct PaddedToLine
{
    T value;
    uint8_t padding[CACHE_LINE_SIZE - (sizeof(T) % CACHE_LINE_SIZE == 0 ? CACHE_LINE_SIZE : sizeof(T) % CACHE_LINE_SIZE)];

    inline T& Get()
    {
        return value;
    }
    inline const T& Get() const
    {
        return value;
    }
    inline T& operator*()
    {
        return value;
    }
    inline const T& operator*() const
    {
        return value;
    }
};

class ZeroCostAbstractionLayer
{
  public:
    static ZeroCostAbstractionLayer& Instance()
    {
        static ZeroCostAbstractionLayer instance;
        return instance;
    }

    static constexpr size_t AlignUp(size_t value, size_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    static constexpr size_t AlignDown(size_t value, size_t alignment)
    {
        return value & ~(alignment - 1);
    }

    static constexpr bool IsPowerOfTwo(size_t value)
    {
        return value != 0 && (value & (value - 1)) == 0;
    }

    static constexpr size_t NextPowerOfTwo(size_t value)
    {
        if (value == 0)
            return 1;
        --value;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;
        return ++value;
    }

    template <typename T>
    static constexpr bool WouldCauseFalseSharing(size_t offsetA, size_t offsetB)
    {
        size_t lineA = offsetA / CACHE_LINE_SIZE;
        size_t lineB = offsetB / CACHE_LINE_SIZE;
        return lineA == lineB;
    }

    template <typename T>
    inline std::string DescribeLayout() const
    {
        auto info = LayoutInfo<T>{};
        std::string desc = "Size=" + std::to_string(info.Size) + " Align=" + std::to_string(info.Alignment)
                           + " CacheLines=" + std::to_string(info.CacheLinesUsed) + " Waste="
                           + std::to_string(info.WastedBytes) + " Copyable=" + (info.IsTriviallyCopyable ? "yes" : "no")
                           + " StdLayout=" + (info.IsStandardLayout ? "yes" : "no");
        return desc;
    }

  private:
    ZeroCostAbstractionLayer() = default;
};

}  // namespace Engine
}  // namespace ExplorerLens
