// DPIScalingPolicy.h — Per-Monitor DPI Scaling Policy
// Copyright (c) 2026 ExplorerLens Project
//
// Defines how thumbnail dimensions scale with per-monitor DPI, including
// snap-to-integer-scale, fractional-DPI support, and shell thumbnail size hints.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

// ---- DPI Bucket Enum --------------------------------------------------------

// Standard Windows DPI buckets (per PROCESS_DPI_AWARENESS / PMv2 model).
enum class DPIBucket : uint8_t {
    DPI_96   = 0,   // 100% — 1.0x scale
    DPI_120  = 1,   // 125% — 1.25x
    DPI_144  = 2,   // 150% — 1.5x
    DPI_168  = 3,   // 175% — 1.75x
    DPI_192  = 4,   // 200% — 2.0x
    DPI_240  = 5,   // 250% — 2.5x
    DPI_288  = 6,   // 300% — 3.0x
    Custom   = 0xFF // Fractional DPI not matching any standard bucket
};

// ---- Scaling Mode -----------------------------------------------------------

enum class DPIScalingMode : uint8_t {
    SnapToInteger      = 0,  // Round scale to nearest integer multiple (safe)
    Exact              = 1,  // Use exact fractional DPI ratio (higher quality)
    ShellHinted        = 2,  // Trust IShellItemImageFactory requested size verbatim
};

// ---- Policy -----------------------------------------------------------------

struct DPIScalingPolicy {
    DPIScalingMode mode           = DPIScalingMode::Exact;
    bool   snapFractionalDPI      = false;  // Force integer scale for HW < Win10 PMv2
    bool   maxSizeCapEnabled      = true;   // Don't exceed maxPhysicalPx
    uint32_t maxPhysicalPx        = 2048;   // Hard cap for thumbnail physical size (px)
    uint32_t minPhysicalPx        = 32;     // Minimum thumbnail physical size
    bool   emitHighDPISuffix      = true;   // Cache key: append "@2x", "@1.5x" etc.

    // Compute the physical pixel size for a logical thumbnail size at a given DPI.
    uint32_t PhysicalSize(uint32_t logicalPx, uint32_t dpi) const;

    // Return the DPI bucket for a given raw DPI value.
    static DPIBucket BucketForDPI(uint32_t dpi);

    // Return the cache-key suffix string for a given scale factor (e.g. "@2x").
    static std::string ScaleSuffix(float scaleFactor);

    // Default policy — sensible defaults for Explorer thumbnail provider.
    static DPIScalingPolicy Default();
};

// ---- Inline Implementation --------------------------------------------------

inline uint32_t DPIScalingPolicy::PhysicalSize(uint32_t logicalPx, uint32_t dpi) const {
    float scale  = static_cast<float>(dpi) / 96.0f;
    if (snapFractionalDPI)
        scale = static_cast<float>(static_cast<uint32_t>(scale + 0.5f));
    uint32_t px = static_cast<uint32_t>(static_cast<float>(logicalPx) * scale + 0.5f);
    if (maxSizeCapEnabled) {
        if (px > maxPhysicalPx) px = maxPhysicalPx;
        if (px < minPhysicalPx) px = minPhysicalPx;
    }
    return px;
}

inline DPIBucket DPIScalingPolicy::BucketForDPI(uint32_t dpi) {
    if (dpi <= 108)  return DPIBucket::DPI_96;
    if (dpi <= 132)  return DPIBucket::DPI_120;
    if (dpi <= 156)  return DPIBucket::DPI_144;
    if (dpi <= 180)  return DPIBucket::DPI_168;
    if (dpi <= 216)  return DPIBucket::DPI_192;
    if (dpi <= 264)  return DPIBucket::DPI_240;
    if (dpi <= 312)  return DPIBucket::DPI_288;
    return DPIBucket::Custom;
}

inline std::string DPIScalingPolicy::ScaleSuffix(float scale) {
    if (scale <= 1.01f)  return "";
    if (scale <= 1.26f)  return "@1.25x";
    if (scale <= 1.51f)  return "@1.5x";
    if (scale <= 1.76f)  return "@1.75x";
    if (scale <= 2.01f)  return "@2x";
    if (scale <= 2.51f)  return "@2.5x";
    return "@3x";
}

inline DPIScalingPolicy DPIScalingPolicy::Default() {
    return DPIScalingPolicy{};
}

} // namespace Engine
} // namespace ExplorerLens
