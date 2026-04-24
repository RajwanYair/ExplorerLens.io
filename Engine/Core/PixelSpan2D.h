//==============================================================================
// ExplorerLens Engine — PixelSpan2D (Sprint S241)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 A11 / ADR-016 — 2D pixel buffer views (Phase 2 scaffold).
//==============================================================================
//
// `std::mdspan` landed in C++23 but MSVC v145 exposes it under <mdspan> only
// when /std:c++latest is enabled. Until we flip the project standard to C++23
// (tracked by ADR-016, gated behind Phase 2 tooling bump), this header gives
// Engine code a header-only stand-in with the same access ergonomics:
//
//     PixelSpan2D<uint32_t> view(pixels, width, height, strideBytes);
//     auto& px = view(x, y);   // bounds-checked in Debug, direct in Release
//
// Design goals:
//   * Non-owning. Caller owns the backing buffer.
//   * Stride-aware: pitch is separate from width*sizeof(T) to handle row
//     alignment (DirectX textures, GDI+ BitmapData, cache blobs).
//   * Trivially copyable: can cross the COM boundary inside a POD request.
//   * No exceptions. Invalid construction returns an Empty() span.
//==============================================================================
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens {
namespace Engine {

template <typename TPixel>
class PixelSpan2D
{
    static_assert(std::is_trivially_copyable_v<TPixel>,
                  "PixelSpan2D requires trivially-copyable pixel types");

  public:
    using PixelType = TPixel;

    constexpr PixelSpan2D() noexcept = default;

    /// <summary>Construct a view. strideBytes must be >= width*sizeof(TPixel).</summary>
    constexpr PixelSpan2D(TPixel* data,
                          std::uint32_t width,
                          std::uint32_t height,
                          std::size_t strideBytes) noexcept
        : m_data(data), m_width(width), m_height(height), m_stride(strideBytes)
    {}

    /// <summary>Tightly packed view (stride == width*sizeof(TPixel)).</summary>
    static constexpr PixelSpan2D<TPixel> Packed(TPixel* data,
                                                std::uint32_t width,
                                                std::uint32_t height) noexcept
    {
        return PixelSpan2D<TPixel>(data, width, height,
                                   static_cast<std::size_t>(width) * sizeof(TPixel));
    }

    static constexpr PixelSpan2D<TPixel> Empty() noexcept { return {}; }

    constexpr bool IsValid() const noexcept
    {
        return m_data != nullptr
            && m_width > 0 && m_height > 0
            && m_stride >= static_cast<std::size_t>(m_width) * sizeof(TPixel);
    }

    constexpr std::uint32_t Width()       const noexcept { return m_width; }
    constexpr std::uint32_t Height()      const noexcept { return m_height; }
    constexpr std::size_t   StrideBytes() const noexcept { return m_stride; }
    constexpr TPixel*       Data()        const noexcept { return m_data; }

    /// <summary>Row pointer — no bounds check.</summary>
    constexpr TPixel* Row(std::uint32_t y) const noexcept
    {
        return reinterpret_cast<TPixel*>(
            reinterpret_cast<std::uint8_t*>(m_data) + static_cast<std::size_t>(y) * m_stride);
    }

    /// <summary>Element access with Debug bounds check.</summary>
    TPixel& operator()(std::uint32_t x, std::uint32_t y) const noexcept
    {
        assert(x < m_width && y < m_height && m_data != nullptr && "PixelSpan2D OOB");
        return Row(y)[x];
    }

    /// <summary>Total byte size of the described region (not capacity).</summary>
    constexpr std::size_t SizeBytes() const noexcept
    {
        return static_cast<std::size_t>(m_height) * m_stride;
    }

  private:
    TPixel*       m_data   = nullptr;
    std::uint32_t m_width  = 0;
    std::uint32_t m_height = 0;
    std::size_t   m_stride = 0;
};

static_assert(std::is_trivially_copyable_v<PixelSpan2D<std::uint32_t>>,
              "PixelSpan2D must be trivially copyable to cross ABI boundaries");

} // namespace Engine
} // namespace ExplorerLens
