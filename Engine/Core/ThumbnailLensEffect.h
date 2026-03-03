// ThumbnailLensEffect.h — Magnification Lens Effect Overlay
// Copyright (c) 2026 ExplorerLens Project
//
// Pure geometry engine for computing a circular magnification lens effect
// over thumbnail pixels. Provides coordinate transforms from source to
// magnified space with configurable position, radius, and power.
//
#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// Axis-aligned bounding rectangle (integer pixel coords)
struct LensRect {
    int32_t left = 0;
    int32_t top = 0;
    int32_t right = 0;
    int32_t bottom = 0;

    int32_t Width()  const noexcept { return right - left; }
    int32_t Height() const noexcept { return bottom - top; }
};

// Coordinate result from the magnification transform
struct MagnifiedCoord {
    double srcX = 0.0;   // source pixel x to sample
    double srcY = 0.0;   // source pixel y to sample
    bool   valid = false;  // false if outside lens or invalid state
};

class ThumbnailLensEffect {
public:
    static constexpr double kMinRadius = 4.0;
    static constexpr double kMaxRadius = 4096.0;
    static constexpr double kMinMagnification = 1.0;
    static constexpr double kMaxMagnification = 16.0;
    static constexpr double kDefaultMag = 2.0;

    ThumbnailLensEffect() noexcept = default;

    // Position the lens centre and its radius (in source-image pixels).
    void SetLensPosition(double x, double y, double radius) noexcept {
        m_cx = x;
        m_cy = y;
        m_radius = (std::max)(kMinRadius,
            (std::min)(radius, kMaxRadius));
    }

    double GetCenterX() const noexcept { return m_cx; }
    double GetCenterY() const noexcept { return m_cy; }
    double GetRadius()  const noexcept { return m_radius; }

    // Set magnification power. Clamped to [1, 16].
    void SetMagnification(double factor) noexcept {
        m_magnification = (std::max)(kMinMagnification,
            (std::min)(factor, kMaxMagnification));
    }

    double GetMagnification() const noexcept { return m_magnification; }

    // Given a destination pixel (srcX, srcY), compute which source pixel
    // it should sample when the lens effect is applied.
    // Uses barrel-distortion-style mapping inside the lens circle.
    MagnifiedCoord ComputeMagnifiedCoord(double srcX, double srcY) const noexcept {
        MagnifiedCoord mc{};
        if (m_radius < kMinRadius)
            return mc;

        const double dx = srcX - m_cx;
        const double dy = srcY - m_cy;
        const double dist = std::sqrt(dx * dx + dy * dy);

        if (dist > m_radius)
            return mc;   // outside lens

        mc.valid = true;

        if (dist < 1e-9) {
            // Exactly at centre — no shift
            mc.srcX = srcX;
            mc.srcY = srcY;
            return mc;
        }

        // Normalised distance [0, 1] within the lens
        const double normDist = dist / m_radius;

        // Power-curve magnification: pixels near centre are magnified more
        // than those near the edge, producing a smooth fisheye-like warp.
        // The mapped distance shrinks by the magnification factor at centre
        // and smoothly approaches 1.0 at the lens edge.
        const double warpedNorm = (1.0 - std::pow(1.0 - normDist,
            m_magnification)) /
            (1.0 - std::pow(1.0 - 1.0,
                m_magnification + 1e-12));
        // Simplified: just divide distance by magnification, tapering to edge
        const double scale = 1.0 / (1.0 + (m_magnification - 1.0) *
            (1.0 - normDist * normDist));

        mc.srcX = m_cx + dx * scale;
        mc.srcY = m_cy + dy * scale;
        return mc;
    }

    // Axis-aligned bounding rectangle of the lens in pixel coords.
    LensRect GetLensBoundingRect() const noexcept {
        LensRect r{};
        const auto iRadius = static_cast<int32_t>(std::ceil(m_radius));
        const auto cx = static_cast<int32_t>(std::floor(m_cx));
        const auto cy = static_cast<int32_t>(std::floor(m_cy));
        r.left = cx - iRadius;
        r.top = cy - iRadius;
        r.right = cx + iRadius;
        r.bottom = cy + iRadius;
        return r;
    }

    // Fast check whether a pixel coordinate falls inside the lens circle.
    bool IsInsideLens(double x, double y) const noexcept {
        const double dx = x - m_cx;
        const double dy = y - m_cy;
        return (dx * dx + dy * dy) <= (m_radius * m_radius);
    }

private:
    double m_cx = 0.0;
    double m_cy = 0.0;
    double m_radius = 32.0;
    double m_magnification = kDefaultMag;
};

} // namespace Engine
} // namespace ExplorerLens
