// SafeDimensions.h — Safe Integer Arithmetic for Decode Dimensions (§15.1 P0)
// Copyright (c) 2026 ExplorerLens Project
//
// ROADMAP §15.1 — Security Hardening Phase 1 (P0): SafeInt<> for dimension math.
//
// PURPOSE
// ───────
// Integer overflow in image dimension arithmetic is a classic class of
// vulnerability (CWE-190).  Multiplying two uint32 dimensions to compute a
// buffer size silently wraps at 2^32, producing a drastically under-sized
// allocation that subsequent decode code will write past.
//
// This header provides:
//   • SafeDim — a value-checked dimension type that rejects out-of-range values
//     at construction time (static factory, no throwing constructors).
//   • SafeDimensions — validated (width, height) pair with pixel/byte budget checks.
//   • SafeMul2, SafeMul3, SafeAdd — generic overflow-safe arithmetic helpers.
//   • kMaxThumbDimension, kMaxThumbPixels, kMaxThumbBytes — project-wide constants.
//
// MSVC NOTE: Compiles at /W4 with MSVC v145.  No <windows.h> included.
// Thread safety: all operations are stateless and unconditionally thread-safe.
//
// OWASP A4 — Insecure Design: prevents resource exhaustion via oversized buffers.
// OWASP A8 — Software and Data Integrity: buffer sizes are always verified.
//
// USAGE
// ─────
//   #include "Engine/Core/SafeDimensions.h"
//   using namespace ExplorerLens::Core;
//
//   auto dims = SafeDimensions::Make(width, height);
//   if (!dims) { return E_INVALIDARG; }  // out-of-range or overflow
//
//   uint64_t byteCount = dims->ByteCount();  // always safe, never wraps
// ────────────────────────────────────────────────────────────────────────────
#pragma once

#include <cstdint>
#include <optional>
#include <limits>

namespace ExplorerLens {
namespace Core {

// ============================================================================
// Project-wide decode budget constants
// ============================================================================

/// Maximum allowed thumbnail dimension (width or height), in pixels.
/// 32768 = 2^15.  Values above this are considered malicious or corrupt.
inline constexpr uint32_t kMaxThumbDimension = 32768u;

/// Maximum allowed pixel count per thumbnail decode.
/// 16K × 16K = 268 435 456 pixels ≈ 268 M pixels.
inline constexpr uint64_t kMaxThumbPixels =
    static_cast<uint64_t>(kMaxThumbDimension) * kMaxThumbDimension;

/// Maximum buffer size for a decoded thumbnail, in bytes (4 bytes / pixel, BGRA32).
/// 268 M pixels × 4 = ~1 GiB — a hard upper bound guarding against allocation bombs.
inline constexpr uint64_t kMaxThumbBytes = kMaxThumbPixels * 4u;

/// Minimum meaningful thumbnail dimension.
inline constexpr uint32_t kMinThumbDimension = 1u;

// ============================================================================
// SafeMul2 / SafeMul3 / SafeAdd — overflow-safe arithmetic
// ============================================================================

/// Multiply two uint32 values, returning the uint64 result, or nullopt on
/// overflow (i.e. when the true result exceeds the given ceiling).
[[nodiscard]] inline constexpr std::optional<uint64_t>
SafeMul2(uint32_t a, uint32_t b,
         uint64_t ceiling = std::numeric_limits<uint64_t>::max()) noexcept
{
    const uint64_t result = static_cast<uint64_t>(a) * static_cast<uint64_t>(b);
    if (result > ceiling) return std::nullopt;
    return result;
}

/// Multiply three uint32 values, returning a uint64 result, or nullopt on overflow.
[[nodiscard]] inline constexpr std::optional<uint64_t>
SafeMul3(uint32_t a, uint32_t b, uint32_t c,
         uint64_t ceiling = std::numeric_limits<uint64_t>::max()) noexcept
{
    const uint64_t ab = static_cast<uint64_t>(a) * static_cast<uint64_t>(b);
    if (c != 0 && ab > ceiling / c) return std::nullopt;
    const uint64_t result = ab * c;
    if (result > ceiling) return std::nullopt;
    return result;
}

/// Add two uint32 values, returning a uint64 result, or nullopt if the result
/// would exceed uint64_max (impossible in practice) or the given ceiling.
[[nodiscard]] inline constexpr std::optional<uint64_t>
SafeAdd(uint64_t a, uint64_t b,
        uint64_t ceiling = std::numeric_limits<uint64_t>::max()) noexcept
{
    if (a > ceiling - b) return std::nullopt;
    return a + b;
}

// ============================================================================
// SafeDim — single dimension value with range validation
// ============================================================================

/// A width or height value that has been validated to be within
/// [kMinThumbDimension, kMaxThumbDimension].
///
/// Construction is only possible via SafeDim::Make(); reject all other paths.
class SafeDim final
{
public:
    SafeDim() = delete;

    /// Factory — returns nullopt if value is 0 or > kMaxThumbDimension.
    [[nodiscard]] static constexpr std::optional<SafeDim>
    Make(uint32_t value) noexcept
    {
        if (value < kMinThumbDimension || value > kMaxThumbDimension)
            return std::nullopt;
        return SafeDim{value};
    }

    /// Access the validated value.
    [[nodiscard]] constexpr uint32_t Value() const noexcept { return m_value; }

    /// Implicit conversion for use in arithmetic.
    [[nodiscard]] constexpr operator uint32_t() const noexcept { return m_value; }

    bool operator==(const SafeDim&) const noexcept = default;
    bool operator!=(const SafeDim&) const noexcept = default;

private:
    explicit constexpr SafeDim(uint32_t v) noexcept : m_value(v) {}
    uint32_t m_value;
};

// ============================================================================
// SafeDimensions — validated (width, height) pair with budget enforcement
// ============================================================================

/// A width/height pair that has been validated against all overflow and
/// budget conditions required by the ExplorerLens decode pipeline.
///
/// Invariants guaranteed at construction:
///   1. width  ∈ [1, kMaxThumbDimension]
///   2. height ∈ [1, kMaxThumbDimension]
///   3. width × height ≤ kMaxThumbPixels
///   4. width × height × 4 ≤ kMaxThumbBytes
class SafeDimensions final
{
public:
    SafeDimensions() = delete;

    /// Factory — validates all invariants and returns nullopt on any failure.
    [[nodiscard]] static constexpr std::optional<SafeDimensions>
    Make(uint32_t width, uint32_t height) noexcept
    {
        // Range checks
        if (width  < kMinThumbDimension || width  > kMaxThumbDimension) return std::nullopt;
        if (height < kMinThumbDimension || height > kMaxThumbDimension) return std::nullopt;

        // Pixel-count overflow check (avoids 64-bit wrap)
        const uint64_t pixels = static_cast<uint64_t>(width) * height;
        if (pixels > kMaxThumbPixels) return std::nullopt;

        // Byte-budget overflow check (pixels × 4, BGRA32)
        if (pixels > kMaxThumbBytes / 4u) return std::nullopt;

        return SafeDimensions{width, height};
    }

    // ── Accessors ────────────────────────────────────────────────────────────

    [[nodiscard]] constexpr uint32_t Width()  const noexcept { return m_width;  }
    [[nodiscard]] constexpr uint32_t Height() const noexcept { return m_height; }

    /// Total pixel count (width × height).  Always ≤ kMaxThumbPixels.
    [[nodiscard]] constexpr uint64_t PixelCount() const noexcept
    {
        return static_cast<uint64_t>(m_width) * m_height;
    }

    /// Byte count for a packed BGRA32 buffer (4 bytes/pixel).
    /// Always ≤ kMaxThumbBytes.
    [[nodiscard]] constexpr uint64_t ByteCount() const noexcept
    {
        return PixelCount() * 4u;
    }

    /// Byte count for a given bytes-per-pixel count (1, 2, 3, or 4).
    /// Returns nullopt if the result would exceed kMaxThumbBytes.
    [[nodiscard]] constexpr std::optional<uint64_t>
    ByteCount(uint32_t bytesPerPixel) const noexcept
    {
        const auto result = SafeMul3(m_width, m_height, bytesPerPixel, kMaxThumbBytes);
        return result;
    }

    /// Row stride (width × bytesPerPixel), checked for overflow.
    [[nodiscard]] constexpr std::optional<uint32_t>
    RowStride(uint32_t bytesPerPixel) const noexcept
    {
        const uint64_t stride = static_cast<uint64_t>(m_width) * bytesPerPixel;
        if (stride > std::numeric_limits<uint32_t>::max()) return std::nullopt;
        return static_cast<uint32_t>(stride);
    }

    // ── Comparisons ──────────────────────────────────────────────────────────

    bool operator==(const SafeDimensions&) const noexcept = default;
    bool operator!=(const SafeDimensions&) const noexcept = default;

    /// True iff these dimensions are a subset of (or equal to) another.
    [[nodiscard]] constexpr bool FitsIn(const SafeDimensions& other) const noexcept
    {
        return m_width <= other.m_width && m_height <= other.m_height;
    }

    // ── Scaling helpers ──────────────────────────────────────────────────────

    /// Compute the largest dimensions that fit within maxEdge while preserving
    /// the aspect ratio of *this.  Returns nullopt if the result is < 1×1.
    [[nodiscard]] constexpr std::optional<SafeDimensions>
    ScaleToFit(uint32_t maxEdge) const noexcept
    {
        if (maxEdge == 0) return std::nullopt;
        const uint32_t longestEdge = (m_width >= m_height) ? m_width : m_height;
        if (longestEdge == 0) return std::nullopt;
        if (longestEdge <= maxEdge) return *this;  // already fits

        const uint32_t newW = static_cast<uint32_t>(
            static_cast<uint64_t>(m_width) * maxEdge / longestEdge);
        const uint32_t newH = static_cast<uint32_t>(
            static_cast<uint64_t>(m_height) * maxEdge / longestEdge);

        return Make(newW > 0 ? newW : 1u, newH > 0 ? newH : 1u);
    }

private:
    explicit constexpr SafeDimensions(uint32_t w, uint32_t h) noexcept
        : m_width(w), m_height(h) {}

    uint32_t m_width;
    uint32_t m_height;
};

} // namespace Core
} // namespace ExplorerLens
